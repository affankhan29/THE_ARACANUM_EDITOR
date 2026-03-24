#include "Document.h"
#include "ManualStringHelpers.h"
#include <windows.h>

Document::Document()
    : pages(nullptr),
    pagesUsed(0),
    maxPages(10),
    currentPage(0),
    currentColumn(0),
    currentLine(0),
    currentChar(0),
    linesPerColumn(20),
    charsPerLine(40),
    columnsPerPage(2),
    alignmentMode(0),
    fileName(nullptr),
    hasChanges(false),
    searchHistory(nullptr),
    searchMatchCounts(nullptr),
    searchHistoryCount(0),
    tocEntries(nullptr),
    tocPageNumbers(nullptr),
    tocCount(0) {

    this->pages = new Page * [this->maxPages];
    this->pages[0] = new Page(1, this->columnsPerPage, this->linesPerColumn, this->charsPerLine);
    this->pagesUsed = 1;

    this->searchHistory = new wchar_t* [5];
    this->searchMatchCounts = new int[5];
    for (int i = 0; i < 5; i = i + 1) {
        this->searchHistory[i] = nullptr;
        this->searchMatchCounts[i] = 0;
    }

    this->tocEntries = new wchar_t* [100];
    this->tocPageNumbers = new int[100];
    for (int i = 0; i < 100; i = i + 1) {
        this->tocEntries[i] = nullptr;
        this->tocPageNumbers[i] = 0;
    }
}

Document::Document(int lines, int chars, int cols)
    :pages(nullptr),
    pagesUsed(0),
    maxPages(10),
    currentPage(0),
    currentColumn(0),
    currentLine(0),
    currentChar(0),
    linesPerColumn(lines),
    charsPerLine(chars),
    columnsPerPage(cols),
    alignmentMode(0),
    fileName(nullptr),
    hasChanges(false),
    searchHistory(nullptr),
    searchMatchCounts(nullptr),
    searchHistoryCount(0),
    tocEntries(nullptr),
    tocPageNumbers(nullptr),
    tocCount(0) {

    this->pages = new Page * [this->maxPages];
    this->pages[0] = new Page(1, this->columnsPerPage, this->linesPerColumn, this->charsPerLine);
    this->pagesUsed = 1;

    this->searchHistory = new wchar_t* [5];
    this->searchMatchCounts = new int[5];
    for (int i = 0; i < 5; i = i + 1) {
        this->searchHistory[i] = nullptr;
        this->searchMatchCounts[i] = 0;
    }

    this->tocEntries = new wchar_t* [100];
    this->tocPageNumbers = new int[100];
    for (int i = 0; i < 100; i = i + 1) {
        this->tocEntries[i] = nullptr;
        this->tocPageNumbers[i] = 0;
    }
}

Document::~Document() {
    for (int i = 0; i < this->pagesUsed; i = i + 1) {
        if (this->pages[i] != nullptr) {
            delete this->pages[i];
            this->pages[i] = nullptr;
        }
    }
    if (this->pages != nullptr) {
        delete[] this->pages;
        this->pages = nullptr;
    }

    if (this->fileName != nullptr) {
        delete[] this->fileName;
        this->fileName = nullptr;
    }

    for (int i = 0; i < this->searchHistoryCount; i = i + 1) {
        if (this->searchHistory[i] != nullptr) {
            delete[] this->searchHistory[i];
            this->searchHistory[i] = nullptr;
        }
    }
    if (this->searchHistory != nullptr) {
        delete[] this->searchHistory;
        this->searchHistory = nullptr;
    }
    if (this->searchMatchCounts != nullptr) {
        delete[] this->searchMatchCounts;
        this->searchMatchCounts = nullptr;
    }

    for (int i = 0; i < this->tocCount; i = i + 1) {
        if (this->tocEntries[i] != nullptr) {
            delete[] this->tocEntries[i];
            this->tocEntries[i] = nullptr;
        }
    }
    if (this->tocEntries != nullptr) {
        delete[] this->tocEntries;
        this->tocEntries = nullptr;
    }
    if (this->tocPageNumbers != nullptr) {
        delete[] this->tocPageNumbers;
        this->tocPageNumbers = nullptr;
    }
}

