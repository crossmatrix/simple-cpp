#ifndef PETOOL_H
#define PETOOL_H

#include <Windows.h>
#include <stdio.h>
#include <winnt.h>

#define log(str,...) printf(str##"\n",__VA_ARGS__)

void* malloc_s(int size);
void copyBin(PCSTR src, PCSTR dest);
long openPE(IN PCSTR path, OUT PVOID* file);
void savePE(PVOID buffer, DWORD size, PCSTR path);
void showPE(PCSTR path);
void peFile2Img(PVOID fileBuffer, PVOID* imgBuffer);
DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer);
DWORD foa2rva(PVOID fileBuffer, DWORD foa);
DWORD rva2foa(PVOID fileBuffer, DWORD rva);
DWORD rva2fa(PVOID fileBuffer, DWORD rva);
DWORD fa2rva(PVOID fileBuffer, DWORD fa);
PIMAGE_SECTION_HEADER getSecByRva(PVOID fileBuffer, DWORD rva);
PIMAGE_SECTION_HEADER getSecByFoa(PVOID fileBuffer, DWORD foa);

#endif