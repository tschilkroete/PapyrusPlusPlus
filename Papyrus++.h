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

#include "npp\PluginInterface.h"
#include <windows.h>

#define VERSION "0.0"

const WCHAR* PLUGIN_NAME = L"Papyrus++";
const int funcCount = 3;
FuncItem funcs[funcCount];
NppData nppData;


void init();
void cleanUp();

void compile();
void settings();
void about();