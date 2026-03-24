
#include "Column.h"

Column::Column(int lines, int chars)
    : maxLines(lines),
      charsPerLine(chars),
      linesUsed(0),
      lines(nullptr) {

    this->lines = new Line * [this->maxLines];

    for (int i = 0; i < this->maxLines; i++) {
        this->lines[i] = new Line(this->charsPerLine);
    }
}

Column::~Column() {
    for (int i = 0; i < this->maxLines; i = i + 1) {
        if (this->lines[i] != nullptr) {
            delete this->lines[i];
            this->lines[i] = nullptr;
        }
    }

    if (this->lines != nullptr) {
        delete[] this->lines;
        this->lines = nullptr;
    }
}

Column::Column(const Column& copy)
    : maxLines(copy.maxLines),
      charsPerLine(copy.charsPerLine),
      linesUsed(copy.linesUsed),
      lines(nullptr) {

    this->lines = new Line * [this->maxLines];

    for (int i = 0; i < this->maxLines; i = i + 1) {
        this->lines[i] = new Line(this->charsPerLine);
    }


    for (int i = 0; i < this->linesUsed; i = i + 1) {
        const wchar_t* otherText = copy.lines[i]->getText();
        int len = str_len(otherText);

        for (int j = 0; j < len; j = j + 1) {
            this->lines[i]->addChar(otherText[j]);
        }
    }
}


Column& Column::operator=(const Column& copy) {

    if (this == &copy) {
        return *this;
    }

    for (int i = 0; i < this->maxLines; i = i + 1) {
        if (this->lines[i] != nullptr) {
            delete this->lines[i];
        }
    }
    delete[] this->lines;

  
    this->maxLines = copy.maxLines;
    this->charsPerLine = copy.charsPerLine;
    this->linesUsed = copy.linesUsed;

    this->lines = new Line * [this->maxLines];

    for (int i = 0; i < this->maxLines; i = i + 1) {
        this->lines[i] = new Line(this->charsPerLine);

        if (i < this->linesUsed) {
            const wchar_t* otherText = copy.lines[i]->getText();
            int len = str_len(otherText);
            for (int j = 0; j < len; j = j + 1) {
                this->lines[i]->addChar(otherText[j]);
            }
        }
    }

    return *this;
}


bool Column::addText(const wchar_t* text) {
    // buffer to hold the current word being processed
    // Max word length we have is 255 characters
    wchar_t wordBuffer[256];
    int wordIndex = 0;

    int textIndex = 0;
    int textLen = str_len(text);

    while (textIndex < textLen) {
        wchar_t currentChar = text[textIndex];

        if (is_break(currentChar) && wordIndex > 0) {
        
            wordBuffer[wordIndex] = L'\0'; 

            Line* currentLine = this->lines[this->linesUsed];

            if (!currentLine->addWord(wordBuffer)) {
               
                this->linesUsed = this->linesUsed + 1;

                if (this->linesUsed >= this->maxLines) {
                   
                    this->linesUsed = this->linesUsed - 1;
                    this->lines[this->linesUsed]->addWord(wordBuffer);
                    return false;  
                }
                currentLine = this->lines[this->linesUsed];
                currentLine->addWord(wordBuffer);
            }

            wordIndex = 0;

            
            if (currentChar != L' ' && currentChar != L'\t') {
                wordBuffer[0] = currentChar;
                wordBuffer[1] = L'\0';
                this->lines[this->linesUsed]->addWord(wordBuffer);
            }
        }
        else if (!is_break(currentChar)) {
            if (wordIndex < 255) { 
                wordBuffer[wordIndex] = currentChar;
                wordIndex = wordIndex + 1;
            }
        }

        textIndex = textIndex + 1;
    }

    if (wordIndex > 0) {
        wordBuffer[wordIndex] = L'\0';
        Line* currentLine = this->lines[this->linesUsed];

        if (!currentLine->addWord(wordBuffer)) {
          
            this->linesUsed = this->linesUsed + 1;
            if (this->linesUsed >= this->maxLines) {
                this->linesUsed = this->linesUsed - 1;
                return false;
            }
            this->lines[this->linesUsed]->addWord(wordBuffer);
        }
    }

    return true;  
}

Line* Column::getLine(int index) {
    if (index >= 0 && index < this->maxLines) {
        if (index >= this->linesUsed) {
            this->linesUsed = index + 1;
        }
        return this->lines[index];
    }
    return nullptr;
}


int Column::getUsedLines() const {
    return this->linesUsed;
}


bool Column::isFull() const {
    return this->linesUsed >= this->maxLines;
}

void Column::clear() {
    for (int i = 0; i < this->linesUsed; i = i + 1) {
        this->lines[i]->clear();
    }
    this->linesUsed = 0;
}

int Column::getMaxLines() const {
    return this->maxLines;
}

int Column::getCharsPerLine() const {
    return this->charsPerLine;
}


