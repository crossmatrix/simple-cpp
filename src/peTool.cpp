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
	log("----------pe overview(fileSize: 0x%p)----------", len);
	log(".....DOS_HEADER key info.....");
	log("ntOffset\t 0x%04X", hDos->e_lfanew);

	PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)((DWORD)fileBuffer + hDos->e_lfanew);
	if (hNt->Signature != IMAGE_NT_SIGNATURE) {
		log("not pe");
		free(fileBuffer);
		return;
	}
	PIMAGE_FILE_HEADER hStd = &hNt->FileHeader;
	PIMAGE_OPTIONAL_HEADER hOp = &hNt->OptionalHeader;
	log(".....FILE_HEADER key info.....");
	log("machine\t\t 0x%04X", hStd->Machine);
	log("secNum\t\t 0x%04X", hStd->NumberOfSections);
	log("opSize\t\t 0x%04X", hStd->SizeOfOptionalHeader);
	log("char\t\t 0x%04X", hStd->Characteristics);
	log(".....OPTIONAL_HEADER key info.....");
	log("magic\t\t 0x%04X", hOp->Magic);
	log("oep\t\t 0x%p", hOp->AddressOfEntryPoint);
	log("imgBase\t\t 0x%p", hOp->ImageBase);
	log("secAlign\t 0x%p", hOp->SectionAlignment);
	log("fileAlign\t 0x%p", hOp->FileAlignment);
	log("sizeImg\t\t 0x%p", hOp->SizeOfImage);
	log("sizeHeader\t 0x%p", hOp->SizeOfHeaders);
	log("checkSum\t 0x%p", hOp->CheckSum);
	log("subSys\t\t 0x%04X", hOp->Subsystem);
	log("dllChar\t\t 0x%04X", hOp->DllCharacteristics);
	PIMAGE_DATA_DIRECTORY dataDir = hOp->DataDirectory;
	for (int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
		log("dataDir_%d\t va: 0x%p sz: 0x%p", i, dataDir[i].VirtualAddress, dataDir[i].Size);
	}

	log(".....SECTION_HEADER key info.....");
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	for (int i = 0; i < hStd->NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		log("section_%d:", i);
		log("    name\t %s", sec->Name);
		log("    vs\t\t 0x%p", sec->Misc.VirtualSize);
		log("    va\t\t 0x%p", sec->VirtualAddress);
		log("    fs\t\t 0x%p", sec->SizeOfRawData);
		log("    fa\t\t 0x%p", sec->PointerToRawData);
		log("    char\t 0x%p", sec->Characteristics);
	}

	free(fileBuffer);
}

void peFile2Img(PVOID fileBuffer, PVOID* imgBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PVOID buf = malloc_s(hNt->OptionalHeader.SizeOfImage);
	if (buf != NULL) {
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
}

DWORD peImg2File(PVOID imgBuffer, PVOID* newBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(imgBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	PIMAGE_SECTION_HEADER lstSec = fstSec + (hNt->FileHeader.NumberOfSections - 1);
	DWORD size = lstSec->PointerToRawData + lstSec->SizeOfRawData;
	PVOID buf = malloc_s(size);
	if (buf != NULL) {
		memcpy(buf, imgBuffer, hNt->OptionalHeader.SizeOfHeaders);
		for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
			PIMAGE_SECTION_HEADER sec = fstSec + i;
			DWORD dst = (DWORD)buf + sec->PointerToRawData;
			DWORD src = (DWORD)imgBuffer + sec->VirtualAddress;
			memcpy((PVOID)dst, (PVOID)src, sec->SizeOfRawData);
		}
		*newBuffer = buf;
		return size;
	} else {
		return 0;
	}
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
	log("not find foa: %p", foa);
	return 0;
}

DWORD rva2foa(PVOID fileBuffer, DWORD rva) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	if (rva >= 0) {
		if (rva < fstSec->VirtualAddress) {
			if (rva < fstSec->PointerToRawData) {
				return rva;
			} else {
				log("rva -> foa fail, in header: %p, img: [0, %p), file: [0, %p)", rva, fstSec->VirtualAddress, fstSec->PointerToRawData);
				return 0;
			}
		} else {
			for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
				PIMAGE_SECTION_HEADER sec = fstSec + i;
				DWORD nextVA = sec->VirtualAddress + align(sec->Misc.VirtualSize, hNt->OptionalHeader.SectionAlignment);
				if (rva < nextVA) {
					if (rva - sec->VirtualAddress < sec->SizeOfRawData) {
						return rva - sec->VirtualAddress + sec->PointerToRawData;
					} else {
						log("rva -> foa fail, in sec%d: %p, img: [%p, %p), file: [%p, %p)", i + 1, rva, sec->VirtualAddress, nextVA, sec->PointerToRawData, sec->PointerToRawData + sec->SizeOfRawData);
						return 0;
					}
				}
			}
		}
	}
	log("not find rva: %p", rva);
	return 0;
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
	log("not find section: %p", rva);
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
	log("not find section: %p", foa);
	return NULL;
}

