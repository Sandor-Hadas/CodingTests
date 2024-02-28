#include "huffman.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Read / write buffer size - also specifies the encoding size for a
// chunk exceeding the buffer size.  Buffer allocation for encoding
// and decoding must be more than the chunk size since the Huffman
// algorithm creates bigger files than original in the worst case
// scenario and 8 bits can turn to be even 12 bits if the character
// distribution is wide and differences are not very significant
#define BUFFER_SIZE	0x4000

CHuffman::CHuffman() {
	// pTop must be 0 at start
	pTop = 0;
}

// Call this method before encoding
bool CHuffman::PrepareForEncoding(char *pString, size_t Length) {
	Clear();
	CountStringContents(pString, Length);
	if (!CreateTree()) {
		return false;
	}
	if (!CreateEncodingTable()) {
		return false;
	}
	return true;
}

bool CHuffman::WriteCodeTree(char *pFileName) {
	if (pTop == 0) {
		printf("CHuffman::WriteCodeTree failed, pTop is 0\n");
		return false;
	}

	FILE *pFile = fopen(pFileName, "wb");
	if (pFile == 0) {
		printf("Error opening code tree file for writing: %s\n", pFileName);
		return false;
	}

	// File is created, call the recursive method, will do all the job
	bool Success = WriteCodeTreeElement(pFile, pTop);

	fclose(pFile);

	if (!Success) {
		printf("Error writing code tree file\n");
		remove(pFileName);
		return false;
	}

	return true;
}

// Code tree element format is pretty simple
// Byte 0: Node type, 'J' for calculated and ' ' for character value
// Byte 1: The character itself for ' ', arbitrary for 'J'
// size_t bytes: Length of character frequency (weight) string
// bytes as defined above: The character frequency string
// File is written from the left to the right so the complete tree goes
//  in and out pretty simple and almost readable
bool CHuffman::WriteCodeTreeElement(FILE *pFile, struct StructTreeData *pElement) {
	char Data[50];
	char CTemp[30];
	if (pElement->JoinedNode) {
		Data[0] = 'J';
	} else {
		Data[0] = ' ';
		Data[1] = pElement->Character;
	}
// From this moment on, it is obvious that I developed on windows and then ported to linux :)
#ifdef _WIN32
	ltoa(pElement->Frequency, CTemp, 10);
#else
	snprintf(CTemp, 20, "%lu", pElement->Frequency);
#endif
	Data[2] = (strlen(CTemp));
	Data[3] = 0;
	strcat(Data + 3, CTemp);
	size_t Length = 3 + strlen(CTemp);
	if (fwrite(Data, 1, Length, pFile) != Length) {
		return false;
	}

	// Data is out by now, check and write children if any
	if (pElement->pNextLeftZeroElement) {
		if (!WriteCodeTreeElement(pFile, pElement->pNextLeftZeroElement)) {
			return false;
		}
	}
	if (pElement->pNextRightOneElement) {
		if (!WriteCodeTreeElement(pFile, pElement->pNextRightOneElement)) {
			return false;
		}
	}
	return true;
}

bool CHuffman::ReadCodeTree(char *pFileName) {
	FILE *pFile = fopen(pFileName, "rb");
	if (pFile == 0) {
		printf("Error opening code tree file for reading: %s\n", pFileName);
		return false;
	}

	// Make sure we are ready to have it
	Clear();

	// Creating pTop is essential
	pTop = new StructTreeData();

	// Call the recursive method and the entire tree
	bool Success = ReadCodeTreeElement(pFile, pTop);

	fclose(pFile);

	return Success;
}

