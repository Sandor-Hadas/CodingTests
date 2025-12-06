#include "BmpReader.h"

BmpReader::BmpReader() {
    hFile = 0;
    hMap = 0;
    pData = 0;
    fileSize = 0;
    currentMappingOffset = 0;
    currentReadingOffset = 0;
    fileHeader.CycleTimeMs = 0;
    fileHeader.NumImages = 0;
}

BmpReader::~BmpReader() {
    close();
}

int BmpReader::init(const std::string& fileName) {
//    printf("sizeofsizet: %lld\n", sizeof(size_t));

    hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed open: %s\n", fileName.c_str());
        return this->FILE_OPEN_ERROR;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        printf("Failed to get file size. Error: %lu\n", GetLastError());
        return this->FILE_OPEN_ERROR_SIZE;
    }

//    printf("File size: %lld bytes\n", size.QuadPart);
    if (size.QuadPart > MAX_FILE_SIZE.QuadPart) {
        printf("File is too big for caching from disk. Max size: %lld bytes, found: %lld bytes\n", MAX_FILE_SIZE.QuadPart, size.QuadPart);
        return this->FILE_OPEN_ERROR_SIZE;
    }
    fileSize = size.QuadPart;

    if (fileSize < sizeof(struct DatFileHeader) + sizeof(struct DatFileContentHeader) + sizeof(struct DatFileContentTrailer) + 1) {
        printf("File too short for minimum file requirements: %s\n", fileName.c_str());
        return this->FILE_TOO_SHORT;
    }

    hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL) {
        printf("Failed to map file: %s\n", fileName.c_str());
        return this->FILE_MAPPING_FAILED_CREATE;
    }

    if (!remapView(0)) {
        printf("Failed to map file: %s\n", fileName.c_str());
        return this->FILE_MAPPING_FAILED;
    }

    return checkFileIntegrity();
}

void BmpReader::close() {
    if (pData) UnmapViewOfFile(pData);
    if (hMap) CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    pData = nullptr;
    hMap = nullptr;
    hFile = INVALID_HANDLE_VALUE;
    fileSize = 0;
}

//
// The file format does not match the specification!
//
BYTE* BmpReader::readNextBmpImage(size_t& outBytes, bool reset) {
    outBytes = 0;

    if (reset) {
        if (currentMappingOffset != TOTAL_FILE_HEADER_SIZE) {
            if (!remapView(TOTAL_FILE_HEADER_SIZE)) {
                printf("remapView failed to the beginning of the file");
                return 0;
            }
        }
        currentReadingOffset = 0;
    }

    // Lets move the view if we expect that it might be needed
    if (fileSize > MAPPED_MEMORY_WINDOW_SIZE) {
        if (currentReadingOffset + MAX_EXPECTED_BMP_SIZE > currentMappingOffset + MAPPED_MEMORY_WINDOW_SIZE) {
            if (!remapView(currentReadingOffset)) {
                printf("Failed to remapView to %lld", currentReadingOffset);
                return 0;
            } else {
                printf("Remapped to %lld", currentMappingOffset);
            }
        }
    }

    if (memcmp(CONTENT_HEADER_TAG, pData + currentReadingOffset, sizeof(CONTENT_HEADER_TAG) != 0)) {
        printf("Content header error, Signature mismatch\n");
        return 0;
    }
//    printf("Content header tag ok\n");

    currentReadingOffset += sizeof(FILE_HEADER_TAG) + 3; // Trailing 0
    size_t size1 = *reinterpret_cast<size_t*>(pData + currentReadingOffset);
    currentReadingOffset += sizeof(size_t);
//    printf("BMP length1: %lld\n", size1);

    BYTE* ret = (BYTE*)(pData + currentReadingOffset);
    currentReadingOffset += size1;

    //char dbg[16] = { 0 };
    //strncpy_s(dbg, (char*)pData + currentReadingOffset, 15);
    //printf("dbg: %s\n", dbg);

    if (memcmp(CONTENT_TRAILER_TAG, pData + currentReadingOffset, sizeof(CONTENT_TRAILER_TAG) != 0)) {
        printf("Content header error, Signature mismatch\n");
        return 0;
    }
//    printf("Content trailer tag ok\n");

    currentReadingOffset += sizeof(FILE_HEADER_TAG) + 3; // Trailing 0
    size_t size2 = *reinterpret_cast<size_t*>(pData + currentReadingOffset);
    currentReadingOffset += sizeof(size_t);
//    printf("BMP length2: %lld\n", size2);

    if (size1 != size2) {
        printf("Content header and trailer mismatch\n");
        return 0;
    }

//    printf("currentReadingOffset: %lld, fileSize: %lld currentMappingOffset:%lld\n", currentReadingOffset, fileSize, currentMappingOffset);
    if (currentMappingOffset + currentReadingOffset == fileSize)
    {
        if (!remapView(TOTAL_FILE_HEADER_SIZE)) {
            printf("remapView failed\n");
            return 0;
        }
 
    }

    outBytes = size1;
    return ret;
}