//[startAddr, endAddr]
bool checkChunk(PVOID buffer, DWORD startOffset, DWORD endOffset, PVOID chunk, DWORD chunkSize) {
	if (endOffset - startOffset + 1 < chunkSize) {
		return false;
	}
	for (int i = 0; i < chunkSize; i++) {
		PBYTE c1 = (PBYTE)((DWORD)buffer + startOffset + i);
		PBYTE c2 = (PBYTE)((DWORD)chunk + i);
		if (*c1 != *c2) {
			return false;
		}
	}
	return true;
}

//result is foa, secIdx: -1 header, >=0 section
bool findEmpty(PVOID fileBuffer, DWORD chunkSize, int secIdx, bool fromEnd, OUT DWORD* targPos) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	//[start, end]
	DWORD start = 0;
	DWORD end = 0;
	if (secIdx == -1) {
		start = 0;
		end = fstSec->PointerToRawData - 1;
	} else {
		if (secIdx < hNt->FileHeader.NumberOfSections) {
			PIMAGE_SECTION_HEADER sec = fstSec + secIdx;
			start = sec->PointerToRawData;
			end = sec->PointerToRawData + sec->SizeOfRawData - 1;
		} else {
			log("secIdx = %d not in [0, %d)", secIdx, hNt->FileHeader.NumberOfSections);
			return false;
		}
	}
	if (chunkSize <= 0) {
		log("size must > 0");
		return false;
	}

	bool isFind = false;
	byte* chunk = new byte[chunkSize]{};
	if (fromEnd) {
		DWORD tmp = start;
		start = end;
		end = tmp;
		for (DWORD p = start; p >= end; p--) {
			if (checkChunk(fileBuffer, p, start, chunk, chunkSize)) {
				*targPos = p;
				isFind = true;
				break;
			}
			if (p == 0) {
				break;
			}
		}
	} else {
		for (DWORD p = start; p <= end; p++) {
			if (checkChunk(fileBuffer, p, end, chunk, chunkSize)) {
				*targPos = p;
				isFind = true;
				break;
			}
		}
	}
	delete[] chunk;
	if (!isFind) {
		log("no avaliable space");
	}
	return isFind;
}

//offsetToBase: [0, code len)
void calcJmp(PVOID fileBuffer, DWORD baseFoa, byte* code, DWORD offsetToBase, DWORD targVa) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD rvaPos = foa2rva(fileBuffer, baseFoa);
	PDWORD p = (PDWORD)((DWORD)code + offsetToBase);
	*p = targVa - (hNt->OptionalHeader.ImageBase + rvaPos + offsetToBase + 4);
}

void showData_0_Export(PVOID fileBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if (!dataRva) {
		log("no export info");
		return;
	}
	PIMAGE_EXPORT_DIRECTORY pData = (PIMAGE_EXPORT_DIRECTORY)rva2fa(fileBuffer, dataRva);
	log("----------export table----------");

	PSTR name = (PSTR)rva2fa(fileBuffer, pData->Name);
	log("name: %s", name);
	log("baseOrdinal: %d, funcNum: %d, nameNum: %d", pData->Base, pData->NumberOfFunctions, pData->NumberOfNames);
	PDWORD funcAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfFunctions);
	PDWORD nameAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfNames);
	PWORD nameOrdAddr = (PWORD)rva2fa(fileBuffer, pData->AddressOfNameOrdinals);

	log("exportOrdinal \trealIndex \tfuncAddr(rva) \tname");
	for (int i = 0; i < pData->NumberOfFunctions; i++) {
		if (funcAddr[i]) {
			DWORD nameRva = 0;
			for (int j = 0; j < pData->NumberOfNames; j++) {
				if (nameOrdAddr[j] == i) {
					nameRva = nameAddr[j];
					break;
				}
			}
			log("%d \t\t%d \t\t%p \t%s", i + pData->Base, i, funcAddr[i], nameRva ? (PSTR)rva2fa(fileBuffer, nameRva) : "noName");
		}
	}
}