// See file format description at WriteCodeTreeElement
bool CHuffman::ReadCodeTreeElement(FILE *pFile, struct StructTreeData *pElement) {
	char ReadBuffer[50];

	// Read 3 Bytes header, must be there
	if (fread(ReadBuffer, 1, 3, pFile) != 3) {
		if (pTop->pNextRightOneElement == pElement) {
			return true;
		}
		return false;
	}

	// Decode header
	if (ReadBuffer[0] == 'J') {
		pElement->JoinedNode = true;
	} else {
		pElement->JoinedNode = false;
		pElement->Character = ReadBuffer[1];
	}
	size_t FrequencyLength = ReadBuffer[2];
	if (FrequencyLength == 0) {
		return false;
	}

	// Read freqency
	if (fread(ReadBuffer, 1, FrequencyLength, pFile) != FrequencyLength) {
		return false;
	}
	ReadBuffer[FrequencyLength] = 0;
	pElement->Frequency = atol(ReadBuffer);

	// If node is not a character node then call children
	if (pElement->JoinedNode) {
		struct StructTreeData *pNewLeft = new StructTreeData();
		pElement->pNextLeftZeroElement = pNewLeft;
		if (!ReadCodeTreeElement(pFile, pNewLeft)) {
			return false;
		}

		struct StructTreeData *pNewRight = new StructTreeData();
		pElement->pNextRightOneElement = pNewRight;
		if (!ReadCodeTreeElement(pFile, pNewRight)) {
			return false;
		}
	}
	return true;
}


// Get read for a new encoding/decoding
void CHuffman::Clear() {
	// Reset character occurence counter
	memset(Occurence, 0, sizeof(unsigned long int) * 256);
	EncodingTable.clear();
	if (pTop != 0) {
		DeleteTree(pTop);
	}
}

// Deletes the entire tree
void CHuffman::DeleteTree(struct StructTreeData *pElement) {
	if (pElement->pNextLeftZeroElement != 0) {
		DeleteTree(pElement->pNextLeftZeroElement);
	}
	if (pElement->pNextRightOneElement != 0 ) {
		DeleteTree(pElement->pNextRightOneElement);
	}
	delete pElement;
	pElement = 0;
}

void CHuffman::CountStringContents(const char *pString, size_t Length) {
	// Just count the string data
	for (size_t i = 0; i < Length; i++) {
		Occurence[(unsigned char)(pString[i])]++;
	}
}

// Open file, read data, count characters inside
bool CHuffman::CountStringContentsFromFile(const char *pFileName) {
	char Buffer[BUFFER_SIZE];
	size_t ReadSize;
	FILE *pFile = fopen(pFileName, "rb");
	if (pFile == 0) {
		return false;
	}
	do {
		ReadSize = fread(Buffer, 1, BUFFER_SIZE, pFile);
		CountStringContents(Buffer, ReadSize);
	} while (ReadSize >= BUFFER_SIZE);
	fclose(pFile);
	
	return true;
}


bool CHuffman::CreateTree() {
	if (pTop != 0) {
		printf("There is something in the tree still, please delete that before creating another tree.\n");
		return false;
	}

	// First create an ordered list of characters found
	list<struct StructTreeData *> Pretree, Parents;
	struct StructTreeData *pPretreeElement;
	for (unsigned int i = 0; i <= 255; i++) {
		if (Occurence[i] > 0) {
			pPretreeElement = new StructTreeData();
			pPretreeElement->Character = i;
			pPretreeElement->Frequency = Occurence[i];
			pPretreeElement->JoinedNode = false;
//printf("PUTTING %c %lu\n", i, Occurence[i]);
			Pretree.push_back(pPretreeElement);
		}
	}
	if (Pretree.size() < 1) {
		printf("Error occured in CHuffman::CreateTree.  Make sure that you loaded and counted string contents before calling this method (0 found).\n");
		return false;
	}
	// Sort the list
	Pretree.sort(StructTreeDataPointerCompare());

	if (Pretree.size() == 1) {
		// Single character in data to be encoded
		pTop = Pretree.front();
		pTop->JoinedNode = false;
		Pretree.pop_front();
	} else {
		// Tree has more than one element
		struct StructTreeData *pParent;
		while (Pretree.size() > 1) {
			pParent = new StructTreeData();

			// Remove first and second elements and assign below parent
			pParent->pNextLeftZeroElement = Pretree.front();
			Pretree.pop_front();
			pParent->pNextRightOneElement = Pretree.front();
			Pretree.pop_front();
	///**/		pParent->Character = '!'; // - it does not matter so can be any arbitrary value; 
			pParent->JoinedNode = true;
			pParent->Frequency = pParent->pNextLeftZeroElement->Frequency + pParent->pNextRightOneElement->Frequency;

			if (Pretree.size() == 0) {
				// No more elements left, must be below parent
				pTop = pParent;
			} else {
				// There is at least one more element so push parent into list
				Pretree.push_front(pParent);
				list<struct StructTreeData *>::iterator DebugIt;
				// Sorting is crucial for valid result
				Pretree.sort(StructTreeDataPointerCompare());
			}
		}
	}

	return true;
}

