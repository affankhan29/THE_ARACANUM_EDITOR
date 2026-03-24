#include "framework.h"
#include "THE_ARACANUM_EDITOR.h"
#include "ManualStringHelpers.h"

// ========================================
// MENU COMMAND IDs
// ========================================
#define ID_FILE_NEW          1001
#define ID_FILE_SAVE         1002
#define ID_FILE_LOAD         1003
#define ID_FILE_EXIT         1004

#define ID_SEARCH_FIND       2001
#define ID_SEARCH_HISTORY    2002

#define ID_ALIGN_LEFT        3001
#define ID_ALIGN_RIGHT       3002
#define ID_ALIGN_CENTER      3003
#define ID_ALIGN_JUSTIFY     3004

#define ID_VIEW_TOC          4001
#define ID_VIEW_STATS        4002
#define ID_VIEW_PREVPAGE     4003
#define ID_VIEW_NEXTPAGE     4004

// ========================================
// GLOBAL VARIABLES
// ========================================
static HINSTANCE g_hInst = nullptr;
static HWND      g_hwnd = nullptr;
static Document* g_doc = nullptr;
static HFONT     g_hFont = nullptr;
static int       g_fontWidth = 10;
static int       g_fontHeight = 16;

static COLORREF g_colorBg = RGB(20, 20, 40);
static COLORREF g_colorText = RGB(220, 220, 220);
static COLORREF g_colorStatusBg = RGB(40, 40, 60);
static COLORREF g_colorStatusText = RGB(200, 200, 200);

// Search state
static wchar_t g_searchTerm[256] = L"";
static bool    g_showingTOC = false;
static bool    g_showingStats = false;
static bool    g_showingHistory = false;
static bool    g_cursorVisible = true;

// Autosave state
#define TIMER_CURSOR   1
#define TIMER_AUTOSAVE 2
#define AUTOSAVE_INTERVAL_MS  60000
#define AUTOSAVE_MAX_COPIES   10
static int  g_autoSaveIndex = 0;
static bool g_autoSaveEnabled = true;

// ========================================
// HELPER: int to wchar_t buffer
// ========================================
static void intToWStr(int value, wchar_t* buf, int bufSize) {
    int temp = value;
    int digits = 0;

    if (temp == 0) {
        digits = 1;
    }
    else {
        while (temp > 0) {
            digits = digits + 1;
            temp = temp / 10;
        }
    }

    if (digits >= bufSize) {
        digits = bufSize - 1;
    }

    buf[digits] = L'\0';
    temp = value;

    for (int i = digits - 1; i >= 0; i = i - 1) {
        buf[i] = L'0' + (temp % 10);
        temp = temp / 10;
    }
}

// ========================================
// MENU BUILDER
// ========================================
static HMENU BuildMenuBar() {
    HMENU hMenuBar = CreateMenu();

    // File menu
    HMENU hFile = CreatePopupMenu();
    AppendMenuW(hFile, MF_STRING, ID_FILE_NEW, L"New\tCtrl+N");
    AppendMenuW(hFile, MF_STRING, ID_FILE_SAVE, L"Save\tCtrl+S");
    AppendMenuW(hFile, MF_STRING, ID_FILE_LOAD, L"Load\tCtrl+O");
    AppendMenuW(hFile, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFile, MF_STRING, ID_FILE_EXIT, L"Exit\tAlt+F4");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");

    // Search menu
    HMENU hSearch = CreatePopupMenu();
    AppendMenuW(hSearch, MF_STRING, ID_SEARCH_FIND, L"Find...\tCtrl+F");
    AppendMenuW(hSearch, MF_STRING, ID_SEARCH_HISTORY, L"Search History\tCtrl+H");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSearch, L"Search");

    // Alignment menu
    HMENU hAlign = CreatePopupMenu();
    AppendMenuW(hAlign, MF_STRING, ID_ALIGN_LEFT, L"Left\tCtrl+L");
    AppendMenuW(hAlign, MF_STRING, ID_ALIGN_RIGHT, L"Right\tCtrl+R");
    AppendMenuW(hAlign, MF_STRING, ID_ALIGN_CENTER, L"Center\tCtrl+E");
    AppendMenuW(hAlign, MF_STRING, ID_ALIGN_JUSTIFY, L"Justify\tCtrl+J");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hAlign, L"Alignment");

    // View menu
    HMENU hView = CreatePopupMenu();
    AppendMenuW(hView, MF_STRING, ID_VIEW_TOC, L"Table of Contents");
    AppendMenuW(hView, MF_STRING, ID_VIEW_STATS, L"Statistics");
    AppendMenuW(hView, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hView, MF_STRING, ID_VIEW_PREVPAGE, L"Previous Page\tCtrl+Left");
    AppendMenuW(hView, MF_STRING, ID_VIEW_NEXTPAGE, L"Next Page\tCtrl+Right");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hView, L"View");

    return hMenuBar;
}

