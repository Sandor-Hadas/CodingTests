#pragma once
#include <Windows.h>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "BmpReader.h"

class ImageThread {
public:
    ImageThread(BmpReader* reader, const RECT& viewport, HDC backDC, HWND hwnd);
    void start();
    void stop();
    void signalRestart();

private:
    void run();

    BmpReader* reader;
    RECT viewport;
    HDC backDC;
    HWND hwnd;
    std::thread worker;
    std::atomic<bool> running;
    std::atomic<bool> restartRequested;
    std::condition_variable cv;
    std::mutex mtx;

    HDC threadDC;
    HBITMAP threadBitmap;

    static std::atomic<size_t> readThreadCount; // Only for the first start so all threads are ready. Then, well, windows timing is a mistery where even usleep does not guarantee anything
    void renderBmp(BYTE* bmp, const size_t& size);
};
