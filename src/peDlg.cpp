#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include "peDlg.h"
using namespace std;

namespace peDlg {
	HINSTANCE appInstance;
	HWND hDlg;
	HWND hTab;
	HWND hSubDlg[2];

	INT_PTR CALLBACK baseInfoDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return FALSE;
	}

	void initTab() {
		hTab = GetDlgItem(hDlg, IDC_TAB);

		PCTCH tabNames[] = {_T("Tab1"), _T("Tab2")};
		PCTCH dlgNames[] = {MAKEINTRESOURCE(IDD_BASE_INFO), MAKEINTRESOURCE(IDD_SECTION)};

		TCITEM tie;
		tie.mask = TCIF_TEXT;
		for (int i = 0; i < 2; i++) {
			tie.pszText = (PTCHAR)tabNames[i];
			TabCtrl_InsertItem(hTab, i, &tie);
			hSubDlg[i] = CreateDialog(appInstance, (PTCHAR)dlgNames[i], hDlg, (DLGPROC)baseInfoDlgProc);
			SetWindowPos(hSubDlg[i], HWND_TOP, 10, 30, 636, 366, 0);
		}
		ShowWindow(hSubDlg[0], SW_SHOW);
	}

	INT_PTR CALLBACK peDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_INITDIALOG: {
				hDlg = aDlg;
				initTab();
				break;
			}
			case WM_CLOSE: {
				EndDialog(hDlg, 0);
				break;
			}
			case WM_NOTIFY: {
				/*NMHDR* pNMHDR = (NMHDR*)lParam;
				if (wParam == IDC_LV_PROC) {
					if (pNMHDR->code == NM_CLICK) {
						refreshMod(pNMHDR->hwndFrom);
					} else if (pNMHDR->code == LVN_COLUMNCLICK) {
						NMLISTVIEW* pNMLV = (NMLISTVIEW*)lParam;
						sortProc(pNMLV->iSubItem);
					}
				}*/
				break;
			}
			case WM_COMMAND: {
				/*switch (wParam) {
					case IDC_BTN_REFRESH:
						onClick_Refresh();
						break;
					case IDC_BTN_PE:
						onClick_PE();
						break;
					default:
						return FALSE;
				}*/
			}
			default: {
				return FALSE;
			}
		}
		return TRUE;
	}

	void showPEDlg(HINSTANCE instance) {
		appInstance = instance;
		DialogBox(appInstance, MAKEINTRESOURCE(IDD_PE), NULL, peDlgProc);
	}
}
