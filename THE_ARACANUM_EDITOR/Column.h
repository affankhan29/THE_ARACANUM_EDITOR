
#pragma once
#include "Line.h"

class Column {
private:
    Line** lines;
    int linesUsed;
    int maxLines;
    int charsPerLine;

public:
    Column(int lines, int chars);

    ~Column();

    Column(const Column& copy);

    Column& operator=(const Column& copy);

    bool addText(const wchar_t* text);

    Line* getLine(int index);

    int getUsedLines() const;

    bool isFull() const;

    void clear();
    int getMaxLines() const;

    int getCharsPerLine() const;
};