bool CHuffman::CreateEncodingTable() {
	if (pTop == 0) {
		return false;
	}
	if (!pTop->JoinedNode) {
		// One single character type is found in the data to be encoded
		struct StructBinaryValue NewBinaryValue;
		NewBinaryValue.Character = pTop->Character;
		NewBinaryValue.EncodedLength = 1;
		NewBinaryValue.Encoded = 0x80;
		EncodingTable.push_back(NewBinaryValue);
		return true;
	}

	// There are more than one characters found.  Evaluate left and right of the tree
	unsigned int Encoded = 0;
	unsigned int EncodedLength = 0;
	string s = "";
	EncodeChild(STEP_DIRECTION_LEFT, pTop->pNextLeftZeroElement, Encoded, EncodedLength);
	EncodeChild(STEP_DIRECTION_RIGHT, pTop->pNextRightOneElement, Encoded, EncodedLength);
	EncodingTable.sort();

	if (EncodingTable.size() == 0) {
		return false;
	}
	return true;
}

void CHuffman::EncodeChild(EnumStepDirection MovedTo, struct StructTreeData *pCurrentLeaf, unsigned int Encoded, unsigned int EncodedLength) {
	// Calculate encoding value
	EncodedLength++;
	unsigned int Mask = 1;
	if (EncodedLength > 1) {
		Mask <<= EncodedLength - 1;
	}
	if (MovedTo == STEP_DIRECTION_LEFT) {
		Mask = ~Mask;
		Encoded &= Mask;
	} else {
		Encoded |= Mask;
	}
	// Check if a value node if found or if a composite
	if (pCurrentLeaf->pNextLeftZeroElement == 0 || pCurrentLeaf->pNextRightOneElement == 0) {
		struct StructBinaryValue NewBinaryValue;
		NewBinaryValue.Character = pCurrentLeaf->Character;
		NewBinaryValue.EncodedLength = EncodedLength;
		NewBinaryValue.Encoded = Encoded;
		EncodingTable.push_back(NewBinaryValue);
/*if (!pCurrentLeaf->JoinedNode) {
	printf("ADDED: %c %3u %2u %s\n", pCurrentLeaf->Character, pCurrentLeaf->Character, EncodedLength, ByteToBinary(Encoded, 2).c_str());
}
//exit(0);
*/
	} else {
		EncodeChild(STEP_DIRECTION_LEFT, pCurrentLeaf->pNextLeftZeroElement, Encoded, EncodedLength);
		EncodeChild(STEP_DIRECTION_RIGHT, pCurrentLeaf->pNextRightOneElement, Encoded, EncodedLength);
	}
	EncodedLength--;
}

