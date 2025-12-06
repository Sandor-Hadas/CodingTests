#include <cstdio>
#include <Windows.h>

#include "BmpRenderer.h"

bool _RenderBmpToDC(BYTE* bmpData, size_t size, HDC hdc, const RECT& target) {
    if (!bmpData || size < sizeof(BITMAPINFOHEADER)) {
        printf("Invalid BMP data: null or too small\n");
        return false;
    }

    BITMAPINFOHEADER* infoHeader = reinterpret_cast<BITMAPINFOHEADER*>(bmpData);
    BYTE* pixelData = bmpData + sizeof(BITMAPINFOHEADER);

    if (infoHeader->biSize != sizeof(BITMAPINFOHEADER)) {
        printf("Unexpected BMP header size: %u\n", infoHeader->biSize);
        return false;
    }

    if (infoHeader->biWidth <= 0 || infoHeader->biHeight == 0) {
        printf("Invalid BMP dimensions: width=%d height=%d\n", infoHeader->biWidth, infoHeader->biHeight);
        return false;
    }

    BITMAPINFO bmpInfo = {};
    memcpy(&bmpInfo.bmiHeader, infoHeader, sizeof(BITMAPINFOHEADER));

    int result = StretchDIBits(
        hdc,
        target.left, target.top,
        target.right - target.left, target.bottom - target.top,
        0, 0,
        infoHeader->biWidth, abs(infoHeader->biHeight),
        pixelData,
        &bmpInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    if (result == GDI_ERROR) {
        printf("StretchDIBits failed\n");
        return false;
    }

    return true;
}

bool RenderBmpToDC(BYTE* bmpData, size_t size, HDC hdc, const RECT& target) {
    printf("RenderBmpToDC size: %lld\n", size);
    if (!bmpData || size < sizeof(BITMAPINFOHEADER)) {
        printf("Invalid BMP data: null or too small\n");
        return false;
    }

    BITMAPINFOHEADER* infoHeader = nullptr;
    BYTE* pixelData = nullptr;

    // Check for 'BM' signature — full BMP file
    if (size >= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
        BITMAPFILEHEADER* fileHeader = reinterpret_cast<BITMAPFILEHEADER*>(bmpData);
        if (fileHeader->bfType == 0x4D42) { // 'BM'
            infoHeader = reinterpret_cast<BITMAPINFOHEADER*>(bmpData + sizeof(BITMAPFILEHEADER));
            if (fileHeader->bfOffBits >= size) {
                printf("Invalid bfOffBits: beyond buffer size\n");
                return false;
            }
            pixelData = bmpData + fileHeader->bfOffBits;
        }
    }

    // Fallback: assume raw DIB (no BITMAPFILEHEADER)
    if (!infoHeader) {
        infoHeader = reinterpret_cast<BITMAPINFOHEADER*>(bmpData);
        pixelData = bmpData + sizeof(BITMAPINFOHEADER);
    }

    if (size < (size_t)(pixelData - bmpData)) {
        printf("Invalid BMP: header exceeds buffer size\n");
        return false;
    }

    // Validate dimensions
    if (infoHeader->biWidth <= 0 || infoHeader->biHeight == 0) {
        printf("Invalid BMP dimensions: width=%d height=%d\n", infoHeader->biWidth, infoHeader->biHeight);
        return false;
    }

    // Prepare BITMAPINFO
    BITMAPINFO bmpInfo = {};
    memcpy(&bmpInfo.bmiHeader, infoHeader, sizeof(BITMAPINFOHEADER));

    // Render
    int result = StretchDIBits(
        hdc,
        target.left, target.top,
        target.right - target.left, target.bottom - target.top,
        0, 0,
        infoHeader->biWidth, abs(infoHeader->biHeight),
        pixelData,
        &bmpInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    if (result == GDI_ERROR) {
        printf("StretchDIBits failed\n");
        return false;
    }

    printf("Target RECT: left=%ld top=%ld right=%ld bottom=%ld\n",
        target.left, target.top, target.right, target.bottom);


    return true;
}
