#pragma once
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <minos/status.h>

intptr_t readline(char* buf, size_t bufmax);
