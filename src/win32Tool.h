#pragma once
#ifndef  WIN32TOOL_H_
#define WIN32TOOL_H_

#include <Windows.h>
#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>
#include <algorithm>
#include "peTool.h"

void __cdecl DbgLogA(const char* format, ...);
void __cdecl DbgLogW(const WCHAR * format, ...);

#ifdef _DEBUG
#ifdef UNICODE
#define qLog(str,...) DbgLogW(_T(str),__VA_ARGS__)
#else
#define qLog(str,...) DbgLogA(_T(str),__VA_ARGS__)
#endif
#else
#define qLog
#endif

#endif // ! WIN32TOOL_H_