Document::Document(const Document& copy)
    :pages(nullptr),
    pagesUsed(copy.pagesUsed),
    maxPages(copy.maxPages),
    currentPage(copy.currentPage),
    currentColumn(copy.currentColumn),
    currentLine(copy.currentLine),
    currentChar(copy.currentChar),
    linesPerColumn(copy.linesPerColumn),
    charsPerLine(copy.charsPerLine),
    columnsPerPage(copy.columnsPerPage),
    alignmentMode(copy.alignmentMode),
    fileName(nullptr),
    hasChanges(copy.hasChanges),
    searchHistory(nullptr),
    searchMatchCounts(nullptr),
    searchHistoryCount(copy.searchHistoryCount),
    tocEntries(nullptr),
    tocPageNumbers(nullptr),
    tocCount(copy.tocCount) {

    this->pages = new Page * [this->maxPages];
    for (int i = 0; i < this->pagesUsed; i = i + 1) {
        this->pages[i] = new Page(
            copy.pages[i]->getPageNumber(),
            copy.columnsPerPage,
            copy.linesPerColumn,
            copy.charsPerLine
        );
        for (int col = 0; col < copy.columnsPerPage; col = col + 1) {
            for (int ln = 0; ln < copy.pages[i]->getColumn(col)->getUsedLines(); ln = ln + 1) {
                Line* srcLine = copy.pages[i]->getColumn(col)->getLine(ln);
                if (srcLine != nullptr) {
                    this->pages[i]->getColumn(col)->addText(srcLine->getText());
                }
            }
        }
    }

    if (copy.fileName != nullptr) {
        int len = str_len(copy.fileName);
        this->fileName = new wchar_t[len + 1];
        str_cpy(this->fileName, copy.fileName);
    }

    this->searchHistory = new wchar_t* [5];
    this->searchMatchCounts = new int[5];
    for (int i = 0; i < 5; i = i + 1) {
        this->searchHistory[i] = nullptr;
        this->searchMatchCounts[i] = 0;
        if (i < copy.searchHistoryCount && copy.searchHistory[i] != nullptr) {
            int len = str_len(copy.searchHistory[i]);
            this->searchHistory[i] = new wchar_t[len + 1];
            str_cpy(this->searchHistory[i], copy.searchHistory[i]);
            this->searchMatchCounts[i] = copy.searchMatchCounts[i];
        }
    }

    this->tocEntries = new wchar_t* [100];
    this->tocPageNumbers = new int[100];
    for (int i = 0; i < 100; i = i + 1) {
        this->tocEntries[i] = nullptr;
        this->tocPageNumbers[i] = 0;
        if (i < copy.tocCount && copy.tocEntries[i] != nullptr) {
            int len = str_len(copy.tocEntries[i]);
            this->tocEntries[i] = new wchar_t[len + 1];
            str_cpy(this->tocEntries[i], copy.tocEntries[i]);
            this->tocPageNumbers[i] = copy.tocPageNumbers[i];
        }
    }
}

