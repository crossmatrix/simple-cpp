#include "win32Tool.h"
#include "pistolRes/pistolResource.h"

namespace pistol {
	HINSTANCE appInstance;

	INT_PTR CALLBACK mainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
			case WM_INITDIALOG: {
				WinLog(_T("init..."), appInstance);
				return TRUE;
			}
			case WM_COMMAND: {

				return TRUE;
			}
			default:
				break;
		}
		return FALSE;
	}

	void showMain() {
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, mainDlgProc);
	}
};

using namespace pistol;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	appInstance = hInstance;
	showMain();
	return 0;
}
