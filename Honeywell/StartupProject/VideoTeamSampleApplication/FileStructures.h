#pragma once

#define FILE_HEADER_TAG "FILE"
#define CONTENT_HEADER_TAG  "IMAG"
#define CONTENT_TRAILER_TAG "TRAI"

struct DatFileHeader
{
	char Signature[sizeof(FILE_HEADER_TAG)];
	unsigned short NumImages; // The number of images in the file
	unsigned short CycleTimeMs; // The duration in milliseconds for each cycle
};

struct DatFileContentHeader
{
	char Signature[sizeof(CONTENT_HEADER_TAG)];
	size_t PayloadSize; // The size of the proceeding .bmp file
};

struct DatFileContentTrailer
{
	char Signature[sizeof(CONTENT_TRAILER_TAG)];
	size_t PreviosuContentPayloadSize; // The size of the preceding .bmp file
};