//rva
DWORD getFuncByOrdinal(PVOID fileBuffer, int ordinal) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY pData = (PIMAGE_EXPORT_DIRECTORY)rva2fa(fileBuffer, dataRva);

	int realIdx = ordinal - pData->Base;
	if (realIdx >= 0 && realIdx < pData->NumberOfFunctions) {
		PDWORD funcAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfFunctions);
		return funcAddr[realIdx];
	}
	return 0;
}

//rva
DWORD getFuncByName(PVOID fileBuffer, PCSTR name) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY pData = (PIMAGE_EXPORT_DIRECTORY)rva2fa(fileBuffer, dataRva);
	PDWORD funcAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfFunctions);
	PDWORD nameAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfNames);
	PWORD nameOrdAddr = (PWORD)rva2fa(fileBuffer, pData->AddressOfNameOrdinals);

	for (int i = 0; i < pData->NumberOfNames; i++) {
		PSTR funcName = (PSTR)rva2fa(fileBuffer, nameAddr[i]);
		if (strcmp(funcName, name) == 0) {
			DWORD realIdx = nameOrdAddr[i];
			return funcAddr[realIdx];
		}
	}
	return 0;
}

bool isZeroBlock(PVOID pos, DWORD size) {
	if (size > 0) {
		PBYTE pb = (PBYTE)pos;
		for (int i = 0; i < size; i++) {
			if (*(pb + i) != 0) {
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
}

void showData_5_Reloc(PVOID fileBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	if (!dataRva) {
		log("no reloc info");
		return;
	}
	PIMAGE_BASE_RELOCATION pData = (PIMAGE_BASE_RELOCATION)rva2fa(fileBuffer, dataRva);
	log("----------reloc table----------");

	DWORD typeSize = sizeof(IMAGE_BASE_RELOCATION);
	while (!isZeroBlock((PVOID)pData, typeSize)) {
		DWORD itemNum = (pData->SizeOfBlock - typeSize) / 2;
		PIMAGE_SECTION_HEADER pSec = getSecByRva(fileBuffer, pData->VirtualAddress);
		log("belong sec: %s, baseVa: %p, blockSize: %p, itemNum: %d", pSec->Name, pData->VirtualAddress, pData->SizeOfBlock, itemNum);

		PWORD fstItem = PWORD((DWORD)pData + typeSize);
		for (int i = 0; i < itemNum; i++) {
			//4(IMAGE_REL_BASED_HIGHLOW)|12
			WORD item = fstItem[i];
			WORD value = item << 4;
			value = value >> 4;
			DWORD rva = pData->VirtualAddress + value;
			log("\t[%d] org item: %04x, item value: %04x, rva: %p", i, item, value, rva);
		}
		pData = (PIMAGE_BASE_RELOCATION)((DWORD)pData + pData->SizeOfBlock);
	}
}

void showData_1_11_Import_Bound(PVOID fileBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (!dataRva) {
		log("no import info");
	} else {
		PIMAGE_IMPORT_DESCRIPTOR pData = (PIMAGE_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, dataRva);
		log("----------import table----------");

		DWORD typeSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		while (!isZeroBlock(pData, typeSize)) {
			PSTR dllName = (PSTR)rva2fa(fileBuffer, pData->Name);
			PDWORD pOFT = (PDWORD)rva2fa(fileBuffer, pData->OriginalFirstThunk);
			PDWORD pFT = (PDWORD)rva2fa(fileBuffer, pData->FirstThunk);
			DWORD funcNum = 0;

			while (*pOFT != 0) {
				if ((*pOFT & 0x80000000) == 0x80000000) {
					DWORD funcOrdinal = *pOFT & 0x7fffffff;
					log("oft: %p, ft: %p, <ordinal>\t %d", *pOFT, *pFT, funcOrdinal);
				} else {
					PIMAGE_IMPORT_BY_NAME pName = (PIMAGE_IMPORT_BY_NAME)rva2fa(fileBuffer, *pOFT);
					log("oft: %p, ft: %p, <name>\t %s", *pOFT, *pFT, pName->Name);
				}
				pOFT++;
				pFT++;
				funcNum++;
			}
			log("%s[%d], OFT/INT(rva): %p, FT/IAT(rva): %p, time: %d\n", dllName, funcNum, pData->OriginalFirstThunk, pData->FirstThunk, pData->TimeDateStamp);
			pData++;
		}
	}

	dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
	if (!dataRva) {
		log("no bound import info");
	} else {
		PIMAGE_BOUND_IMPORT_DESCRIPTOR pData = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, dataRva);
		log("----------bound import table----------");
		DWORD fstBound = (DWORD)pData;
		DWORD typeSize = sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);
		while (!isZeroBlock(pData, typeSize)) {
			PSTR name = (PSTR)(fstBound + pData->OffsetModuleName);
			log("%s(%p) %p", name, (DWORD)name - (DWORD)fileBuffer, pData->TimeDateStamp);
			pData++;
		}
	}
}

bool tryMoveBoundImportData(PVOID fileBuffer, DWORD notEmptyBegin, DWORD notEmptyLen) {
	bool isSuc = false;
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_DATA_DIRECTORY pDir = &(hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT]);
	DWORD dataRva = pDir->VirtualAddress;

	if (dataRva) {
		DWORD begin = (DWORD)rva2foa(fileBuffer, dataRva);
		if (begin == notEmptyBegin) {
			//log("%p %p %d %d", notEmptyBegin, begin, pDir->Size, notEmptyLen);
			DWORD leftSpace = notEmptyLen - 32 - pDir->Size;
			if (leftSpace >= sizeof(IMAGE_SECTION_HEADER)) {
				//copy
				DWORD targBegin = notEmptyBegin + leftSpace;
				PVOID dst = (PVOID)((DWORD)fileBuffer + targBegin);
				PVOID src = (PVOID)((DWORD)fileBuffer + notEmptyBegin);
				memcpy(dst, src, pDir->Size);
				//clear
				ZeroMemory(src, targBegin - notEmptyBegin);
				//reset
				pDir->VirtualAddress = foa2rva(fileBuffer, targBegin);
				isSuc = true;
			}
		}
	}
	return isSuc;
}

