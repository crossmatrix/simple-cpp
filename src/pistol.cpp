#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include <CommCtrl.h>

namespace pistol {
	HINSTANCE appInstance;

	void RefreshProc(HWND hDlg) {
		HWND hListProc = GetDlgItem(hDlg, IDC_PROC);

		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		item.pszText = (LPWSTR)_T("aa");
		item.iItem = 0;
		item.iSubItem = 0;
		ListView_InsertItem(hListProc, &item);

		item.pszText = (LPWSTR)_T("bb");
		item.iItem = 0;
		item.iSubItem = 1;
		ListView_SetItem(hListProc, &item);

		item.pszText = (LPWSTR)_T("cc");
		item.iItem = 1;
		item.iSubItem = 0;
		ListView_InsertItem(hListProc, &item);

		item.pszText = (LPWSTR)_T("dd");
		item.iItem = 1;
		item.iSubItem = 1;
		ListView_SetItem(hListProc, &item);
	}

	void InitList(HWND hDlg) {
		HWND hListProc = GetDlgItem(hDlg, IDC_PROC);
		SendMessage(hListProc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;

		int colNum = 4;
		LPWSTR procColName[] = {
			(LPWSTR)_T("process"), (LPWSTR)_T("pid"), (LPWSTR)_T("imageBase"), (LPWSTR)_T("imageSize")
		};
		int procColWidth[] = {450, 60, 100, 100};

		for (int i = 0; i < colNum; i++) {
			col.pszText = procColName[i];
			col.cx = procColWidth[i];
			ListView_InsertColumn(hListProc, i, &col);
		}

		HWND hListMod = GetDlgItem(hDlg, IDC_MOD);
		SendMessage(hListMod, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		colNum = 2;
		LPWSTR modColName[] = {
			(LPWSTR)_T("name"), (LPWSTR)_T("position")
		};
		int modColWidth[] = {610, 100};

		for (int i = 0; i < colNum; i++) {
			col.pszText = modColName[i];
			col.cx = modColWidth[i];
			ListView_InsertColumn(hListMod, i, &col);
		}

		RefreshProc(hDlg);
	}

	void RefreshMod(HWND hListproc) {
		int rowId = ListView_GetNextItem(hListproc, -1, LVNI_SELECTED);
		if (rowId < 0) {
			return;
		}
		TCHAR pid[0x20];
		ListView_GetItemText(hListproc, rowId, 1, pid, 0x20);
		
		WinLog(_T("pid = %s"), pid);
	}

	INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_CLOSE: {
				EndDialog(hDlg, 0);
				break;
			}
			case WM_INITDIALOG: {
				InitList(hDlg);
				break;
			}
			case WM_NOTIFY: {
				NMHDR* pNMHDR = (NMHDR*)lParam;
				if (wParam == IDC_PROC && pNMHDR->code == NM_CLICK) {
					RefreshMod(pNMHDR->hwndFrom);
				}
				break;
			}
			case WM_COMMAND: {
				WinLog(_T("command: %p %p"), wParam, lParam);
				break;
			}
			default: {
				return FALSE;
			}
		}
		return TRUE;
	}

	void ShowMain() {
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}
};

using namespace pistol;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	appInstance = hInstance;
	ShowMain();
	return 0;
}
