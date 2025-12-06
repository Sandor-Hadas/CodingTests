#pragma once

#include <string>
#include <vector>
#include <deque>
#include <limits.h>
#include <Windows.h>

#include "FileStructures.h"

class BmpReader {
public:
	BmpReader();
	virtual ~BmpReader();

	int init(const std::string& filename);
	BYTE* getFirstImage(size_t& size);
	BYTE* getNextImage(size_t& size);
	bool loadNextImage();
	void resetLoadedImages();

	unsigned short getNumImages() const;
	unsigned short getCycleTimeMs() const;

	enum {
		OK = 0,
		FILE_OPEN_ERROR,
		FILE_OPEN_ERROR_SIZE,
		FILE_MAPPING_FAILED,
		FILE_MAPPING_FAILED_CREATE,
		FILE_HEADER_TAG_ERROR,
		FILE_TOO_SHORT,
		//	FILE_CONTENT_HEADER_ERROR,
		FORMAT_ERROR,
		BMP_ERROR
	};

	const LARGE_INTEGER MAX_FILE_SIZE      = { (ULONGLONG)UINT_MAX };
	const size_t MAPPED_MEMORY_WINDOW_SIZE = 1024 * 1024 * 16; // 16 MB view size - bigger than any example files so fully cached for this example
	const size_t MAX_EXPECTED_BMP_SIZE     = 1024 * 1024 * 4;
	const size_t TOTAL_FILE_HEADER_SIZE    = sizeof(FILE_HEADER_TAG) + 1 + (2 * sizeof(unsigned short));

private:
	HANDLE hFile, hMap;
	BYTE * pData;
	size_t fileSize, currentMappingOffset, currentReadingOffset;

	struct DatFileHeader fileHeader;

	struct ImageData {
		BYTE* location;
		size_t size;
	};

	std::vector<BYTE> firstImage;
	std::deque<ImageData> loadedImages;

	void close();
	int checkFileIntegrity();
	BYTE* readNextBmpImage(size_t& outBytes, bool reset = false);
	bool remapView(size_t offset);
};
