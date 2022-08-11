#include "peTool.h"

namespace peTest {
	const char* p1 = "E:/code/cpp/simple-cpp/res/";
	const char* p2 = "D:/Code/vsDir/simple-cpp/res/";
	const char* FILE_ROOT = p2;


	void test1() {
		char srcPath[50];
		sprintf(srcPath, "%s%s", FILE_ROOT, "notepad.exe");
		char dstPath[50];
		sprintf(dstPath, "%s%s", FILE_ROOT, "copyBin.exe");
		copyBin(srcPath, dstPath);
		log("finish");
	}
}

using namespace peTest;

int main() {
	test1();
	return 0;
}