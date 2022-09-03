#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include <CommCtrl.h>
#include <TlHelp32.h>
#include <vector>
#include <algorithm>
using namespace std;

namespace pistol {
	HINSTANCE appInstance;
	HWND hDlg;
	HWND hListProc;
	HWND hListMod;
	HWND hTextProc;

	struct SimpleProcInfo {
		DWORD pid;
		TCHAR name[MAX_PATH];

		SimpleProcInfo(DWORD aPid, PCTCH aName) {
			pid = aPid;
			_tcscpy(name, aName);
		}
	};

	bool CmpPid(SimpleProcInfo& x, SimpleProcInfo& y) {
		return x.pid < y.pid;
	}

	void GetProcessList(vector<SimpleProcInfo>& vec) {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		PROCESSENTRY32 proc = {};
		proc.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &proc)) {
			//BOOL isGetMod = FALSE;
			//DWORD priorityCls;
			//MODULEENTRY32 mod = {};

			do {
				//isGetMod = GetProcessModule(proc.th32ProcessID, proc.th32ModuleID, &mod, sizeof(MODULEENTRY32));
				//if (isGetMod) {
				//	// Get the actual priority class. 
				//	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc.th32ProcessID);
				//	priorityCls = GetPriorityClass(hProcess);
				//	CloseHandle(hProcess);

				//	// Print the process's information. 
				//	printf("\nPriority Class Base\t%d\n",
				//		proc.pcPriClassBase);
				//	printf("PID\t\t\t%d\n", proc.th32ProcessID);
				//	printf("Thread Count\t\t%d\n", proc.cntThreads);
				//	printf("Module Name\t\t%s\n", mod.szModule);
				//	printf("Full Path\t\t%s\n\n", mod.szExePath);
				//}

				SimpleProcInfo info(proc.th32ProcessID, proc.szExeFile);
				vec.push_back(info);
			} while (Process32Next(hSnapshot, &proc));
		}

		CloseHandle(hSnapshot);

		std::sort(vec.begin(), vec.end(), CmpPid);
	}

	void RefreshProc() {
		ListView_DeleteAllItems(hListProc);

		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		vector<SimpleProcInfo> vec;
		GetProcessList(vec);
		int size = vec.size();
		TCHAR pid[0x20] = {};
		for (int i = 0; i < size; i++) {
		  	SimpleProcInfo* p = &vec.at(i);
			
			item.pszText = p->name;
			item.iItem = i;
			item.iSubItem = 0;
			ListView_InsertItem(hListProc, &item);

			_itot(p->pid, pid, 10);
			item.pszText = pid;
			item.iItem = i;
			item.iSubItem = 1;
			ListView_SetItem(hListProc, &item);
		}
		
		TCHAR title[0x20] = {};
		_sntprintf(title, 0x20, _T("process(%d)"), size);
		SetWindowText(hTextProc, title);
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

		RefreshProc();
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
				if (wParam == IDC_LV_PROC && pNMHDR->code == NM_CLICK) {
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