Document& Document::operator=(const Document& copy) {
    if (this == &copy) {
        return *this;
    }

    for (int i = 0; i < this->pagesUsed; i = i + 1) {
        if (this->pages[i] != nullptr) {
            delete this->pages[i];
        }
    }
    delete[] this->pages;

    if (this->fileName != nullptr) {
        delete[] this->fileName;
    }

    for (int i = 0; i < this->searchHistoryCount; i = i + 1) {
        if (this->searchHistory[i] != nullptr) {
            delete[] this->searchHistory[i];
        }
    }
    delete[] this->searchHistory;
    delete[] this->searchMatchCounts;

    for (int i = 0; i < this->tocCount; i = i + 1) {
        if (this->tocEntries[i] != nullptr) {
            delete[] this->tocEntries[i];
        }
    }
    delete[] this->tocEntries;
    delete[] this->tocPageNumbers;

    this->pagesUsed = copy.pagesUsed;
    this->maxPages = copy.maxPages;
    this->currentPage = copy.currentPage;
    this->currentColumn = copy.currentColumn;
    this->currentLine = copy.currentLine;
    this->currentChar = copy.currentChar;
    this->linesPerColumn = copy.linesPerColumn;
    this->charsPerLine = copy.charsPerLine;
    this->columnsPerPage = copy.columnsPerPage;
    this->alignmentMode = copy.alignmentMode;
    this->hasChanges = copy.hasChanges;
    this->searchHistoryCount = copy.searchHistoryCount;
    this->tocCount = copy.tocCount;

    this->pages = new Page * [this->maxPages];
    for (int i = 0; i < this->pagesUsed; i = i + 1) {
        this->pages[i] = new Page(
            copy.pages[i]->getPageNumber(),
            copy.columnsPerPage,
            copy.linesPerColumn,
            copy.charsPerLine
        );
        for (int col = 0; col < copy.columnsPerPage; col = col + 1) {
            for (int ln = 0; ln < copy.pages[i]->getColumn(col)->getUsedLines(); ln = ln + 1) {
                Line* srcLine = copy.pages[i]->getColumn(col)->getLine(ln);
                if (srcLine != nullptr) {
                    this->pages[i]->getColumn(col)->addText(srcLine->getText());
                }
            }
        }
    }

    if (copy.fileName != nullptr) {
        int len = str_len(copy.fileName);
        this->fileName = new wchar_t[len + 1];
        str_cpy(this->fileName, copy.fileName);
    }

    this->searchHistory = new wchar_t* [5];
    this->searchMatchCounts = new int[5];
    for (int i = 0; i < 5; i = i + 1) {
        this->searchHistory[i] = nullptr;
        this->searchMatchCounts[i] = 0;
        if (i < copy.searchHistoryCount && copy.searchHistory[i] != nullptr) {
            int len = str_len(copy.searchHistory[i]);
            this->searchHistory[i] = new wchar_t[len + 1];
            str_cpy(this->searchHistory[i], copy.searchHistory[i]);
            this->searchMatchCounts[i] = copy.searchMatchCounts[i];
        }
    }

    this->tocEntries = new wchar_t* [100];
    this->tocPageNumbers = new int[100];
    for (int i = 0; i < 100; i = i + 1) {
        this->tocEntries[i] = nullptr;
        this->tocPageNumbers[i] = 0;
        if (i < copy.tocCount && copy.tocEntries[i] != nullptr) {
            int len = str_len(copy.tocEntries[i]);
            this->tocEntries[i] = new wchar_t[len + 1];
            str_cpy(this->tocEntries[i], copy.tocEntries[i]);
            this->tocPageNumbers[i] = copy.tocPageNumbers[i];
        }
    }

    return *this;
}

void Document::ensurePageExists() {
    // SAFETY: Limit maximum pages to prevent memory exhaustion
    if (this->pagesUsed >= 1000) {
        return;
    }

    if (this->currentPage >= this->pagesUsed - 1) {
        if (this->pagesUsed >= this->maxPages) {
            int newMax = this->maxPages * 2;

            // SAFETY: Limit max pages growth
            if (newMax > 1000) {
                newMax = 1000;
            }

            Page** newPages = new Page * [newMax];
            for (int i = 0; i < this->pagesUsed; i = i + 1) {
                newPages[i] = this->pages[i];
            }
            delete[] this->pages;
            this->pages = newPages;
            this->maxPages = newMax;
        }

        this->pages[this->pagesUsed] = new Page(
            this->pagesUsed + 1,
            this->columnsPerPage,
            this->linesPerColumn,
            this->charsPerLine
        );
        this->pagesUsed = this->pagesUsed + 1;
    }
}
void Document::insertCharacter(wchar_t ch) {
    this->ensurePageExists();

    Page* currentPg = this->pages[this->currentPage];
    if (currentPg == nullptr) return;

    Column* currentCol = currentPg->getColumn(this->currentColumn);
    if (currentCol == nullptr) return;

    // Get or create the current line directly
    Line* currentLn = currentCol->getLine(this->currentLine);
    if (currentLn == nullptr) {
        // Line doesn't exist yet � add blank lines up to currentLine
        // by using addText with empty string approach, but safer:
        // just advance to a new line via handleNewLine and retry
        this->currentLine = currentCol->getUsedLines();
        currentLn = currentCol->getLine(this->currentLine);
        if (currentLn == nullptr) return;
    }

    if (currentLn->insertAt(this->currentChar, ch)) {
        this->currentChar = this->currentChar + 1;
        this->hasChanges = true;

        if (currentLn->getLength() >= this->charsPerLine) {
            this->handleNewLine();
        }
    }
}
void Document::insertText(const wchar_t* text) {
    int index = 0;
    while (text[index] != L'\0') {
        if (text[index] == L'\n') {
            this->handleNewLine();
        }
        else {
            this->insertCharacter(text[index]);
        }
        index = index + 1;
    }
}

