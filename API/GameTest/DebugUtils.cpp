#include "stdafx.h"
#include "DebugUtils.h"
#include <stdio.h>

void DebugPrint(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, format, args);
    va_end(args);
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}