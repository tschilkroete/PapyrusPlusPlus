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

!include "MUI2.nsh"

Name "Papyrus++"
OutFile "bin\Papyrus++_2.0.0_Installer.exe"
InstallDirRegKey HKLM "SOFTWARE\Notepad++" ""

RequestExecutionLevel admin

!define MUI_DIRECTORYPAGE_TEXT_TOP "Make sure that the path points to your Notepad++ installation"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section ""
	SetOutPath "$InstDir\plugins"
	File "bin\Papyrus++.dll"
	SetOutPath "$APPDATA\Roaming\Notepad++\plugins\config"
	File "..\Papyrus++.xml"
SectionEnd
