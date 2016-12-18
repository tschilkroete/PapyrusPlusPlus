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

#include "plbridge.hpp"
#include "scintilla\Scintilla.h"

#include <codecvt>
#include <locale>
#include <vector>

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
		break;
	case FOLDSTART:
		wordList = &wordListFoldStart;
		break;
	case FOLDEND:
		wordList = &wordListFoldEnd;
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

void SCI_METHOD PapyrusLexer::Lex(unsigned int startPos, int lengthDoc, int stateInit, IDocument* idocument) {
	Accessor accessor(idocument, nullptr);
	StyleContext  styleContext(startPos, lengthDoc, accessor.StyleAt(startPos - 1), accessor);

	//Check if the properties still exist and update the content
	for (std::list<Property>::iterator iterProperties = propertyLines.begin(); iterProperties != propertyLines.end();) {
		std::vector<Token> tokens = tokenize(accessor, (*iterProperties).line);
		bool found = false;
		for (std::vector<Token>::iterator iterToken = tokens.begin(); iterToken != tokens.end(); iterToken++) {
			if ((*iterToken).content == "property" && std::next(iterToken) != tokens.end() && !isComment(accessor.StyleAt((*iterToken).startPos)) && !isComment(accessor.StyleAt((*std::next(iterToken)).startPos))) {
				(*iterProperties).name = (*std::next(iterToken)).content;
				found = true;
				break;
			}
		}
		if (found) {
			iterProperties++;
		} else {
			iterProperties = propertyLines.erase(iterProperties);
		}
	}

	//This state is saved in the line feed character. It can be used to initialize the state of the next line
	State messageStateLast = static_cast<State>(accessor.StyleAt(startPos - 1));
	for (int line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc - 1); line++) {
		std::vector<Token> tokens = tokenize(accessor, line);
		State messageState = messageStateLast;

		//Check if a new property needs to be added
		for (std::vector<Token>::iterator iterToken = tokens.begin(); iterToken != tokens.end(); iterToken++) {
			if ((*iterToken).content == "property" && std::next(iterToken) != tokens.end() && !isComment(accessor.StyleAt((*iterToken).startPos)) && !isComment(accessor.StyleAt((*std::next(iterToken)).startPos))) {
				bool found = false;
				for (std::list<Property>::iterator iterProperties = propertyLines.begin(); iterProperties != propertyLines.end(); iterProperties++) {
					if ((*iterProperties).line == line) {
						found = true;
						break;
					}
				}
				if (!found) {
					Property property;
					property.line = line;
					property.name = (*std::next(iterToken)).content;
					propertyLines.push_back(property);
					break;
				}
			}
		}

		//Styling
		for (std::vector<Token>::iterator tokensIter = tokens.begin(); tokensIter != tokens.end(); tokensIter++) {
			if (messageState == COMMENTDOC) {
				colorToken(styleContext, *tokensIter, COMMENTDOC);
				if ((*tokensIter).content == "}") {
					messageState = DEFAULT;
				}
			} else if(messageState == COMMENTMULTILINE) {
				colorToken(styleContext, *tokensIter, COMMENTMULTILINE);
				if ((*tokensIter).content == ";" && tokensIter != tokens.begin() && (*std::prev(tokensIter)).content == "/") {
					messageState = DEFAULT;
				}
			} else if(messageState == COMMENT) {
				colorToken(styleContext, *tokensIter, COMMENT);
			} else if(messageState == STRING) {
				colorToken(styleContext, *tokensIter, STRING);
				if ((*tokensIter).content == "\"") {
					messageState = DEFAULT;
				}
			} else {
				//Determine the type of the token and color it
				if ((*tokensIter).content == "{") {
					colorToken(styleContext, *tokensIter, COMMENTDOC);
					messageState = COMMENTDOC;
				} else if ((*tokensIter).content == ";") {
					if (std::next(tokensIter) != tokens.end() && (*std::next(tokensIter)).content == "/") {
						colorToken(styleContext, *tokensIter, COMMENTMULTILINE);
						messageState = COMMENTMULTILINE;
					} else {
						colorToken(styleContext, *tokensIter, COMMENT);
						messageState = COMMENT;
					}
				} else if((*tokensIter).content == "\"") {
					colorToken(styleContext, *tokensIter, STRING);
					messageState = STRING;
				} else if ((*tokensIter).tokenType == NUMERIC) {
					colorToken(styleContext, *tokensIter, NUMBER);
				} else if ((*tokensIter).tokenType == IDENTIFIER) {
					if (wordListTypes.InList((*tokensIter).content.c_str())) {
						colorToken(styleContext, *tokensIter, TYPE);
					} else if (wordListFlowControl.InList((*tokensIter).content.c_str())) {
						colorToken(styleContext, *tokensIter, FLOWCONTROL);
					} else if (wordListKeywords.InList((*tokensIter).content.c_str())) {
						colorToken(styleContext, *tokensIter, KEYWORD);
					} else if (wordListOperators.InList((*tokensIter).content.c_str())) {
						colorToken(styleContext, *tokensIter, TYPE);
					} else {
						bool found = false;
						for (Property property : propertyLines) {
							if (property.name == (*tokensIter).content) {
								colorToken(styleContext, *tokensIter, PROPERTY);
								found = true;
								break;
							}
						}
						if (!found) {
							for (std::wstring path : plbridge.includes) {
								path.append(L"\\");
								path.append(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes((*tokensIter).content));
								path.append(L".psc");
								FILE* file;
								if (_wfopen_s(&file, path.c_str(), L"r") == 0) {
									fclose(file);
									colorToken(styleContext, *tokensIter, CLASS);
									found = true;
									break;
								}
							}

							if (!found) {
								colorToken(styleContext, *tokensIter, DEFAULT);
							}
						}
					}
				} else if ((*tokensIter).tokenType == SPECIAL) {
					if (wordListOperators.InList((*tokensIter).content.c_str())) {
						colorToken(styleContext, *tokensIter, OPERATOR);
					} else {
						colorToken(styleContext, *tokensIter, DEFAULT);
					}
				}
			}
		}
		if (messageState == COMMENT || messageState == STRING) {
			messageState = DEFAULT;
		}
		if(styleContext.ch == '\r') styleContext.Forward();
		if (styleContext.ch == '\n') {
			styleContext.SetState(messageState);
			styleContext.Forward();
		}
		messageStateLast = messageState;
	}
	styleContext.Complete();
}

