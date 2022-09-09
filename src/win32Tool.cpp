#include "win32Tool.h"

void __cdecl DbgLogA(const char* format, ...) {
    va_list vlArgs;
    char* strBuffer = (char*)GlobalAlloc(GPTR, 4096);
    va_start(vlArgs, format);
    _vsnprintf(strBuffer, 4096 - 1, format, vlArgs);
    va_end(vlArgs);
    strcat(strBuffer, "\n");
    OutputDebugStringA(strBuffer);
    GlobalFree(strBuffer);
    return;
}

void __cdecl DbgLogW(const WCHAR* format, ...) {
    setlocale(LC_ALL, "chs");
    va_list vlArgs;
    WCHAR* strBuffer = (WCHAR*)GlobalAlloc(GPTR, 2048);
    va_start(vlArgs, format);
    _vsnwprintf(strBuffer, 2048 - 1, format, vlArgs);
    va_end(vlArgs);
    lstrcatW(strBuffer, L"\n");
    OutputDebugStringW(strBuffer);
    GlobalFree(strBuffer);
    return;
}