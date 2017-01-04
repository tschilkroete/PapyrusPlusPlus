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

#include "windowSettings.hpp"

#include <vector>

const wchar_t* SETTINGS_CLASS_NAME = L"Papyrus++ settings";

WindowSettings::WindowSettings(Settings& settings, HINSTANCE instance, HWND parent) : settings(settings), instance(instance) {
	WNDCLASS windowClass = {};
	windowClass.hInstance = instance;
	windowClass.lpfnWndProc = windowProcedure;
	windowClass.lpszClassName = SETTINGS_CLASS_NAME;
	::RegisterClass(&windowClass);

	window = ::CreateWindow(SETTINGS_CLASS_NAME, SETTINGS_CLASS_NAME, WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 855, 250, parent, nullptr, instance, nullptr);
	::SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
	compilerPath = createTextEdit(settings.getString(L"compilerPath").c_str(), 0, 160, 10, 680, 20);
	std::wstring imports = settings.getString(L"importDirectories");
	size_t index = 0;
	while ((index = imports.find(L";", index)) != std::wstring::npos) {
		imports.replace(index, 1, L"\r\n");
	}
	importDirectories = createTextEdit(imports.c_str(), ES_AUTOVSCROLL | ES_MULTILINE, 160, 40, 680, 80);
	outputDirectory = createTextEdit(settings.getString(L"outputDirectory").c_str(), 0, 160, 130, 680, 20);
	flagFile = createTextEdit(settings.getString(L"flagFile").c_str(), 0, 160, 160, 680, 20);
	additionalArguments = createTextEdit(settings.getString(L"additionalArguments").c_str(), 0, 160, 190, 680, 20);
	::ShowWindow(window, SW_SHOWNORMAL);

	MSG msg;
	while (::GetMessage(&msg, window, 0, 0) > 0) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void WindowSettings::save() {
	settings.putString(L"compilerPath", getText(compilerPath));
	std::wstring imports = getText(importDirectories);
	size_t index = 0;
	while ((index = imports.find(L"\r\n", index)) != std::wstring::npos) {
		imports.replace(index, 2, L";");
	}
	settings.putString(L"importDirectories", imports);
	settings.putString(L"outputDirectory", getText(outputDirectory));
	settings.putString(L"flagFile", getText(flagFile));
	settings.putString(L"additionalArguments", getText(additionalArguments));
	settings.save();
}

std::wstring WindowSettings::getText(HWND edit) {
	size_t length = ::GetWindowTextLength(edit);
	std::wstring content(length + 1, L' ');
	::GetWindowText(edit, &content[0], (int)content.size());
	content.pop_back();//Remove \0
	return content;
}

LRESULT WindowSettings::windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE: {
		((WindowSettings*)::GetWindowLongPtr(window, GWLP_USERDATA))->save();

		DestroyWindow(window);
		return 0;
	}case WM_PAINT:{
		PAINTSTRUCT paintstruct;
		HDC hdc = ::BeginPaint(window, &paintstruct);

		::TextOut(hdc, 10, 12, L"Compiler path:", 14);
		::TextOut(hdc, 10, 42, L"Import directories:", 19);
		::TextOut(hdc, 20, 60, L"(One per line)", 14);
		::TextOut(hdc, 10, 132, L"Output directory:", 17);
		::TextOut(hdc, 10, 162, L"Flag file:", 10);
		::TextOut(hdc, 10, 192, L"Additional arguments:", 21);

		::EndPaint(window, &paintstruct);
		return 0;
	}default:
		return DefWindowProc(window, message, wParam, lParam);
	}
}

HWND WindowSettings::createTextEdit(const wchar_t* text, DWORD style, int x, int y, int width, int height) {
	return ::CreateWindow(L"EDIT", text, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | style, x, y, width, height, window, nullptr, instance, nullptr);
}
