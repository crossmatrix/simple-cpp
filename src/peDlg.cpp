#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include "peDlg.h"
#include "peTool.h"
using namespace std;

namespace peDlg {
	HINSTANCE appInstance = NULL;
	HWND hDlg = NULL;
	//HWND hTab = NULL;
	//HWND hSubDlg[2] = {};
	TCHAR pFilePath[MAX_PATH] = {};
	HWND hListSec = NULL;

	//INT_PTR CALLBACK baseInfoDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//	switch (uMsg) {
	//		default:
	//			break;
	//	}
	//	return FALSE;
	//}

	void initComp() {
		hListSec = GetDlgItem(hDlg, IDC_LV_SEC);
		SendMessage(hListSec, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		int colNum = 6;
		PCTCH procColName[] = {
			_T("Name"), _T("VirtualSize"), _T("VirtualAddress"), _T("SizeOfRawData"), _T("PointerToRawData"), _T("Characteristics")
		};
		int procColWidth[] = {140, 120, 120, 120, 120, 120};
		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)procColName[i];
			col.cx = procColWidth[i];
			ListView_InsertColumn(hListSec, i, &col);
		}

		/*hTab = GetDlgItem(hDlg, IDC_TAB);
		PCTCH tabNames[] = {_T("base"), _T("section")};
		PCTCH dlgNames[] = {MAKEINTRESOURCE(IDD_BASE_INFO), MAKEINTRESOURCE(IDD_SECTION)};

		RECT rect = {};
		GetWindowRect(hDlg, &rect);
		LONG w = rect.right - rect.left - 15;
		LONG h = rect.bottom - rect.top - 62;

		TCITEM tci;
		tci.mask = TCIF_TEXT;
		for (int i = 0; i < 2; i++) {
			tci.pszText = (PTCHAR)tabNames[i];
			TabCtrl_InsertItem(hTab, i, &tci);
			hSubDlg[i] = CreateDialog(appInstance, (PTCHAR)dlgNames[i], hDlg, (DLGPROC)baseInfoDlgProc);
			SetWindowPos(hSubDlg[i], HWND_TOP, 0, 50, w, h, 0);
			ShowWindow(hSubDlg[i], SW_SHOW);
		}
		*/
	}

	void setDirData() {
		//export, reloc, import, boundImp, iat, delayImp, res, dbg, loadCfg
	}

	void setInfo() {
		PVOID fileBuffer = 0;
		long len = openPE(pFilePath, &fileBuffer);
		if (!len) {
			winLog(_T("open pe error: %s", pFilePath));
			return;
		}
		TCHAR cont[0x10] = {};
		PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
		PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)NT_HEADER(fileBuffer);
		PIMAGE_SECTION_HEADER fstSec = IMAGE_FIRST_SECTION(hNt);

		//---base
		_sntprintf(cont, 0x10, _T("%04X"), hDos->e_lfanew);
		SetDlgItemText(hDlg, IDC_EDIT1, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->Signature);
		SetDlgItemText(hDlg, IDC_EDIT2, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Machine);
		SetDlgItemText(hDlg, IDC_EDIT3, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.NumberOfSections);
		SetDlgItemText(hDlg, IDC_EDIT4, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.SizeOfOptionalHeader);
		SetDlgItemText(hDlg, IDC_EDIT5, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Characteristics);
		SetDlgItemText(hDlg, IDC_EDIT6, cont);

		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Magic);
		SetDlgItemText(hDlg, IDC_EDIT7, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.AddressOfEntryPoint);
		SetDlgItemText(hDlg, IDC_EDIT8, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.ImageBase);
		SetDlgItemText(hDlg, IDC_EDIT9, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SectionAlignment);
		SetDlgItemText(hDlg, IDC_EDIT10, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.FileAlignment);
		SetDlgItemText(hDlg, IDC_EDIT11, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfImage);
		SetDlgItemText(hDlg, IDC_EDIT12, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfHeaders);
		SetDlgItemText(hDlg, IDC_EDIT13, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.CheckSum);
		SetDlgItemText(hDlg, IDC_EDIT14, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Subsystem);
		SetDlgItemText(hDlg, IDC_EDIT15, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.DllCharacteristics);
		SetDlgItemText(hDlg, IDC_EDIT16, cont);

		//IDC_EDIT 17~46
		for (int i = 0; i < 15; i++) {
			int fstEditId = IDC_EDIT17 + i * 2;
			PIMAGE_DATA_DIRECTORY pData = &hNt->OptionalHeader.DataDirectory[i];
			_sntprintf(cont, 0x10, _T("%p"), pData->VirtualAddress);
			SetDlgItemText(hDlg, fstEditId, cont);
			_sntprintf(cont, 0x10, _T("%p"), pData->Size);
			SetDlgItemText(hDlg, fstEditId + 1, cont);
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
		setDirData();

		free(fileBuffer);
	}

	//void onSelChanged_Tab() {
	//	for (int i = 0; i < 2; i++) {
	//		ShowWindow(hSubDlg[i], SW_HIDE);
	//	}
	//	int pageIdx = TabCtrl_GetCurFocus(hTab);
	//	ShowWindow(hSubDlg[pageIdx], SW_SHOW);
	//}

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
			HWND hEC = GetDlgItem(hDlg, IDC_FILE_PATH);
			SetWindowText(hEC, path);
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
				hDlg = aDlg;
				initComp();
				//onSelChanged_Tab();
				return TRUE;
			}
			case WM_CLOSE: {
				EndDialog(aDlg, 0);
				return TRUE;
			}
			/*case WM_NOTIFY: {
				NMHDR* pNM = (NMHDR*)lParam;
				if (wParam == IDC_TAB) {
					if (pNM->code == TCN_SELCHANGE) {
						onSelChanged_Tab();
						return TRUE;
					}
				}
				break;
			}*/
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
