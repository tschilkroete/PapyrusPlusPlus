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

#include "PapyrusLexer.hpp"

#include <cassert>
#include "scintilla\LexAccessor.h"
#include "scintilla\Accessor.h"
#include "scintilla\StyleContext.h"

int SCI_METHOD PapyrusLexer::Version() const {return 0;}
void SCI_METHOD PapyrusLexer::Release() {}
const char * SCI_METHOD PapyrusLexer::PropertyNames() { return ""; }
int SCI_METHOD PapyrusLexer::PropertyType(const char *name) { return 0; }
const char * SCI_METHOD PapyrusLexer::DescribeProperty(const char *name) { return ""; }
int SCI_METHOD PapyrusLexer::PropertySet(const char *key, const char *val) { return 0; }
const char * SCI_METHOD PapyrusLexer::DescribeWordListSets() { return ""; }
int SCI_METHOD PapyrusLexer::WordListSet(int n, const char *wl) { return 0; }

void SCI_METHOD PapyrusLexer::Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {
	Accessor accessor(pAccess, nullptr);
	StyleContext  styleContext(startPos, lengthDoc, initStyle, accessor);
	while (styleContext.More()) {
		switch (styleContext.state) {
		case DEFAULT:
			if (styleContext.ch == ';') {
				styleContext.SetState(COMMENTLINE);
			} else if (styleContext.ch == '{') {
				styleContext.SetState(COMMENT);
			}
			break;
		case COMMENTLINE:
			if (styleContext.atLineEnd) {
				styleContext.SetState(DEFAULT);
			}
			break;
		case COMMENT:
			if (styleContext.chPrev == '}') {
				styleContext.SetState(DEFAULT);
				//The comment ends here, but the current char might already change the state again, so it needs to be relexed.
				continue;
			}
			break;
		}
		styleContext.Forward();
	}
	accessor.Flush();
}

void SCI_METHOD PapyrusLexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {

}

void * SCI_METHOD PapyrusLexer::PrivateCall(int operation, void *pointer) {
	return nullptr;
}