bool correctRva(DWORD& checkRva, PVOID fileBuffer, PVOID oldBuffer, DWORD refPos, DWORD delta) {
	DWORD foa = rva2foa(oldBuffer, checkRva);
	if (foa >= refPos) {
		foa += delta;
		checkRva = foa2rva(fileBuffer, foa);
		return true;
	}
	return false;
}

void restoreDir(PVOID fileBuffer, PVOID oldBuffer, int secIdx, DWORD secSize, DWORD secFoa) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	//Import
	PIMAGE_DATA_DIRECTORY pDir = &hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	bool isFix = correctRva(pDir->VirtualAddress, fileBuffer, oldBuffer, secFoa, secSize);
	if (isFix) {
		PIMAGE_IMPORT_DESCRIPTOR pData = (PIMAGE_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, pDir->VirtualAddress);
		DWORD typeSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		while (!isZeroBlock(pData, typeSize)) {
			correctRva(pData->Name, fileBuffer, oldBuffer, secFoa, secSize);
			correctRva(pData->OriginalFirstThunk, fileBuffer, oldBuffer, secFoa, secSize);
			correctRva(pData->FirstThunk, fileBuffer, oldBuffer, secFoa, secSize);
			//PSTR dllName = (PSTR)rva2fa(fileBuffer, pData->Name);
			//log("---%s", dllName);
			PDWORD pOFT = (PDWORD)rva2fa(fileBuffer, pData->OriginalFirstThunk);
			while (*pOFT != 0) {
				if ((*pOFT & 0x80000000) != 0x80000000) {
					correctRva(*pOFT, fileBuffer, oldBuffer, secFoa, secSize);
					//PIMAGE_IMPORT_BY_NAME pName = (PIMAGE_IMPORT_BY_NAME)rva2fa(fileBuffer, *pOFT);
					//log("\t%s", pName->Name);
				}
				pOFT++;
			}
			pData++;
		}
	}
	//Res
	pDir = &hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];
	isFix = correctRva(pDir->VirtualAddress, fileBuffer, oldBuffer, secFoa, secSize);
	if (isFix) {
		PIMAGE_RESOURCE_DIRECTORY pData = (PIMAGE_RESOURCE_DIRECTORY)rva2fa(fileBuffer, pDir->VirtualAddress);
	}
	//Dbg
	pDir = &hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	isFix = correctRva(pDir->VirtualAddress, fileBuffer, oldBuffer, secFoa, secSize);
	if (isFix) {
		PIMAGE_DEBUG_DIRECTORY pData = (PIMAGE_DEBUG_DIRECTORY)rva2fa(fileBuffer, pDir->VirtualAddress);
		correctRva(pData->AddressOfRawData, fileBuffer, oldBuffer, secFoa, secSize);
		if (pData->PointerToRawData >= secFoa) {
			pData->PointerToRawData += secSize;
		}
		//todo
	}
	//IAT
	pDir = &hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];
	isFix = correctRva(pDir->VirtualAddress, fileBuffer, oldBuffer, secFoa, secSize);
	//Config
	pDir = &hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
	isFix = correctRva(pDir->VirtualAddress, fileBuffer, oldBuffer, secFoa, secSize);
	//Dir(export, reloc)
}

