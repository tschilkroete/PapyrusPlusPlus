/*
This file is part of Papyrus++

Copyright (C) 2016 Tschilkroete <tschilkroete@gmail.com>

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

#include "settingsWindow.hpp"

#include <vector>

const wchar_t* SETTINGS_CLASS_NAME = L"Papyrus++ settings";

SettingsWindow::SettingsWindow(Settings& settings, HINSTANCE instance, HWND parent) : settings(settings) {
	WNDCLASS windowClass = {};
	windowClass.hInstance = instance;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.lpszClassName = SETTINGS_CLASS_NAME;
	::RegisterClass(&windowClass);

	HWND window = ::CreateWindow(SETTINGS_CLASS_NAME, SETTINGS_CLASS_NAME, WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 555, 160, parent, nullptr, nullptr, nullptr);
	::SetWindowLong(window, GWLP_USERDATA, (LONG)this);
	compilerPath = ::CreateWindow(L"EDIT", settings.getString(L"compilerPath").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 160, 10, 380, 20, window, nullptr, nullptr, nullptr);
	importDirectories = ::CreateWindow(L"EDIT", settings.getString(L"importDirectories").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 160, 40, 380, 20, window, nullptr, nullptr, nullptr);
	exportDirectory = ::CreateWindow(L"EDIT", settings.getString(L"exportDirectory").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 160, 70, 380, 20, window, nullptr, nullptr, nullptr);
	additionalArguments = ::CreateWindow(L"EDIT", settings.getString(L"additionalArguments").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 160, 100, 380, 20, window, nullptr, nullptr, nullptr);
	::ShowWindow(window, SW_SHOWNORMAL);

	MSG msg;
	while (::GetMessage(&msg, window, 0, 0) > 0) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void SettingsWindow::save() {
	settings.putString(L"compilerPath", getText(compilerPath));
	settings.putString(L"importDirectories", getText(importDirectories));
	settings.putString(L"exportDirectory", getText(exportDirectory));
	settings.putString(L"additionalArguments", getText(additionalArguments));
	settings.save();
}

std::wstring SettingsWindow::getText(HWND edit) {
	int length = ::SendMessage(edit, EM_LINELENGTH, 0, 0);
	std::vector<wchar_t> content(length);
	content[0] = length;
	::SendMessage(edit, EM_GETLINE, 0, (LPARAM)&content[0]);
	return std::wstring(&content[0], length);
}

LRESULT SettingsWindow::windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE: {
		((SettingsWindow*)::GetWindowLong(window, GWLP_USERDATA))->save();

		DestroyWindow(window);
		return 0;
	}case WM_PAINT:{
		PAINTSTRUCT paintstruct;
		HDC hdc = ::BeginPaint(window, &paintstruct);

		::TextOut(hdc, 10, 12, L"Compiler path:", 14);
		::TextOut(hdc, 10, 42, L"Import directories:", 19);
		::TextOut(hdc, 10, 72, L"Export directory:", 19);
		::TextOut(hdc, 10, 102, L"Additional arguments:", 21);

		::EndPaint(window, &paintstruct);
		return 0;
	}default:
		return DefWindowProc(window, message, wParam, lParam);
	}
}
