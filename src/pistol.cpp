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

	int CALLBACK compareFunc(LPARAM p1, LPARAM p2, LPARAM data) {
		SortData* pData = (SortData*)data;
		//winLog(_T("%d %d"), pData->asc, pData->colId);
		//winLog(_T("%s %s"), cont1, cont2);

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

	void sortProc(int col) {
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
		ListView_SortItemsEx(hListProc, compareFunc, &data);
	}

	void getProcessList() {
		//vecProc
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		vector<SimpleProcInfo> vecProc;
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

	void initList() {
		hListProc = GetDlgItem(hDlg, IDC_LV_PROC);
		hTextProc = GetDlgItem(hDlg, IDC_TITLE_PROC);
		SendMessage(hListProc, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;

		int colNum = 2;
		PCTCH procColName[] = {
			_T("process"), _T("pid")
		};
		int procColWidth[] = {400, 100};

		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)procColName[i];
			col.cx = procColWidth[i];
			ListView_InsertColumn(hListProc, i, &col);
		}

		hListMod = GetDlgItem(hDlg, IDC_LV_MOD);
		SendMessage(hListMod, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		colNum = 4;
		PCTCH modColName[] = {
			_T("name"), _T("oep"), _T("imageSize"), _T("path")
		};
		int modColWidth[] = {100, 100, 100, 400};

		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)modColName[i];
			col.cx = modColWidth[i];
			ListView_InsertColumn(hListMod, i, &col);
		}

		getProcessList();
		sortProc(0);
	}

	void getModuleList(int pid) {
		ListView_DeleteAllItems(hListMod);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProcess) {
			item.iItem = 0;
			item.iSubItem = 0;
			TCHAR msg[0x20];
			_sntprintf(msg, sizeof(msg), _T("err: %d"), GetLastError());
			item.pszText = msg;
			ListView_InsertItem(hListMod, &item);
			return;
		}

		DWORD cbNeeded;
		HMODULE hMods[1024];
		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			TCHAR name[MAX_PATH] = {};
			TCHAR path[MAX_PATH] = {};
			MODULEINFO info = {};
			for (int i = 0, max = (cbNeeded / sizeof(HMODULE)); i < max; i++) {
				item.iItem = i;
				if (GetModuleBaseName(hProcess, hMods[i], name, MAX_PATH)) {
					item.iSubItem = 0;
					item.pszText = name;
					ListView_InsertItem(hListMod, &item);
				}
				if (GetModuleFileNameEx(hProcess, hMods[i], path, MAX_PATH)) {
					item.iSubItem = 3;
					item.pszText = path;
					ListView_SetItem(hListMod, &item);
				}
				if (GetModuleInformation(hProcess, hMods[i], &info, sizeof(info))) {
					TCHAR oepAddr[10] = {};
					_sntprintf(oepAddr, sizeof(oepAddr), _T("%08X"), info.EntryPoint);
					item.iSubItem = 1;
					item.pszText = oepAddr;
					ListView_SetItem(hListMod, &item);

					TCHAR imgSize[10] = {};
					_sntprintf(imgSize, sizeof(imgSize), _T("%08X"), info.SizeOfImage);
					item.iSubItem = 2;
					item.pszText = imgSize;
					ListView_SetItem(hListMod, &item);
				}
			}
		} else {
			item.iItem = 0;
			item.iSubItem = 0;
			TCHAR msg[0x20];
			_sntprintf(msg, sizeof(msg), _T("err: %d"), GetLastError());
			item.pszText = msg;
			ListView_InsertItem(hListMod, &item);
		}

		CloseHandle(hProcess);
	}

	void refreshMod(HWND hListproc) {
		int rowId = ListView_GetNextItem(hListproc, -1, LVNI_SELECTED);
		if (rowId < 0) {
			return;
		}
		TCHAR pid[0x20] = {};
		ListView_GetItemText(hListproc, rowId, 1, pid, 0x20);
		getModuleList(_ttoi(pid));
	}

	INT_PTR CALLBACK mainDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_INITDIALOG: {
				hDlg = aDlg;
				initList();
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
						refreshMod(pNMHDR->hwndFrom);
					} else if (pNMHDR->code == LVN_COLUMNCLICK) {
						NMLISTVIEW* pNMLV = (NMLISTVIEW*)lParam;
						sortProc(pNMLV->iSubItem);
					}
				}
				break;
			}
			case WM_COMMAND: {
				winLog(_T("command: %p %p"), wParam, lParam);
				break;
			}
			default: {
				return FALSE;
			}
		}
		return TRUE;
	}

	void showMain(HINSTANCE hInstance) {
		appInstance = hInstance;
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, mainDlgProc);
	}
};

using namespace pistol;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	showMain(hInstance);
	return 0;
}