// Create encoded byte array what is formatted as of:
// (size_t) Original size
// (bits)    Data
// The extra bits at the end of the encoded array will be ignored at decompression
// Return value is the size of pTarget
// pTarget must be destoyed by caller
size_t CHuffman::Encode(const char *pSource, size_t SourceSize, char* &pTarget) {
	if (SourceSize < 1) {
		return 0;
	}

//printf("E: %u '%s'\n", SourceSize, pSource);

	// Do not allow caller to allocate memory
	if (pTarget != 0) {
		printf("Please pass pTarget initialized to 0 when calling CHuffman::Encode\n");
		return 0;
	}
	pTarget = new char[sizeof(size_t) + (SourceSize * 2)];
	if (pTarget == 0) {
		printf("Failed to allocate pTarget in CHuffman::Encode. Check memory or the chunk to be encoded\n");
		return 0;
	}
	// Set all bits to 1 so OR operation will work fine
	memset(pTarget, 0, sizeof(size_t) + (SourceSize * 2));

	// Store original length
	SaveOriginalLength(pTarget, SourceSize);


	// Store payload
	size_t ByteCounter = sizeof(size_t), BitCounter = 0;
	list<struct StructBinaryValue>::iterator FindIt;
	//unsigned int Mask;
	unsigned long int Encoded, EncodedLength;
//unsigned char TestChar = 0;
	for (size_t i = 0; i < SourceSize; i++) {
		// Find encoding for current character
		for (FindIt = EncodingTable.begin(); FindIt != EncodingTable.end(); FindIt++) {
			if ((*FindIt).Character == (unsigned char)pSource[i]) {
//TestChar = (unsigned char)pSource[i];
				break;
			}
		}
		// Check if it was found
		if (FindIt == EncodingTable.end()) {
			printf("Error in CHuffman::Encode when looking up for encoding character %c\n", pSource[i]);
			delete pTarget;
			return 0;
		}
//TODO BitCounter == 7, size = 11, so encoding shall go onto 3 bytes
		// Do the encoding
/*
		Encoded <<= BitCounter;					// 1
		pTarget[ByteCounter] |= Encoded;		// 2
size_t bct = BitCounter;
		BitCounter += EncodedLength;			// 3
		EncodedLength -= (8 - bct);
		if (BitCounter >= 8) {
			BitCounter = 0;						// 4
			ByteCounter++;						// 5
		}
		while (EncodedLength >= 8) {
			Encoded >>= 8;						// 6
			pTarget[ByteCounter++] |= Encoded;	// 7
			EncodedLength -= 8;					// 8
		}
		if (EncodedLength > 0) {
			Encoded >>= 8;						// 9
			pTarget[ByteCounter] |= Encoded;	// 10
			BitCounter = EncodedLength;			// 11
		}
*/

		Encoded = (*FindIt).Encoded;
		EncodedLength = (*FindIt).EncodedLength;

		while (BitCounter + EncodedLength >= 8) {
			// Must split so do part one
			Encoded <<= BitCounter;
			pTarget[ByteCounter] |= Encoded;
			EncodedLength -= (8 - BitCounter);
			if (BitCounter + EncodedLength >= 8) {
				Encoded = (*FindIt).Encoded;
				Encoded >>= (8 - BitCounter);
			} else {
				Encoded >>= 8;
			}
			BitCounter = 0;
			ByteCounter++;
		}

		// Do leftover or all
		if (EncodedLength > 0) {
			Encoded <<= BitCounter;
			pTarget[ByteCounter] |= Encoded;
			BitCounter += EncodedLength;
			if (BitCounter >= 8) {
				BitCounter = 0;
				ByteCounter++;
			}
		}

/*
// Decode test, one character
unsigned char T2 = GetNextCharacter(pTarget, ByC, BiC, pTop);
if (T2 != TestChar) {
	printf("++\n");
	printf("%3u %c %3u %c | ByC: %u, BiC: %u, %u/%u\n", T2, T2, TestChar, TestChar, ByC, BiC, ByteCounter, BitCounter);
	printf("E: %u, EL: %u\n", (*FindIt).Encoded, (*FindIt).EncodedLength);
EncodingTable.sort(StructDebugSortEncodingTable());
PrintEncodingTable();
printf("%s %s\n", ByteToBinary(pTarget[ByC - 1], 1).c_str(), ByteToBinary(pTarget[ByC], 1).c_str());
	exit(0);
} else {
//	printf("NG\n");
}
*/

	}

//	return ByteCounter;
//	printf("BC:%d\n", BitCounter);

	if (BitCounter == 0) {
		return ByteCounter;
	}
	return ByteCounter + 1;
}

size_t CHuffman::Decode(const char *pSource, char *&pTarget, bool TreatAsString) {
	if (pTarget != 0) {
		printf("Please pass pTarget initialized to 0 when calling CHuffman::Decode\n");
		return 0;
	}

	// Get decoded size
	size_t Size = GetOriginalLength(pSource);
	if (Size < 1) {
		printf("Error at CHuffman::Decode, original length found to be invalid: %lu\n", Size);
		return 0;
	}
//printf("DecodedSize: %u\n", Size);

	// Allocate target
	if (TreatAsString) {
		pTarget = new char[Size + 1];
		if (pTarget) {
			pTarget[Size] = 0;
		}
	} else {
		pTarget = new char[Size];
	}
	if (pTarget == 0) {
		printf("Failed to allocate pTarget in CHuffman::Decode. Check memory or the chunk to be encoded, size: %lu\n", Size);
		return 0;
	}

	// Do the actual decoding by recursively trying to find a node what is character (not joined)
	size_t DecodedSize = 0;
	size_t ByteCounter = sizeof(size_t), BitCounter = 0;
	struct StructTreeData *pParent = new StructTreeData();
	while (DecodedSize < Size) {
		pTarget[DecodedSize++] = GetNextCharacter(pSource, ByteCounter, BitCounter, pTop);
	}
	delete pParent;

//printf("%u ", Size);

	return Size;
}