void Document::deleteChar() {
    if (this->pagesUsed == 0) return;

    Page* currentPg = this->pages[this->currentPage];
    Column* currentCol = currentPg->getColumn(this->currentColumn);

    if (this->currentLine < currentCol->getUsedLines()) {
        Line* currentLn = currentCol->getLine(this->currentLine);
        if (currentLn != nullptr && this->currentChar < currentLn->getLength()) {
            currentLn->deleteAt(this->currentChar);
            this->hasChanges = true;
        }
    }
}

void Document::backspace() {
    if (this->pagesUsed == 0) return;

    if (this->currentChar > 0) {
        this->currentChar = this->currentChar - 1;
        this->deleteChar();
    }
    else if (this->currentLine > 0) {
        this->currentLine = this->currentLine - 1;
        Page* currentPg = this->pages[this->currentPage];
        Column* currentCol = currentPg->getColumn(this->currentColumn);
        Line* prevLine = currentCol->getLine(this->currentLine);
        if (prevLine != nullptr) {
            this->currentChar = prevLine->getLength();
        }
        this->hasChanges = true;
    }
    else if (this->currentColumn > 0) {
        this->currentColumn = this->currentColumn - 1;
        Page* currentPg = this->pages[this->currentPage];
        Column* currentCol = currentPg->getColumn(this->currentColumn);
        this->currentLine = currentCol->getUsedLines() - 1;
        if (this->currentLine < 0) this->currentLine = 0;
        Line* prevLine = currentCol->getLine(this->currentLine);
        if (prevLine != nullptr) {
            this->currentChar = prevLine->getLength();
        }
        this->hasChanges = true;
    }
    else if (this->currentPage > 0) {
        this->currentPage = this->currentPage - 1;
        this->currentColumn = this->columnsPerPage - 1;
        Page* currentPg = this->pages[this->currentPage];
        Column* currentCol = currentPg->getColumn(this->currentColumn);
        this->currentLine = currentCol->getUsedLines() - 1;
        if (this->currentLine < 0) this->currentLine = 0;
        Line* prevLine = currentCol->getLine(this->currentLine);
        if (prevLine != nullptr) {
            this->currentChar = prevLine->getLength();
        }
        this->hasChanges = true;
    }
}

void Document::handleNewLine() {
    this->currentLine = this->currentLine + 1;
    this->currentChar = 0;

    if (this->currentLine >= this->linesPerColumn) {
        this->currentColumn = this->currentColumn + 1;
        this->currentLine = 0;

        if (this->currentColumn >= this->columnsPerPage) {
            this->currentColumn = 0;
            this->currentPage = this->currentPage + 1;
            this->ensurePageExists();
        }
    }
    this->hasChanges = true;
}

void Document::moveCursor(int deltaX, int deltaY) {
    this->currentChar = this->currentChar + deltaX;
    this->currentLine = this->currentLine + deltaY;

    if (this->currentLine < 0) {
        this->currentLine = 0;
        this->currentChar = 0;
    }
    if (this->currentColumn < 0) this->currentColumn = 0;
    if (this->currentPage < 0) this->currentPage = 0;
    if (this->currentPage >= this->pagesUsed) this->currentPage = this->pagesUsed - 1;

    Page* currentPg = this->pages[this->currentPage];
    if (currentPg != nullptr) {
        Column* currentCol = currentPg->getColumn(this->currentColumn);
        if (currentCol != nullptr && this->currentLine >= currentCol->getUsedLines()) {
            this->currentLine = currentCol->getUsedLines() - 1;
            if (this->currentLine < 0) this->currentLine = 0;
        }
    }
}

