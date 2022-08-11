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

	}
}

using namespace peTest;

int main() {
	test2();
	return 0;
}