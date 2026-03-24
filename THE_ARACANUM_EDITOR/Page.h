#pragma once
#include "Column.h"

class Page {
private:
    Column** columns;
    int columnCount;
    int pageNumber;
    int linesPerColumn;
    int charsPerLine;

public:
    Page(int pageNum, int cols, int lines, int chars);
    ~Page();
    Page(const Page& other);
    Page& operator=(const Page& other);
    bool addText(const wchar_t* text);
    Column* getColumn(int index);
    int getPageNumber() const;
    bool isFull() const;
    bool hasContent() const;
    const wchar_t* getFirstParagraph() const;
    void clear();
    int getColumnCount() const;
};