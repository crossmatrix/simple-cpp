#include "peTool.h"

namespace peTest {
	const char* p1 = "E:/code/cpp/simple-cpp/res/";
	const char* p2 = "D:/Code/vsDir/simple-cpp/res/";
	const char* FILE_ROOT = p2;

	char* res(const char* name) {
		char* rs = (char*)malloc_s(50);
		sprintf(rs, "%s%s", FILE_ROOT, name);
		return rs;
	}

	void test1() {
		char* s1 = res("notepad.exe");
		char* s2 = res("1_copyBin.exe");
		copyBin(s1, s2);
		free(s1);
		free(s2);
		log("finish");
	}

	void test2() {
		char* s1 = res("notepad.exe");
		showPE(s1);
		free(s1);
		log("finish");
	}

	void test3() {
		char* s1 = res("notepad.exe");
		PVOID fileBuffer = 0;
		openPE(s1, &fileBuffer);

		PVOID imgBuffer = 0;
		peFile2Img(fileBuffer, &imgBuffer);
		log("file > image");

		PVOID newBuffer = 0;
		DWORD size = peImg2File(imgBuffer, &newBuffer);
		log("image > file");

		char* s2 = res("2_notepad_stretchBack.exe");
		savePE(newBuffer, size, s2);

		free(s1);
		free(fileBuffer);
		if (imgBuffer) {
			log("free img");
			free(imgBuffer);
		}
		if (newBuffer) {
			log("free newBuf");
			free(newBuffer);
		}
		log("finish");
	}

