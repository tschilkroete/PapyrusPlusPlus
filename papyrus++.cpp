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

#pragma once

#include "errorWindow.hpp"
#include "messages.hpp"
#include "papyrusLexer.hpp"
#include "papyrus++.hpp"
#include "plbridge.hpp"
#include "settingsWindow.hpp"

#include "scintilla\LexerModule.h"

#include <string>
#include <sstream>

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		::instance = instance;
	} else if (reason == DLL_PROCESS_DETACH) {
		cleanUp();
	}
	return TRUE;
}

const TCHAR* getName() {
	return PLUGIN_NAME;
}

BOOL isUnicode() {
	return true;
}

void setInfo(NppData nppData) {
	::nppData = nppData;
	init();
}

FuncItem* getFuncsArray(int* count) {
	*count = funcCount;
	return funcs;
}

void beNotified(SCNotification* notification) {

}

LRESULT messageProc(UINT message, WPARAM wParam, LPARAM lParam) {
	return TRUE;
}
 
int SCI_METHOD GetLexerCount() {
	return 1;
}

void SCI_METHOD GetLexerName(int i, char* name, int length) {
	strcpy_s(name, length, "Papyrus");
}

void SCI_METHOD GetLexerStatusText(int i, wchar_t* text, int length) {
	wcscpy_s(text, length, L"Papyrus Script");
}

LexerFactoryFunction SCI_METHOD GetLexerFactory(int index) {
	return PapyrusLexer::lexerFactory;
}

void init() {
	WNDCLASS messageHandleClass = {};
	messageHandleClass.hInstance = instance;
	messageHandleClass.lpfnWndProc = messageHandleProc;
	messageHandleClass.lpszClassName = L"MESSAGE_WINDOW";
	::RegisterClass(&messageHandleClass);
	messageHandle = ::CreateWindow(L"MESSAGE_WINDOW", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);

	funcs[0] = FuncItem{ L"Compile", compile, 0, false, new ShortcutKey{true, false, true, 0x43} };
	funcs[1] = FuncItem{ L"Settings", settingsWindow, 1, false, nullptr };
	funcs[2] = FuncItem{ L"About", about, 2, false, nullptr };

	wchar_t settingsPath[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)settingsPath);
	if (!settings.load(std::wstring(settingsPath))) {
		::MessageBox(nppData._nppHandle, L"Confirm the compiler settings.\nYou can change them later if you want in Plugins > Papyrus++ > Settings", L"Papyrus++ setup", MB_OK);
		
		DWORD size;
		::RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\bethesda softworks\\skyrim", L"installed path", RRF_RT_ANY, nullptr, nullptr, &size);
		std::vector<wchar_t> skyrimPathC(size / sizeof(wchar_t));
		::RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\bethesda softworks\\skyrim", L"installed path", RRF_RT_ANY, nullptr, &skyrimPathC[0], &size);
		std::wstring skyrimPath(&skyrimPathC[0]);
		settings.putString(L"compilerPath", skyrimPath + L"Papyrus Compiler\\PapyrusCompiler.exe");
		settings.putString(L"importDirectories", skyrimPath + L"Data\\Scripts\\Source");
		settings.putString(L"outputDirectory", skyrimPath + L"Data\\Scripts");
		settings.putString(L"flagFile", L"TESV_Papyrus_Flags.flg");
		settings.putString(L"additionalArguments", L"");
		settings.save();

		settingsWindow();
	} else {
		updateBridgeData();
	}
}

void cleanUp() {
}

LRESULT CALLBACK messageHandleProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
		case PPPM_COMPILATIONDONE: {
			::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compilation successful"));
			return 0;
		}case PPPM_COMPILATIONFAILED: {
			static ErrorWindow errorWindow(nppData, instance);
			errorWindow.clear();
			std::wstring output = reinterpret_cast<wchar_t*>(wParam);
			std::vector<Error> errors;
			while (!output.empty()) {
				std::wstring line = output.substr(0, output.find_first_of(L"\r\n"));
				output.erase(0, line.size() + 2);
				Error error;
				int bracketStartIndex = line.find_first_of(L'(');
				error.line = std::stoi(line.substr(bracketStartIndex + 1, line.find_first_of(L',') - (bracketStartIndex + 1)));
				int commaIndex = line.find_first_of(L',');
				error.column = std::stoi(line.substr(commaIndex + 1, line.find_first_of(L')') - (commaIndex - 1)));
				error.message = line.substr(line.find_first_of(L')') + 3);
				errors.push_back(error);
			}
			errorWindow.show(errors);
			::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compilation failed"));
			return 0;
		}case PPPM_COMPILERNOTFOUND: {
			::MessageBox(nppData._nppHandle, L"Can't find the compiler executable", L"Papyrus error", MB_OK);
			return 0;
		}default:
			return DefWindowProc(window, message, wParam, lParam);
	}
}

void updateBridgeData() {
	plbridge.includes.clear();
	std::wstringstream stream(settings.getString(L"importDirectories"));
	std::wstring path;
	while (std::getline(stream, path, L';')) {
		plbridge.includes.push_back(path);
	}
}

void compile() {
	::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(L"Compiling..."));
	wchar_t inputFile[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)inputFile);

	static CompilationThread compilationThread(messageHandle, settings);
	compilationThread.start(std::wstring(inputFile));
}

void settingsWindow() {
	SettingsWindow(settings, instance, nppData._nppHandle);
	updateBridgeData();
}

void about() {
	::MessageBox(nullptr, L"Copyright 2016\r\n"
	                       "Contributors:\r\n"
	                       "Tschilkroete\r\n"
	                       "\r\n"
	                       "This plugin is licenced under the GNU General Public Licence 3 https://www.gnu.org/licenses/gpl-3.0.txt \r\n"
	                       "Get the source code: https://github.com/tschilkroete/PapyrusPlusPlus \r\n"
	                       "\r\n"
	                       "This plugin uses parts of notepad++ licenced under the GNU General Public Licence 2. See ? -> About Notepad++ for more information."
	                       "\r\n"
	                       "This plugin uses the scintilla library:\r\n"
	                       "Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>\r\n"
	                       "All Rights Reserved", L"Papyrus++ " VERSION, MB_OK);
}