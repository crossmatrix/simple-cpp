#include "peTool.h"

namespace peTest {
	const char* p1 = "E:/code/cpp/simple-cpp/res/";
	const char* p2 = "D:/Code/vsDir/simple-cpp/res/";
	const char* FILE_ROOT = p1;

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
		log("%08x %08x", rva1, rva2);
		DWORD foa1 = rva2foa(fileBuffer, rva1);
		DWORD foa2 = rva2foa(fileBuffer, rva2);
		log("%08x %08x\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x400);
		rva2 = foa2rva(fileBuffer, 0x7BFF);
		log("%08x %08x", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%08x %08x\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x7C00);
		rva2 = foa2rva(fileBuffer, 0x83FF);
		log("%08x %08x", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%08x %08x\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, 0x8400);
		rva2 = foa2rva(fileBuffer, 0x103FF);
		log("%08x %08x", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%08x %08x\n", foa1, foa2);

		rva1 = foa2rva(fileBuffer, -1);
		rva2 = foa2rva(fileBuffer, 0x10400);
		log("%08x %08x", rva1, rva2);
		foa1 = rva2foa(fileBuffer, rva1);
		foa2 = rva2foa(fileBuffer, rva2);
		log("%08x %08x\n", foa1, foa2);

		log("%08x", rva2foa(fileBuffer, 0x0fff));
		log("%08x", rva2foa(fileBuffer, 0x1000));
		log("%08x", rva2foa(fileBuffer, 0x1001));
		log("%08x", rva2foa(fileBuffer, 0x8800 - 1));
		log("%08x", rva2foa(fileBuffer, 0x8fff));
		log("%08x", rva2foa(fileBuffer, 0x8fff + 1));
		log("%08x", rva2foa(fileBuffer, 0xb000));
		log("%08x", rva2foa(fileBuffer, 0xb000 + 0x10400 - 0x8400 - 1));
		log("%08x", rva2foa(fileBuffer, 0x13000));

		free(path);
		free(fileBuffer);
	}

	void test5() {
		
	}
}

using namespace peTest;

int main() {
	test5();
	return 0;
}