	void test4() {
		PSTR path = res("notepad.exe");
		showPE(path);
		//foa header: 0x0 0x3FF
		//foa sec1: 0x400 0x7BFF
		//foa sec2: 0x7C00 0x83FF
		//foa sec3: 0x8400 0x103FF
		//rva: 0~1000~9000~b000~13000

		LPVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		DWORD rva1 = foa2rva(fileBuffer, 0);
		DWORD rva2 = foa2rva(fileBuffer, 0x3FF);
		log("%p %p", rva1, rva2);
		DWORD foa1 = rva2foa(fileBuffer, rva1);
		DWORD foa2 = rva2foa(fileBuffer, rva2);
		log("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x400);
		rva2 = foa2rva(fileBuffer, 0x7BFF);
		log("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x7C00);
		rva2 = foa2rva(fileBuffer, 0x83FF);
		log("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x8400);
		rva2 = foa2rva(fileBuffer, 0x103FF);
		log("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, -1);
		rva2 = foa2rva(fileBuffer, 0x10400);
		log("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%p %p\n", foa1, foa2);

		log("%p", rva2foa(fileBuffer, 0x0fff));
		log("%p", rva2foa(fileBuffer, 0x1000));
		log("%p", rva2foa(fileBuffer, 0x1001));
		log("%p", rva2foa(fileBuffer, 0x8800 - 1));
		log("%p", rva2foa(fileBuffer, 0x8fff));
		log("%p", rva2foa(fileBuffer, 0x8fff + 1));
		log("%p", rva2foa(fileBuffer, 0xb000));
		log("%p", rva2foa(fileBuffer, 0xb000 + 0x10400 - 0x8400 - 1));
		log("%p", rva2foa(fileBuffer, 0x13000));

		free(path);
		free(fileBuffer);
	}

	void logSec(PIMAGE_SECTION_HEADER sec, int id) {
		if (sec != NULL) {
			log("[%d] %s %p %p", id, sec->Name, sec->PointerToRawData, sec->VirtualAddress);
		} 
	}

	void test5() {
		char* path = res("notepad.exe");
		//showPE(path);

		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);
		PIMAGE_SECTION_HEADER sec = NULL;

		sec	= getSecByFoa(fileBuffer, 0);
		logSec(sec, 1);
		
		sec = getSecByFoa(fileBuffer, 0x3FF);
		logSec(sec, 2);
		sec = getSecByFoa(fileBuffer, 0x3FF + 1);
		logSec(sec, 3);

		sec = getSecByFoa(fileBuffer, 0x7C00);
		logSec(sec, 4);
		sec = getSecByFoa(fileBuffer, 0x8400 - 1);
		logSec(sec, 5);

		sec = getSecByFoa(fileBuffer, 0x8400);
		logSec(sec, 6);
		sec = getSecByFoa(fileBuffer, 0x10400 - 1);
		logSec(sec, 7);
		sec = getSecByFoa(fileBuffer, 0x10400);
		logSec(sec, 8);

		free(path);
		free(fileBuffer);
	}

	void test6() {
		char* path = res("notepad.exe");
		//showPE(path);

		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);
		PIMAGE_SECTION_HEADER sec = NULL;

		sec = getSecByRva(fileBuffer, 0);
		logSec(sec, 1);

		sec = getSecByRva(fileBuffer, 0x1000 - 1);
		logSec(sec, 2);
		sec = getSecByRva(fileBuffer, 0x1000);
		logSec(sec, 3);

		sec = getSecByRva(fileBuffer, 0x9000);
		logSec(sec, 4);
		sec = getSecByRva(fileBuffer, 0x9000 - 1);
		logSec(sec, 5);

		sec = getSecByRva(fileBuffer, 0xb000 - 1);
		logSec(sec, 6);
		sec = getSecByRva(fileBuffer, 0xb000);
		logSec(sec, 7);
		sec = getSecByRva(fileBuffer, 0x13000 - 1);
		logSec(sec, 8);
		sec = getSecByRva(fileBuffer, 0x13000);
		logSec(sec, 9);

		free(path);
		free(fileBuffer);
	}

	byte shellCode[] = {
		0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00,
		0xE8, 0x00, 0x00, 0x00, 0x00,
		0xE9, 0x00, 0x00, 0x00, 0x00,
		0x00
	};

	void inj(PVOID fileBuffer, DWORD pos) {
		PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
		calcJmp(fileBuffer, pos, shellCode, 9, (DWORD)MessageBox);
		calcJmp(fileBuffer, pos, shellCode, 14, hNt->OptionalHeader.ImageBase + hNt->OptionalHeader.AddressOfEntryPoint);
		memcpy((PVOID)((DWORD)fileBuffer + pos), shellCode, ARRAYSIZE(shellCode));
		hNt->OptionalHeader.AddressOfEntryPoint = foa2rva(fileBuffer, pos);
	}

	void test7() {
		char* path = res("notepad.exe");
		PVOID fileBuffer = 0;
		long fileSize = openPE(path, &fileBuffer);

		int secIdx = 1;	//01234
		DWORD pos = 0;
		bool isFind = findEmpty(fileBuffer, ARRAYSIZE(shellCode), secIdx, true, &pos);
		if (isFind) {
			inj(fileBuffer, pos);

			char* outPath = res("3_injcode.exe");
			savePE(fileBuffer, fileSize, outPath);
			log("save finish: %s", outPath);
			free(outPath);
		}

		free(path);
		free(fileBuffer);
	}

	void test8() {
		//add sec(last or other)
		//extend sec(last or other)
		//merge
	}

	#pragma comment(lib, "myDll.lib")
	extern "C" __declspec(dllimport) int add(int, int);
	extern "C" __declspec(dllimport) int sub(int, int);
	void test9() {
		int v1 = add(30, 10);
		int v2 = sub(30, 10);
		log("%d %d", v1, v2);
		log("%p %p", add, sub);

		HMODULE mod = LoadLibrary("myDll.dll");
		if (mod) {
			FARPROC fp1 = GetProcAddress(mod, "add");
			FARPROC fp2 = GetProcAddress(mod, "sub");
			log("%p %p", fp1, fp2);

			int(*func1)(int, int) = (int(*) (int, int))fp1;
			int(*func2)(int, int) = (int(*) (int, int))fp2;
			log("%d %d", func1(30, 10), func2(30, 10));
		}
	}

	void test10() {
		char* path = res("testDll2.dll");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showPE(path);
		showData_0_Export(fileBuffer);

		log("---------");
		HMODULE mod = LoadLibrary("testDll2.dll");
		if (mod) {
			FARPROC fp1 = GetProcAddress(mod, "makeRight");
			FARPROC fp2 = GetProcAddress(mod, "makeLeft");
			FARPROC fp3 = GetProcAddress(mod, "makeMul");
			FARPROC fp4 = GetProcAddress(mod, PSTR(18));
			log("%p %p %p %p", fp1, fp2, fp3, fp4);
			int rs1 = ((int(*)(int, int))fp1)(100, 2);
			int rs2 = ((int(*)(int, int))fp2)(100, 2);
			int rs3 = ((int(*)(int, int))fp3)(30, 2);
			int rs4 = ((int(*)(int, int))fp4)(30, 2);
			log("%d %d %d %d", rs1, rs2, rs3, rs4);
		}
		log("---------");
		if (mod) {
			log("mod ImageBase: %p\n", mod);

			int test[] = { -1, 0, 3, 4, 12, 18, 19, 100 };
			for (int i = 0; i < 8; i++) {
				DWORD rs = GetFuncByOrdinal(fileBuffer, test[i]);
				if (rs) {
					log("valid: %d, %p", test[i], rs);
					DWORD funcPos = (DWORD)mod + rs;
					int v = ((int(*)(int, int))funcPos)(100, 2);
					log("> %d", v);
				}
			}

			PCSTR funcName = "makeLeft";
			DWORD rs = GetFuncByName(fileBuffer, funcName);
			if (rs) {
				log("%p", rs);
				DWORD funcPos = (DWORD)mod + rs;
				int v = ((int(*)(int, int))funcPos)(100, 2);
				log("> %d", v);
			}
		}

		free(path);
		free(fileBuffer);
	}

	void test11() {
		char* path = res("testDll2.dll");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showData_5_Reloc(fileBuffer);

		free(path);
		free(fileBuffer);
	}
}

using namespace peTest;

int main() {
	test11();
	return 0;
}