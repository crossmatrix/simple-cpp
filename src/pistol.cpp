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

	struct SortData {
		int colId;
		bool asc;
	};

	HINSTANCE appInstance;
	HWND hDlg;
	HWND hListProc;
	HWND hListMod;
	HWND hTextProc;
	vector<SimpleProcInfo> vecProc;

	int CALLBACK CompareFunc(LPARAM p1, LPARAM p2, LPARAM data) {
		SortData* pData = (SortData*)data;
		//WinLog(_T("%d %d"), pData->asc, pData->colId);
		//WinLog(_T("%s %s"), cont1, cont2);

		TCHAR cont1[MAX_PATH] = {0};
		ListView_GetItemText(hListProc, p1, pData->colId, cont1, MAX_PATH);
		TCHAR cont2[MAX_PATH] = {0};
		ListView_GetItemText(hListProc, p2, pData->colId, cont2, MAX_PATH);

		BOOL flag;
		if (pData->colId == 0) {
			flag = _tcscmp(cont1, cont2) > 0;
		} else if(pData->colId == 1){
			int v1 = _ttoi(cont1);
			int v2 = _ttoi(cont2);
			flag = v1 > v2;
		} else {
			return FALSE;
		}
		if (!pData->asc) {
			flag = !flag;
		}
		return flag;
	}

	void SortProc(int col) {
		static int sortColumn = 0;
		static BOOL asc = TRUE;
		if (col != sortColumn)
			asc = TRUE;
		else
			asc = !asc;
		sortColumn = col;

		SortData data;
		data.colId = sortColumn;
		data.asc = asc;
		ListView_SortItemsEx(hListProc, CompareFunc, &data);
	}

	void GetProcessList() {
		//vecProc
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		vecProc.clear();
		PROCESSENTRY32 proc = {};
		proc.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &proc)) {
			do {
				SimpleProcInfo info(proc.th32ProcessID, proc.szExeFile);
				vecProc.push_back(info);
			} while (Process32Next(hSnapshot, &proc));
		}
		CloseHandle(hSnapshot);

		//listview items
		ListView_DeleteAllItems(hListProc);

		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		int size = vecProc.size();
		TCHAR pid[0x20] = {};
		for (int i = 0; i < size; i++) {
			SimpleProcInfo p = vecProc.at(i);
			item.lParam = i;
			item.iItem = i;

			item.pszText = p.name;
			item.iSubItem = 0;
			ListView_InsertItem(hListProc, &item);

			_itot(p.pid, pid, 10);
			item.pszText = pid;
			item.iSubItem = 1;
			ListView_SetItem(hListProc, &item);
		}
		
		//title
		TCHAR title[0x20] = {};
		_sntprintf(title, 0x20, _T("Process(%d)"), size);
		SetWindowText(hTextProc, title);
	}

	void InitList() {
		hListProc = GetDlgItem(hDlg, IDC_LV_PROC);
		hTextProc = GetDlgItem(hDlg, IDC_TITLE_PROC);
		SendMessage(hListProc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LV_COLUMN col = {};
		col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

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
		SortProc(0);
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
