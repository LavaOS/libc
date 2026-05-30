#include <readline.h>

intptr_t readline(char* buf, size_t bufmax) {
    intptr_t e;
    size_t n = 0;
    while(n < bufmax) {
        e = read(STDIN_FILENO, buf + n, bufmax - n);
        if(e < 0) return e;
        assert(e != 0);
        n += e;
        if(buf[n-1] == '\n') break;
    }
    if(n >= bufmax) return -BUFFER_TOO_SMALL;
    return n;
}