void Document::goToPage(int pageNum) {
    if (pageNum >= 1 && pageNum <= this->pagesUsed) {
        this->currentPage = pageNum - 1;
        this->currentColumn = 0;
        this->currentLine = 0;
        this->currentChar = 0;
    }
}

void Document::setAlignment(int mode) {
    if (mode >= 0 && mode <= 3) {
        this->alignmentMode = mode;
    }
}

int Document::getAlignment() const {
    return this->alignmentMode;
}

int Document::search(const wchar_t* term) {
    int totalMatches = 0;

    wchar_t searchLower[256];
    str_cpy(searchLower, term);
    str_to_lower(searchLower);

    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = currentPg->getColumn(col);

            for (int ln = 0; ln < currentCol->getUsedLines(); ln = ln + 1) {
                Line* currentLn = currentCol->getLine(ln);
                if (currentLn == nullptr) continue;

                const wchar_t* lineText = currentLn->getText();
                wchar_t lineLower[512];
                str_cpy(lineLower, lineText);
                str_to_lower(lineLower);

                int lineLen = str_len(lineLower);
                int termLen = str_len(searchLower);

                for (int pos = 0; pos <= lineLen - termLen; pos = pos + 1) {
                    bool found = true;
                    for (int k = 0; k < termLen; k = k + 1) {
                        if (lineLower[pos + k] != searchLower[k]) {
                            found = false;
                            break;
                        }
                    }
                    if (found) {
                        totalMatches = totalMatches + 1;
                    }
                }
            }
        }
    }

    if (this->searchHistoryCount < 5) {
        int len = str_len(term);
        this->searchHistory[this->searchHistoryCount] = new wchar_t[len + 1];
        str_cpy(this->searchHistory[this->searchHistoryCount], term);
        this->searchMatchCounts[this->searchHistoryCount] = totalMatches;
        this->searchHistoryCount = this->searchHistoryCount + 1;
    }

    return totalMatches;
}

bool Document::isMatch(int pageIdx, int colIdx, int lineIdx, const wchar_t* term) {
    if (pageIdx >= this->pagesUsed) return false;

    Page* pg = this->pages[pageIdx];
    Column* col = pg->getColumn(colIdx);
    Line* ln = col->getLine(lineIdx);

    if (ln == nullptr) return false;

    const wchar_t* lineText = ln->getText();
    int lineLen = str_len(lineText);
    int termLen = str_len(term);

    wchar_t lineLower[512], termLower[256];
    str_cpy(lineLower, lineText);
    str_cpy(termLower, term);
    str_to_lower(lineLower);
    str_to_lower(termLower);

    for (int pos = 0; pos <= lineLen - termLen; pos = pos + 1) {
        bool found = true;
        for (int k = 0; k < termLen; k = k + 1) {
            if (lineLower[pos + k] != termLower[k]) {
                found = false;
                break;
            }
        }
        if (found) return true;
    }

    return false;
}

wchar_t* Document::getSearchHistory(int index) {
    if (index >= 0 && index < this->searchHistoryCount) {
        return this->searchHistory[index];
    }
    return nullptr;
}

int Document::getSearchMatchCount(int index) {
    if (index >= 0 && index < this->searchHistoryCount) {
        return this->searchMatchCounts[index];
    }
    return 0;
}

int Document::getSearchHistoryCount() const {
    return this->searchHistoryCount;
}

int Document::calculateHeadingScore(const wchar_t* text) {
    int score = 0;
    int len = str_len(text);

    int wordCount = 0;
    bool inWord = false;
    for (int i = 0; i < len; i = i + 1) {
        if (is_break(text[i])) {
            if (inWord) {
                wordCount = wordCount + 1;
                inWord = false;
            }
        }
        else {
            inWord = true;
        }
    }
    if (inWord) wordCount = wordCount + 1;

    if (wordCount <= 8) {
        score = score + 2;
    }

    if (len < 60) {
        score = score + 1;
    }

    if (len > 0) {
        wchar_t lastChar = text[len - 1];
        if (lastChar != L'.' && lastChar != L'!' && lastChar != L'?') {
            score = score + 2;
        }
    }

    int capCount = 0;
    int wordStarts = 0;
    bool atWordStart = true;

    for (int i = 0; i < len; i = i + 1) {
        if (is_break(text[i])) {
            atWordStart = true;
        }
        else if (atWordStart) {
            wordStarts = wordStarts + 1;
            if (text[i] >= L'A' && text[i] <= L'Z') {
                capCount = capCount + 1;
            }
            atWordStart = false;
        }
    }

    if (wordStarts > 0 && capCount >= wordStarts - 1) {
        score = score + 2;
    }

    bool hasComma = false;
    for (int i = 0; i < len; i = i + 1) {
        if (text[i] == L',') {
            hasComma = true;
            break;
        }
    }
    if (!hasComma) {
        score = score + 1;
    }

    return score;
}

