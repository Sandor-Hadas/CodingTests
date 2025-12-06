#include "ImageThread.h"
#include "BmpRenderer.h"

std::atomic <size_t> ImageThread::readThreadCount = 0;

ImageThread::ImageThread(BmpReader* r, const RECT& vp, HDC dc, HWND h)
    : reader(r), viewport(vp), backDC(dc), hwnd(h), running(false), restartRequested(false) {
    LONG viewportWidth = viewport.right - viewport.left;
    LONG viewportHeight = viewport.bottom - viewport.top;

    HDC hdcScreen = GetDC(hwnd);
    threadDC = CreateCompatibleDC(hdcScreen);
    threadBitmap = CreateCompatibleBitmap(hdcScreen, viewportWidth, viewportHeight);
    SelectObject(threadDC, threadBitmap);
    ReleaseDC(hwnd, hdcScreen);
}

void ImageThread::start() {
    running = true;
    worker = std::thread(&ImageThread::run, this);
}

void ImageThread::stop() {
    running = false;
    cv.notify_all();
    if (worker.joinable()) worker.join();
}

void ImageThread::signalRestart() {
    //mtx2.lock();
    restartRequested = true;
    cv.notify_all();
    //mtx2.unlock();
}

void ImageThread::renderBmp(BYTE* bmp, const size_t& size) {
    RECT localViewport = { 0, 0, viewport.right - viewport.left, viewport.bottom - viewport.top };
    RenderBmpToDC(bmp, size, threadDC, localViewport);

    HDC hdcWindow = GetDC(hwnd);
    BitBlt(hdcWindow,
        viewport.left, viewport.top,
        viewport.right - viewport.left,
        viewport.bottom - viewport.top,
        threadDC,
        0, 0,
        SRCCOPY);
    ReleaseDC(hwnd, hdcWindow);
    reader->loadNextImage();
}

void ImageThread::run() {
    size_t size;
    size_t waitTimeMs = reader->getCycleTimeMs();
    unsigned short totalImages = reader->getNumImages();
    if (totalImages < 1) {
        printf("No images to be loaded, thread exit\n");
        return;
    }

    reader->resetLoadedImages();

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    size_t elapsedMs, waitMs;

    bool startup = true;
    BYTE* bmp = NULL;

    readThreadCount.fetch_add(1);
    while (readThreadCount.load() < 4) {
        printf("readThreadCount: %zu\n", readThreadCount.load());
        Sleep(10);
    }

    while (running) {
        QueryPerformanceCounter(&start);

        if (startup) {
            bmp = reader->getFirstImage(size);
            startup = false;
        }

        renderBmp(bmp, size);

        if (restartRequested) {
            reader->resetLoadedImages();
            restartRequested = false;
        }

        QueryPerformanceCounter(&end);

        elapsedMs = static_cast<size_t>((end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart);
        if (elapsedMs >= waitTimeMs) {
            waitMs = 0;
        } else {
            waitMs = waitTimeMs - elapsedMs;
        }

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(waitMs), [&] {
                return restartRequested || !running;
            }
        );

        if (!running) {
            return;
        }

        if (restartRequested) {
            printf("Restart requested.................................................................\n");
            startup = true;
        } else {
            bmp = reader->getNextImage(size);
        }
    }
}
