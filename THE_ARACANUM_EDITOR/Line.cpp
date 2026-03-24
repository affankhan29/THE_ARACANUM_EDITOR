
#include "Line.h"

Line::Line(int maxChars)
    : maxLength(maxChars),
      currentLength(0),
      textContent(nullptr) {

    this->textContent = new wchar_t[this->maxLength + 1];

    this->textContent[0] = L'\0';
}

Line::~Line() {
    if (this->textContent != nullptr) {
        delete[] this->textContent;
        this->textContent = nullptr;
    }
}

Line::Line(const Line& other)
    : maxLength(other.maxLength),
      currentLength(other.currentLength),
      textContent(nullptr) {

    this->textContent = new wchar_t[this->maxLength + 1];
    str_cpy(this->textContent, other.textContent);
}

Line& Line::operator=(const Line& other) {
    if (this == &other) {
        return *this;
    }

    if (this->textContent != nullptr) {
        delete[] this->textContent;
    }

    this->maxLength = other.maxLength;
    this->currentLength = other.currentLength;

    this->textContent = new wchar_t[this->maxLength + 1];
    str_cpy(this->textContent, other.textContent);

    return *this;
}

bool Line::addChar(wchar_t ch) {
    if (this->currentLength >= this->maxLength) {
        return false;
    }

    this->textContent[this->currentLength] = ch;
    this->currentLength = this->currentLength + 1;
    this->textContent[this->currentLength] = L'\0';

    return true;
}

bool Line::addWord(const wchar_t* word) {
    int wordLen = str_len(word);

    if (wordLen == 0) {
        return true;
    }

    int spaceNeeded = wordLen;
    if (this->currentLength > 0) {
        spaceNeeded = spaceNeeded + 1; 
    }

    if (this->currentLength + spaceNeeded > this->maxLength) {
        return false;
    }

    if (this->currentLength > 0) {
        this->textContent[this->currentLength] = L' ';
        this->currentLength = this->currentLength + 1;
    }

    int wordIndex = 0;
    while (word[wordIndex] != L'\0') {
        this->textContent[this->currentLength] = word[wordIndex];
        this->currentLength = this->currentLength + 1;
        wordIndex = wordIndex + 1;
    }

    this->textContent[this->currentLength] = L'\0';

    return true;
}

bool Line::insertAt(int position, wchar_t ch) {

    if (position < 0 || position > this->currentLength) {
        return false;
    }

    if (this->currentLength >= this->maxLength) {
        return false;
    }

    for (int i = this->currentLength; i > position; i = i - 1) {
        this->textContent[i] = this->textContent[i - 1];
    }
    this->textContent[position] = ch;
    this->currentLength = this->currentLength + 1;
    this->textContent[this->currentLength] = L'\0';

    return true;
}

bool Line::deleteAt(int position) {
  
    if (position < 0 || position >= this->currentLength) {
        return false;
    }

    for (int i = position; i < this->currentLength - 1; i = i + 1) {
        this->textContent[i] = this->textContent[i + 1];
    }

    this->currentLength = this->currentLength - 1;
    this->textContent[this->currentLength] = L'\0';

    return true;
}
const wchar_t* Line::getText() const {
    return this->textContent;
}

int Line::getLength() const {
    return this->currentLength;
}

int Line::getCapacity() const {
    return this->maxLength;
}

void Line::clear() {
    this->currentLength = 0;
    this->textContent[0] = L'\0';
}

bool Line::isFull() const {
    return this->currentLength >= this->maxLength;
}

bool Line::isEmpty() const {
    return this->currentLength == 0;
}