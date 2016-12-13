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

#include <cassert>
#include "scintilla\ILexer.h"
#include "scintilla\LexAccessor.h"
#include "scintilla\PropSetSimple.h"
#include "scintilla\StyleContext.h"
#include "scintilla\WordList.h"
#include "scintilla\Accessor.h"

#include <list>
#include <vector>

class PapyrusLexer : public ILexer
{
public:
	static ILexer* lexerFactory() {
		return new PapyrusLexer();
	}
	virtual int SCI_METHOD Version() const;
	virtual void SCI_METHOD Release();
	virtual const char* SCI_METHOD PropertyNames();
	virtual int SCI_METHOD PropertyType(const char* name);
	virtual const char* SCI_METHOD DescribeProperty(const char* name);
	virtual int SCI_METHOD PropertySet(const char* key, const char* val);
	virtual const char* SCI_METHOD DescribeWordListSets();
	virtual int SCI_METHOD WordListSet(int n, const char* wl);
	virtual void SCI_METHOD Lex(unsigned int startPos, int lengthDoc, int stateInit, IDocument* idocument);
	virtual void SCI_METHOD Fold(unsigned int startPos, int lengthDoc, int stateInit, IDocument* idocument);
	virtual void * SCI_METHOD PrivateCall(int operation, void* pointer);
private:
	enum State {
		DEFAULT,
		TYPE,
		FLOWCONTROL,
		FOLDSTART,
		FOLDEND,
		KEYWORD,
		OPERATOR,
		COMMENT,
		COMMENTDOC,
		COMMENTMULTILINE,
		NUMBER,
		STRING,
		PROPERTY
	};
	static constexpr const char* foldStarts[] = {"if", "while", "function", "event"};
	static constexpr const char* foldEnds[] = {"endif", "endwhile", "endfunction", "endevent"};
	WordList wordListTypes;
	WordList wordListFlowControl;
	WordList wordListKeywords;
	WordList wordListOperators;
	WordList wordListProperties;
	WordList wordListFoldStart;
	WordList wordListFoldEnd;
	struct Property {
		std::string name;
		int line;
	};
	std::list<Property> propertyLines;
	enum TokenType {
		IDENTIFIER,
		NUMERIC,
		SPECIAL
	};
	struct Token {
		std::string content;
		TokenType tokenType;
		unsigned int startPos;
	};
	std::vector<Token> tokenize(Accessor& accessor, int line);
	void colorToken(StyleContext& styleContext, Token token, State state);
	bool isAlphabetic(int ch) { return (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122); }
	bool isNumeric(int ch) { return ch >= 48 && ch <= 57; }
	bool isHex(int ch) { return isNumeric(ch) || (ch >= 65 && ch <= 70) || (ch >= 97 && ch <= 102); }
	bool isAlphanumeric(int ch) { return isAlphabetic(ch) || isNumeric(ch); }
	int toLower(int ch) {
		if (ch >= 65 && ch <= 90) {
			return ch + 32;
		} else {
			return ch;
		}
	}
};
