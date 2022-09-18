#include "win32Tool.h"
#include "pistolRes/pistolResource.h"
#include "peDlg.h"
#include "peTool.h"
using namespace std;

namespace peDlg {
	typedef void(*ParseDirData)(HWND hwnd, PIMAGE_NT_HEADERS hNt);
	typedef void(*ParseBotData)(HWND hwnd, DWORD idx);
	HINSTANCE appInstance = NULL;
	HWND hDlgRoot = NULL;
	HWND hTabDir = NULL;
	TCHAR pFilePath[MAX_PATH] = {};
	PVOID fileBuffer = 0;
	HWND hListSec = NULL;
	HWND hDlgDir[15] = {};
	ParseDirData parseFuncs[15] = {};
	ParseBotData botDealFuncs[15] = {};

	int getDirIdx() {
		int tabIdx = TabCtrl_GetCurFocus(hTabDir);
		TCITEM tci = {};
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(hTabDir, tabIdx, &tci);
		return tci.lParam;
	}

	INT_PTR CALLBACK dirDlgProc(HWND aDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
			case WM_NOTIFY: {
				NMHDR* pNM = (NMHDR*)lParam;
				if (wParam == IDC_LV_TOP) {
					NMLISTVIEW* pLV = (NMLISTVIEW*)lParam;
					if (pLV->hdr.code == NM_CLICK) {
						int dirIdx = getDirIdx();
						if (botDealFuncs[dirIdx] != 0 && pLV->iItem >= 0) {
							botDealFuncs[dirIdx](GetDlgItem(hDlgDir[dirIdx], IDC_LV_BOTTOM), pLV->iItem);
						}
						return TRUE;
					}
				}
				break;
			}
			default:
				break;
		}
		return FALSE;
	}

	void initListView(HWND hLV, int* colWidthArr, PCTCH* colNameArr, int colNum) {
		SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
		LV_COLUMN col = {};
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		for (int i = 0; i < colNum; i++) {
			col.pszText = (PTCHAR)colNameArr[i];
			col.cx = colWidthArr[i];
			ListView_InsertColumn(hLV, i, &col);
		}
	}

	WCHAR box[MAX_PATH] = {};
	PWCHAR m2w(PCSTR org) {
		ZeroMemory(box, sizeof(box));
		MultiByteToWideChar(CP_ACP, 0, org, -1, box, MAX_PATH);
		return box;
	}

	//-----export-----
	void parse_export(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
		static HWND hListTop = NULL;
		static HWND hListBottom = NULL;
		if (!initHead) {
			initHead = true;
			hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int topWidth[] = {200, 120, 200};
			PCTCH topName[] = {
				_T("Member"), _T("Value"), _T("Description")
			};
			initListView(hListTop, topWidth, topName, ARRAYSIZE(topWidth));
			hListBottom = GetDlgItem(hwnd, IDC_LV_BOTTOM);
			int botWidth[] = {120, 120, 120, 200};
			PCTCH botName[] = {
				_T("ExportOrdinal"), _T("FuncAddr"), _T("NameRva"), _T("Name")
			};
			initListView(hListBottom, botWidth, botName, ARRAYSIZE(botWidth));
		}

		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		PIMAGE_EXPORT_DIRECTORY pData = (PIMAGE_EXPORT_DIRECTORY)rva2fa(fileBuffer, dataRva);

		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListTop);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		PCWCH nameArr[] = {L"Name", L"Base", L"NumberOfFunctions", L"NumberOfNames", 
			L"AddressOfFunctions", L"AddressOfNames", L"AddressOfNameOrdinals"};
		DWORD valueArr[] = {pData->Name, pData->Base, pData->NumberOfFunctions, pData->NumberOfNames, 
			pData->AddressOfFunctions, pData->AddressOfNameOrdinals, pData->AddressOfNames};
		for (int i = 0, sz = ARRAYSIZE(nameArr); i < sz; i++) {
			item.iItem = i;
			{
				item.pszText = (LPWSTR)nameArr[i];
				item.iSubItem = 0;
				ListView_InsertItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), valueArr[i]);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListTop, &item);

				if (i == 0) {
					PWCHAR wcName = m2w((PCSTR)rva2fa(fileBuffer, pData->Name));
					item.pszText = wcName;
					item.iSubItem = 2;
					ListView_SetItem(hListTop, &item);
				}
			}
		}

		PDWORD funcAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfFunctions);
		PDWORD nameAddr = (PDWORD)rva2fa(fileBuffer, pData->AddressOfNames);
		PWORD nameOrdAddr = (PWORD)rva2fa(fileBuffer, pData->AddressOfNameOrdinals);
		int order = 0;
		for (int i = 0; i < pData->NumberOfFunctions; i++) {
			if (funcAddr[i]) {
				DWORD nameRva = 0;
				for (int j = 0; j < pData->NumberOfNames; j++) {
					if (nameOrdAddr[j] == i) {
						nameRva = nameAddr[j];
						break;
					}
				}

				item.iItem = order++; 
				{
					_sntprintf(cont, 0x10, _T("%p"), i + pData->Base);
					item.pszText = cont;
					item.iSubItem = 0;
					ListView_InsertItem(hListBottom, &item);

					_sntprintf(cont, 0x10, _T("%p"), funcAddr[i]);
					item.pszText = cont;
					item.iSubItem = 1;
					ListView_SetItem(hListBottom, &item);

					_sntprintf(cont, 0x10, _T("%p"), nameRva);
					item.pszText = cont;
					item.iSubItem = 2;
					ListView_SetItem(hListBottom, &item);

					PWCHAR wcName = m2w(nameRva ? (PSTR)rva2fa(fileBuffer, nameRva) : "noName");
					item.pszText = wcName;
					item.iSubItem = 3;
					ListView_SetItem(hListBottom, &item);
				}
			}
		}
	}

	//-----reloc-----
	void parse_reloc(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
		static HWND hListTop = NULL;
		static HWND hListBottom = NULL;
		if (!initHead) {
			initHead = true;
			hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int topWidth[] = {120, 120, 120, 200};
			PCTCH topName[] = {
				_T("VirtualAddress"), _T("SizeOfBlock"), _T("ItemNum"), _T("Section")
			};
			initListView(hListTop, topWidth, topName, ARRAYSIZE(topWidth));
			hListBottom = GetDlgItem(hwnd, IDC_LV_BOTTOM);
			int botWidth[] = {120, 120, 120};
			PCTCH botName[] = {
				_T("Item"), _T("RVA"), _T("Type")
			};
			initListView(hListBottom, botWidth, botName, ARRAYSIZE(botWidth));
		}

		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListTop);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		PIMAGE_BASE_RELOCATION pData = (PIMAGE_BASE_RELOCATION)rva2fa(fileBuffer, dataRva);
		DWORD typeSize = sizeof(IMAGE_BASE_RELOCATION);

		int order = 0;
		while (!isZeroBlock((PVOID)pData, typeSize)) {
			DWORD itemNum = (pData->SizeOfBlock - typeSize) / 2;
			PIMAGE_SECTION_HEADER pSec = getSecByRva(fileBuffer, pData->VirtualAddress);

			item.iItem = order++;
			{
				_sntprintf(cont, 0x10, _T("%p"), pData->VirtualAddress);
				item.pszText = cont;
				item.iSubItem = 0;
				ListView_InsertItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), pData->SizeOfBlock);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%d"), itemNum);
				item.pszText = cont;
				item.iSubItem = 2;
				ListView_SetItem(hListTop, &item);

				PWCHAR wcName = m2w((PSTR)pSec->Name);
				item.pszText = wcName;
				item.iSubItem = 3;
				ListView_SetItem(hListTop, &item);
			}
			pData = (PIMAGE_BASE_RELOCATION)((DWORD)pData + pData->SizeOfBlock);
		}
	}

	void bot_reloc(HWND hListBot, DWORD idx) {
		//qLog("%p %d", hListBot, idx);
		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListBot);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		PIMAGE_BASE_RELOCATION pData = (PIMAGE_BASE_RELOCATION)rva2fa(fileBuffer, dataRva);
		DWORD typeSize = sizeof(IMAGE_BASE_RELOCATION);

		int order = 0;
		while (!isZeroBlock((PVOID)pData, typeSize)) {
			if (order == idx) {
				break;
			}
			order++;
			pData = (PIMAGE_BASE_RELOCATION)((DWORD)pData + pData->SizeOfBlock);
		}
		
		int itemDataNum = (pData->SizeOfBlock - typeSize) / 2;
		PWORD fstItemData = PWORD((DWORD)pData + typeSize);
		for (int i = 0; i < itemDataNum; i++) {
			//4(IMAGE_REL_BASED_HIGHLOW)|12
			WORD itemData = fstItemData[i];
			WORD value = itemData << 4;
			value = value >> 4;
			DWORD rva = pData->VirtualAddress + value;
			DWORD type = itemData >> 12;
			//print("\t[%d] org item: %04x, item value: %04x, rva: %p", i, itemData, value, rva);

			item.iItem = i;
			{
				_sntprintf(cont, 0x10, _T("%04X"), itemData);
				item.pszText = cont;
				item.iSubItem = 0;
				ListView_InsertItem(hListBot, &item);

				_sntprintf(cont, 0x10, _T("%p"), rva);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListBot, &item);

				_sntprintf(cont, 0x10, _T("%d"), type);
				item.pszText = cont;
				item.iSubItem = 2;
				ListView_SetItem(hListBot, &item);
			}
		}
	}

	//-----import-----
	void parse_import(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
		static HWND hListTop = NULL;
		static HWND hListBottom = NULL;
		if (!initHead) {
			initHead = true;
			hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int topWidth[] = {200, 60, 120, 120, 120};
			PCTCH topName[] = {
				_T("ModuleName"), _T("Num"), _T("OFT_RVA"), _T("FT_RVA"), _T("TimeDateStamp")
			};
			initListView(hListTop, topWidth, topName, ARRAYSIZE(topWidth));
			hListBottom = GetDlgItem(hwnd, IDC_LV_BOTTOM);
			int botWidth[] = {300, 120, 120};
			PCTCH botName[] = {
				_T("Name"), _T("OFT"), _T("FT")
			};
			initListView(hListBottom, botWidth, botName, ARRAYSIZE(botWidth));
		}

		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListTop);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		PIMAGE_IMPORT_DESCRIPTOR pData = (PIMAGE_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, dataRva);
		DWORD typeSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);
		
		int order = 0;
		while (!isZeroBlock(pData, typeSize)) {
			PSTR dllName = (PSTR)rva2fa(fileBuffer, pData->Name);
			PDWORD pOFT = (PDWORD)rva2fa(fileBuffer, pData->OriginalFirstThunk);
			PDWORD pFT = (PDWORD)rva2fa(fileBuffer, pData->FirstThunk);
			DWORD funcNum = 0;
			while (*pOFT != 0) {
				pOFT++;
				funcNum++;
			}
			item.iItem = order++;
			{
				item.pszText = m2w(dllName);
				item.iSubItem = 0;
				ListView_InsertItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%d"), funcNum);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), pData->OriginalFirstThunk);
				item.pszText = cont;
				item.iSubItem = 2;
				ListView_SetItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), pData->FirstThunk);
				item.pszText = cont;
				item.iSubItem = 3;
				ListView_SetItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), pData->TimeDateStamp);
				item.pszText = cont;
				item.iSubItem = 4;
				ListView_SetItem(hListTop, &item);
			}
			pData++;
		}
	}

	void bot_import(HWND hListBot, DWORD idx) {
		TCHAR cont[MAX_PATH] = {};
		ListView_DeleteAllItems(hListBot);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		PIMAGE_IMPORT_DESCRIPTOR pData = (PIMAGE_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, dataRva);
		DWORD typeSize = sizeof(IMAGE_IMPORT_DESCRIPTOR);

		pData = pData + idx;
		PDWORD pOFT = (PDWORD)rva2fa(fileBuffer, pData->OriginalFirstThunk);
		PDWORD pFT = (PDWORD)rva2fa(fileBuffer, pData->FirstThunk);
		int order = 0;
		while (*pOFT != 0) {
			if ((*pOFT & 0x80000000) == 0x80000000) {
				DWORD funcOrdinal = *pOFT & 0x7fffffff;
				_sntprintf(cont, MAX_PATH, _T("Ordinal:%p"), funcOrdinal);
			} else {
				PIMAGE_IMPORT_BY_NAME pName = (PIMAGE_IMPORT_BY_NAME)rva2fa(fileBuffer, *pOFT);
				_sntprintf(cont, MAX_PATH, _T("%s"), m2w(pName->Name));
			}

			item.iItem = order++;
			{
				item.pszText = cont;
				item.iSubItem = 0;
				ListView_InsertItem(hListBot, &item);

				_sntprintf(cont, MAX_PATH, _T("%p"), *pOFT);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListBot, &item);

				_sntprintf(cont, MAX_PATH, _T("%p"), *pFT);
				item.pszText = cont;
				item.iSubItem = 2;
				ListView_SetItem(hListBot, &item);
			}

			pOFT++;
			pFT++;
		}
	}

	//-----bound-----
	void parse_bound(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
		static HWND hListTop = NULL;
		if (!initHead) {
			initHead = true;
			hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int topWidth[] = {200, 120, 120};
			PCTCH topName[] = {
				_T("ModuleName"), _T("Name_FOA"), _T("TimeDateStamp")
			};
			initListView(hListTop, topWidth, topName, ARRAYSIZE(topWidth));
		}

		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListTop);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress;
		PIMAGE_BOUND_IMPORT_DESCRIPTOR pData = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)rva2fa(fileBuffer, dataRva);
		DWORD typeSize = sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);
		DWORD fstBound = (DWORD)pData;

		int order = 0;
		while (!isZeroBlock(pData, typeSize)) {
			PSTR name = (PSTR)(fstBound + pData->OffsetModuleName);
			item.iItem = order++;
			{
				item.pszText = m2w(name);
				item.iSubItem = 0;
				ListView_InsertItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), (DWORD)name - (DWORD)fileBuffer);
				item.pszText = cont;
				item.iSubItem = 1;
				ListView_SetItem(hListTop, &item);

				_sntprintf(cont, 0x10, _T("%p"), pData->TimeDateStamp);
				item.pszText = cont;
				item.iSubItem = 2;
				ListView_SetItem(hListTop, &item);
			}
			pData++;
		}
	}

	//-----iat-----
	void parse_iat(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
		static HWND hListTop = NULL;
		static HWND hListBottom = NULL;
		if (!initHead) {
			initHead = true;
			hListTop = GetDlgItem(hwnd, IDC_LV_TOP);
			int topWidth[] = {120};
			PCTCH topName[] = {
				_T("FT_RVA")
			};
			initListView(hListTop, topWidth, topName, ARRAYSIZE(topWidth));
			hListBottom = GetDlgItem(hwnd, IDC_LV_BOTTOM);
			int botWidth[] = {120};
			PCTCH botName[] = {
				_T("FT")
			};
			initListView(hListBottom, botWidth, botName, ARRAYSIZE(botWidth));
		}

		TCHAR cont[0x10] = {};
		ListView_DeleteAllItems(hListTop);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
		PDWORD pFT = (PDWORD)rva2fa(fileBuffer, dataRva);

		int num = 0;
		while (true) {
			if (*pFT == 0 && *(pFT + 1) == 0) {
				break;
			}
			if (*pFT == 0 || num == 0) {
				item.iItem = num;
				{
					DWORD fa = (DWORD)pFT;
					if (num > 0) {
						fa += 4;
					}
					_sntprintf(cont, 0x10, _T("%p"), fa2rva(fileBuffer, fa));
					item.pszText = cont;
					item.iSubItem = 0;
					ListView_InsertItem(hListTop, &item);
				}
				num++;
			}
			pFT++;
		}
	}

	void bot_iat(HWND hListBot, DWORD idx) {
		TCHAR cont[MAX_PATH] = {};
		ListView_DeleteAllItems(hListBot);
		LV_ITEM item = {};
		item.mask = LVIF_TEXT;

		PIMAGE_NT_HEADERS hNt = NT_HEADER(fileBuffer);
		DWORD dataRva = hNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
		PDWORD pFT = (PDWORD)rva2fa(fileBuffer, dataRva);

		int num = 0;
		while (true) {
			if (*pFT == 0 && *(pFT + 1) == 0) {
				break;
			}
			if (*pFT == 0 || num == 0) {
				DWORD fa = (DWORD)pFT;
				if (num > 0) {
					fa += 4;
				}

				int order = 0;
				while (*(PDWORD)fa != 0) {
					item.iItem = order++;
					{
						_sntprintf(cont, 0x10, _T("%p"), *(PDWORD)fa);
						item.pszText = cont;
						item.iSubItem = 0;
						ListView_InsertItem(hListBot, &item);
					}
					fa += 4;
				}
				num++;
			}
			pFT++;
		}
	}

	//-----delay-----
	void parse_delay(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
	}

	//-----res-----
	void parse_res(HWND hwnd, PIMAGE_NT_HEADERS hNt) {
		static BOOL initHead = false;
	}

	void initComp() {
		hListSec = GetDlgItem(hDlgRoot, IDC_LV_SEC);
		int colWidthArr[] = {140, 120, 120, 120, 120, 120};
		PCTCH colNameArr[] = {
			_T("Name"), _T("VirtualSize"), _T("VirtualAddress"), _T("SizeOfRawData"), _T("PointerToRawData"), _T("Characteristics")
		};
		initListView(hListSec, colWidthArr, colNameArr, ARRAYSIZE(colWidthArr));

		hTabDir = GetDlgItem(hDlgRoot, IDC_TAB_DIR);
		
		parseFuncs[IMAGE_DIRECTORY_ENTRY_EXPORT] = parse_export;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_BASERELOC] = parse_reloc;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_IMPORT] = parse_import;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT] = parse_bound;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_IAT] = parse_iat;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT] = parse_delay;
		parseFuncs[IMAGE_DIRECTORY_ENTRY_RESOURCE] = parse_res;

		botDealFuncs[IMAGE_DIRECTORY_ENTRY_BASERELOC] = bot_reloc;
		botDealFuncs[IMAGE_DIRECTORY_ENTRY_IMPORT] = bot_import;
		botDealFuncs[IMAGE_DIRECTORY_ENTRY_IAT] = bot_iat;

		RECT rect = {};
		GetWindowRect(hTabDir, &rect);
		LONG w = rect.right - rect.left - 15;
		LONG h = rect.bottom - rect.top - 62;

		for (int i = 0; i < 15; i++) {
			hDlgDir[i] = CreateDialog(appInstance, MAKEINTRESOURCE(IDD_DIR), hTabDir, (DLGPROC)dirDlgProc);
			SetWindowPos(hDlgDir[i], HWND_TOP, 0, 50, w, h, 0);
		}
	}

	void setDirData() {
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
				if (parseFuncs[i]) {
					parseFuncs[i](hDlgDir[i], NT_HEADER(fileBuffer));
				}
			}
		}
	}

	void setInfo() {
		if (fileBuffer != 0) {
			free(fileBuffer);
			fileBuffer = 0;
		}

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
		setDirData();
	}

	void onSelChanged_DirPage() {
		for (int i = 0, len = ARRAYSIZE(hDlgDir); i < len; i++) {
			ShowWindow(hDlgDir[i], SW_HIDE);
		}
		int idx = getDirIdx();
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