bool Document::isStopWord(const wchar_t* word) {
    const wchar_t* stopWords[] = {
        L"was", L"is", L"are", L"and", L"the", L"a", L"an",
        L"of", L"in", L"to", L"for", L"on", L"with"
    };
    int numStops = 13;

    for (int i = 0; i < numStops; i = i + 1) {
        if (str_cmp(word, stopWords[i]) == 0) {
            return true;
        }
    }
    return false;
}

void Document::buildTableOfContents() {
    for (int i = 0; i < this->tocCount; i = i + 1) {
        if (this->tocEntries[i] != nullptr) {
            delete[] this->tocEntries[i];
            this->tocEntries[i] = nullptr;
        }
    }
    this->tocCount = 0;

    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];
        const wchar_t* firstPara = currentPg->getFirstParagraph();

        if (str_len(firstPara) == 0) continue;

        int score = this->calculateHeadingScore(firstPara);

        if (score >= 5) {
            int entryLen = str_len(firstPara);
            this->tocEntries[this->tocCount] = new wchar_t[entryLen + 1];
            str_cpy(this->tocEntries[this->tocCount], firstPara);
            this->tocPageNumbers[this->tocCount] = currentPg->getPageNumber();
            this->tocCount = this->tocCount + 1;
        }
        else {
            wchar_t fallback[256] = L"";
            int wordCount = 0;
            int wordStart = 0;
            bool inWord = false;

            int len = str_len(firstPara);
            for (int i = 0; i < len && wordCount < 4; i = i + 1) {
                if (is_break(firstPara[i])) {
                    if (inWord) {
                        int wordLen = i - wordStart;
                        wchar_t word[64];
                        for (int j = 0; j < wordLen && j < 63; j = j + 1) {
                            word[j] = firstPara[wordStart + j];
                        }
                        word[wordLen] = L'\0';

                        if (!this->isStopWord(word)) {
                            if (word[0] >= L'a' && word[0] <= L'z') {
                                word[0] = word[0] - (L'a' - L'A');
                            }

                            if (str_len(fallback) > 0) {
                                wchar_t temp[256];
                                str_cpy(temp, fallback);
                                fallback[0] = L'\0';
                                str_cpy(fallback, temp);
                                wchar_t space[2] = L" ";
                                str_cpy(fallback + str_len(fallback), space);
                            }
                            str_cpy(fallback + str_len(fallback), word);
                        }

                        wordCount = wordCount + 1;
                        inWord = false;
                    }
                }
                else if (!inWord) {
                    wordStart = i;
                    inWord = true;
                }
            }

            if (str_len(fallback) > 0) {
                int entryLen = str_len(fallback);
                this->tocEntries[this->tocCount] = new wchar_t[entryLen + 1];
                str_cpy(this->tocEntries[this->tocCount], fallback);
                this->tocPageNumbers[this->tocCount] = currentPg->getPageNumber();
                this->tocCount = this->tocCount + 1;
            }
        }
    }
}

const wchar_t* Document::getTOCEntry(int index, int& pageNum) {
    if (index >= 0 && index < this->tocCount) {
        pageNum = this->tocPageNumbers[index];
        return this->tocEntries[index];
    }
    pageNum = -1;
    return nullptr;
}

int Document::getTOCCount() const {
    return this->tocCount;
}

Page* Document::getPage(int index) {
    if (index >= 0 && index < this->pagesUsed) {
        return this->pages[index];
    }
    return nullptr;
}

int Document::getPageCount() const {
    return this->pagesUsed;
}

int Document::getCurrentPage() const { return this->currentPage; }
int Document::getCurrentColumn() const { return this->currentColumn; }
int Document::getCurrentLine() const { return this->currentLine; }
int Document::getCurrentChar() const { return this->currentChar; }

