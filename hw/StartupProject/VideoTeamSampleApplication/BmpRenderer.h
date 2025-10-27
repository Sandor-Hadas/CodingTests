#pragma once
#include <Windows.h>

bool RenderBmpToDC(BYTE* bmpData, size_t size, HDC hdc, const RECT& target);
