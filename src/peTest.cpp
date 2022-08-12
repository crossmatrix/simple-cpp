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
		char* s2 = res("copyBin.exe");
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
		PVOID peFile = 0;
		long peSize = openPE(s1, &peFile);

		PVOID peImg = 0;
		peFile2Img(peFile, &peImg);
		log("file > image");

		PVOID newBuf = 0;
		peImg2File(peImg, &newBuf);
		log("image > file");

		free(s1);
		free(peFile);
		if (peImg) {
			log("free img");
			free(peImg);
		}
		if (newBuf) {
			log("free newBuf");
			free(newBuf);
		}
		log("finish");
	}
}

using namespace peTest;

int main() {
	test3();
	return 0;
}