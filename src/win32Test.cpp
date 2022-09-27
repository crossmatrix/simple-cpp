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

	class Car {
	public:
		float price;
		TCHAR name[0x10];

		Car(float price, PCWCH name) {
			this->price = price;
			_tcscpy(this->name, name);
			qLog("ctor %s", this->name);
		}

		~Car() {
			qLog("dtor %s", this->name);
		}

		void run() {
			qLog("%s %.2f is running", this->name, this->price);
		}
	};

	HWND hDlg = NULL;
	HANDLE hThread = NULL;
	BOOL isQuit = FALSE;
	Car* c = NULL;
	DWORD WINAPI SelfSub(LPVOID param){
		c = new Car(100.1, _T("RedCar"));
		c->run();
		//when thread quit, c will not auto release!

		WCHAR cont[0x10] = {};
		GetDlgItemText(hDlg, IDC_EDIT_COUNT, cont, 0x10);
		int val = _wtoi(cont);
		while (val > 0) {
			if (isQuit) {
				ExitThread(1);
			}
			val -= (int)param;
			wsprintf(cont, L"%d", val);
			SetDlgItemText(hDlg, IDC_EDIT_COUNT, cont);
			Sleep(500);
		}
		return 0;
	}

	INT_PTR CALLBACK DlgProc3(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//qLog("%x", uMsg);
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hDlg = hwndDlg;
				HWND hEdit = GetDlgItem(hwndDlg, IDC_EDIT_COUNT);
				SetWindowText(hEdit, (LPCWSTR)L"100");
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BUTTON_START: {
						hThread = CreateThread(0, 0, SelfSub, (LPVOID)2, 0, 0);
						/*if (hThread) {
							CloseHandle(hThread);
						}*/
						return TRUE;
					}
					case IDC_BUTTON_SUSPEND: {
						SuspendThread(hThread);
						return TRUE;
					}
					case IDC_BUTTON_RESUME: {
						ResumeThread(hThread);
						return TRUE;
					}
					case IDC_BUTTON_EXIT: {
						isQuit = TRUE;
						return TRUE;
					}
					case IDC_BUTTON_TERMINATE: {
						TerminateThread(hThread, 2);
						return TRUE;
					}
					case IDC_BTN_SHOW: {
						c->run();
						return TRUE;
					}
					case IDC_BUTTON_CONTEXT: {
						SuspendThread(hThread);
						CONTEXT context = {};
						context.ContextFlags = CONTEXT_ALL;
						GetThreadContext(hThread, &context);
						context.Eip = 0x00410000;
						SetThreadContext(hThread, &context);
						ResumeThread(hThread);
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

	void test4(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc3);
		//GetExitCodeThread();
	}

	bool valid = true;
	DWORD WINAPI Test_Cust_Lock(LPVOID param) {
		while (!valid) {
		}
		valid = false;
		for (int i = 0; i < (int)param; i++) {
			WCHAR cont[0x10] = {};
			GetDlgItemText(hDlg, IDC_EDIT_COUNT, cont, 0x10);
			int val = _wtoi(cont);
			val++;
			wsprintf(cont, L"%d", val);
			SetDlgItemText(hDlg, IDC_EDIT_COUNT, cont);
			//Sleep(500);
		}
		valid = true;
		return 0;
	}

	CRITICAL_SECTION crtSec;
	DWORD WINAPI Test_Crt_Lock(LPVOID param) {
		for (int i = 0; i < (int)param; i++) {
			EnterCriticalSection(&crtSec);
			WCHAR cont[0x10] = {};
			GetDlgItemText(hDlg, IDC_EDIT_COUNT, cont, 0x10);
			int val = _wtoi(cont);
			val++;
			wsprintf(cont, L"%d", val);
			SetDlgItemText(hDlg, IDC_EDIT_COUNT, cont);
			//Sleep(500);
			LeaveCriticalSection(&crtSec);
		}
		return 0;
	}

	CRITICAL_SECTION crtSec2;
	DWORD WINAPI Test_CrtDeadLock1(LPVOID param) {
		qLog("t1 ready s1");
		EnterCriticalSection(&crtSec);
		Sleep(1000);
		qLog("t1 ready s2");
		EnterCriticalSection(&crtSec2);
		qLog("t1 has s1 s2, no lock");
		LeaveCriticalSection(&crtSec2);
		LeaveCriticalSection(&crtSec);
		return 0;
	}

	DWORD WINAPI Test_CrtDeadLock2(LPVOID param) {
		qLog("t2 ready s2");
		EnterCriticalSection(&crtSec2);
		Sleep(1000);
		qLog("t2 ready s1");
		EnterCriticalSection(&crtSec);
		qLog("t2 has s1 s2, no lock");
		LeaveCriticalSection(&crtSec);
		LeaveCriticalSection(&crtSec2);
		return 0;
	}

	INT_PTR CALLBACK DlgProc4(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//qLog("%x", uMsg);
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				DeleteCriticalSection(&crtSec);
				DeleteCriticalSection(&crtSec2);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hDlg = hwndDlg;
				HWND hEdit = GetDlgItem(hwndDlg, IDC_EDIT_COUNT);
				SetWindowText(hEdit, (LPCWSTR)L"0");
				InitializeCriticalSection(&crtSec);
				InitializeCriticalSection(&crtSec2);
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BUTTON_START: {
						HANDLE hThread = CreateThread(0, 0, Test_CrtDeadLock1, (LPVOID)10000, 0, 0);
						if (hThread) {
							CloseHandle(hThread);
						}
						return TRUE;
					}
					case IDC_BUTTON_SUSPEND: {
						HANDLE hThread = CreateThread(0, 0, Test_CrtDeadLock2, (LPVOID)10000, 0, 0);
						if (hThread) {
							CloseHandle(hThread);
						}
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

	void test5(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc4);
	}

	DWORD WINAPI Test_Block(LPVOID param) {
		int num = (int)param;
		for (int i = 0; i < num; i++) {
			if (num % 2 == 0) {
				qLog("----%d", i);
			} else {
				qLog("++++%d", i);
			}
			Sleep(1000);
		}
		qLog("--finish thread-- %d", param);
		return 0;
	}

	INT_PTR CALLBACK DlgProc5(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//qLog("%x", uMsg);
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BUTTON_START: {
						HANDLE hThread = CreateThread(0, 0, Test_Block, (LPVOID)5, 0, 0);
						//TerminateThread(hThread, 0);
						//CloseHandle(hThread);

						DWORD flag1 = WaitForSingleObject(hThread, 3000);
						qLog("+++1 %d", flag1);
						//DWORD flag2 = WaitForSingleObject(hThread, 3000);
						//qLog("+++2 %d", flag2);
						//DWORD flag3 = WaitForSingleObject(hThread, 3000);
						//qLog("+++3 %d", flag3);

						CloseHandle(hThread);
						return TRUE;
					}
					case IDC_BUTTON_SUSPEND: {
						HANDLE handleArr[2];
						handleArr[0] = CreateThread(0, 0, Test_Block, (LPVOID)5, 0, 0);
						handleArr[1] = CreateThread(0, 0, Test_Block, (LPVOID)10, 0, 0);
						DWORD flag = WaitForMultipleObjects(2, handleArr, TRUE, -1);
						qLog("all finish %d", flag);
						for (int i = 0; i < 2; i++) {
							CloseHandle(handleArr[i]);
						}
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

	void test6(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc5);
	}

	DWORD WINAPI Test_Mutext1(LPVOID param) {
		int idx = 0;
		while (1) {
			qLog("1: %d", idx++);
			Sleep(1000);
		}
		return 0;
	}

	HANDLE hMt;
	INT_PTR CALLBACK DlgProc6(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		//qLog("%x", uMsg);
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hMt = CreateMutex(NULL, FALSE, L"mt");
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BUTTON_START: {
						HANDLE hThread = CreateThread(0, 0, Test_Mutext1, 0, 0, 0);
						//CloseHandle(hThread);
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

	void test7(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)DlgProc6);
	}

	int totalMoney = 100;
	DWORD WINAPI TestWork(LPVOID param) {
		int id = (int)param;
		
		while (1) {
			//1.
			/*HANDLE lock = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"myLock");
			if (!lock) {
				qLog("not find lock");
				return 0;
			}
			WaitForSingleObject(lock, -1);*/
			//2.
			EnterCriticalSection(&crtSec);

			int leftMoney = totalMoney - (id + 1);
			if (leftMoney < 10) {
				qLog("quit %d", id);
				//ReleaseMutex(lock);
				LeaveCriticalSection(&crtSec);
				return 0;
			}
			WCHAR cont[0x10] = {};
			GetDlgItemText(hDlg, IDC_EDIT2 + id, cont, 0x10);
			int val = _wtoi(cont);
			val += (id + 1);
			wsprintf(cont, L"%d", val);
			SetDlgItemText(hDlg, IDC_EDIT2 + id, cont);
			wsprintf(cont, L"%d", leftMoney);
			SetDlgItemText(hDlg, IDC_EDIT1, cont);
			totalMoney = leftMoney;

			//ReleaseMutex(lock);
			LeaveCriticalSection(&crtSec);
			Sleep(200);
		}

		return 0;
	}

	DWORD WINAPI DoWork(LPVOID param) {
		HANDLE threadArr[3];
		for (int i = 0; i < 3; i++) {
			threadArr[i] = CreateThread(NULL, 0, TestWork, (LPVOID)i, 0, 0);
		}
		//main thread is block! so it must use new thread!!
		int flag = WaitForMultipleObjects(3, threadArr, TRUE, -1);
		qLog("all finish %d", flag);
		for (int i = 0; i < 3; i++) {
			CloseHandle(threadArr[i]);
			threadArr[i] = 0;
		}
		
		return 0;
	}

	INT_PTR CALLBACK DlgProc7(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE: {
				CloseHandle(hMt);
				DeleteCriticalSection(&crtSec);
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hDlg = hwndDlg;
				hMt = CreateMutex(NULL, FALSE, L"myLock");
				InitializeCriticalSection(&crtSec);
				SetDlgItemText(hwndDlg, IDC_EDIT1, L"100");
				SetDlgItemText(hwndDlg, IDC_EDIT2, L"0");
				SetDlgItemText(hwndDlg, IDC_EDIT3, L"0");
				SetDlgItemText(hwndDlg, IDC_EDIT4, L"0");
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BTN_RUN: {
						HANDLE hThread = CreateThread(NULL, 0, DoWork, 0, 0, 0);
						CloseHandle(hThread);
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

	void test8(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc7);
	}

	int product = -1;
	HANDLE hEv1, hEv2;

	DWORD WINAPI Producter(LPVOID param) {
		for (int i = 0; i < 10; i++) {
			//EnterCriticalSection(&crtSec);
			WaitForSingleObject(hEv1, -1);
			product = 1;
			qLog("p %d", product);
			SetEvent(hEv2);
			//LeaveCriticalSection(&crtSec);
		}
		return 0;
	}

	DWORD WINAPI Consumer(LPVOID param) {
		for (int i = 0; i < 10; i++) {
			//EnterCriticalSection(&crtSec);
			WaitForSingleObject(hEv2, -1);
			product = 0;
			qLog("c %d", product);
			SetEvent(hEv1);
			//LeaveCriticalSection(&crtSec);
		}
		return 0;
	}

	INT_PTR CALLBACK DlgProc8(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE: {
				//DeleteCriticalSection(&crtSec);
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hDlg = hwndDlg;
				//InitializeCriticalSection(&crtSec);
				hEv1 = CreateEvent(NULL, FALSE, TRUE, NULL);
				hEv2 = CreateEvent(NULL, FALSE, FALSE, NULL);
				SetDlgItemText(hwndDlg, IDC_EDIT1, L"100");
				SetDlgItemText(hwndDlg, IDC_EDIT2, L"0");
				SetDlgItemText(hwndDlg, IDC_EDIT3, L"0");
				SetDlgItemText(hwndDlg, IDC_EDIT4, L"0");
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BTN_RUN: {
						qLog("start");
						CreateThread(NULL, 0, Producter, 0, 0, 0);
						CreateThread(NULL, 0, Consumer, 0, 0, 0);
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

	void test9(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc8);
	}

	HANDLE hSmp;
	DWORD WINAPI SmpTest1(LPVOID param) {
		WaitForSingleObject(hSmp, -1);
		for (int i = 0; i < 5; i++) {
			qLog("t1 %d", i);
			Sleep(200);
		}
		ReleaseSemaphore(hSmp, 1, NULL);
		return 0;
	}

	DWORD WINAPI SmpTest2(LPVOID param) {
		WaitForSingleObject(hSmp, -1);
		for (int i = 0; i < 5; i++) {
			qLog("t2 %d", i);
			Sleep(200);
		}
		ReleaseSemaphore(hSmp, 1, NULL);
		return 0;
	}

	DWORD WINAPI SmpTest3(LPVOID param) {
		WaitForSingleObject(hSmp, -1);
		for (int i = 0; i < 5; i++) {
			qLog("t3 %d", i);
			Sleep(200);
		}
		ReleaseSemaphore(hSmp, 1, NULL);
		return 0;
	}

	INT_PTR CALLBACK DlgProc9(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				CloseHandle(hSmp);
				return TRUE;
			}
			case WM_INITDIALOG: {
				hSmp = CreateSemaphore(NULL, 0, 3, NULL);
				return TRUE;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BTN_RUN: {
						qLog("start");
						CreateThread(NULL, 0, SmpTest1, 0, 0, 0);
						CreateThread(NULL, 0, SmpTest2, 0, 0, 0);
						CreateThread(NULL, 0, SmpTest3, 0, 0, 0);

						for (int i = 0; i < 5; i++) {
							qLog("main %d", i);
							Sleep(200);
						}
						ReleaseSemaphore(hSmp, 2, NULL);

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

	void test10(HINSTANCE hInstance) {
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc9);
	}
}

using namespace win32Test;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//test1(hInstance, lpCmdLine);
	//test2(hInstance);
	//test3(hInstance);
	
	//thread
	//test4(hInstance);

	//critical
	//test5(hInstance);

	//wait
	//test6(hInstance);

	//mutex1
	//test7(hInstance);

	//crt/mutex work
	//test8(hInstance);

	//event
	//test9(hInstance);

	//semaphore
	test10(hInstance);

	return 0;
}
