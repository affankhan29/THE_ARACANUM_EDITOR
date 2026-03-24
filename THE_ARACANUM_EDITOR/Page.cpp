#include "Page.h"

Page::Page(int pageNum, int cols, int lines, int chars)
    : pageNumber(pageNum),
      columnCount(cols),
      linesPerColumn(lines),
      charsPerLine(chars),
      columns(nullptr) {

    this->columns = new Column * [this->columnCount];

    for (int i = 0; i < this->columnCount; i = i + 1) {
        this->columns[i] = new Column(this->linesPerColumn, this->charsPerLine);
    }
}

Page::~Page() {
    for (int i = 0; i < this->columnCount; i = i + 1) {
        if (this->columns[i] != nullptr) {
            delete this->columns[i];
            this->columns[i] = nullptr;
        }
    }

    if (this->columns != nullptr) {
        delete[] this->columns;
        this->columns = nullptr;
    }
}

Page::Page(const Page& copy)
    : pageNumber(copy.pageNumber),
      columnCount(copy.columnCount),
      linesPerColumn(copy.linesPerColumn),
      charsPerLine(copy.charsPerLine),
      columns(nullptr) {

    this->columns = new Column * [this->columnCount];

    for (int i = 0; i < this->columnCount; i = i + 1) {
        this->columns[i] = new Column(this->linesPerColumn, this->charsPerLine);

        Column* otherCol = copy.columns[i];
        for (int j = 0; j < otherCol->getUsedLines(); j = j + 1) {
            Line* otherLine = otherCol->getLine(j);
            if (otherLine != nullptr) {
                this->columns[i]->addText(otherLine->getText());
            }
        }
    }
}

Page& Page::operator=(const Page& copy) {
    if (this == &copy) {
        return *this;
    }

    for (int i = 0; i < this->columnCount; i = i + 1) {
        if (this->columns[i] != nullptr) {
            delete this->columns[i];
        }
    }
    delete[] this->columns;

    this->pageNumber = copy.pageNumber;
    this->columnCount = copy.columnCount;
    this->linesPerColumn = copy.linesPerColumn;
    this->charsPerLine = copy.charsPerLine;

    this->columns = new Column * [this->columnCount];

    for (int i = 0; i < this->columnCount; i = i + 1) {
        this->columns[i] = new Column(this->linesPerColumn, this->charsPerLine);

        Column* otherCol = copy.columns[i];
        for (int j = 0; j < otherCol->getUsedLines(); j = j + 1) {
            Line* otherLine = otherCol->getLine(j);
            if (otherLine != nullptr) {
                this->columns[i]->addText(otherLine->getText());
            }
        }
    }

    return *this;
}

bool Page::addText(const wchar_t* text) {
    for (int i = 0; i < this->columnCount; i = i + 1) {
        if (!this->columns[i]->isFull()) {
            if (this->columns[i]->addText(text)) {
                return true;
            }
        }
    }
    return false;
}

Column* Page::getColumn(int index) {
    if (index >= 0 && index < this->columnCount) {
        return this->columns[index];
    }
    return nullptr;
}

int Page::getPageNumber() const {
    return this->pageNumber;
}

bool Page::isFull() const {
    for (int i = 0; i < this->columnCount; i = i + 1) {
        if (!this->columns[i]->isFull()) {
            return false;
        }
    }
    return true;
}

bool Page::hasContent() const {
    for (int i = 0; i < this->columnCount; i = i + 1) {
        if (this->columns[i]->getUsedLines() > 0) {
            return true;
        }
    }
    return false;
}

const wchar_t* Page::getFirstParagraph() const {
    if (this->columnCount > 0) {
        Column* firstCol = this->columns[0];
        if (firstCol->getUsedLines() > 0) {
            Line* firstLine = firstCol->getLine(0);
            if (firstLine != nullptr) {
                return firstLine->getText();
            }
        }
    }
    return L"";
}

void Page::clear() {
    for (int i = 0; i < this->columnCount; i = i + 1) {
        this->columns[i]->clear();
    }
}

int Page::getColumnCount() const {
    return this->columnCount;
}