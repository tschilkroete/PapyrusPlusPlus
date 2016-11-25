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

#include "settings.hpp"

#include <Windows.h>

#include <codecvt>
#include <fstream>

Settings::Settings() {
}

bool Settings::load(std::wstring configDir) {
	settingsPath = configDir + L"\\papyrus++.ini";

	std::wifstream settingsFile(settingsPath);
	settingsFile.imbue(std::locale(settingsFile.getloc(), new std::codecvt_utf8<wchar_t>()));
	std::wstring line;
	while (std::getline(settingsFile, line)) {
		int equalsIndex = line.find_first_of(L'=');
		std::wstring key = line.substr(0, equalsIndex);
		std::wstring value;
		if(equalsIndex != line.length() - 1)
			value = line.substr(equalsIndex + 1);
		data.push_back(std::pair<std::wstring, std::wstring>(key, value));
	}
	settingsFile.close();
	if (data.size())
		return true;
	else
		return false;
}

void Settings::save() {
	std::wofstream settingsFile(settingsPath, std::wofstream::trunc);
	settingsFile.imbue(std::locale(settingsFile.getloc(), new std::codecvt_utf8<wchar_t>()));
	for (std::pair<std::wstring, std::wstring> p : data) {
		settingsFile << p.first << L'=' << p.second << std::endl;
	}
	settingsFile.close();
}

std::wstring Settings::getString(std::wstring key, std::wstring defaultValue) const {
	for (std::pair<std::wstring, std::wstring> p : data) {
		if (p.first == key) {
			return p.second;
		}
	}
	return defaultValue;
}

void Settings::putString(std::wstring key, std::wstring value) {
	for (std::pair<std::wstring, std::wstring>& p : data) {
		if (p.first == key) {
			p.second = value;
			return;
		}
	}
	data.push_back(std::pair<std::wstring, std::wstring>(key, value));
}
