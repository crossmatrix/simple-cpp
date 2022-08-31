#include "win32Tool.h"

namespace win32Test {
	void test1(HINSTANCE hInstance, LPSTR lpCmdLine) {
		winLog(T("----------- %p %s"), hInstance, lpCmdLine);

		winLog(T("show log: %s %d"), T("aa"), 102);
		winLog(T("show log: %s %d"), T("ÄãºÃ"), 102);

		MessageBox(0, 0, 0, 0);
	}

	LRESULT CALLBACK MyWindowProc(HWND, UINT, WPARAM, LPARAM);

	void test2(HINSTANCE hInstance) {
		WNDCLASS wndCls = {};
		TCHAR clsName[] = T("MyWindow");
		wndCls.lpfnWndProc = MyWindowProc;
		wndCls.hInstance = hInstance;
		wndCls.lpszClassName = clsName;
		wndCls.hbrBackground = (HBRUSH)COLOR_MENU;
		RegisterClass(&wndCls);

		TCHAR wndName[] = T("MyFstWnd");
		HWND hwnd = CreateWindow(clsName, wndName, WS_OVERLAPPEDWINDOW, 700, 500, 500, 350, NULL, NULL, hInstance, NULL);
		if (!hwnd) {
			winLog(T("create %s error"), wndName);
			return;
		}
		ShowWindow(hwnd, SW_SHOW);

		MSG msg;
		while (GetMessage(&msg, 0, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_COMMAND:
		{
			winLog(T("WM_COMMAND: %d %d"), wParam, lParam);
			switch (wParam) {
			case 1001:
			{
				winLog(T("be called 1, wnd: %d"), lParam);
				break;
			}
			case 1002:
			{
				winLog(T("be called 2, wnd: %d"), lParam);
				break;
			}
			case 1003:
			{
				winLog(T("be called 3, wnd: %d"), lParam);
				break;
			}
			default:
				break;
			}
			break;
		}
		case WM_CREATE:
		{
			winLog(T("WM_CREATE %d %d"), wParam, lParam);
			CREATESTRUCT* createst = (CREATESTRUCT*)lParam;
			winLog(T("CREATESTRUCT %s"), createst->lpszClass);
			break;
		}
		case WM_MOVE:
		{
			winLog(T("WM_MOVE %d %d"), wParam, lParam);
			POINTS points = MAKEPOINTS(lParam);
			winLog(T("X Y %d %d"), points.x, points.y);
			break;
		}
		case WM_SIZE:
		{
			winLog(T("WM_SIZE %d %d"), wParam, lParam);
			int newWidth = (int)(short)LOWORD(lParam);
			int newHeight = (int)(short)HIWORD(lParam);
			winLog(T("WM_SIZE %d %d"), newWidth, newHeight);
			break;
		}
		case WM_DESTROY:
		{
			winLog(T("WM_DESTROY %d %d"), wParam, lParam);
			PostQuitMessage(0);
			break;
		}
		case WM_KEYUP:
		{
			//winLog("WM_KEYUP %d %d\n", wParam, lParam);
			break;
		}
		case WM_KEYDOWN:
		{
			winLog(T("WM_KEYDOWN %d %d"), wParam, lParam);
			switch (wParam) {
			case 65:
			{
				TCHAR c[10] = {};
				_itoa(wParam, c, 10);
				const TCHAR* h = T("suc1");
				MessageBox(0, c, h, 0);
				break;
			}
			case 70:
			{
				TCHAR c[10] = {};
				_itoa(wParam, c, 10);
				const TCHAR* h = T("suc2");
				MessageBox(0, c, h, 0);
				break;
			}
			default:
				break;
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			winLog(T("WM_LBUTTONDOWN %d %d"), wParam, lParam);
			POINTS points = MAKEPOINTS(lParam);
			winLog(T("WM_LBUTTONDOWN %d %d"), points.x, points.y);
			break;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}
}

using namespace win32Test;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//test1(hInstance, lpCmdLine);
	test2(hInstance);
	return 0;
}