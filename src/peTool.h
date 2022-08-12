#ifndef PETOOL_H
#define PETOOL_H

#include <Windows.h>
#include <stdio.h>
#include <winnt.h>

#define log(str,...) printf(str##"\n",__VA_ARGS__)

void* malloc_s(int size);
void copyBin(PCSTR src, PCSTR dest);
long openPE(IN PCSTR path, OUT PVOID* file);
void showPE(PCSTR path);
void peFile2Img(PVOID fileBuffer, PVOID* imgBuffer);
DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer);
void savePE(PVOID buffer, DWORD size, PCSTR path);

#endif