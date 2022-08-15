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
	log("fileSize\t 0x%08X", len);

	PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
	if (hDos->e_magic != IMAGE_DOS_SIGNATURE) {
		log("not mz");
		free(fileBuffer);
		return;
	}
	log("----------------DOS_HEADER important info----------------");
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
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PVOID buf = malloc_s(hNt->OptionalHeader.SizeOfImage);
	memcpy(buf, fileBuffer, hNt->OptionalHeader.SizeOfHeaders);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD dst = (DWORD)buf + sec->VirtualAddress;
		DWORD src = (DWORD)fileBuffer + sec->PointerToRawData;
		memcpy((PVOID)dst, (PVOID)src, sec->SizeOfRawData);
	}
	*imgBuffer = buf;
}

DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(imgBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	PIMAGE_SECTION_HEADER lstSec = fstSec + (hNt->FileHeader.NumberOfSections - 1);
	DWORD size = lstSec->PointerToRawData + lstSec->SizeOfRawData;
	PVOID buf = malloc_s(size);
	memcpy(buf, imgBuffer, hNt->OptionalHeader.SizeOfHeaders);
	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD dst = (DWORD)buf + sec->PointerToRawData;
		DWORD src = (DWORD)imgBuffer + sec->VirtualAddress;
		memcpy((PVOID)dst, (PVOID)src, sec->SizeOfRawData);
	}
	*newBuffer = buf;
	return size;
}

DWORD align(DWORD value, DWORD fmt) {
	int div = value / fmt;
	if (div * fmt == value) {
		return value;
	} else {
		return (div + 1) * fmt;
	}
}

DWORD foa2rva(PVOID fileBuffer, DWORD foa) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	
	if (foa >= 0) {
		if (foa < fstSec->PointerToRawData) {
			return foa;
		} else {
			for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
				PIMAGE_SECTION_HEADER sec = fstSec + i;
				if (foa < sec->PointerToRawData + sec->SizeOfRawData) {
					return foa - sec->PointerToRawData + sec->VirtualAddress;
				}
			}
		}
	}
	log("not find foa: %08x", foa);
	return -1;
}

DWORD rva2foa(PVOID fileBuffer, DWORD rva) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	if (rva >= 0) {
		if (rva < fstSec->VirtualAddress) {
			if (rva < fstSec->PointerToRawData) {
				return rva;
			} else {
				log("rva -> foa fail, in header: %08x, img: [0, %08x), file: [0, %08x)", rva, fstSec->VirtualAddress, fstSec->PointerToRawData);
				return -1;
			}
		} else {
			for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
				PIMAGE_SECTION_HEADER sec = fstSec + i;
				DWORD nextVA = sec->VirtualAddress + align(sec->Misc.VirtualSize, hNt->OptionalHeader.SectionAlignment);
				if (rva < nextVA) {
					if (rva - sec->VirtualAddress < sec->SizeOfRawData) {
						return rva - sec->VirtualAddress + sec->PointerToRawData;
					} else {
						log("rva -> foa fail, in sec%d: %08x, img: [%08x, %08x), file: [%08x, %08x)", i + 1, rva, sec->VirtualAddress, nextVA, sec->PointerToRawData, sec->PointerToRawData + sec->SizeOfRawData);
						return -1;
					}
				}
			}
		}
	}
	log("not find rva: %08x", rva);
	return -1;
}

DWORD rva2fa(PVOID fileBuffer, DWORD rva) {
	return (DWORD)fileBuffer + rva2foa(fileBuffer, rva);
}

DWORD fa2rva(PVOID fileBuffer, DWORD fa) {
	return foa2rva(fileBuffer, fa - (DWORD)fileBuffer);
}

PIMAGE_SECTION_HEADER getSecByRva(PVOID fileBuffer, DWORD rva) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD start = sec->VirtualAddress;
		DWORD end = sec->VirtualAddress + align(sec->Misc.VirtualSize, hNt->OptionalHeader.SectionAlignment);
		if (rva >= start && rva < end) {
			return sec;
		}
	}
	log("not find section: %08x", rva);
	return NULL;
}

PIMAGE_SECTION_HEADER getSecByFoa(PVOID fileBuffer, DWORD foa) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		DWORD start = sec->PointerToRawData;
		DWORD end = sec->PointerToRawData + sec->SizeOfRawData;
		if (foa >= start && foa < end) {
			return sec;
		}
	}
	log("not find section: %08x", foa);
	return NULL;
}

bool checkChunk(PVOID buffer, DWORD pos, PVOID chunk, DWORD size) {

	return false;
}

//secIdx: 0 header, >0 section
DWORD findEmpty(PVOID fileBuffer, DWORD size, int secIdx, bool fromEnd) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	DWORD start = 0;
	DWORD end = 0;
	if (secIdx == 0) {
		start = 0;
		end = fstSec->PointerToRawData - 1;
	} else {
		if (secIdx <= hNt->FileHeader.NumberOfSections) {
			PIMAGE_SECTION_HEADER sec = fstSec + (secIdx - 1);
			start = sec->PointerToRawData;
			end = sec->PointerToRawData + sec->SizeOfRawData - 1;
		} else {
			log("secIdx error: %d, maxSecIdx: %d", secIdx, hNt->FileHeader.NumberOfSections);
			return -1;
		}
	}

	if (size <= 0) {
		log("size must > 0");
		return -1;
	}

	byte* chunk = new byte[size]{};
	if (fromEnd) {
		DWORD tmp = start;
		start = end;
		end = tmp;
		for (DWORD p = start; p >= end; p--) {
			if (checkChunk(fileBuffer, p, chunk, size)) {
				return p;
			}
			if (p == 0) {
				break;
			}
		}
	} else {
		for (DWORD p = start; p <= end; p++) {
			if (checkChunk(fileBuffer, p, chunk, size)) {
				return p;
			}
		}
	}
	log("no avaliable space");
	return -1;
}

void injCode(PVOID fileBuffer, byte* code, int num, DWORD pos) {
	
}