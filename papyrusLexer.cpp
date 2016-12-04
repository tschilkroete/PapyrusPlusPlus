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
	case FLOWCONTROL:
		wordList = &wordListFlowControl;
		break;
	case KEYWORDS:
		wordList = &wordListKeywords;
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

void SCI_METHOD PapyrusLexer::Lex(unsigned int startPos, int lengthDoc, int stateInit, IDocument *pAccess) {
	Accessor accessor(pAccess, nullptr);
	StyleContext  styleContext(startPos, lengthDoc, stateInit, accessor);
	while (styleContext.More()) {
		styleContext.SetState(DEFAULT);

		if (styleComment(styleContext, "{", "}", COMMENTDOC, stateInit)) continue;
		if (styleComment(styleContext, ";/", "/;", COMMENTMULTILINE, stateInit)) continue;

		if (styleContext.ch == ';') {
			styleContext.SetState(COMMENT);
			while (!styleContext.atLineEnd) styleContext.Forward();
			styleContext.SetState(DEFAULT);
		}
		styleWordList(styleContext, wordListTypes, TYPE);
		styleWordList(styleContext, wordListFlowControl, FLOWCONTROL);
		styleWordList(styleContext, wordListKeywords, KEYWORDS);
		styleContext.Forward();
	}
	accessor.Flush();
}

void SCI_METHOD PapyrusLexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess) {

}

void * SCI_METHOD PapyrusLexer::PrivateCall(int operation, void *pointer) {
	return nullptr;
}

void PapyrusLexer::styleWordList(StyleContext& styleContext, const WordList& wordList, State state) {
	if (!isalpha(styleContext.chPrev) && styleContext.chPrev != '_' && isalpha(styleContext.ch)) {
		for (int i = 0; i < wordList.Length(); i++) {
			int length = strlen(wordList.WordAt(i));
			if (styleContext.MatchIgnoreCase(wordList.WordAt(i)) && !isalpha(styleContext.GetRelative(length)) && styleContext.GetRelative(length) != '_') {
				styleContext.SetState(state);
				styleContext.Forward(length);
				styleContext.SetState(DEFAULT);
				break;
			}
		}
	}
}

bool PapyrusLexer::styleComment(StyleContext & styleContext, const char * start, const char * end, State stateComment, int& stateInit) {
	if (styleContext.Match(start) || stateInit == stateComment) {
		stateInit = DEFAULT;
		styleContext.SetState(stateComment);
		styleContext.Forward(strlen(start));
		while (styleContext.More() && !styleContext.Match(end)) {
			styleContext.Forward();
		}
		styleContext.Forward(strlen(end));
		styleContext.SetState(DEFAULT);
		return true;
	}
	return false;
}