// ========================================
// DIALOG: simple text input (no .rc needed)
// ========================================
static wchar_t g_dialogInput[256] = L"";
static HWND    g_hSearchEdit = nullptr;
static bool    g_searchConfirmed = false;

static LRESULT CALLBACK SearchWndProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        // Edit box
        g_hSearchEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", g_searchTerm,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            10, 15, 260, 24, hDlg, (HMENU)101, g_hInst, nullptr
        );
        // OK button
        CreateWindowW(
            L"BUTTON", L"Search",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            10, 50, 80, 26, hDlg, (HMENU)IDOK, g_hInst, nullptr
        );
        // Cancel button
        CreateWindowW(
            L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            100, 50, 80, 26, hDlg, (HMENU)IDCANCEL, g_hInst, nullptr
        );
        SetFocus(g_hSearchEdit);
        return 0;
    }
    if (msg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK) {
            GetWindowTextW(g_hSearchEdit, g_dialogInput, 255);
            g_searchConfirmed = true;
            DestroyWindow(hDlg);
            return 0;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            g_searchConfirmed = false;
            DestroyWindow(hDlg);
            return 0;
        }
    }
    if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        g_searchConfirmed = false;
        DestroyWindow(hDlg);
        return 0;
    }
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hDlg, msg, wParam, lParam);
}

static bool ShowSearchDialog(HWND hwnd) {
    // Register dialog window class once
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = SearchWndProc;
        wc.hInstance = g_hInst;
        wc.lpszClassName = L"AracanumSearchDlg";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&wc);
        classRegistered = true;
    }

    g_searchConfirmed = false;
    g_dialogInput[0] = L'\0';

    // Get parent window position to center the dialog
    RECT parentRect;
    GetWindowRect(hwnd, &parentRect);
    int dlgW = 300;
    int dlgH = 110;
    int dlgX = parentRect.left + ((parentRect.right - parentRect.left) - dlgW) / 2;
    int dlgY = parentRect.top + ((parentRect.bottom - parentRect.top) - dlgH) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"AracanumSearchDlg",
        L"Find",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        dlgX, dlgY, dlgW, dlgH,
        hwnd, nullptr, g_hInst, nullptr
    );

    if (hDlg == nullptr) return false;

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    // Run a local message loop until the dialog closes
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return g_searchConfirmed;
}

