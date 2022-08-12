#ifndef PETOOL_H
#define PETOOL_H

#include <Windows.h>
#include <stdio.h>
#include <winnt.h>

#define log(str,...) printf(str##"\n",__VA_ARGS__)

void* malloc_s(int size);
void copyBin(const char* src, const char* dest);
long openPE(IN const char* path, OUT PVOID* file);
void showPE(const char* path);
void peFile2Img(PVOID peFile, PVOID* img);
void peImg2File(PVOID img, PVOID* newBuf);

#endif