// File format:
// (size_t) File name length
// (n) File name
// Data
// [...]
bool CHuffman::EncodeFile(const char *pSource, FILE *pTarget, bool TextFile) {
	char Buffer[BUFFER_SIZE];

	// Open file for reading
	FILE *pFile = fopen(pSource, "rb");
	if (pFile == 0) {
		return false;
	}

	size_t ReadSize, EncodedSize = 0;
	char *pEncoded;
	while ((ReadSize = fread(Buffer, 1, BUFFER_SIZE, pFile)) > 0) {
		// Read from the file as long as there is anything and then encode what was read

#ifndef _WIN32
		// Get rid of the POSIX EOF character (otherwise it goes into the encoding what is not good in text files)
		if (TextFile) {
			if (Buffer[ReadSize - 1] == 10) {
				ReadSize--;
			}
		}
#endif

		pEncoded = 0;
		EncodedSize = Encode(Buffer, ReadSize, pEncoded);
		if (EncodedSize == 0) {
			return false;
		}

// Verify encoding
/*
char *pTest = 0;
size_t DecodedSize = Decode(pEncoded, pTest);
if (DecodedSize != ReadSize) {
	printf("DecodedSize != ReadSize\n");
	exit(0);
}
int m;
if ((m = memcmp(pTest, Buffer, ReadSize)) != 0) {
//	PrintEncodedAsBinary(pEncoded, EncodedSize);
EncodingTable.sort(StructDebugSortEncodingTable());
	PrintEncodingTable();
	PrintEncodedAsBinary(pEncoded, 20);
for (int i = 0; i < ReadSize; i++) {
	if ((unsigned char)Buffer[i] != (unsigned char)pTest[i]) {
		printf("%03u %03u <<", (unsigned char)Buffer[i], (unsigned char)pTest[i]);
	} else {
		printf("%03u %03u  |", (unsigned char)Buffer[i], (unsigned char)pTest[i]);
	}
}
//printf("%s\n", Buffer);
//printf("%s\n", pTest);
printf("decode memcmp failed, m: %d\n", m);
	exit(0);
}

delete pTest;
*/

		if (pTarget == 0) {
			// Print to console if there is no output specified
			PrintEncodedAsBinary(pEncoded, EncodedSize);
		} else {
			// Write into file if there is output file specified
			SaveOriginalLength(Buffer, EncodedSize);
//Buffer[sizeof(size_t)] = 0;
//printf("Write l: %u '%s'\n", EncodedSize, Buffer);
//printf("Working... write length: %u\n", EncodedSize);
			if (fwrite(Buffer, 1, sizeof(size_t), pTarget) != sizeof(size_t)) {
				return false;
			}
			if (fwrite(pEncoded, 1, EncodedSize, pTarget) != EncodedSize) {
				return false;
			}
		}
		delete pEncoded;
		pEncoded = 0;
	}
	fclose(pFile);

	if (pTarget != 0) {
		if (ftell(pTarget) == 0) {
			fclose(pTarget);
			printf("Failed to encode file: %s; unreadable or empty\n", pSource);
			return false;
		}
	}

	return true;
}

bool CHuffman::DecodeFile(const char *pSource, FILE *pTarget) {
	char Buffer[BUFFER_SIZE * 2];

	// Open source file
	FILE *pFile = fopen(pSource, "rb");
	if (pFile == 0) {
		printf("Error opening file for reading: %s\n", pSource);
		return false;
	}
	
	size_t ReadSize;
	string OutputFileName;
	char *pDecoded = 0;
	bool Error = false;
	while (fread(Buffer, 1, sizeof(size_t), pFile) == sizeof(size_t)) {
		// Loop while header can be read
		ReadSize = GetOriginalLength(Buffer);
//printf("Working... Read length: %u\n", ReadSize);
		// Read data
		if (fread(Buffer, 1, ReadSize, pFile) != ReadSize) {
			return false;
		}

		// Decode data to screen or to file
		if (pTarget == 0) {
			// To console, no file open
			pDecoded = 0;
			ReadSize = Decode(Buffer, pDecoded, true);
			if (ReadSize == 0) {
				Error = true;
				break;
			}
			printf("%s", pDecoded);
		} else {
			// To file
			pDecoded = 0;
			ReadSize = Decode(Buffer, pDecoded);
			if (ReadSize == 0) {
				Error = true;
				break;
			}
			if (fwrite(pDecoded, 1, ReadSize, pTarget) != ReadSize) {
				Error = true;
				break;
			}
		}

		if (pDecoded != 0) {
			delete pDecoded;
		}
		if (Error) {
			return false;
		}
	}

	fclose(pFile);

	return true;
}

