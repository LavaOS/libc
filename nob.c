#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "../nob.h"
static bool walk_directory(
    File_Paths* dirs,
    File_Paths* c_sources,
    File_Paths* nasm_sources,
    const char* path
) {
    DIR *dir = opendir(path);
    if(!dir) {
        nob_log(NOB_ERROR, "Could not open directory %s: %s", path, strerror(errno));
        return false;
    }
    errno = 0;
    struct dirent *ent;
    while((ent = readdir(dir))) {
        if(strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0) continue;
        const char* fext = nob_get_ext(ent->d_name);
        size_t temp = nob_temp_save();
        const char* p = nob_temp_sprintf("%s/%s", path, ent->d_name); 
        Nob_File_Type type = nob_get_file_type(p);
        if(type == NOB_FILE_DIRECTORY) {
            da_append(dirs, p);
            if(!walk_directory(dirs, c_sources, nasm_sources, p)) {
                closedir(dir);
                return false;
            }
            continue;
        }
        if(strcmp(fext, "c") == 0 || strcmp(fext, "nasm") == 0) {
            if(strcmp(fext, "c") == 0) 
                nob_da_append(c_sources, p);
            else if(strcmp(fext, "nasm") == 0)
                nob_da_append(nasm_sources, p);
            continue;
        }
        nob_temp_rewind(temp);
    }
    closedir(dir);
    return true;
}
int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    char* cc = getenv("CC");
    if(!cc) cc = "cc";
    char* bindir = getenv("BINDIR");
    if(!bindir) bindir = "bin";
    char* kroot = getenv("KROOT");
    if(!kroot) {
        nob_log(NOB_ERROR, "Missing KROOT environmental variable");
        nob_log(NOB_INFO, "KROOT should point to the root of the MinOS **kernel**");
        return false;
    }
    if(!nob_mkdir_if_not_exists_silent(bindir)) return 1;
    if(!nob_mkdir_if_not_exists_silent(nob_temp_sprintf("%s/libc", bindir))) return 1;
    if(!nob_mkdir_if_not_exists_silent(nob_temp_sprintf("%s/crt" , bindir))) return 1;
    File_Paths dirs = { 0 };
    File_Paths c_sources = { 0 };
    File_Paths nasm_sources = { 0 };
    if(!walk_directory(&dirs, &c_sources, &nasm_sources, "src")) return 1;
    for(size_t i = 0; i < dirs.count; ++i) {
        size_t temp = nob_temp_save();
        const char* dir = nob_temp_sprintf("%s/libc/%s", bindir, dirs.items[i] + 4);
        if(!nob_mkdir_if_not_exists_silent(dir)) return 1;
        nob_temp_rewind(temp);
    }
    Cmd cmd = { 0 };
    String_Builder stb = { 0 };
    File_Paths pathb = { 0 };
    for(size_t i = 0; i < nasm_sources.count; ++i) {
        const char* src = nasm_sources.items[i];
        const char* out = nob_temp_sprintf("%s/libc/%.*s.o", bindir, (int)(strlen(src + 4)-5), src + 4);
        // TODO: smart rebuilding for nasm maybe
        if(nob_needs_rebuild1(out, src) == 0) continue;
        const char* include = nob_temp_sprintf("%.*s", (int)(nob_path_name(src)-src), src);
        cmd_append(&cmd, "nasm");
        cmd_append(&cmd, "-I", include); 
        cmd_append(&cmd, "-f", "elf64");
        cmd_append(&cmd, src);
        cmd_append(&cmd, "-o", out);
        if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    }
    for(size_t i = 0; i < c_sources.count; ++i) {
        const char* src = c_sources.items[i];
        const char* out = nob_temp_sprintf("%s/libc/%.*s.o", bindir, (int)(strlen(src + 4)-2), src + 4);
        if(!nob_c_needs_rebuild1(&stb, &pathb, out, src)) continue;
        cmd_append(&cmd, cc);
        cmd_append(&cmd,
            "-nostdlib",
            "-march=x86-64",
            "-ffreestanding",
            "-static", 
            // "-Werror",
            "-Wno-unused-function",
            "-Wall", 
            "-fno-stack-protector", 
            "-mno-mmx",
            "-MMD",
            "-MP",
            "-O1",
            // "-mno-sse", "-mno-sse2",
            "-mno-3dnow",
            "-fPIC",
            "-I", nob_temp_sprintf("%s/shared/include", kroot),
            "-I", "include"
        );
        cmd_append(&cmd, "-c", src, "-o", out);
        if(!nob_cmd_run_sync_and_reset(&cmd)) {
            size_t temp = nob_temp_save();
            char* str = nob_temp_strdup(out);
            size_t str_len = strlen(str);
            assert(str_len);
            str[str_len-1] = 'd';
            nob_delete_file(str);
            nob_temp_rewind(temp);
            return 1;
        }
    }

    // LET IT LEAK (the temp memory)
    dirs.count = 0;
    c_sources.count = 0;
    nasm_sources.count = 0;
    
    if(!walk_directory(&dirs, &c_sources, &nasm_sources, "crt")) return 1;
    assert(dirs.count == 0 && "Update crt building");
    for(size_t i = 0; i < nasm_sources.count; ++i) {
        const char* src = nasm_sources.items[i];
        const char* out = nob_temp_sprintf("%s/crt/%.*s.o", bindir, (int)(strlen(src + 4) - 5), src + 4);
        // TODO: smart rebuilding for nasm maybe
        if(nob_needs_rebuild1(out, src) == 0) continue;
        cmd_append(&cmd, "nasm");
        cmd_append(&cmd, "-f", "elf64");
        cmd_append(&cmd, src);
        cmd_append(&cmd, "-o", out);
        if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    }
    for(size_t i = 0; i < c_sources.count; ++i) {
        const char* src = c_sources.items[i];
        const char* out = nob_temp_sprintf("%s/crt/%.*s.o", bindir, (int)(strlen(src + 4)-2), src + 4);
        if(!nob_c_needs_rebuild1(&stb, &pathb, out, src)) continue;
        cmd_append(&cmd, cc);
        cmd_append(&cmd,
            "-nostdlib",
            "-march=x86-64",
            "-ffreestanding",
            "-static", 
            // "-Werror",
            "-Wno-unused-function",
            "-Wall", 
            "-fno-stack-protector", 
            "-mno-mmx",
            "-MMD",
            "-MP",
            "-O1",
            // "-mno-sse", "-mno-sse2",
            "-mno-3dnow",
            "-fPIC",
            "-I", nob_temp_sprintf("%s/shared/include", kroot),
            "-I", "include"
        );
        cmd_append(&cmd, "-c", src, "-o", out);
        if(!nob_cmd_run_sync_and_reset(&cmd)) {
            size_t temp = nob_temp_save();
            char* str = nob_temp_strdup(out);
            size_t str_len = strlen(str);
            assert(str_len);
            str[str_len-1] = 'd';
            nob_delete_file(str);
            nob_temp_rewind(temp);
            return 1;
        }
    }

    // Link libc.so from all object files
    const char* libc_so = nob_temp_sprintf("%s/libc/libc.so", bindir);
    const char* libc_dir = nob_temp_sprintf("%s/libc", bindir);
    {
        cmd_append(&cmd, cc);
        cmd_append(&cmd, "-shared", "-nostdlib", "-ffreestanding", "-fPIC");
        cmd_append(&cmd, "-march=x86-64", "-mno-mmx", "-mno-sse", "-mno-sse2", "-mno-3dnow");
        cmd_append(&cmd, "-Wl,--hash-style=sysv");
        cmd_append(&cmd, "-o", libc_so);

        // Collect all libc .o files
        size_t temp = nob_temp_save();
        DIR* d = opendir(libc_dir);
        if(!d) {
            nob_log(NOB_ERROR, "Failed to open %s", libc_dir);
            return 1;
        }
        struct dirent* ent;
        while((ent = readdir(d))) {
            if(ent->d_name[0] == '.') continue;
            size_t len = strlen(ent->d_name);
            if(len > 2 && strcmp(ent->d_name + len - 2, ".o") == 0) {
                cmd_append(&cmd, nob_temp_sprintf("%s/%s", libc_dir, ent->d_name));
            }
        }
        closedir(d);
        nob_temp_rewind(temp);

        if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;

        // Strip
        cmd_append(&cmd, "strip", "-s", libc_so);
        if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    }

    // Install libc.so and libc.a to sysroot
    const char* sysroot = nob_temp_sprintf("%s/usr/lib", nob_temp_sprintf("%s/sysroot", kroot));
    if(nob_mkdir_if_not_exists_silent(sysroot)) {
        nob_copy_file(libc_so, nob_temp_sprintf("%s/libc.so", sysroot));
        const char* libc_a = nob_temp_sprintf("%s/libc.a", sysroot);

        // Build libc.a (static archive)
        {
            const char* ar = getenv("AR");
            if(!ar) ar = "ar";
            cmd_append(&cmd, ar, "-cr", libc_a);
            DIR* d2 = opendir(libc_dir);
            if(d2) {
                struct dirent* ent2;
                while((ent2 = readdir(d2))) {
                    if(ent2->d_name[0] == '.') continue;
                    size_t len = strlen(ent2->d_name);
                    if(len > 2 && strcmp(ent2->d_name + len - 2, ".o") == 0) {
                        cmd_append(&cmd, nob_temp_sprintf("%s/%s", libc_dir, ent2->d_name));
                    }
                }
                closedir(d2);
            }
            nob_cmd_run_sync_and_reset(&cmd);
        }
    }

    // Copy libc headers to initrd/include/
    char* rootdir = getenv("ROOTDIR");
    if(rootdir) {
        cmd_append(&cmd, "cp", "-r", "include/.", temp_sprintf("%s/include/", rootdir));
        nob_cmd_run_sync_and_reset(&cmd);
    }
}