// ========================================
// OVERLAY: TOC, Stats, Search History
// ========================================
static void RenderOverlay(HDC hdc, RECT* clientRect) {
    if (g_doc == nullptr) return;

    if (!g_showingTOC && !g_showingStats && !g_showingHistory) return;

    // Draw semi-transparent overlay box
    int boxLeft = clientRect->right / 4;
    int boxTop = 40;
    int boxRight = clientRect->right - (clientRect->right / 4);
    int boxBottom = clientRect->bottom - 60;

    HBRUSH overlayBrush = CreateSolidBrush(RGB(10, 10, 30));
    RECT   boxRect = { boxLeft, boxTop, boxRight, boxBottom };
    FillRect(hdc, &boxRect, overlayBrush);
    DeleteObject(overlayBrush);

    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 160));
    SelectObject(hdc, borderPen);
    MoveToEx(hdc, boxLeft, boxTop, nullptr);
    LineTo(hdc, boxRight, boxTop);
    LineTo(hdc, boxRight, boxBottom);
    LineTo(hdc, boxLeft, boxBottom);
    LineTo(hdc, boxLeft, boxTop);
    DeleteObject(borderPen);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(220, 220, 220));
    SelectObject(hdc, g_hFont);

    int lineHeight = g_fontHeight;
    int textX = boxLeft + 15;
    int textY = boxTop + 10;

    // ---- TABLE OF CONTENTS ----
    if (g_showingTOC) {
        g_doc->buildTableOfContents();

        RECT titleRect = { textX, textY, boxRight - 10, textY + lineHeight };
        SetTextColor(hdc, RGB(255, 220, 100));
        DrawTextW(hdc, L"TABLE OF CONTENTS", -1, &titleRect, DT_LEFT | DT_SINGLELINE);
        SetTextColor(hdc, RGB(100, 100, 140));

        // Separator line
        textY = textY + lineHeight + 4;
        MoveToEx(hdc, textX, textY, nullptr);
        LineTo(hdc, boxRight - 10, textY);
        textY = textY + 8;

        SetTextColor(hdc, RGB(220, 220, 220));

        int tocCount = g_doc->getTOCCount();
        if (tocCount == 0) {
            RECT emptyRect = { textX, textY, boxRight - 10, textY + lineHeight };
            DrawTextW(hdc, L"No entries found.", -1, &emptyRect, DT_LEFT | DT_SINGLELINE);
        }
        else {
            for (int i = 0; i < tocCount; i = i + 1) {
                if (textY + lineHeight > boxBottom - 10) break;

                int          pageNum = 0;
                const wchar_t* entry = g_doc->getTOCEntry(i, pageNum);
                if (entry == nullptr) continue;

                // Build index number
                wchar_t indexBuf[8];
                intToWStr(i + 1, indexBuf, 8);

                // Build page number
                wchar_t pageBuf[8];
                intToWStr(pageNum, pageBuf, 8);

                // Compose line: "1. Entry ............ Page X"
                wchar_t lineBuf[256];
                str_cpy(lineBuf, indexBuf);
                str_cpy(lineBuf + str_len(lineBuf), L". ");
                str_cpy(lineBuf + str_len(lineBuf), entry);
                str_cpy(lineBuf + str_len(lineBuf), L" ... Page ");
                str_cpy(lineBuf + str_len(lineBuf), pageBuf);

                RECT lineRect = { textX, textY, boxRight - 10, textY + lineHeight };
                DrawTextW(hdc, lineBuf, str_len(lineBuf), &lineRect, DT_LEFT | DT_SINGLELINE);
                textY = textY + lineHeight + 2;
            }
        }

        textY = textY + 10;
        SetTextColor(hdc, RGB(150, 150, 150));
        RECT hintRect = { textX, boxBottom - lineHeight - 8, boxRight - 10, boxBottom - 8 };
        DrawTextW(hdc, L"Press Escape to close", -1, &hintRect, DT_LEFT | DT_SINGLELINE);
    }

    // ---- STATISTICS ----
    if (g_showingStats) {
        RECT titleRect = { textX, textY, boxRight - 10, textY + lineHeight };
        SetTextColor(hdc, RGB(255, 220, 100));
        DrawTextW(hdc, L"DOCUMENT STATISTICS", -1, &titleRect, DT_LEFT | DT_SINGLELINE);

        textY = textY + lineHeight + 4;
        HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 140));
        SelectObject(hdc, sepPen);
        MoveToEx(hdc, textX, textY, nullptr);
        LineTo(hdc, boxRight - 10, textY);
        DeleteObject(sepPen);
        textY = textY + 8;

        SetTextColor(hdc, RGB(220, 220, 220));

        // Helper to draw a stat row
        auto drawStat = [&](const wchar_t* label, int value) {
            if (textY + lineHeight > boxBottom - 20) return;
            wchar_t valBuf[16];
            intToWStr(value, valBuf, 16);
            wchar_t rowBuf[128];
            str_cpy(rowBuf, label);
            str_cpy(rowBuf + str_len(rowBuf), valBuf);
            RECT rowRect = { textX, textY, boxRight - 10, textY + lineHeight };
            DrawTextW(hdc, rowBuf, str_len(rowBuf), &rowRect, DT_LEFT | DT_SINGLELINE);
            textY = textY + lineHeight + 4;
            };

        drawStat(L"Pages:                   ", g_doc->getPageCount());
        drawStat(L"Words:                   ", g_doc->countWords());
        drawStat(L"Characters (with spaces):  ", g_doc->countChars(true));
        drawStat(L"Characters (no spaces):    ", g_doc->countChars(false));
        drawStat(L"Sentences:               ", g_doc->countSentences());
        drawStat(L"Est. reading time (min): ", g_doc->estimateReadingTime());

        // Alignment mode
        textY = textY + 4;
        const wchar_t* alignNames[4] = { L"Left", L"Right", L"Center", L"Justify" };
        int            alignMode = g_doc->getAlignment();
        if (alignMode < 0 || alignMode > 3) alignMode = 0;

        wchar_t alignRow[64];
        str_cpy(alignRow, L"Alignment:               ");
        str_cpy(alignRow + str_len(alignRow), alignNames[alignMode]);
        RECT alignRect = { textX, textY, boxRight - 10, textY + lineHeight };
        DrawTextW(hdc, alignRow, str_len(alignRow), &alignRect, DT_LEFT | DT_SINGLELINE);

        SetTextColor(hdc, RGB(150, 150, 150));
        RECT hintRect = { textX, boxBottom - lineHeight - 8, boxRight - 10, boxBottom - 8 };
        DrawTextW(hdc, L"Press Escape to close", -1, &hintRect, DT_LEFT | DT_SINGLELINE);
    }

    // ---- SEARCH HISTORY ----
    if (g_showingHistory) {
        RECT titleRect = { textX, textY, boxRight - 10, textY + lineHeight };
        SetTextColor(hdc, RGB(255, 220, 100));
        DrawTextW(hdc, L"SEARCH HISTORY", -1, &titleRect, DT_LEFT | DT_SINGLELINE);

        textY = textY + lineHeight + 4;
        HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 140));
        SelectObject(hdc, sepPen);
        MoveToEx(hdc, textX, textY, nullptr);
        LineTo(hdc, boxRight - 10, textY);
        DeleteObject(sepPen);
        textY = textY + 8;

        SetTextColor(hdc, RGB(220, 220, 220));

        int histCount = g_doc->getSearchHistoryCount();
        if (histCount == 0) {
            RECT emptyRect = { textX, textY, boxRight - 10, textY + lineHeight };
            DrawTextW(hdc, L"No searches yet.", -1, &emptyRect, DT_LEFT | DT_SINGLELINE);
        }
        else {
            for (int i = 0; i < histCount; i = i + 1) {
                if (textY + lineHeight > boxBottom - 20) break;

                wchar_t* term = g_doc->getSearchHistory(i);
                int      matches = g_doc->getSearchMatchCount(i);

                if (term == nullptr) continue;

                wchar_t matchBuf[16];
                intToWStr(matches, matchBuf, 16);

                wchar_t rowBuf[256];
                str_cpy(rowBuf, L"\"");
                str_cpy(rowBuf + str_len(rowBuf), term);
                str_cpy(rowBuf + str_len(rowBuf), L"\"  -  ");
                str_cpy(rowBuf + str_len(rowBuf), matchBuf);
                str_cpy(rowBuf + str_len(rowBuf), L" match(es)");

                RECT rowRect = { textX, textY, boxRight - 10, textY + lineHeight };
                DrawTextW(hdc, rowBuf, str_len(rowBuf), &rowRect, DT_LEFT | DT_SINGLELINE);
                textY = textY + lineHeight + 4;
            }
        }

        SetTextColor(hdc, RGB(150, 150, 150));
        RECT hintRect = { textX, boxBottom - lineHeight - 8, boxRight - 10, boxBottom - 8 };
        DrawTextW(hdc, L"Press Escape to close", -1, &hintRect, DT_LEFT | DT_SINGLELINE);
    }
}

