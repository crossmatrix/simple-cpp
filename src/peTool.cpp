#include "peTool.h"

void* malloc_s(int size) {
	void* mem = malloc(size);
	if (mem != NULL) {
		ZeroMemory(mem, size);
		return mem;
	} else {
		log("malloc error: %d", size);
		return NULL;
	}
}

void copyBin(PCSTR src, PCSTR dest) {
	FILE* fp = fopen(src, "rb");
	if (fp == NULL) {
		return;
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	FILE* wp = fopen(dest, "wb");
	if (wp == NULL) {
		fclose(fp);
		return;
	}

	if (len > 0) {
		byte* buf = (byte*)malloc_s(len);
		if (buf != NULL) {
			fseek(fp, 0, SEEK_SET);
			fread(buf, 1, len, fp);
			fwrite(buf, 1, len, wp);
			free(buf);
			log("copy suc!");
		}
	}
	fclose(wp);
	fclose(fp);
}

long openPE(IN PCSTR path, OUT PVOID* file) {
	FILE* fp = fopen(path, "rb");
	if (fp == NULL) {
		log("file not exist: %s", path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	void* buf = malloc_s(len);
	if (buf != NULL) {
		fseek(fp, 0, SEEK_SET);
		size_t size = fread(buf, 1, len, fp);
		if (!size) {
			free(buf);
			len = size;
		} else {
			*file = buf;
		}
	}
	fclose(fp);
	return len;
}

void savePE(PVOID buffer, DWORD size, PCSTR path) {
	FILE* fp = fopen(path, "wb");
	if (!fp) {
		log("open file error: %s", path);
		return;
	}
	fwrite(buffer, 1, size, fp);
	fclose(fp);
}

void showPE(PCSTR path) {
	PVOID fileBuffer = 0;
	long len = openPE(path, &fileBuffer);

	PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
	if (hDos->e_magic != IMAGE_DOS_SIGNATURE) {
		log("not mz");
		free(fileBuffer);
		return;
	}
	log("ntOffset\t 0x%04X", hDos->e_lfanew);

	PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)((DWORD)fileBuffer + hDos->e_lfanew);
	if (hNt->Signature != IMAGE_NT_SIGNATURE) {
		log("not pe");
		free(fileBuffer);
		return;
	}
	PIMAGE_FILE_HEADER hStd = &hNt->FileHeader;
	PIMAGE_OPTIONAL_HEADER hOp = &hNt->OptionalHeader;
	log("----------------FILE_HEADER important info----------------");
	log("machine\t\t 0x%04X", hStd->Machine);
	log("secNum\t\t 0x%04X", hStd->NumberOfSections);
	log("opSize\t\t 0x%04X", hStd->SizeOfOptionalHeader);
	log("char\t\t 0x%04X", hStd->Characteristics);
	log("----------------OPTIONAL_HEADER important info----------------");
	log("magic\t\t 0x%04X", hOp->Magic);
	log("oep\t\t 0x%08X", hOp->AddressOfEntryPoint);
	log("imgBase\t\t 0x%08X", hOp->ImageBase);
	log("secAlign\t 0x%08X", hOp->SectionAlignment);
	log("fileAlign\t 0x%08X", hOp->FileAlignment);
	log("sizeImg\t\t 0x%08X", hOp->SizeOfImage);
	log("sizeHeader\t 0x%08X", hOp->SizeOfHeaders);
	log("checkSum\t 0x%08X", hOp->CheckSum);
	log("subSys\t\t 0x%04X", hOp->Subsystem);
	log("dllChar\t\t 0x%04X", hOp->DllCharacteristics);
	PIMAGE_DATA_DIRECTORY dataDir = hOp->DataDirectory;
	for (int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
		log("dataDir_%d\t va: 0x%08X sz: 0x%08X", i + 1, dataDir[i].VirtualAddress, dataDir[i].Size);
	}

	log("----------------Section important info----------------");
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	for (int i = 0; i < hStd->NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		log("section_%d:", i + 1);
		log("    name\t %s", sec->Name);
		log("    vs\t\t 0x%08X", sec->Misc.VirtualSize);
		log("    va\t\t 0x%08X", sec->VirtualAddress);
		log("    fs\t\t 0x%08X", sec->SizeOfRawData);
		log("    fa\t\t 0x%08X", sec->PointerToRawData);
	}

	free(fileBuffer);
}

void peFile2Img(PVOID fileBuffer, PVOID* imgBuffer) {
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((DWORD)fileBuffer + ((PIMAGE_DOS_HEADER)fileBuffer)->e_lfanew);
	PVOID buf = malloc_s(pNt->OptionalHeader.SizeOfImage);
	memcpy(buf, fileBuffer, pNt->OptionalHeader.SizeOfHeaders);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(pNt);
	for (int i = 0; i < pNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD dst = (DWORD)buf + sec->VirtualAddress;
		DWORD src = (DWORD)fileBuffer + sec->PointerToRawData;
		memcpy((PVOID)dst, (PVOID)src, sec->SizeOfRawData);
	}
	*imgBuffer = buf;
}

DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer) {
	PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)((DWORD)imgBuffer + ((PIMAGE_DOS_HEADER)imgBuffer)->e_lfanew);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	PIMAGE_SECTION_HEADER lstSec = fstSec + (hNt->FileHeader.NumberOfSections - 1);
	DWORD size = lstSec->PointerToRawData + lstSec->SizeOfRawData;
	PVOID buf = malloc_s(size);
	memcpy(buf, imgBuffer, hNt->OptionalHeader.SizeOfHeaders);
	//log("%08x", hNt->OptionalHeader.SizeOfHeaders);
	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD dst = (DWORD)buf + sec->PointerToRawData;
		DWORD src = (DWORD)imgBuffer + sec->VirtualAddress;
		memcpy((PVOID)dst, (PVOID)src, sec->SizeOfRawData);
	}
	*newBuffer = buf;
	return size;
}