void Document::setCursor(int page, int col, int line, int ch) {
    if (page < 0) page = 0;
    if (page >= this->pagesUsed) page = this->pagesUsed - 1;
    if (col < 0) col = 0;
    if (col >= this->columnsPerPage) col = this->columnsPerPage - 1;

    this->currentPage = page;
    this->currentColumn = col;

    Page* currentPg = this->pages[this->currentPage];
    Column* currentCol = currentPg->getColumn(this->currentColumn);

    int usedLines = currentCol->getUsedLines();

    if (line < 0) line = 0;
    if (line >= usedLines) line = usedLines - 1;
    if (line < 0) line = 0;

    this->currentLine = line;

    Line* currentLn = currentCol->getLine(this->currentLine);
    int   lineLen = 0;
    if (currentLn != nullptr) {
        lineLen = currentLn->getLength();
    }

    if (ch < 0) ch = 0;
    if (ch > lineLen) ch = lineLen;

    this->currentChar = ch;
}

int Document::getLinesPerColumn() const { return this->linesPerColumn; }
int Document::getCharsPerLine() const { return this->charsPerLine; }
int Document::getColumnsPerPage() const { return this->columnsPerPage; }

bool Document::getHasChanges() const { return this->hasChanges; }
void Document::setHasChanges(bool value) { this->hasChanges = value; }

const wchar_t* Document::getFileName() const {
    return this->fileName;
}
void Document::setFileName(const wchar_t* name) {
    if (this->fileName != nullptr) {
        delete[] this->fileName;
    }
    if (name != nullptr) {
        int len = str_len(name);
        this->fileName = new wchar_t[len + 1];
        str_cpy(this->fileName, name);
    }
    else {
        this->fileName = nullptr;
    }
}

int Document::countWords() const {
    int total = 0;

    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = currentPg->getColumn(col);

            for (int ln = 0; ln < currentCol->getUsedLines(); ln = ln + 1) {
                Line* currentLn = currentCol->getLine(ln);
                if (currentLn == nullptr) continue;

                const wchar_t* text = currentLn->getText();
                int len = str_len(text);

                bool inWord = false;
                for (int i = 0; i < len; i = i + 1) {
                    if (is_break(text[i])) {
                        if (inWord) {
                            total = total + 1;
                            inWord = false;
                        }
                    }
                    else {
                        inWord = true;
                    }
                }
                if (inWord) total = total + 1;
            }
        }
    }

    return total;
}

int Document::countChars(bool includeSpaces) const {
    int total = 0;

    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = currentPg->getColumn(col);

            for (int ln = 0; ln < currentCol->getUsedLines(); ln = ln + 1) {
                Line* currentLn = currentCol->getLine(ln);
                if (currentLn == nullptr) continue;

                const wchar_t* text = currentLn->getText();
                int len = str_len(text);

                if (includeSpaces) {
                    total = total + len;
                }
                else {
                    for (int i = 0; i < len; i = i + 1) {
                        if (!is_break(text[i])) {
                            total = total + 1;
                        }
                    }
                }
            }
        }
    }

    return total;
}

int Document::countSentences() const {
    int total = 0;

    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = currentPg->getColumn(col);

            for (int ln = 0; ln < currentCol->getUsedLines(); ln = ln + 1) {
                Line* currentLn = currentCol->getLine(ln);
                if (currentLn == nullptr) continue;

                const wchar_t* text = currentLn->getText();
                int len = str_len(text);

                for (int i = 0; i < len; i = i + 1) {
                    if (text[i] == L'.' || text[i] == L'!' || text[i] == L'?') {
                        total = total + 1;
                    }
                }
            }
        }
    }

    return total;
}

int Document::estimateReadingTime() const {
    int words = this->countWords();
    return (words + 199) / 200;
}