unsigned char CHuffman::GetNextCharacter(const char* pSource, size_t &ByteCounter, size_t &BitCounter, struct StructTreeData *pParent) {
	if (!pParent->JoinedNode) {
		// Node shall have no children
		return pParent->Character;
	}

	// Decode next element direction - there shall be children
	struct StructTreeData *pCurrentItem;
	unsigned int Mask = 1;
	Mask <<= BitCounter;
	if (pSource[ByteCounter] & Mask) {
		// Right
		pCurrentItem = pParent->pNextRightOneElement;
	} else {
		// Left
		pCurrentItem = pParent->pNextLeftZeroElement;
	}
	BitCounter++;
	if (BitCounter >= 8) {
		BitCounter = 0;
		ByteCounter++;
	}
	return GetNextCharacter(pSource, ByteCounter, BitCounter, pCurrentItem);
}

void CHuffman::DebugPrintCount() {
	// Print how many characters were found total
	for (unsigned int i = 0; i <= 255; i++) {
		printf("%d. %c %010lu\n", i, i, Occurence[i]);
	}
}

void CHuffman::PrintEncodingTable() {
	// Print the complete encoding table.  Conver special characters to readable
	list<struct StructBinaryValue>::iterator DebugIt;
	string Character;
	for (DebugIt = EncodingTable.begin(); DebugIt != EncodingTable.end(); DebugIt++) {
		switch ((*DebugIt).Character) {
		case '\t':
			Character = "<tab>";
			break;
		case '\r':
			Character = "<CR>";
			break;
		case '\n':
			Character = "<LF>";
			break;
		case 255:
			Character = "<255>";
			break;
		case 0:
			Character = "<END>";
			break;

		default:
			Character = (*DebugIt).Character;
		}
		printf("%5s %3u %04u %s %d\n", Character.c_str(), (*DebugIt).Character, (*DebugIt).EncodedLength, ByteToBinary((*DebugIt).Encoded, 2).c_str(), (*DebugIt).Encoded);
	}
}

void CHuffman::PrintEncodedAsBinary(char *pData, size_t Size) {
	if (Size <= sizeof(size_t)) {
		printf("Error printing encoded data as binary: data too small (%lu bytes, minimum: %lu bytes)\n", Size, sizeof(size_t) + 1);
		return;
	}
	printf("Original length of encoded data: %lu\n", GetOriginalLength(pData));

	printf("Encoded data:\n");
	for (size_t i = sizeof(size_t); i < Size; i++) {
		printf("%s  ", ByteToBinary(pData[i], 1).c_str());
	}
	printf("\n");
}

void CHuffman::SaveOriginalLength(char *pBuffer, size_t Length) {
	// Put size_t long Length into buffer
	for (size_t i = 0; i < sizeof(size_t); i++) {
		pBuffer[sizeof(size_t) - 1 - i] = Length;
		Length >>= 8;
	}
}

size_t CHuffman::GetOriginalLength(const char *pData) {
	// Restore size_t data from pData
	size_t Length = 0;
	for (size_t i = 0; i < sizeof(size_t); i++) {
		Length <<= 8;
		Length += (unsigned char)pData[i];
	}
	return Length;
}

string CHuffman::ByteToBinary(unsigned int UInt, unsigned int Bytes) {
	string Converted = "";
	// Calculate length
	unsigned int From = 128;
	for (unsigned int i = 1; i < Bytes; i++) {
		From <<= 8;
	}

	// Convert
    for (unsigned int i = From; i > 0; i >>= 1) {
		if ((UInt & i) == i) {
			Converted += "1";
		} else {
			Converted += "0";
		}
    }

	return Converted;
}