void restoreData(PVOID fileBuffer, int secIdx, PCSTR secName, DWORD secSize, DWORD secFoa, PVOID oldBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	hNt->FileHeader.NumberOfSections += 1;
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

	DWORD typeSize = sizeof(IMAGE_SECTION_HEADER);
	DWORD offsetFa = (DWORD)fstSec + secIdx * typeSize;
	memcpy((PVOID)(offsetFa + typeSize), (PVOID)offsetFa, (hNt->FileHeader.NumberOfSections - 1 - secIdx) * typeSize);
	ZeroMemory((PVOID)offsetFa, typeSize);
	//Section
	DWORD sizeOfImg = 0;
	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		PIMAGE_SECTION_HEADER sec = fstSec + i;
		if (i == secIdx) {
			strncpy((CHAR*)sec->Name, secName, 7);
			sec->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
			sec->SizeOfRawData = secSize;
			sec->PointerToRawData = secFoa;
			sec->Misc.VirtualSize = secSize;
		}
		if (i >= secIdx) {
			DWORD va;
			if (i == 0) {
				va = align(hNt->OptionalHeader.SizeOfHeaders, hNt->OptionalHeader.SectionAlignment);
			} else {
				PIMAGE_SECTION_HEADER prevSec = fstSec + (i - 1);
				va = prevSec->VirtualAddress + align(prevSec->Misc.VirtualSize, hNt->OptionalHeader.SectionAlignment);
				sec->PointerToRawData = prevSec->PointerToRawData + prevSec->SizeOfRawData;
			}
			sec->VirtualAddress = va;
			log("%s %p %p %p %p", sec->Name, sec->Misc.VirtualSize, sec->VirtualAddress, sec->SizeOfRawData, sec->PointerToRawData);
		}
		if (i == hNt->FileHeader.NumberOfSections - 1) {
			sizeOfImg = sec->VirtualAddress + align(sec->Misc.VirtualSize, hNt->OptionalHeader.SectionAlignment);
		}
	}

	//SizeOfImage
	hNt->OptionalHeader.SizeOfImage = sizeOfImg;
	//OEP
	correctRva(hNt->OptionalHeader.AddressOfEntryPoint, fileBuffer, oldBuffer, secFoa, secSize);
	//Dir
	restoreDir(fileBuffer, oldBuffer, secIdx, secSize, secFoa);
}

