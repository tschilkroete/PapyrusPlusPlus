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

#include <thread>

class CompilationThread
{
public:
	CompilationThread(HWND window, const Settings& settings);
	void start(std::wstring inputFile);
private:
	static void run(CompilationThread& compilationThread, std::wstring inputFile);
	std::thread compilationThread;
	HWND window;
	const Settings& settings;
};