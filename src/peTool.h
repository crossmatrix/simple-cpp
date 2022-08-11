#ifndef PETOOL_H
#define PETOOL_H

#include <Windows.h>
#include <stdio.h>
#include <winnt.h>

#define log(str,...) printf(str##"\n",__VA_ARGS__)

void* malloc_s(int size);
void copyBin(const char* src, const char* dest);

#endif