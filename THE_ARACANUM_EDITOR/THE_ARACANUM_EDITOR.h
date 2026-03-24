#pragma once
#include "resource.h"
#include <windows.h>
#include "Document.h"


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RenderDocument(HDC hdc);
void RenderStatusBar(HDC hdc, RECT* clientRect);
void HandleInput(WPARAM wParam);
void InitializeFont();
void CleanupResources();