int BmpReader::checkFileIntegrity() {
    if (memcmp(FILE_HEADER_TAG, pData, sizeof(FILE_HEADER_TAG) != 0)) {
        printf("DatFileHeader error, Signature mismatch\n");
        return this->FILE_HEADER_TAG_ERROR;
    }
    printf("Header tag ok\n");

    currentReadingOffset += sizeof(FILE_HEADER_TAG) + 1; // Trailing 0

    fileHeader.NumImages = *reinterpret_cast<unsigned short*>(pData + currentReadingOffset);
    currentReadingOffset += sizeof(unsigned short);

    fileHeader.CycleTimeMs = *reinterpret_cast<unsigned short*>(pData + currentReadingOffset);
    currentReadingOffset += sizeof(unsigned short);

    printf("num images: %hu\n", fileHeader.NumImages);
    printf("cycle time ms: %hu\n", fileHeader.CycleTimeMs);

    // Test
    //for (int j =0; j < 3; j++) //debug
    size_t bytes;
    for (size_t i = 0; i < fileHeader.NumImages; i++) {
        BYTE* pTmp = readNextBmpImage(bytes);
        if (pTmp == 0) {
            printf("Failed to read image #%lld image\n", i);
            return this->BMP_ERROR;
        }
    }

    // If we are here, it is good to go
    // Cache the first image
    BYTE* pTmp = readNextBmpImage(bytes);

    firstImage.resize(bytes);
    std::memcpy(firstImage.data(), pTmp, bytes);

    return this->OK;
}

bool BmpReader::remapView(size_t offset) {
    if (offset >= fileSize) {
        return false;
    }

    // Get system allocation granularity (usually 64 KB)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    size_t granularity = sysInfo.dwAllocationGranularity;

    // Align offset down to granularity boundary
    size_t alignedOffset = offset & ~(granularity - 1);
    size_t delta = offset - alignedOffset;

    // Compute max size to map, including delta
    size_t maxSize = (fileSize > alignedOffset)
        ? min(MAPPED_MEMORY_WINDOW_SIZE + delta, fileSize - alignedOffset)
        : 0;

    if (pData) {
        UnmapViewOfFile(pData);
    }

    BYTE* rawView = static_cast<BYTE*>(MapViewOfFile(
        hMap,
        FILE_MAP_READ,
        static_cast<DWORD>(alignedOffset >> 32),
        static_cast<DWORD>(alignedOffset & 0xFFFFFFFF),
        maxSize
    ));

    if (!rawView) {
        return false;
    }

    pData = rawView + delta;
    currentMappingOffset = offset;
    currentReadingOffset = 0;

    return true;
}

unsigned short BmpReader::getNumImages() const {
    return fileHeader.NumImages;
}

unsigned short BmpReader::getCycleTimeMs() const {
    return fileHeader.CycleTimeMs;
}

BYTE* BmpReader::getFirstImage(size_t& size) {
    size = firstImage.size();
    return &firstImage[0];
}

BYTE* BmpReader::getNextImage(size_t& size) {
    if (loadedImages.size() < 1) {
        size = 0;
        return nullptr;
    }
    size = loadedImages.front().size;
    BYTE* ret = loadedImages.front().location;
    loadedImages.pop_front();
    return ret;
}

bool BmpReader::loadNextImage() {
    struct ImageData data;
    data.location = readNextBmpImage(data.size);
    if (data.location == nullptr) {
        return false;
    }
    loadedImages.push_back(data);
    return true;
}

void BmpReader::resetLoadedImages() {
    loadedImages.clear();
    size_t tmp;
    readNextBmpImage(tmp, true);
}
