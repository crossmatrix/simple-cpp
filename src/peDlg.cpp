#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include "peDlg.h"
#include "peTool.h"
using namespace std;

namespace peDlg {
	HINSTANCE appInstance;
	HWND hDlg;
	HWND hTab;
	HWND hSubDlg[2];
	PCTCH pFilePath = 0;

	INT_PTR CALLBACK baseInfoDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
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

	void initTab() {
		hTab = GetDlgItem(hDlg, IDC_TAB);
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
		}
		ShowWindow(hSubDlg[0], SW_SHOW);
	}

	void setBaseInfo() {
		PVOID fileBuffer = 0;
		long len = openPE(pFilePath, &fileBuffer);
		PIMAGE_DOS_HEADER hDos = (PIMAGE_DOS_HEADER)fileBuffer;
		PIMAGE_NT_HEADERS hNt = (PIMAGE_NT_HEADERS)NT_HEADER(fileBuffer);

		HWND dlg = hSubDlg[0];
		TCHAR cont[0x10] = {};

		_sntprintf(cont, 0x10, _T("%04X"), hDos->e_lfanew);
		SetDlgItemText(dlg, IDC_EDIT1, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->Signature);
		SetDlgItemText(dlg, IDC_EDIT2, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Machine);
		SetDlgItemText(dlg, IDC_EDIT3, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.NumberOfSections);
		SetDlgItemText(dlg, IDC_EDIT4, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.SizeOfOptionalHeader);
		SetDlgItemText(dlg, IDC_EDIT5, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->FileHeader.Characteristics);
		SetDlgItemText(dlg, IDC_EDIT6, cont);

		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Magic);
		SetDlgItemText(dlg, IDC_EDIT7, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.AddressOfEntryPoint);
		SetDlgItemText(dlg, IDC_EDIT8, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.ImageBase);
		SetDlgItemText(dlg, IDC_EDIT9, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SectionAlignment);
		SetDlgItemText(dlg, IDC_EDIT10, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.FileAlignment);
		SetDlgItemText(dlg, IDC_EDIT11, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfImage);
		SetDlgItemText(dlg, IDC_EDIT12, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.SizeOfHeaders);
		SetDlgItemText(dlg, IDC_EDIT13, cont);
		_sntprintf(cont, 0x10, _T("%p"), hNt->OptionalHeader.CheckSum);
		SetDlgItemText(dlg, IDC_EDIT14, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.Subsystem);
		SetDlgItemText(dlg, IDC_EDIT15, cont);
		_sntprintf(cont, 0x10, _T("%04X"), hNt->OptionalHeader.DllCharacteristics);
		SetDlgItemText(dlg, IDC_EDIT16, cont);

		//IDC_EDIT 17~46
		for (int i = 0; i < 15; i++) {
			int fstEditId = IDC_EDIT17 + i * 2;
			PIMAGE_DATA_DIRECTORY pData = &hNt->OptionalHeader.DataDirectory[i];
			_sntprintf(cont, 0x10, _T("%p"), pData->VirtualAddress);
			SetDlgItemText(dlg, fstEditId, cont);
			_sntprintf(cont, 0x10, _T("%p"), pData->Size);
			SetDlgItemText(dlg, fstEditId + 1, cont);
		}
	}

	void setSections() {

	}

	void onSelChanged_Tab() {
		if (pFilePath == 0) {
			return;
		}

		int pageIdx = TabCtrl_GetCurFocus(hTab);
		if (pageIdx == 0) {
			setBaseInfo();
		} else if (pageIdx == 1) {
			setSections();
		}
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
			HWND hEC = GetDlgItem(hDlg, IDC_FILE_PATH);
			SetWindowText(hEC, path);
			pFilePath = path;

			int pageIdx = TabCtrl_GetCurFocus(hTab);
			if (pageIdx == 0) {
				onSelChanged_Tab();
			} else {
				TabCtrl_SetCurFocus(hTab, 0);
			}
		}
	}

	INT_PTR CALLBACK peDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_INITDIALOG: {
				hDlg = aDlg;
				initTab();
				return TRUE;
			}
			case WM_CLOSE: {
				EndDialog(hDlg, 0);
				return TRUE;
			}
			case WM_NOTIFY: {
				NMHDR* pNM = (NMHDR*)lParam;
				if (wParam == IDC_TAB) {
					if (pNM->code == TCN_SELCHANGE) {
						onSelChanged_Tab();
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
