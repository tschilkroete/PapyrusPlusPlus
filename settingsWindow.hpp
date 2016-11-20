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

#include "settings.hpp"

#include <Windows.h>

#include <string>

class SettingsWindow
{
public:
	SettingsWindow(Settings& settings, HINSTANCE instance,HWND parent);
private:
	void save();
	std::wstring getText(HWND edit);
	static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
	Settings& settings;
	HWND compilerPath;
	HWND importDirectories;
	HWND outputDirectory;
	HWND additionalArguments;
};