void SCI_METHOD PapyrusLexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *idocument) {
	Accessor accessor(idocument, nullptr);
	int levelPrev = accessor.LevelAt(accessor.GetLine(startPos)) & SC_FOLDLEVELNUMBERMASK;
	//Lines
	for (int line = accessor.GetLine(startPos); line <= accessor.GetLine(startPos + lengthDoc); line++) {
		int levelDelta = 0;
		//Chars
		std::vector<Token> tokens = tokenize(accessor, line);
		for (Token token : tokens) {
			if (!isComment(accessor.StyleAt(token.startPos))) {
				if (wordListFoldStart.InList(token.content.c_str())) {
					levelDelta++;
				} else if (wordListFoldEnd.InList(token.content.c_str())) {
					levelDelta--;
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

std::vector<PapyrusLexer::Token> PapyrusLexer::tokenize(Accessor& accessor, int line) {
	std::vector<Token> tokens;
	int index = accessor.LineStart(line);
	int indexNext = index;
	int ch = getChar(accessor, index, indexNext);
	while (index < accessor.LineEnd(line)) {
		if (ch == '\r' || ch == '\n') break;

		if (ch == ' ' || ch == '\t') {
			ch = getChar(accessor, index, indexNext);
		} else if (isAlphabetic(ch) || ch == '_') {
			Token token;
			token.tokenType = IDENTIFIER;
			token.startPos = index;
			while (isAlphanumeric(ch) || ch == '_') {
				token.content.push_back(toLower(ch));
				ch = getChar(accessor, index, indexNext);
			}
			tokens.push_back(token);
		} else if (isNumeric(ch)) {
			Token token;
			token.tokenType = NUMERIC;
			token.startPos = index;
			while (isNumeric(ch) || (tolower(ch) == 'x' && index == token.startPos + 1) || (isHex(ch) && token.content.size() > 2 &&  tolower(token.content.at(1)) == 'x')) {
				token.content.push_back(toLower(ch));
				ch = getChar(accessor, index, indexNext);
			}
			tokens.push_back(token);
		} else {
			Token token;
			token.tokenType = SPECIAL;
			token.startPos = index;
			token.content.push_back(toLower(ch));
			tokens.push_back(token);
			ch = getChar(accessor, index, indexNext);
		}
	}
	return tokens;
}

void PapyrusLexer::colorToken(StyleContext & styleContext, Token token, State state) {
	while (styleContext.currentPos != token.startPos) {
		styleContext.Forward();
	}

	styleContext.SetState(state);
	styleContext.Forward(token.content.size());
}
