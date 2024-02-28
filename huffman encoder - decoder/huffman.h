#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include <list>
#include <string>
#include <cstdlib>

using namespace std;

class CHuffman {
public:
	// Constuctor.  Default destructor is used
	CHuffman();
 
	// Get ready for encoding (clear, count characters, create tree and the encoding table)
	bool PrepareForEncoding(char *pString, size_t Length);

	/* Encode SourceSize bytes of data from pSource into pTarget.  pTarget must be
	 * 0 and is created in the method.  Caller must discard pTarget once it is not
	 * needed anymore.  Return value is length of pTarget
	 */
	size_t Encode(const char *pSource, size_t SourceSize, char *&pTarget);
	/* Decode data from pSource (length is included) into pTarget. pTarget must be
	 * 0 and is create in the method.  Caller must discard pTarget once it is not
	 * needed anymore.  TreatAsString adds an extra \0 terminator to the end of the
	 * decoded string.  Return value is the number of decoded bytes
	 */
	size_t Decode(const char *pSource, char *&pTarget, bool TreatAsString = false);

	/* EncodeFile encodes a given file where pSource is the file name into a file
	 * already open for writing.  Return value is true for success, false for failure
	 */
	bool EncodeFile(const char *pSource, FILE *pTarget, bool TextFile);
	/* DecodeFile decodes a given file where pSource is the file name into pTarget
	 * which shall be a file already open for writing (or to the console if that is
	 * null).  If TextFile is true then the source file is opened in text mode.
	 * Return value is true for success, false for failure
	 */
	bool DecodeFile(const char *pSource, FILE *pTarget);

	/* WriteCodeTree writes the encoding tree to the given file name.  Returns true
	 * for success and false for failure
	 */
	bool WriteCodeTree(char *pFileName);
	/* ReadCodeTree reads the code tree from the given file.  Return true for success
	 * and false for failure
	 */
	bool ReadCodeTree(char *pFileName);

	/* Clear clears all internal data and gets ready for a new action
	 */
	void Clear();

	/* CountStringContentsFromFile counts the number of character occurences in a file
	 * specified in pFileName.  Returns true on success
	 */
	bool CountStringContentsFromFile(const char *pFileName);
	/* CreateTree creates the encoding tree after the number of character occurences in
	 * the given file to be encoded are counted.  Return true on success
	 */
	bool CreateTree();
	/* CreateEncodingTable creates the encoding table based on an existing tree.  The
	 * encoding table is used for encoding, decoding is based on the tree.  Return true
	 * on success
	 */
	bool CreateEncodingTable();

	/* PrintEncodedAsBinary prints the encoded data in a binary format to the console.
	 * pData is the data pointer, Size is the number of bytes to be printed
	 */
	void PrintEncodedAsBinary(char *pData, size_t Size);
	/* PrintEncodingTable prints the encoding table to the console
	 */
	void PrintEncodingTable();
	/* DebugPrintCount prints the number of character occurences to the console
	 */
	void DebugPrintCount();

protected:
	/* StructTreeData holds tree elements of the Huffman tree
	 */
	struct StructTreeData {
		/* Default constructor initializes both children to 0
		 */
		StructTreeData() {
			pNextLeftZeroElement = pNextRightOneElement = 0;
		}

		unsigned char Character;		// 0..255
		unsigned long int Frequency;	// How often occurs
		bool JoinedNode;				// Use it so there is no need to check next elements always - also simpler to save tree
		StructTreeData *pNextLeftZeroElement, *pNextRightOneElement;	// Children

		/* operator < is overloaded for proper sorting
		 */
		bool operator < (const StructTreeData &Data) {
			return Frequency < Data.Frequency;	// Ascending
		}
	};
	/* StructTreeDataPointerCompare is used for sorting the tree list when
	 * it built up
	 */
	struct StructTreeDataPointerCompare {
		bool operator()(const StructTreeData *pData1, const StructTreeData *pData2) {
			return pData1->Frequency < pData2->Frequency;
		}
	};

	/* StructBinaryValue is the base of the encoding table
	 */
	struct StructBinaryValue {
		unsigned char Character;				// Character
		unsigned int Encoded, EncodedLength;	// How it is encoded and on how many bits

		/* operator < is used for sorting the encoding tree.  Sorting is done
		 * to speed the encoding up a little bit
		 */
		bool operator < (const StructBinaryValue &Data) const{
			if (EncodedLength == Data.EncodedLength) {
				return Encoded < Data.Encoded;
			}
			return EncodedLength <= Data.EncodedLength;
		}
	};
struct StructDebugSortEncodingTable {
	bool operator()(const StructBinaryValue Data1, const StructBinaryValue Data2) {
		return Data1.Character < Data2.Character;
	}
};

	/* EnumStepDirection is used to specify if child is to the left (0)
	 * or to the right (1)
	 */
	enum EnumStepDirection {
		STEP_DIRECTION_LEFT = 0,
		STEP_DIRECTION_RIGHT
	};

private:
	unsigned long int Occurence[256];	// Handle all 256 characters (0x00..0xFF), not just 127 standard ASCII
	struct StructTreeData *pTop;		// First element of the Huffman tree
	list<struct StructBinaryValue> EncodingTable;	// The Encoding table

	/* CountStringContents counts occurence in pString for Length bytes
	 */
	void CountStringContents(const char *pString, size_t Length);

	/* EncodeChild is recursive method use used for creating the encoding table based on the tree.
	 * MovedTo tells if the child is to the left or right, pCurrentLeaf holds data of current leaf,
	 * Encoded and EncodedLength are the current encoding to be used
	 */
	void EncodeChild(EnumStepDirection MovedTo, struct StructTreeData *pCurrentLeaf, unsigned int Encoded, unsigned int EncodedLength);

	/* SaveOriginalLength converts Length into the pBuffer
	 */
	void SaveOriginalLength(char *pBuffer, size_t Length);

	/* GetOriginalLength reverts data create by SaveOriginalLength from pData and uses that as
	 * return value
	 */
	size_t GetOriginalLength(const char *pData);

	/* ByteToBinary Creates and returns string representation of UInt as binary.  Length is 8
	 * characters if Bytes == 1, 16 is Bytes == 2, etc.
	 */
	string ByteToBinary(unsigned int UInt, unsigned int Bytes);

	/* GetNextCharacter is a recursive method used for decoding an encoded string.  It iterates
	 * through the tree based on the bits encoded and returns the decoded character.
	 * pSource is the decoded data, ByteCounter and BitCounter are pointers showing the current
	 * position in pSource, pParent is the tree element what called the method.  Return value is
	 * the actual character belonging to the encoding
	 */
	unsigned char GetNextCharacter(const char *pSource, size_t &ByteCounter, size_t &BitCounter, struct StructTreeData *pParent);

	/* WriteCodeTreeElement write one single tree element into the given and already open pFile
	 * where pElement is the tree data and return value shows success or failure
	 */
	bool WriteCodeTreeElement(FILE *pFile, struct StructTreeData *pElement);
	/* ReadCodeTreeElement reads one single tree element into a pElement (created before) from
	 * pFile.  Return value signals success or failure
	 */
	bool ReadCodeTreeElement(FILE *pFile, struct StructTreeData *pElement);

	/* DeleteTree deletes the complete Huffman tree in a recursive way.  First call shall be made
	 * with pTop
	 */
	void DeleteTree(struct StructTreeData *pElement);
};

#endif