bool Document::saveToFile(const wchar_t* path) {
    HANDLE hFile = CreateFileW(
        path,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;

    // Write header: linesPerColumn, charsPerLine, columnsPerPage, pagesUsed
    WriteFile(hFile, &this->linesPerColumn, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->charsPerLine, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->columnsPerPage, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->pagesUsed, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->currentPage, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->currentColumn, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->currentLine, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->currentChar, sizeof(int), &written, nullptr);
    WriteFile(hFile, &this->alignmentMode, sizeof(int), &written, nullptr);

    // Write all pages
    for (int pg = 0; pg < this->pagesUsed; pg = pg + 1) {
        Page* currentPg = this->pages[pg];

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = currentPg->getColumn(col);
            int usedLines = currentCol->getUsedLines();

            WriteFile(hFile, &usedLines, sizeof(int), &written, nullptr);

            for (int ln = 0; ln < usedLines; ln = ln + 1) {
                Line* currentLn = currentCol->getLine(ln);
                if (currentLn == nullptr) {
                    int zero = 0;
                    WriteFile(hFile, &zero, sizeof(int), &written, nullptr);
                    continue;
                }

                int lineLen = currentLn->getLength();
                WriteFile(hFile, &lineLen, sizeof(int), &written, nullptr);

                if (lineLen > 0) {
                    WriteFile(hFile, currentLn->getText(),
                        lineLen * sizeof(wchar_t), &written, nullptr);
                }
            }
        }
    }

    CloseHandle(hFile);
    this->hasChanges = false;
    return true;
}

bool Document::loadFromFile(const wchar_t* path) {
    HANDLE hFile = CreateFileW(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD bytesRead = 0;

    // Read header
    int newLines = 0, newChars = 0, newCols = 0, newPages = 0;
    int newCurPage = 0, newCurCol = 0, newCurLine = 0, newCurChar = 0;
    int newAlign = 0;

    ReadFile(hFile, &newLines, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newChars, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newCols, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newPages, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newCurPage, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newCurCol, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newCurLine, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newCurChar, sizeof(int), &bytesRead, nullptr);
    ReadFile(hFile, &newAlign, sizeof(int), &bytesRead, nullptr);

    // Sanity check
    if (newLines <= 0 || newChars <= 0 || newCols <= 0 ||
        newPages <= 0 || newPages > 1000) {
        CloseHandle(hFile);
        return false;
    }

    // Clear existing pages
    for (int i = 0; i < this->pagesUsed; i = i + 1) {
        if (this->pages[i] != nullptr) {
            delete this->pages[i];
            this->pages[i] = nullptr;
        }
    }
    delete[] this->pages;

    // Rebuild with loaded settings
    this->linesPerColumn = newLines;
    this->charsPerLine = newChars;
    this->columnsPerPage = newCols;
    this->alignmentMode = newAlign;
    this->pagesUsed = 0;
    this->maxPages = newPages + 10;

    this->pages = new Page * [this->maxPages];

    for (int pg = 0; pg < newPages; pg = pg + 1) {
        this->pages[pg] = new Page(
            pg + 1,
            this->columnsPerPage,
            this->linesPerColumn,
            this->charsPerLine
        );
        this->pagesUsed = this->pagesUsed + 1;

        for (int col = 0; col < this->columnsPerPage; col = col + 1) {
            Column* currentCol = this->pages[pg]->getColumn(col);

            int usedLines = 0;
            ReadFile(hFile, &usedLines, sizeof(int), &bytesRead, nullptr);

            for (int ln = 0; ln < usedLines; ln = ln + 1) {
                int lineLen = 0;
                ReadFile(hFile, &lineLen, sizeof(int), &bytesRead, nullptr);

                if (lineLen > 0 && lineLen <= this->charsPerLine) {
                    wchar_t* buf = new wchar_t[lineLen + 1];
                    ReadFile(hFile, buf, lineLen * sizeof(wchar_t),
                        &bytesRead, nullptr);
                    buf[lineLen] = L'\0';

                    Line* ln2 = currentCol->getLine(ln);
                    if (ln2 != nullptr) {
                        for (int ci = 0; ci < lineLen; ci = ci + 1) {
                            ln2->addChar(buf[ci]);
                        }
                    }

                    delete[] buf;
                }
            }
        }
    }

    // Restore cursor
    this->currentPage = newCurPage;
    this->currentColumn = newCurCol;
    this->currentLine = newCurLine;
    this->currentChar = newCurChar;

    CloseHandle(hFile);
    this->hasChanges = false;
    return true;
}