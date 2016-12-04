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

#include <locale>

int SCI_METHOD PapyrusLexer::Version() const {return 0;}
void SCI_METHOD PapyrusLexer::Release() { delete this; }
const char * SCI_METHOD PapyrusLexer::PropertyNames() { return ""; }
int SCI_METHOD PapyrusLexer::PropertyType(const char *name) { return 0; }
const char * SCI_METHOD PapyrusLexer::DescribeProperty(const char *name) { return ""; }
int SCI_METHOD PapyrusLexer::PropertySet(const char *key, const char *val) { return -1; }
const char * SCI_METHOD PapyrusLexer::DescribeWordListSets() { return ""; }

int SCI_METHOD PapyrusLexer::WordListSet(int n, const char *wl) {
	WordList* wordList = nullptr;
	switch (n) {
	case TYPE:
		wordList = &wordListTypes;
		break;
	}
	if (wordList) {
		WordList newList;
		newList.Set(wl);
		if (newList != *wordList) {
			wordList->Set(wl);
			return 0;
		}
	}
	return -1;
}

void SCI_METHOD PapyrusLexer::Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {
	Accessor accessor(pAccess, nullptr);
	StyleContext  styleContext(startPos, lengthDoc, initStyle, accessor);
	while (styleContext.More()) {
		styleContext.SetState(DEFAULT);
		if (styleContext.ch == '{' || initStyle == COMMENT) {
			initStyle = DEFAULT;
			styleContext.SetState(COMMENT);
			while (styleContext.More() && styleContext.chPrev != '}') styleContext.Forward();
			styleContext.SetState(DEFAULT);
			continue;
		}
		if (styleContext.ch == ';') {
			styleContext.SetState(COMMENT);
			while (!styleContext.atLineEnd) styleContext.Forward();
			styleContext.SetState(DEFAULT);
		}
		if (!isalpha(styleContext.chPrev) && isalpha(styleContext.ch)) {
			for (int i = 0; i < wordListTypes.Length(); i++) {
				int length = strlen(wordListTypes.WordAt(i));
				if (styleContext.MatchIgnoreCase(wordListTypes.WordAt(i)) && !isalpha(styleContext.GetRelative(length))) {
					styleContext.SetState(TYPE);
					styleContext.Forward(length);
					styleContext.SetState(DEFAULT);
					break;
				}
			}
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
