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

#include "scintilla\Scintilla.h"

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
	case KEYWORD:
		wordList = &wordListKeywords;
		break;
	case OPERATOR:
		wordList = &wordListOperators;
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

void SCI_METHOD PapyrusLexer::Lex(unsigned int startPos, int lengthDoc, int stateInit, IDocument *idocument) {
	Accessor accessor(idocument, nullptr);
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

		if (styleContext.ch == '"') {
			styleContext.SetState(STRING);
			styleContext.Forward();
			while (!styleContext.atLineEnd && styleContext.ch != '"') styleContext.Forward();
			styleContext.ForwardSetState(DEFAULT);
			continue;
		}

		if (isdigit(styleContext.ch) && !isalnum(styleContext.chPrev) && styleContext.chPrev != '_') {
			styleContext.SetState(NUMBER);
			do {
				styleContext.Forward();
			} while (isdigit(styleContext.ch) || styleContext.ch == '.');
			styleContext.SetState(DEFAULT);
			continue;
		}

		styleWordList(styleContext, wordListTypes, TYPE);
		styleWordList(styleContext, wordListFlowControl, FLOWCONTROL);
		styleWordList(styleContext, wordListKeywords, KEYWORD);
		bool found = false;
		for (int i = 0; i < wordListOperators.Length(); i++) {
			if (styleContext.Match(wordListOperators.WordAt(i))) {
				styleContext.SetState(OPERATOR);
				styleContext.Forward(strlen(wordListOperators.WordAt(i)));
				found = true;
				break;
			}
		}
		if (found)
			continue;

		styleContext.Forward();
	}
	accessor.Flush();
}

void SCI_METHOD PapyrusLexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *idocument) {
	Accessor accessor(idocument, nullptr);
	int levelPrev = accessor.LevelAt(accessor.GetLine(startPos)) & SC_FOLDLEVELNUMBERMASK;
	//Lines
	for (int line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc); line++) {
		int levelDelta = 0;
		//Chars
		for (int charIndex = accessor.LineStart(line); charIndex <= accessor.LineEnd(line); charIndex++) {
			char style = accessor.StyleAt(charIndex);
			if (style != COMMENT && style != COMMENTDOC && style != COMMENTMULTILINE
				&& !isalnum(accessor.SafeGetCharAt(charIndex - 1)) && accessor.SafeGetCharAt(charIndex - 1) != '_') {
				for (const char* start : foldStarts) {
					if (accessor.Match(charIndex, start)) {
						levelDelta++;
						break;
					}
				}
				if (levelDelta == 0) {
					for (const char* end : foldEnds) {
						if (accessor.Match(charIndex, end)) {
							levelDelta--;
							break;
						}
					}
				}
				if (levelDelta != 0)
					break;
			}
		}
		int level = levelPrev;
		if (levelDelta == 1) {
			level |= SC_FOLDLEVELHEADERFLAG;
		}
		accessor.SetLevel(line, level);
		levelPrev += levelDelta;
	}
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
		return true;
	}
	return false;
}
