#include "peTool.h"
#include <locale.h>

namespace win32Ready {
	void test1() {
		setlocale(LC_ALL, "chs");
		//e3
		char a = '你';
		//e3 c4
		wchar_t b = '你';
		//60
		char c = L'你';
		//60 4f
		wchar_t d = L'你';

		//c4 e3 ba c3 00
		char s1[] = "你a好";
		//60 4f 7d 59 00 00
		wchar_t s2[] = L"你a好";

		//5
		printf("[%d]%s\n", strlen(s1), s1);
		//3
		wprintf(L"[%d]%s\n", wcslen(s2), s2);

		char cp1[6] = {};
		strcpy(cp1, s1);
		wchar_t cp2[4] = {};
		wcscpy(cp2, s2);

		char cat1[30] = "hello ";
		strcat(cat1, "world");
		printf("%s\n", cat1);
		wchar_t cat2[20] = L"你好a ";
		wcscat(cat2, L"世界");
		wprintf(L"%s\n", cat2);

		char find1[] = "abc def ggg";
		int pos1 = strstr(find1, "def") - find1;
		printf("%d\n", pos1);
		wchar_t find2[] = L"你aaa好 世2界";
		int pos2 = wcsstr(find2, L"世") - find2;
		printf("%d\n", pos2);

		TCHAR ts[] = TEXT("aa");

		MessageBox(0, TEXT("h"), 0, 0);
	}

	void test2() {
	
	}
}

using namespace win32Ready;

void main() {
	test2();
}
