/*
This file is part of Papyrus++

Copyright (C) 2016 - 2017 Tschilkroete <tschilkroete@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "windowErrors.hpp"

#include <Commctrl.h>
#include "npp\Notepad_plus_msgs.h"

WindowErrors::WindowErrors(const NppData& nppData, HINSTANCE instance) : DockingDlgInterface(IDD_WINDOWERROR), nppData(nppData) {
	DockingDlgInterface::init(instance, nppData._nppHandle);
	tTbData data = {};
	create(&data);
	data.dlgID = -1;
	data.uMask = DWS_DF_CONT_BOTTOM;
	data.pszModuleName = L"Papyrus++.dll";
	data.pszName = L"Papyrus errors";
	::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));
	display(false);
	_hParent = nppData._nppHandle;
	listView = ::GetDlgItem(getHSelf(), IDC_ERRORLIST);
	ListView_SetExtendedListViewStyle(listView, LVS_EX_FULLROWSELECT);
	LVCOLUMN column = {};
	column.mask = LVCF_WIDTH | LVCF_TEXT;
	column.cx = 180;
	column.pszText = L"File";
	ListView_InsertColumn(listView, 0, &column);
	column.cx = 100;
	column.pszText = L"Message";
	ListView_InsertColumn(listView, 1, &column);
	column.cx = 45;
	column.pszText = L"Ln";
	ListView_InsertColumn(listView, 2, &column);
	column.cx = 45;
	column.pszText = L"Col";
	ListView_InsertColumn(listView, 3, &column);
	resize();
}

void WindowErrors::show(std::vector<Error> errors) {
	this->errors = errors;
	for (unsigned int i = 0; i < errors.size(); i++) {
		LVITEM item = {};
		item.mask = LVIF_TEXT;
		item.iItem = i;
		std::wstring fileShort = errors[i].file.substr(errors[i].file.find_last_of(L"/\\") + 1);
		item.pszText = &fileShort[0];
		ListView_InsertItem(listView, &item);
		item.iSubItem = 1;
		item.pszText = &errors[i].message[0];
		ListView_SetItem(listView, &item);
		item.iSubItem = 2;
		std::wstring line = std::to_wstring(errors[i].line);
		item.pszText = &line[0];
		ListView_SetItem(listView, &item);
		item.iSubItem = 3;
		std::wstring column = std::to_wstring(errors[i].column);
		item.pszText = &column[0];
		ListView_SetItem(listView, &item);
	}
	DockingDlgInterface::display();
}

INT_PTR CALLBACK WindowErrors::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SIZE: {
		resize();
		return 0;
	} case WM_NOTIFY:{
		NMITEMACTIVATE* item = (NMITEMACTIVATE*)lParam;
		if (item->hdr.hwndFrom == listView && item->hdr.code == NM_DBLCLK) {
			if (item->iItem != -1) {
				Error error = errors[item->iItem];
				::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(&error.file[0]));
				int scintilla;
				::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&scintilla));
				::SendMessage(scintilla ? nppData._scintillaSecondHandle : nppData._scintillaMainHandle, SCI_GOTOLINE, error.line - 1, 0);
			}
			return true;
		} else
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}default:
		return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}

void WindowErrors::resize() {
	RECT windowSize;
	::GetClientRect(getHSelf(), &windowSize);
	::SetWindowPos(listView, HWND_TOP, 2, 2, windowSize.right - windowSize.left - 4, windowSize.bottom - windowSize.top - 2, 0);
	int width = ListView_GetColumnWidth(listView, 0) + ListView_GetColumnWidth(listView, 2) + ListView_GetColumnWidth(listView, 3) + 8;
	ListView_SetColumnWidth(listView, 1, windowSize.right - windowSize.left - width);
}

void WindowErrors::clear() {
	ListView_DeleteAllItems(listView);
	errors.clear();
}
