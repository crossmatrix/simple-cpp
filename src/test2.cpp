#include <Windows.h>
#include <stdio.h>

#define log(str,...) printf(str##"\n",__VA_ARGS__)

#pragma comment(lib, "myDll.lib")
extern "C" __declspec(dllimport) int add(int, int);
extern "C" __declspec(dllimport) int sub(int, int);

#pragma comment(lib, "myDll2.lib")
__declspec(dllimport) int makeLeft(int, int);
__declspec(dllimport) int makeDiv(int, int);

int main_test2() {
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

	log("-----");
	log("%d %d", makeLeft(10, 2), makeDiv(100, 2));

	/*mod = LoadLibrary("myDll2.dll");
	if (mod) {
		FARPROC fp1 = GetProcAddress(mod, "makeLeft");
		FARPROC fp2 = GetProcAddress(mod, "makeDiv");
		log("%p %p", fp1, fp2);

		int(*func1)(int, int) = (int(*) (int, int))fp1;
		log("%d", func1(10, 2));
	}*/

	return 0;
}