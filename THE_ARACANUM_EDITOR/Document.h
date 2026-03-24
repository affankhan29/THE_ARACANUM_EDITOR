#pragma once
#include "Page.h"

class Document {
private:
    Page** pages;
    int pagesUsed;
    int maxPages;

    int currentPage;
    int currentColumn;
    int currentLine;
    int currentChar;

    int linesPerColumn;
    int charsPerLine;
    int columnsPerPage;

    int alignmentMode;

    wchar_t* fileName;
    bool hasChanges;

    wchar_t** searchHistory;
    int* searchMatchCounts;
    int searchHistoryCount;

    wchar_t** tocEntries;
    int* tocPageNumbers;
    int tocCount;

    void ensurePageExists();
    int calculateHeadingScore(const wchar_t* text);
    bool isStopWord(const wchar_t* word);
    void rebuildTOC();

public:
    Document();
    Document(int lines, int chars, int cols);
    ~Document();
    Document(const Document& other);
    Document& operator=(const Document& other);

    void insertCharacter(wchar_t ch);
    void insertText(const wchar_t* text);
    void deleteChar();
    void backspace();
    void handleNewLine();

    void moveCursor(int deltaX, int deltaY);
    void goToPage(int pageNum);

    bool saveToFile(const wchar_t* path);
    bool loadFromFile(const wchar_t* path);

    int search(const wchar_t* term);
    bool isMatch(int pageIdx, int colIdx, int lineIdx, const wchar_t* term);
    wchar_t* getSearchHistory(int index);
    int getSearchMatchCount(int index);
    int getSearchHistoryCount() const;

    void setAlignment(int mode);
    int getAlignment() const;

    void buildTableOfContents();
    const wchar_t* getTOCEntry(int index, int& pageNum);
    int getTOCCount() const;

    Page* getPage(int index);
    int getPageCount() const;
    int getCurrentPage() const;
    int getCurrentColumn() const;
    int getCurrentLine() const;
    int getCurrentChar() const;

    void setCursor(int page, int col, int line, int ch);

    int getLinesPerColumn() const;
    int getCharsPerLine() const;
    int getColumnsPerPage() const;

    bool getHasChanges() const;
    void setHasChanges(bool value);

    const wchar_t* getFileName() const;
    void setFileName(const wchar_t* name);

    int countWords() const;
    int countChars(bool includeSpaces) const;
    int countSentences() const;
    int estimateReadingTime() const;
};