// ========================================
// HANDLE MENU COMMANDS
// ========================================
static void HandleMenuCommand(HWND hwnd, WORD commandId) {
    switch (commandId) {

    case ID_FILE_NEW:
        if (g_doc != nullptr) {
            delete g_doc;
        }
        g_doc = new Document(30, 60, 2);
        InvalidateRect(hwnd, nullptr, TRUE);
        break;

    case ID_FILE_SAVE:
        if (g_doc != nullptr) {
            if (g_doc->saveToFile(L"document.arc")) {
                g_doc->setHasChanges(false);
                // Reset autosave index on manual save so rotation starts fresh
                g_autoSaveIndex = 0;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        break;

    case ID_FILE_LOAD:
        if (g_doc != nullptr) {
            g_doc->loadFromFile(L"document.arc");
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;

    case ID_FILE_EXIT:
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
        break;

    case ID_SEARCH_FIND:
        if (g_doc != nullptr) {
            g_dialogInput[0] = L'\0';
            if (ShowSearchDialog(hwnd)) {
                str_cpy(g_searchTerm, g_dialogInput);
                g_doc->search(g_searchTerm);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        break;

    case ID_SEARCH_HISTORY:
        g_showingTOC = false;
        g_showingStats = false;
        g_showingHistory = !g_showingHistory;
        InvalidateRect(hwnd, nullptr, TRUE);
        break;

    case ID_ALIGN_LEFT:
        if (g_doc != nullptr) g_doc->setAlignment(0);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case ID_ALIGN_RIGHT:
        if (g_doc != nullptr) g_doc->setAlignment(1);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case ID_ALIGN_CENTER:
        if (g_doc != nullptr) g_doc->setAlignment(2);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case ID_ALIGN_JUSTIFY:
        if (g_doc != nullptr) g_doc->setAlignment(3);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case ID_VIEW_TOC:
        g_showingHistory = false;
        g_showingStats = false;
        g_showingTOC = !g_showingTOC;
        InvalidateRect(hwnd, nullptr, TRUE);
        break;

    case ID_VIEW_STATS:
        g_showingHistory = false;
        g_showingTOC = false;
        g_showingStats = !g_showingStats;
        InvalidateRect(hwnd, nullptr, TRUE);
        break;

    case ID_VIEW_PREVPAGE:
        if (g_doc != nullptr) {
            int pg = g_doc->getCurrentPage();
            if (pg > 0) g_doc->goToPage(pg);
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;

    case ID_VIEW_NEXTPAGE:
        if (g_doc != nullptr) {
            int pg = g_doc->getCurrentPage();
            g_doc->goToPage(pg + 2);
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;
    }
}

// ========================================
// FONT
// ========================================
void InitializeFont() {
    g_hFont = CreateFontW(
        g_fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
    );

    HDC hdc = GetDC(nullptr);
    SelectObject(hdc, g_hFont);
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    g_fontWidth = tm.tmAveCharWidth;
    g_fontHeight = tm.tmHeight;
    ReleaseDC(nullptr, hdc);
}

// ========================================
// CLEANUP
// ========================================
void CleanupResources() {
    if (g_doc != nullptr) {
        delete g_doc;
        g_doc = nullptr;
    }
    if (g_hFont != nullptr) {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
}

// ========================================
// RENDER DOCUMENT
// ========================================
// ========================================
// HIGHLIGHT HELPER
// Draws a single line then overlays highlight boxes on any matches
// ========================================
static void DrawLineWithHighlight(HDC hdc, const wchar_t* text, int len,
    RECT rc, UINT dtFlags) {
    // Draw the line normally first
    DrawTextW(hdc, text, len, &rc, dtFlags | DT_SINGLELINE | DT_NOPREFIX);

    // No search term — nothing to highlight
    if (g_searchTerm[0] == L'\0') return;

    int termLen = str_len(g_searchTerm);
    if (termLen == 0 || termLen > len) return;

    // Make lowercase copies for case-insensitive comparison
    wchar_t lineLower[512];
    wchar_t termLower[256];

    int copyLen = len;
    if (copyLen > 511) copyLen = 511;
    for (int i = 0; i < copyLen; i = i + 1) {
        wchar_t ch = text[i];
        if (ch >= L'A' && ch <= L'Z') ch = ch + 32;
        lineLower[i] = ch;
    }
    lineLower[copyLen] = L'\0';

    int tLen = termLen;
    if (tLen > 255) tLen = 255;
    for (int i = 0; i < tLen; i = i + 1) {
        wchar_t ch = g_searchTerm[i];
        if (ch >= L'A' && ch <= L'Z') ch = ch + 32;
        termLower[i] = ch;
    }
    termLower[tLen] = L'\0';

    // Scan for matches and draw highlight box + redrawn text for each
    for (int pos = 0; pos <= copyLen - tLen; pos = pos + 1) {
        bool found = true;
        for (int k = 0; k < tLen; k = k + 1) {
            if (lineLower[pos + k] != termLower[k]) {
                found = false;
                break;
            }
        }

        if (found) {
            // Measure pixel offset to match start
            SIZE beforeSize;
            beforeSize.cx = 0;
            if (pos > 0) {
                GetTextExtentPoint32W(hdc, text, pos, &beforeSize);
            }

            // Measure width of the matched text
            SIZE matchSize;
            GetTextExtentPoint32W(hdc, text + pos, tLen, &matchSize);

            int highlightX = rc.left + beforeSize.cx;

            // Draw highlight background
            RECT highlightRect;
            highlightRect.left = highlightX;
            highlightRect.top = rc.top;
            highlightRect.right = highlightX + matchSize.cx;
            highlightRect.bottom = rc.bottom;

            HBRUSH highlightBrush = CreateSolidBrush(RGB(180, 140, 0));
            FillRect(hdc, &highlightRect, highlightBrush);
            DeleteObject(highlightBrush);

            // Redraw matched text in dark colour on top of highlight
            SetTextColor(hdc, RGB(10, 10, 10));
            SetBkMode(hdc, TRANSPARENT);
            DrawTextW(hdc, text + pos, tLen, &highlightRect,
                DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            SetTextColor(hdc, RGB(220, 220, 220));
        }
    }
}

void RenderDocument(HDC hdc) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, g_colorText);
    SelectObject(hdc, g_hFont);

    if (g_doc == nullptr) return;

    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);

    int linesPerCol = g_doc->getLinesPerColumn();
    int colsPerPage = g_doc->getColumnsPerPage();
    int lineHeight = g_fontHeight;

    int availableWidth = clientRect.right - clientRect.left;
    int colWidth = g_doc->getCharsPerLine() * g_fontWidth;
    int gap = 40;

    int totalContentWidth = (colWidth * colsPerPage) + (gap * (colsPerPage - 1));
    int startX = (availableWidth - totalContentWidth) / 2;
    if (startX < 20) startX = 20;

    int startY = 20;

    int pageCount = g_doc->getPageCount();
    int currentPageIdx = g_doc->getCurrentPage();

    if (currentPageIdx < 0 || currentPageIdx >= pageCount) return;

    Page* page = g_doc->getPage(currentPageIdx);
    if (page == nullptr) return;

    for (int col = 0; col < colsPerPage; col = col + 1) {
        Column* column = page->getColumn(col);
        if (column == nullptr) continue;

        int colX = startX + (col * (colWidth + gap));
        int colY = startY;

        int usedLines = column->getUsedLines();
        if (usedLines > linesPerCol) usedLines = linesPerCol;

        int alignMode = g_doc->getAlignment();

        for (int line = 0; line < usedLines; line = line + 1) {
            Line* ln = column->getLine(line);
            if (ln == nullptr) continue;

            const wchar_t* text = ln->getText();
            int            len = str_len(text);
            if (len > 500) len = 500;

            RECT rc;
            rc.left = colX;
            rc.top = colY + (line * lineHeight);
            rc.right = colX + colWidth;
            rc.bottom = rc.top + lineHeight;

            if (alignMode == 1) {
                DrawLineWithHighlight(hdc, text, len, rc, DT_RIGHT);
            }
            else if (alignMode == 2) {
                DrawLineWithHighlight(hdc, text, len, rc, DT_CENTER);
            }
            else if (alignMode == 3) {
                bool isLastLine = (line == usedLines - 1);
                int  spaceCount = 0;
                for (int si = 0; si < len; si = si + 1) {
                    if (text[si] == L' ') spaceCount = spaceCount + 1;
                }
                if (isLastLine || spaceCount == 0) {
                    DrawLineWithHighlight(hdc, text, len, rc, DT_LEFT);
                }
                else {
                    SIZE totalSize;
                    GetTextExtentPoint32W(hdc, text, len, &totalSize);
                    int extraPixels = (rc.right - rc.left) - totalSize.cx;
                    int extraPerSpace = 0;
                    if (extraPixels > 0 && extraPixels < colWidth) {
                        extraPerSpace = extraPixels / spaceCount;
                    }
                    int     drawX = rc.left;
                    int     wordStart = 0;
                    wchar_t wordBuf[256];
                    for (int si = 0; si <= len; si = si + 1) {
                        if (si == len || text[si] == L' ') {
                            int wordLen = si - wordStart;
                            if (wordLen > 0) {
                                for (int wi = 0; wi < wordLen; wi = wi + 1) {
                                    wordBuf[wi] = text[wordStart + wi];
                                }
                                wordBuf[wordLen] = L'\0';
                                RECT wordRc = rc;
                                wordRc.left = drawX;
                                DrawLineWithHighlight(hdc, wordBuf, wordLen, wordRc, DT_LEFT);
                                SIZE wordSize;
                                GetTextExtentPoint32W(hdc, wordBuf, wordLen, &wordSize);
                                drawX = drawX + wordSize.cx + g_fontWidth + extraPerSpace;
                            }
                            wordStart = si + 1;
                        }
                    }
                }
            }
            else {
                DrawLineWithHighlight(hdc, text, len, rc, DT_LEFT);
            }
        }

        // Draw cursor on the active column
        if (col == g_doc->getCurrentColumn() && g_cursorVisible) {
            int curLine = g_doc->getCurrentLine();
            int curChar = g_doc->getCurrentChar();

            int cursorX = colX;
            int cursorY = startY + (curLine * lineHeight);

            Line* curLn = column->getLine(curLine);
            if (curLn != nullptr) {
                const wchar_t* lineText = curLn->getText();
                int            lineLen = str_len(lineText);
                int            measureLen = curChar;
                if (measureLen > lineLen) measureLen = lineLen;

                if (measureLen > 0) {
                    SIZE sz;
                    GetTextExtentPoint32W(hdc, lineText, measureLen, &sz);
                    cursorX = colX + sz.cx;
                }
            }

            HPEN cursorPen = CreatePen(PS_SOLID, 2, RGB(220, 220, 220));
            SelectObject(hdc, cursorPen);
            MoveToEx(hdc, cursorX, cursorY, nullptr);
            LineTo(hdc, cursorX, cursorY + lineHeight);
            DeleteObject(cursorPen);
        }

        // Column border
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 100));
        SelectObject(hdc, pen);
        int pageHeight = (linesPerCol * lineHeight);

        MoveToEx(hdc, colX - 5, startY - 5, nullptr);
        LineTo(hdc, colX + colWidth + 5, startY - 5);
        LineTo(hdc, colX + colWidth + 5, startY + pageHeight);
        LineTo(hdc, colX - 5, startY + pageHeight);
        LineTo(hdc, colX - 5, startY - 5);
        DeleteObject(pen);

        // Page footer
        wchar_t pageNumBuf[16];
        intToWStr(page->getPageNumber(), pageNumBuf, 16);

        wchar_t footerBuf[32];
        str_cpy(footerBuf, L"\x97 ");
        str_cpy(footerBuf + str_len(footerBuf), pageNumBuf);
        str_cpy(footerBuf + str_len(footerBuf), L" \x97");

        RECT footerRect;
        footerRect.left = colX;
        footerRect.top = startY + pageHeight + 10;
        footerRect.right = colX + colWidth;
        footerRect.bottom = footerRect.top + lineHeight;

        SetTextColor(hdc, RGB(150, 150, 150));
        DrawTextW(hdc, footerBuf, str_len(footerBuf), &footerRect, DT_CENTER | DT_SINGLELINE);
        SetTextColor(hdc, g_colorText);
    }
}

// ========================================
// RENDER STATUS BAR
// ========================================
void RenderStatusBar(HDC hdc, RECT* clientRect) {
    int  statusHeight = 30;
    RECT statusRect;
    statusRect.left = 0;
    statusRect.top = clientRect->bottom - statusHeight;
    statusRect.right = clientRect->right;
    statusRect.bottom = clientRect->bottom;

    HBRUSH brush = CreateSolidBrush(g_colorStatusBg);
    FillRect(hdc, &statusRect, brush);
    DeleteObject(brush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, g_colorStatusText);
    SelectObject(hdc, g_hFont);

    wchar_t statusBuf[512] = L"";

    if (g_doc != nullptr) {
        wchar_t numBuf[16];

        str_cpy(statusBuf, L"Page: ");
        intToWStr(g_doc->getCurrentPage() + 1, numBuf, 16);
        str_cpy(statusBuf + str_len(statusBuf), numBuf);

        str_cpy(statusBuf + str_len(statusBuf), L"  Line: ");
        intToWStr(g_doc->getCurrentLine() + 1, numBuf, 16);
        str_cpy(statusBuf + str_len(statusBuf), numBuf);

        str_cpy(statusBuf + str_len(statusBuf), L"  Words: ");
        intToWStr(g_doc->countWords(), numBuf, 16);
        str_cpy(statusBuf + str_len(statusBuf), numBuf);

        str_cpy(statusBuf + str_len(statusBuf), L"  Chars: ");
        intToWStr(g_doc->countChars(true), numBuf, 16);
        str_cpy(statusBuf + str_len(statusBuf), numBuf);

        // Alignment indicator
        const wchar_t* alignLabels[4] = { L"[Left]", L"[Right]", L"[Center]", L"[Justify]" };
        int alignMode = g_doc->getAlignment();
        if (alignMode >= 0 && alignMode <= 3) {
            str_cpy(statusBuf + str_len(statusBuf), L"  ");
            str_cpy(statusBuf + str_len(statusBuf), alignLabels[alignMode]);
        }

        if (g_doc->getHasChanges()) {
            str_cpy(statusBuf + str_len(statusBuf), L"  [*]");
        }

        // Active search term
        if (g_searchTerm[0] != L'\0') {
            str_cpy(statusBuf + str_len(statusBuf), L"  Search: \"");
            str_cpy(statusBuf + str_len(statusBuf), g_searchTerm);
            str_cpy(statusBuf + str_len(statusBuf), L"\"");
        }
    }
    else {
        str_cpy(statusBuf, L"No Document Open");
    }

    DrawTextW(hdc, statusBuf, str_len(statusBuf), &statusRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

// ========================================
// HANDLE KEYBOARD INPUT
// ========================================
void HandleInput(WPARAM wParam) {
    if (g_doc == nullptr) return;

    if (wParam == VK_BACK) {
        g_doc->backspace();
    }
    else if (wParam == VK_DELETE) {
        g_doc->deleteChar();
    }
    else if (wParam == VK_LEFT) {
        g_doc->moveCursor(-1, 0);
    }
    else if (wParam == VK_RIGHT) {
        g_doc->moveCursor(1, 0);
    }
    else if (wParam == VK_UP) {
        g_doc->moveCursor(0, -1);
    }
    else if (wParam == VK_DOWN) {
        g_doc->moveCursor(0, 1);
    }
    else if (wParam == VK_RETURN) {
        g_doc->handleNewLine();
    }
    else if (wParam == VK_ESCAPE) {
        // Close any open overlay
        g_showingTOC = false;
        g_showingStats = false;
        g_showingHistory = false;
    }
}

// ========================================
// AUTO SAVE
// ========================================
static void PerformAutoSave() {
    if (g_doc == nullptr) return;
    if (!g_autoSaveEnabled) return;

    // Build filename: autosave_00.arc ... autosave_09.arc
    wchar_t path[64];
    path[0] = L'\0';

    wchar_t prefix[] = L"autosave_";
    for (int i = 0; prefix[i] != L'\0'; i = i + 1) {
        path[i] = prefix[i];
        path[i + 1] = L'\0';
    }

    int  len = str_len(path);
    int  idx = g_autoSaveIndex;

    // Two digit index: 00 - 09
    path[len] = L'0' + (idx / 10);
    path[len + 1] = L'0' + (idx % 10);
    path[len + 2] = L'.';
    path[len + 3] = L'a';
    path[len + 4] = L'r';
    path[len + 5] = L'c';
    path[len + 6] = L'\0';

    g_doc->saveToFile(path);

    // Advance index and wrap at max copies
    g_autoSaveIndex = g_autoSaveIndex + 1;
    if (g_autoSaveIndex >= AUTOSAVE_MAX_COPIES) {
        g_autoSaveIndex = 0;
    }

    // Restore hasChanges flag since autosave should not clear the [*] indicator
    g_doc->setHasChanges(true);
}

// ========================================
// WINDOW PROCEDURE
// ========================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE:
        InitializeFont();
        g_doc = new Document(30, 60, 2);
        SetMenu(hwnd, BuildMenuBar());
        SetTimer(hwnd, TIMER_CURSOR, 500, nullptr);
        SetTimer(hwnd, TIMER_AUTOSAVE, AUTOSAVE_INTERVAL_MS, nullptr);
        return 0;

    case WM_COMMAND:
        HandleMenuCommand(hwnd, LOWORD(wParam));
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         hdc = BeginPaint(hwnd, &ps);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        HBRUSH bgBrush = CreateSolidBrush(g_colorBg);
        FillRect(hdc, &clientRect, bgBrush);
        DeleteObject(bgBrush);

        RenderDocument(hdc);
        RenderOverlay(hdc, &clientRect);
        RenderStatusBar(hdc, &clientRect);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CHAR:
        // Block input when overlay is open
        if (g_showingTOC || g_showingStats || g_showingHistory) return 0;
        // Block all Ctrl+letter combinations (they are handled in WM_KEYDOWN)
        if (GetKeyState(VK_CONTROL) & 0x8000) return 0;
        if (wParam >= 32 && wParam != 127) {
            g_cursorVisible = true;
            g_doc->insertCharacter((wchar_t)wParam);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_KEYDOWN: {
        bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        g_cursorVisible = true;

        // Ctrl shortcuts
        if (ctrlDown) {
            if (wParam == 'N') { HandleMenuCommand(hwnd, ID_FILE_NEW);       return 0; }
            if (wParam == 'S') { HandleMenuCommand(hwnd, ID_FILE_SAVE);      return 0; }
            if (wParam == 'O') { HandleMenuCommand(hwnd, ID_FILE_LOAD);      return 0; }
            if (wParam == 'F') { HandleMenuCommand(hwnd, ID_SEARCH_FIND);    return 0; }
            if (wParam == 'H') { HandleMenuCommand(hwnd, ID_SEARCH_HISTORY); return 0; }
            if (wParam == 'L') { HandleMenuCommand(hwnd, ID_ALIGN_LEFT);     return 0; }
            if (wParam == 'R') { HandleMenuCommand(hwnd, ID_ALIGN_RIGHT);    return 0; }
            if (wParam == 'E') { HandleMenuCommand(hwnd, ID_ALIGN_CENTER);   return 0; }
            if (wParam == 'J') { HandleMenuCommand(hwnd, ID_ALIGN_JUSTIFY);  return 0; }
            if (wParam == VK_LEFT) { HandleMenuCommand(hwnd, ID_VIEW_PREVPAGE); return 0; }
            if (wParam == VK_RIGHT) { HandleMenuCommand(hwnd, ID_VIEW_NEXTPAGE); return 0; }
        }

        HandleInput(wParam);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_TIMER:
        if (wParam == TIMER_CURSOR) {
            g_cursorVisible = !g_cursorVisible;
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        else if (wParam == TIMER_AUTOSAVE) {
            PerformAutoSave();
        }
        return 0;

    case WM_LBUTTONDOWN: {
        if (g_doc == nullptr) return 0;

        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        // Recalculate layout geometry (must match RenderDocument exactly)
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        int colsPerPage = g_doc->getColumnsPerPage();
        int linesPerCol = g_doc->getLinesPerColumn();
        int lineHeight = g_fontHeight;
        int colWidth = g_doc->getCharsPerLine() * g_fontWidth;
        int gap = 40;
        int availableWidth = clientRect.right - clientRect.left;

        int totalContentWidth = (colWidth * colsPerPage) + (gap * (colsPerPage - 1));
        int startX = (availableWidth - totalContentWidth) / 2;
        if (startX < 20) startX = 20;
        int startY = 20;

        int   currentPageIdx = g_doc->getCurrentPage();
        Page* page = g_doc->getPage(currentPageIdx);
        if (page == nullptr) return 0;

        for (int col = 0; col < colsPerPage; col = col + 1) {
            int colX = startX + (col * (colWidth + gap));

            if (mouseX >= colX && mouseX <= colX + colWidth &&
                mouseY >= startY && mouseY <= startY + (linesPerCol * lineHeight)) {

                int clickedLine = (mouseY - startY) / lineHeight;

                Column* column = page->getColumn(col);
                if (column == nullptr) continue;

                int usedLines = column->getUsedLines();
                if (clickedLine >= usedLines) clickedLine = usedLines - 1;
                if (clickedLine < 0)          clickedLine = 0;

                // Find closest character position by measuring text
                int clickedChar = 0;
                int clickOffsetX = mouseX - colX;

                Line* ln = column->getLine(clickedLine);
                if (ln != nullptr) {
                    const wchar_t* text = ln->getText();
                    int            lineLen = str_len(text);

                    HDC hdc = GetDC(hwnd);
                    SelectObject(hdc, g_hFont);

                    int bestDist = clickOffsetX;

                    for (int c = 1; c <= lineLen; c = c + 1) {
                        SIZE sz;
                        GetTextExtentPoint32W(hdc, text, c, &sz);
                        int dist = sz.cx - clickOffsetX;
                        if (dist < 0) dist = -dist;
                        if (dist < bestDist) {
                            bestDist = dist;
                            clickedChar = c;
                        }
                    }

                    ReleaseDC(hwnd, hdc);
                }

                g_doc->setCursor(currentPageIdx, col, clickedLine, clickedChar);
                g_cursorVisible = true;
                InvalidateRect(hwnd, nullptr, FALSE);
                break;
            }
        }
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_CURSOR);
        KillTimer(hwnd, TIMER_AUTOSAVE);
        CleanupResources();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ========================================
// WINMAIN
// ========================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;

    const wchar_t CLASS_NAME[] = L"AracanumEditorClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"The Aracanum Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) return 0;

    g_hwnd = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}