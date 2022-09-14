#include "win32Tool.h"
#include "testRes/testResource.h"

namespace win32Test {
	void test1(HINSTANCE hInstance, LPSTR lpCmdLine) {
		qLog("----------- %p %s", hInstance, lpCmdLine);

		qLog("show log: %s %d", _T("aa"), 102);
		qLog("show log: %s %d", _T("ÄãºÃ"), 102);

		MessageBox(0, 0, 0, 0);
	}

	LRESULT CALLBACK MyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_COMMAND: {
				qLog("WM_COMMAND: %d %d", wParam, lParam);
				switch (wParam) {
					case 1001: {
						qLog("be called 1, wnd: %d", lParam);
						break;
					}
					case 1002: {
						qLog("be called 2, wnd: %d", lParam);
						break;
					}
					case 1003: {
						qLog("be called 3, wnd: %d", lParam);
						break;
					}
					default:
						break;
				}
				break;
			}
			case WM_CREATE: {
				qLog("WM_CREATE %d %d", wParam, lParam);
				CREATESTRUCT* createst = (CREATESTRUCT*)lParam;
				qLog("CREATESTRUCT %s", createst->lpszClass);
				break;
			}
			case WM_MOVE: {
				qLog("WM_MOVE %d %d", wParam, lParam);
				POINTS points = MAKEPOINTS(lParam);
				qLog("X Y %d %d", points.x, points.y);
				break;
			}
			case WM_SIZE: {
				qLog("WM_SIZE %d %d", wParam, lParam);
				int newWidth = (int)(short)LOWORD(lParam);
				int newHeight = (int)(short)HIWORD(lParam);
				qLog("WM_SIZE %d %d", newWidth, newHeight);
				break;
			}
			case WM_DESTROY: {
				qLog("WM_DESTROY %d %d", wParam, lParam);
				PostQuitMessage(0);
				break;
			}
			case WM_KEYUP: {
				//qLog("WM_KEYUP %d %d\n", wParam, lParam);
				break;
			}
			case WM_KEYDOWN: {
				qLog("WM_KEYDOWN %d %d", wParam, lParam);
				switch (wParam) {
					case 65: {
						TCHAR c[10] = {};
						_itot(wParam, c, 10);
						const TCHAR* h = _T("suc1");
						MessageBox(0, c, h, 0);
						break;
					}
					case 70: {
						TCHAR c[10] = {};
						_itot(wParam, c, 10);
						const TCHAR* h = _T("suc2");
						MessageBox(0, c, h, 0);
						break;
					}
					default:
						break;
				}
				break;
			}
			case WM_LBUTTONDOWN: {
				qLog("WM_LBUTTONDOWN %d %d", wParam, lParam);
				POINTS points = MAKEPOINTS(lParam);
				qLog("WM_LBUTTONDOWN %d %d", points.x, points.y);
				break;
			}
			default:
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return 0;
	}

	void CreateSubWnd(HINSTANCE hInstance, HWND hwnd) {
		HWND wnd1 = CreateWindow(
			_T("Button"), _T("btn1"),
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			50, 20,
			100, 30,
			hwnd,
			(HMENU)1001,
			hInstance,
			NULL
		);
		HWND wnd2 = CreateWindow(
			_T("Button"), _T("btn2"),
			WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,
			50, 60,
			100, 30,
			hwnd,
			(HMENU)1002,
			hInstance,
			NULL
		);
		HWND wnd3 = CreateWindow(
			_T("Button"), _T("btn2"),
			WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
			50, 100,
			100, 30,
			hwnd,
			(HMENU)1003,
			hInstance,
			NULL
		);

		qLog("%d %d %d", wnd1, wnd2, wnd3);

		TCHAR clsName[20];
		GetClassName(wnd1, clsName, 20);
		WNDCLASS wndCls;
		GetClassInfo(hInstance, clsName, &wndCls);
		qLog(">>1. %s %p", clsName, wndCls.lpfnWndProc);

		GetClassName(hwnd, clsName, 20);
		GetClassInfo(hInstance, clsName, &wndCls);
		qLog(">>2. %s %p", clsName, wndCls.lpfnWndProc);
	}

	void test2(HINSTANCE hInstance) {
		WNDCLASS wndCls = {};
		TCHAR clsName[] = _T("MyWindow");
		wndCls.lpfnWndProc = MyWindowProc;
		wndCls.hInstance = hInstance;
		wndCls.lpszClassName = clsName;
		wndCls.hbrBackground = (HBRUSH)COLOR_MENU;
		RegisterClass(&wndCls);

		TCHAR wndName[] = _T("MyFstWnd");
		HWND hwnd = CreateWindow(clsName, wndName, WS_OVERLAPPEDWINDOW, 700, 500, 500, 350, NULL, NULL, hInstance, NULL);
		if (!hwnd) {
			qLog("create %s error", wndName);
			return;
		}

		CreateSubWnd(hInstance, hwnd);

		ShowWindow(hwnd, SW_SHOW);

		MSG msg;
		while (GetMessage(&msg, 0, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//qLog("%x", uMsg);
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				qLog("init dialog...");
				HINSTANCE hInstance = (HINSTANCE)lParam;
				HICON hIcon = LoadIcon(hInstance, PTCHAR(IDI_ICON));
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BTN_SHOW: {
						qLog("btnShow be called");
						HWND ipt1 = GetDlgItem(hwndDlg, IDC_EDIT1);
						TCHAR cont1[50] = {};
						GetWindowText(ipt1, cont1, 50);
						HWND ipt2 = GetDlgItem(hwndDlg, IDC_EDIT2);
						TCHAR cont2[50] = {};
						GetWindowText(ipt2, cont2, 50);

						TCHAR tip[100] = {};
						_tcscat(tip, cont1);
						_tcscat(tip, cont2);

						MessageBox(NULL, tip, _T("OK"), MB_OK);
						return TRUE;
					}
					default:
						break;
				}
				break;
			}
			default:
				break;
		}
		return FALSE;
	}

	void test3(HINSTANCE hInstance) {
		//INT_PTR ptr = DialogBox(hInstance, PTCHAR(IDD_DIALOG), NULL, DlgProc);
		DialogBoxParam(hInstance, PTCHAR(IDD_DIALOG), NULL, DlgProc, (LPARAM)hInstance);
	}
}

using namespace win32Test;

int WINAPI WinMain_test(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//test1(hInstance, lpCmdLine);
	//test2(hInstance);
	test3(hInstance);
	return 0;
}
