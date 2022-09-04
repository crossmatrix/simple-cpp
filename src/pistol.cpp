#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include <CommCtrl.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>
#include <algorithm>
#include "peTool.h"
using namespace std;

namespace pistol {
	struct SimpleProcInfo {
		DWORD pid;
		TCHAR name[MAX_PATH];

		SimpleProcInfo(DWORD aPid, PCTCH aName) {
			pid = aPid;
			_tcscpy(name, aName);
		}
	};

	HINSTANCE appInstance;
	HWND hDlg;
	HWND hListProc;
	HWND hListMod;
	HWND hTextProc;
	vector<SimpleProcInfo> vecProc;
	BOOL sortPid = FALSE;
	BOOL sortProcName = FALSE;

	void RefreshProc() {
		ListView_DeleteAllItems(hListProc);

		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		int size = vecProc.size();
		TCHAR pid[0x20] = {};
		for (int i = 0; i < size; i++) {
			SimpleProcInfo p = vecProc.at(i);

			item.pszText = p.name;
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hListProc, &item);

			_itot(p.pid, pid, 10);
			item.pszText = pid;
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem(hListProc, &item);
		}

		TCHAR title[0x20] = {};
		_sntprintf(title, 0x20, _T("Process(%d)"), size);
		SetWindowText(hTextProc, title);
	}

	bool CmpProcName(SimpleProcInfo& x, SimpleProcInfo& y) {
		return _tcscmp(x.name, y.name) < 0;
	}

	bool CmpPid(SimpleProcInfo& x, SimpleProcInfo& y) {
		return x.pid < y.pid;
	}

	void SortProc(int col) {
		if (col == 0) {
			sortProcName = ~sortProcName;
			if (sortProcName) {
				std::sort(vecProc.begin(), vecProc.end(), CmpProcName);
			} else {
				std::sort(vecProc.rbegin(), vecProc.rend(), CmpProcName);
			}
		} else if (col == 1) {
			sortPid = ~sortPid;
			if (sortPid) {
				std::sort(vecProc.begin(), vecProc.end(), CmpPid);
			} else {
				std::sort(vecProc.rbegin(), vecProc.rend(), CmpPid);
			}
		}
		RefreshProc();
	}

	void GetProcessList() {
		vecProc.clear();
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		PROCESSENTRY32 proc = {};
		proc.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &proc)) {
			do {
				SimpleProcInfo info(proc.th32ProcessID, proc.szExeFile);
				vecProc.push_back(info);
			} while (Process32Next(hSnapshot, &proc));
		}
		CloseHandle(hSnapshot);
	}

	void InitList() {
		hListProc = GetDlgItem(hDlg, IDC_LV_PROC);
		hTextProc = GetDlgItem(hDlg, IDC_TITLE_PROC);
		SendMessage(hListProc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;

		int colNum = 4;
		PCTCH procColName[] = {
			_T("process"), _T("pid"), _T("imageBase"), _T("imageSize")
		};
		int procColWidth[] = {400, 100, 100, 100};

		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)procColName[i];
			col.cx = procColWidth[i];
			ListView_InsertColumn(hListProc, i, &col);
		}

		hListMod = GetDlgItem(hDlg, IDC_LV_MOD);
		SendMessage(hListMod, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		colNum = 2;
		PCTCH modColName[] = {
			_T("name"), _T("position")
		};
		int modColWidth[] = {600, 100};

		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)modColName[i];
			col.cx = modColWidth[i];
			ListView_InsertColumn(hListMod, i, &col);
		}

		GetProcessList();
		SortProc(1);
	}

	void GetModuleList(int pid) {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProcess) {
			MessageBox(0, _T("no privilege"), 0, 0);
			return;
		}

		DWORD cbNeeded;
		HMODULE hMods[1024];
		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			TCHAR szModName[MAX_PATH];
			for (int i = 0, max = (cbNeeded / sizeof(HMODULE)); i < max; i++) {

				//GetModuleBaseName
				//GetModuleFileName
				//GetModuleHandle
				//GetModuleInformation

				if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
					//_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
					WinLog(_T("%s"), szModName);
				}
			}
		}

		CloseHandle(hProcess);
	}

	void RefreshMod(HWND hListproc) {
		int rowId = ListView_GetNextItem(hListproc, -1, LVNI_SELECTED);
		if (rowId < 0) {
			return;
		}
		TCHAR pid[0x20];
		ListView_GetItemText(hListproc, rowId, 1, pid, 0x20);
		GetModuleList(_ttoi(pid));
	}

	INT_PTR CALLBACK MainDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_INITDIALOG: {
				hDlg = aDlg;
				InitList();
				break;
			}
			case WM_CLOSE: {
				EndDialog(hDlg, 0);
				break;
			}
			case WM_NOTIFY: {
				NMHDR* pNMHDR = (NMHDR*)lParam;
				if (wParam == IDC_LV_PROC) {
					if (pNMHDR->code == NM_CLICK) {
						RefreshMod(pNMHDR->hwndFrom);
					} else if (pNMHDR->code == LVN_COLUMNCLICK) {
						NMLISTVIEW* pNMLV = (NMLISTVIEW*)lParam;
						SortProc(pNMLV->iSubItem);
					}
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

	void ShowMain(HINSTANCE hInstance) {
		appInstance = hInstance;
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	}
};

using namespace pistol;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ShowMain(hInstance);
	return 0;
}
