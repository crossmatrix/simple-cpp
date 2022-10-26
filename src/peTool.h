#pragma once
#ifndef PETOOL_H
#define PETOOL_H

#include <Windows.h>
#include <stdio.h>
#include <winnt.h>
#include <tchar.h>
#include <functional>
#include <ntstatus.h>

#define print(str,...) printf(str##"\n",__VA_ARGS__)
#define NT_HEADER(buffer) ((PIMAGE_NT_HEADERS)((DWORD)buffer + ((PIMAGE_DOS_HEADER)buffer)->e_lfanew))

void* malloc_s(int size);
void copyBin(PCSTR src, PCSTR dest);
long openPE(IN PCTCH path, OUT PVOID* file);
void savePE(PVOID buffer, DWORD size, PCTCH path);
void showPE(PCTCH path);
void peFile2Img(PVOID fileBuffer, PVOID* imgBuffer);
DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer);
DWORD foa2rva(PVOID fileBuffer, DWORD foa);
DWORD rva2foa(PVOID fileBuffer, DWORD rva);
DWORD rva2fa(PVOID fileBuffer, DWORD rva);
DWORD fa2rva(PVOID fileBuffer, DWORD fa);
bool isZeroBlock(PVOID pos, DWORD size);
PIMAGE_SECTION_HEADER getSecByRva(PVOID fileBuffer, DWORD rva);
PIMAGE_SECTION_HEADER getSecByFoa(PVOID fileBuffer, DWORD foa);
bool findEmpty(PVOID fileBuffer, DWORD chunkSize, int secIdx, bool fromEnd, OUT DWORD* targPos);
void calcJmp(PVOID fileBuffer, DWORD baseFoa, byte* code, DWORD offsetToBase, DWORD targVa);
void showData_0_Export(PVOID fileBuffer);
DWORD getFuncByOrdinal(PVOID fileBuffer, int ordinal);
DWORD getFuncByName(PVOID fileBuffer, PCSTR name);
void showData_5_Reloc(PVOID fileBuffer);
void showData_1_11_Import_Bound(PVOID fileBuffer);
DWORD addSection(PVOID fileBuffer, int secIdx, PCSTR secName, DWORD secSize, OUT PVOID* newBuffer);
void resourceInfo(PVOID fileBuffer, DWORD dataRva,
	void(*nameFunc)(PIMAGE_RESOURCE_DIRECTORY, PIMAGE_RESOURCE_DIRECTORY_ENTRY, int, int),
	std::function<void(PIMAGE_RESOURCE_DATA_ENTRY, int, PVOID)> dataFunc
);
void showData_2_Resource(PVOID fileBuffer);
void addShell(PCTCH path_src, PCTCH path_shell, PCTCH path_save, PCCH secName);
void makeShell();

#endif