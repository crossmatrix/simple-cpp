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
		print("finish");
	}

	void test2() {
		char* s1 = res("notepad.exe");
		showPE(s1);
		free(s1);
		print("finish");
	}

	void test3() {
		char* s1 = res("notepad.exe");
		PVOID fileBuffer = 0;
		openPE(s1, &fileBuffer);

		PVOID imgBuffer = 0;
		peFile2Img(fileBuffer, &imgBuffer);
		print("file > image");

		PVOID newBuffer = 0;
		DWORD size = peImg2File(imgBuffer, &newBuffer);
		print("image > file");

		char* s2 = res("2_notepad_stretchBack.exe");
		savePE(newBuffer, size, s2);

		free(s1);
		free(fileBuffer);
		if (imgBuffer) {
			print("free img");
			free(imgBuffer);
		}
		if (newBuffer) {
			print("free newBuf");
			free(newBuffer);
		}
		print("finish");
	}

	void test4() {
		PSTR path = res("notepad.exe");
		showPE(path);
		//foa header: 0x0 0x3FF
		//foa sec1: 0x400 0x7BFF
		//foa sec2: 0x7C00 0x83FF
		//foa sec3: 0x8400 0x103FF
		//rva: 0~1000~9000~b000~13000

		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		DWORD rva1 = foa2rva(fileBuffer, 0);
		DWORD rva2 = foa2rva(fileBuffer, 0x3FF);
		print("%p %p", rva1, rva2);
		DWORD foa1 = rva2foa(fileBuffer, rva1);
		DWORD foa2 = rva2foa(fileBuffer, rva2);
		print("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x400);
		rva2 = foa2rva(fileBuffer, 0x7BFF);
		print("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		print("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x7C00);
		rva2 = foa2rva(fileBuffer, 0x83FF);
		print("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		print("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x8400);
		rva2 = foa2rva(fileBuffer, 0x103FF);
		print("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		print("%p %p\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, -1);
		rva2 = foa2rva(fileBuffer, 0x10400);
		print("%p %p", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		print("%p %p\n", foa1, foa2);

		print("%p", rva2foa(fileBuffer, 0x0fff));
		print("%p", rva2foa(fileBuffer, 0x1000));
		print("%p", rva2foa(fileBuffer, 0x1001));
		print("%p", rva2foa(fileBuffer, 0x8800 - 1));
		print("%p", rva2foa(fileBuffer, 0x8fff));
		print("%p", rva2foa(fileBuffer, 0x8fff + 1));
		print("%p", rva2foa(fileBuffer, 0xb000));
		print("%p", rva2foa(fileBuffer, 0xb000 + 0x10400 - 0x8400 - 1));
		print("%p", rva2foa(fileBuffer, 0x13000));

		free(path);
		free(fileBuffer);
	}

	void logSec(PIMAGE_SECTION_HEADER sec, int id) {
		if (sec != NULL) {
			print("[%d] %s %p %p", id, sec->Name, sec->PointerToRawData, sec->VirtualAddress);
		}
	}

	void test5() {
		char* path = res("notepad.exe");
		//showPE(path);

		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);
		PIMAGE_SECTION_HEADER sec = NULL;

		sec = getSecByFoa(fileBuffer, 0);
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

		int secIdx = 0;	//-1,0,1,2
		DWORD pos = 0;
		bool isFind = findEmpty(fileBuffer, ARRAYSIZE(shellCode), secIdx, true, &pos);
		if (isFind) {
			inj(fileBuffer, pos);

			char* outPath = res("3_injcode.exe");
			savePE(fileBuffer, fileSize, outPath);
			print("save finish: %s", outPath);
			free(outPath);
		}

		free(path);
		free(fileBuffer);
	}

#pragma comment(lib, "myDll.lib")
	extern "C" __declspec(dllimport) int add(int, int);
	extern "C" __declspec(dllimport) int sub(int, int);
	void test8() {
		int v1 = add(30, 10);
		int v2 = sub(30, 10);
		print("%d %d", v1, v2);
		print("%p %p", add, sub);

		HMODULE mod = LoadLibrary("myDll.dll");
		if (mod) {
			FARPROC fp1 = GetProcAddress(mod, "add");
			FARPROC fp2 = GetProcAddress(mod, "sub");
			print("%p %p", fp1, fp2);

			int(*func1)(int, int) = (int(*) (int, int))fp1;
			int(*func2)(int, int) = (int(*) (int, int))fp2;
			print("%d %d", func1(30, 10), func2(30, 10));
		}
	}

	void test9() {
		char* path = res("myDll2.dll");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showPE(path);
		showData_0_Export(fileBuffer);

		print("---------");
		HMODULE mod = LoadLibrary("myDll2.dll");
		if (mod) {
			FARPROC fp1 = GetProcAddress(mod, "makeRight");
			FARPROC fp2 = GetProcAddress(mod, "makeLeft");
			FARPROC fp3 = GetProcAddress(mod, "makeMul");
			FARPROC fp4 = GetProcAddress(mod, PSTR(18));
			print("%p %p %p %p", fp1, fp2, fp3, fp4);
			int rs1 = ((int(*)(int, int))fp1)(100, 2);
			int rs2 = ((int(*)(int, int))fp2)(100, 2);
			int rs3 = ((int(*)(int, int))fp3)(30, 2);
			int rs4 = ((int(*)(int, int))fp4)(30, 2);
			print("%d %d %d %d", rs1, rs2, rs3, rs4);
		}
		print("---------");
		if (mod) {
			print("mod ImageBase: %p\n", mod);

			int test[] = {-1, 0, 3, 4, 12, 18, 19, 100};
			for (int i = 0; i < 8; i++) {
				DWORD rs = getFuncByOrdinal(fileBuffer, test[i]);
				if (rs) {
					print("valid: %d, %p", test[i], rs);
					DWORD funcPos = (DWORD)mod + rs;
					int v = ((int(*)(int, int))funcPos)(100, 2);
					print("> %d", v);
				}
			}

			PCSTR funcName = "makeLeft";
			DWORD rs = getFuncByName(fileBuffer, funcName);
			if (rs) {
				print("%p", rs);
				DWORD funcPos = (DWORD)mod + rs;
				int v = ((int(*)(int, int))funcPos)(100, 2);
				print("> %d", v);
			}
		}

		free(path);
		free(fileBuffer);
	}

	void test10() {
		char* path = res("myDll2.dll");
		//char* path = res("notepad.exe");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showData_0_Export(fileBuffer);

		free(path);
		free(fileBuffer);
	}

	void test11() {
		char* path = res("myDll2.dll");
		//char* path = res("notepad.exe");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showData_5_Reloc(fileBuffer);

		free(path);
		free(fileBuffer);
	}

	void test12() {
		char* path = res("notepad.exe");
		//char* path = res("myDll2.dll");
		//char* path = res("test2.exe");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showData_1_11_Import_Bound(fileBuffer);

		free(path);
		free(fileBuffer);
	}

	void test13() {
		//add sec(last or other)
		//extend sec(last or other)
		//merge

		char* path = res("notepad.exe");
		//char* path = res("test2.exe");
		//char* path = res("myDll2.dll");
		//char* path = res("myDll.dll");
		PVOID fileBuffer = 0;
		DWORD oldFileSize = openPE(path, &fileBuffer);

		//showPE(path);

		PVOID newBuffer = 0;
		int secIdx = 0; //012,tail
		DWORD newFileSize = addSection(fileBuffer, secIdx, ".test", 800, &newBuffer);
		//savePE(fileBuffer, oldFileSize, res("notepad_test.exe"));

		if (newBuffer) {
			char* savePath = res("notepad_test.exe");
			savePE(newBuffer, newFileSize, savePath);
			print("save finish: %s", savePath);

			free(savePath);
			free(newBuffer);
		}
		free(path);
		free(fileBuffer);
	}

	void test14() {
		char* path = res("notepad.exe");
		//char* path = res("win32.exe");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showData_2_Resource(fileBuffer);

		free(path);
		free(fileBuffer);
	}

	void test15() {
		char* path = res("notepad_test.exe");
		PVOID fileBuffer = 0;
		openPE(path, &fileBuffer);

		showPE(path);

		showData_0_Export(fileBuffer);
		showData_5_Reloc(fileBuffer);
		showData_2_Resource(fileBuffer);
		showData_1_11_Import_Bound(fileBuffer);

		free(path);
		free(fileBuffer);
	}
}

using namespace peTest;

int main() {
	test14();
	return 0;
}