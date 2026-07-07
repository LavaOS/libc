#pragma once
#include <minos/fcntl.h>
#ifndef O_EXCL
#define O_EXCL (1 << 6)
#endif
int open(const char* path, int flags, ...);