//secIdx: [0, maxSecNum - 1], otherwise tail
DWORD addSection(PVOID fileBuffer, int secIdx, PCSTR secName, DWORD secSize, OUT PVOID* newBuffer){
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);
	PIMAGE_SECTION_HEADER lstSec = fstSec + (hNt->FileHeader.NumberOfSections - 1);
	DWORD oldFileSize = lstSec->PointerToRawData + lstSec->SizeOfRawData;

	if (secIdx < 0 || secIdx > hNt->FileHeader.NumberOfSections) {
		secIdx = hNt->FileHeader.NumberOfSections;
	}
	secSize = align(secSize, hNt->OptionalHeader.FileAlignment);
	DWORD newFileSize = oldFileSize + secSize;
	log("%d/%d 0x%p [0x%p] 0x%p", secIdx, hNt->FileHeader.NumberOfSections, oldFileSize, newFileSize, secSize);

	DWORD typeSize = sizeof(IMAGE_SECTION_HEADER);
	DWORD beginFoa = (DWORD)lstSec + typeSize - (DWORD)fileBuffer;
	DWORD endFoa = hNt->OptionalHeader.SizeOfHeaders;
	PVOID beginFa = (PVOID)(beginFoa + (DWORD)fileBuffer);
	bool isEmptyBehindSec = isZeroBlock(beginFa, endFoa - beginFoa);
	if (!isEmptyBehindSec) {
		if (!tryMoveBoundImportData(fileBuffer, beginFoa, endFoa - beginFoa)) {
			log("no available header space for new section...");
			return 0;
		}
		log("move bound finish!");
	}

	*newBuffer = malloc_s(newFileSize);
	DWORD bufferBase = (DWORD)*newBuffer;
	DWORD part1Size = 0;
	for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
		if (i == secIdx) {
			part1Size = (fstSec + i)->PointerToRawData;
			break;
		}
	}
	if (!part1Size) {
		part1Size = oldFileSize;
	}

	//log("%p", part1Size);
	memcpy((PVOID)bufferBase, fileBuffer, part1Size);
	DWORD part2Size = oldFileSize - part1Size;
	//log("%p %p", part1Size + secSize, part2Size);
	memcpy((PVOID)(bufferBase + part1Size + secSize), (PVOID)((DWORD)fileBuffer + part1Size), part2Size);

	restoreData((PVOID)bufferBase, secIdx, secName, secSize, part1Size, fileBuffer);

	return newFileSize;
}

void showData_2_Resource(PVOID fileBuffer) {
	PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
	DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	if (!dataRva) {
		log("no resource info");
		return;
	}
	//RT_XX, 17
	const char* nameRefTb[] = {
		0, "cursor", "bitMap", "icon", "menu", "dialog", "string",
		"fontDir", "font", "acce", "rawData", "msgTable",
		"cursorGroup", "undefine1", "iconGroup", "undefine2", "version"
	};

	log("----------resource table----------");
	DWORD typeSize = sizeof(IMAGE_RESOURCE_DIRECTORY);
	PIMAGE_RESOURCE_DIRECTORY pData = (PIMAGE_RESOURCE_DIRECTORY)rva2fa(fileBuffer, dataRva);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pData + typeSize);
	DWORD entryNum = pData->NumberOfIdEntries + pData->NumberOfNamedEntries;

	for (int i = 0; i < entryNum; i++) {
		if (pEntry[i].NameIsString) {
			PIMAGE_RESOURCE_DIR_STRING_U pResStr = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)pData + pEntry[i].NameOffset);
			log("[%d]%s", pResStr->Length, pResStr->NameString);
		} else {
			WORD resId = pEntry[i].Id;
			if (resId < 17) {
				log("internal: %s(%d)", nameRefTb[resId], resId);
			} else {
				log("custom: %d", resId);
			}
		}

		if (pEntry[i].DataIsDirectory) {
			PIMAGE_RESOURCE_DIRECTORY pData2 = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pData + pEntry[i].OffsetToDirectory);
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntry2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pData2 + typeSize);
			DWORD entryNum2 = pData2->NumberOfIdEntries + pData2->NumberOfNamedEntries;

			for (int j = 0; j < entryNum2; j++) {
				if (pEntry2[j].NameIsString) {
					PIMAGE_RESOURCE_DIR_STRING_U pResStr = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)pData + pEntry2[j].NameOffset);
					log("\t[%d]%s", pResStr->Length, pResStr->NameString);
				} else {
					WORD resId = pEntry2[j].Id;
					if (resId < 17) {
						log("\tinternal: %s(%d)", nameRefTb[resId], resId);
					} else {
						log("\tcustom: %d", resId);
					}
				}

				if (pEntry2[j].DataIsDirectory) {
					PIMAGE_RESOURCE_DIRECTORY pData3 = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pData + pEntry2[j].OffsetToDirectory);
					PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntry3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pData3 + typeSize);
					DWORD entryNum3 = pData3->NumberOfIdEntries + pData3->NumberOfNamedEntries;
					log("\t\tthird: %d", entryNum3);
				}
			}
		} else {
			log("not dir");
		}
	}
}
