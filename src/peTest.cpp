#include "peTool.h"

namespace peTest {
	void test1() {
		copyBin("E:/code/cpp/simple-cpp/res/notepad.exe", "E:/code/cpp/simple-cpp/res/copyBin.exe");
		log("finish");
	}
}

using namespace peTest;

int main() {
	test1();
	return 0;
}