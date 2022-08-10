#include "peTool.h"

namespace peTest {
	void test1() {
		copyBin("E:/code/cpp/test1/review/notepad.exe", "E:/code/cpp/test1/review/copyBin.exe");
		log("finish");
	}
}

using namespace peTest;

int main() {
	test1();
}