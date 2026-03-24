
#pragma once
#include "ManualStringHelpers.h"

class Line {
private:
    wchar_t* textContent;   // the text we have
    int currentLength;       // count of the chars use
    int maxLength;           // maxium capcity

public:
    Line(int maxChars);

    ~Line();

    Line(const Line& other);

    Line& operator=(const Line& other);

    bool addChar(wchar_t ch);

    bool addWord(const wchar_t* word);

    bool insertAt(int position, wchar_t ch);

    bool deleteAt(int position);

    const wchar_t* getText() const;

    int getLength() const;

    int getCapacity() const;

    void clear();

    bool isFull() const;

    bool isEmpty() const;
};