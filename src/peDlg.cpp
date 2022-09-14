#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include "peDlg.h"
#include "peTool.h"
using namespace std;

namespace peDlg {
	typedef void(*ParseDirData)(PVOID fileBuffer, HWND hwnd);
	HINSTANCE appInstance = NULL;
	HWND hDlgRoot = NULL;
	HWND hTabDir = NULL;
	TCHAR pFilePath[MAX_PATH] = {};
	HWND hListSec = NULL;
	HWND hDlgDir[15] = {};
	ParseDirData parseFuns[15] = {};

	INT_PTR CALLBACK dirDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			default:
				break;
		}
		return FALSE;
	}

	void initListView(HWND hLV, int* colWidthArr, PCTCH* colNameArr, int colNum) {
		SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)colNameArr[i];
			col.cx = colWidthArr[i];
			ListView_InsertColumn(hLV, i, &col);
		}
	}

	void parse_export(PVOID fileBuffer, HWND hwnd) {
		static BOOL initHead = false;
		if (!initHead) {
			initHead = true;
			HWND hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int colWidthArr[] = {100, 200};
			PCTCH colNameArr[] = {
				_T("Name"), _T("VirtualSize")
			};
			initListView(hListTop, colWidthArr, colNameArr, ARRAYSIZE(colWidthArr));
		}
	}

	void parse_reloc(PVOID fileBuffer, HWND hwnd) {
	}

	void initComp() {
		hListSec = GetDlgItem(hDlgRoot, IDC_LV_SEC);
		int colWidthArr[] = {140, 120, 120, 120, 120, 120};
		PCTCH colNameArr[] = {
			_T("Name"), _T("VirtualSize"), _T("VirtualAddress"), _T("SizeOfRawData"), _T("PointerToRawData"), _T("Characteristics")
		};
		initListView(hListSec, colWidthArr, colNameArr, ARRAYSIZE(colWidthArr));

		hTabDir = GetDlgItem(hDlgRoot, IDC_TAB_DIR);
		parseFuns[IMAGE_DIRECTORY_ENTRY_EXPORT] = parse_export;

		RECT rect = {};
		GetWindowRect(hTabDir, &rect);
		LONG w = rect.right - rect.left - 15;
		LONG h = rect.bottom - rect.top - 62;

		for (int i = 0; i < 15; i++) {
			hDlgDir[i] = CreateDialog(appInstance, MAKEINTRESOURCE(IDD_DIR), hTabDir, (DLGPROC)dirDlgProc);
			SetWindowPos(hDlgDir[i], HWND_TOP, 0, 50, w, h, 0);
		}
	}

	void setDirData(PVOID fileBuffer) {
		PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
		PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)NT_HEADER(fileBuffer);
		PIMAGE_DATA_DIRECTORY pDir = hNt->OptionalHeader.DataDirectory;
		static PCTCH tabNames[] = {
			_T("0_Export"), _T("1_Import"), _T("2_Resource"), _T("3_Exception"), _T("4_Security"), _T("5_Reloc"), _T("6_Debug"),
			_T("7_Arch"), _T("8_GP"), _T("9_TLS"), _T("10_LoadCfg"), _T("11_Bound_I"), _T("12_IAT"), _T("13_Delay_I"), _T("14_COM")
		};

		TabCtrl_DeleteAllItems(hTabDir);
		TCITEM tci = {};
		tci.mask = TCIF_TEXT | TCIF_PARAM;
		int subIdx = 0;

		for (int i = 0; i < 15; i++) {
			DWORD va = pDir[i].VirtualAddress;
			DWORD vs = pDir[i].Size;
			if (vs != 0) {
				tci.pszText = (PTCHAR)tabNames[i];
				tci.lParam = i;
				TabCtrl_InsertItem(hTabDir, subIdx++, &tci);
				if (parseFuns[i]) {
					parseFuns[i](fileBuffer, hDlgDir[i]);
				}
			}
		}
	}

	void setInfo() {
		PVOID fileBuffer = 0;
		long len = openPE(pFilePath, &fileBuffer);
		if (!len) {
			qLog("open pe error: %s", pFilePath);
			return;
		}
		TCHAR cont[0x10] = {};
		PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
		PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)NT_HEADER(fileBuffer);
		PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

		//---base
		_sntprintf(cont, 0x10, _T("%04X"), hDos->e_magic);
		SetDlgItemText(hDlgRoot, IDC_EDIT47, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hDos->e_lfanew);
		SetDlgItemText(hDlgRoot, IDC_EDIT1, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->Signature);
		SetDlgItemText(hDlgRoot, IDC_EDIT2, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Machine);
		SetDlgItemText(hDlgRoot, IDC_EDIT3, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.NumberOfSections);
		SetDlgItemText(hDlgRoot, IDC_EDIT4, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.SizeOfOptionalHeader);
		SetDlgItemText(hDlgRoot, IDC_EDIT5, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Characteristics);
		SetDlgItemText(hDlgRoot, IDC_EDIT6, cont);

		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Magic);
		SetDlgItemText(hDlgRoot, IDC_EDIT7, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.AddressOfEntryPoint);
		SetDlgItemText(hDlgRoot, IDC_EDIT8, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.ImageBase);
		SetDlgItemText(hDlgRoot, IDC_EDIT9, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SectionAlignment);
		SetDlgItemText(hDlgRoot, IDC_EDIT10, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.FileAlignment);
		SetDlgItemText(hDlgRoot, IDC_EDIT11, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfImage);
		SetDlgItemText(hDlgRoot, IDC_EDIT12, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfHeaders);
		SetDlgItemText(hDlgRoot, IDC_EDIT13, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.CheckSum);
		SetDlgItemText(hDlgRoot, IDC_EDIT14, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Subsystem);
		SetDlgItemText(hDlgRoot, IDC_EDIT15, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.DllCharacteristics);
		SetDlgItemText(hDlgRoot, IDC_EDIT16, cont);

		//IDC_EDIT 17~46
		for (int i = 0; i < 15; i++) {
			int fstEditId = IDC_EDIT17 + i * 2;
			PIMAGE_DATA_DIRECTORY pData = &hNt->OptionalHeader.DataDirectory[i];
			_sntprintf(cont, 0x10, _T("%p"), pData->VirtualAddress);
			SetDlgItemText(hDlgRoot, fstEditId, cont);
			_sntprintf(cont, 0x10, _T("%p"), pData->Size);
			SetDlgItemText(hDlgRoot, fstEditId + 1, cont);
		}

		//---section
		ListView_DeleteAllItems(hListSec);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;
		for (int i = 0; i < hNt->FileHeader.NumberOfSections; i++) {
			PIMAGE_SECTION_HEADER sec = fstSec + i;
			item.lParam = i;
			item.iItem = i;

			WCHAR name[10] = {};
			MultiByteToWideChar(CP_ACP, 0, (PCSTR)sec->Name, -1, name, 8);
			item.pszText = name;
			item.iSubItem = 0;
			ListView_InsertItem(hListSec, &item);

			_sntprintf(cont, 0x10, _T("%p"), sec->Misc.VirtualSize);
			item.pszText = cont;
			item.iSubItem = 1;
			ListView_SetItem(hListSec, &item);

			_sntprintf(cont, 0x10, _T("%p"), sec->VirtualAddress);
			item.pszText = cont;
			item.iSubItem = 2;
			ListView_SetItem(hListSec, &item);

			_sntprintf(cont, 0x10, _T("%p"), sec->SizeOfRawData);
			item.pszText = cont;
			item.iSubItem = 3;
			ListView_SetItem(hListSec, &item);

			_sntprintf(cont, 0x10, _T("%p"), sec->PointerToRawData);
			item.pszText = cont;
			item.iSubItem = 4;
			ListView_SetItem(hListSec, &item);

			_sntprintf(cont, 0x10, _T("%p"), sec->Characteristics);
			item.pszText = cont;
			item.iSubItem = 5;
			ListView_SetItem(hListSec, &item);
		}

		//---dirData
		setDirData(fileBuffer);

		free(fileBuffer);
	}

	void onSelChanged_DirPage() {
		for (int i = 0, len = ARRAYSIZE(hDlgDir); i < len; i++) {
			ShowWindow(hDlgDir[i], SW_HIDE);
		}
		int tabIdx = TabCtrl_GetCurFocus(hTabDir);
		TCITEM tci = {};
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(hTabDir, tabIdx, &tci);
		int idx = tci.lParam;
		ShowWindow(hDlgDir[idx], SW_SHOW);
		//qLog("idx: %d wnd: %d", idx, hDlgDir[idx]);
	}

	void onClick_ChooseFile() {
		OPENFILENAME file = {};
		TCHAR path[MAX_PATH] = {};
		file.lStructSize = sizeof(file);
		file.lpstrFile = path;
		file.nMaxFile = sizeof(path);
		file.lpstrFilter = _T("*.*");
		file.nFilterIndex = 1;
		file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&file)) {
			HWND hEd = GetDlgItem(hDlgRoot, IDC_FILE_PATH);
			SetWindowText(hEd, path);
			_tcscpy(pFilePath, path);
			setInfo();

			/*int pageIdx = TabCtrl_GetCurFocus(hTab);
			if (pageIdx == 0) {
				onSelChanged_Tab();
			} else {
				TabCtrl_SetCurFocus(hTab, 0);
			}*/
		}
	}

	INT_PTR CALLBACK peDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_INITDIALOG: {
				hDlgRoot = aDlg;
				initComp();
				return TRUE;
			}
			case WM_CLOSE: {
				EndDialog(aDlg, 0);
				return TRUE;
			}
			case WM_NOTIFY: {
				NMHDR* pNM = (NMHDR*)lParam;
				if (wParam == IDC_TAB_DIR) {
					if (pNM->code == TCN_SELCHANGE) {
						onSelChanged_DirPage();
						return TRUE;
					}
				}
				break;
			}
			case WM_COMMAND: {
				switch (wParam) {
					case IDC_BTN_FILE:
						onClick_ChooseFile();
						return TRUE;
					default:
						break;
				}
			}
			case WM_CTLCOLORSTATIC: {
				int id = GetDlgCtrlID((HWND)lParam);
				if (id >= IDC_EDIT17 && id <= IDC_EDIT46) {
					TCHAR cont[0x10] = {};
					GetDlgItemText(aDlg, id, cont, 0x10);
					if (!_tcscmp(cont, _T("00000000"))) {
						SetTextColor((HDC)wParam, RGB(255, 0, 0));
						SetBkMode((HDC)wParam, OPAQUE);
						return (INT_PTR)GetStockObject(NULL_BRUSH);
					}
				}
			}
			default:
				break;
		}
		return FALSE;
	}

	void showPEDlg(HINSTANCE instance) {
		appInstance = instance;
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_PE), NULL, peDlgProc);
	}
}
