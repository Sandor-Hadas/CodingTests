// READING HELP:
// Set tab size to 4 or convert tabs to spaces where 1 tab == 4 spaces;
//  I prefer tabs since if is quicker to navigate in the code compared
//  to the use of spaces - works fine if tab spacing is agreed
// The use of stderr for errors could improve the program - the use case was not
//  specified so printf is used for output and all outputs go to stdout.
//
// HOW TO USE:
// compile and type 'Huffman -h' and read the help
//
// HOW TO TEST:
// go to the 'test' subdirectory, run 'runtest' and it is pretty straightforward from then on

/* IGNORE THE FOLLOWING COMMENTS - was used for cmd.exe when tested on Windows
// -e -i C:\dev\src\Huffman\test\go.txt -o C:\dev\src\Huffman\test\go.txt.encoded -c C:\dev\src\Huffman\test\go.codetree
// -d -i C:\dev\src\Huffman\test\go.txt.encoded -o C:\dev\src\Huffman\test\go.decoded.txt -c C:\dev\src\Huffman\test\go.codetree

// -e -i C:\dev\src\Huffman\test\1.exe -o C:\dev\src\Huffman\test\1.exe.encoded -c C:\dev\src\Huffman\test\1.exe.codetree
// -d -i C:\dev\src\Huffman\test\1.exe.encoded -o C:\dev\src\Huffman\test\1.exe.decoded -c C:\dev\src\Huffman\test\1.exe.codetree

// -e -i C:\dev\src\Huffman\test\1.dll -o C:\dev\src\Huffman\test\1.dll.encoded -c C:\dev\src\Huffman\test\1.dll.codetree
// -d -i C:\dev\src\Huffman\test\1.dll.encoded -o C:\dev\src\Huffman\test\1.exe.decoded -c C:\dev\src\Huffman\test\1.dll.codetree

// -e -i C:\dev\src\Huffman\test\merge1.txt -i C:\dev\src\Huffman\test\merge2.txt -o C:\dev\src\Huffman\test\merged.txt.encoded -c C:\dev\src\Huffman\test\merged.codetree
// -d -i C:\dev\src\Huffman\test\merged.txt.encoded -o C:\dev\src\Huffman\test\merged.txt -c C:\dev\src\Huffman\test\merged.codetree

// -e -i C:\dev\src\Huffman\test\nastytest.txt -o C:\dev\src\Huffman\test\nastytest.txt.encoded -c C:\dev\src\Huffman\test\nastytest.codetree
// -d -i C:\dev\src\Huffman\test\nastytest.txt.encoded -o C:\dev\src\Huffman\test\nastytest.txt.decoded -c C:\dev\src\Huffman\test\nastytest.codetree
*/

#include "huffman.h"

#include <iostream>
#include <string>

#include <string.h>

using namespace std;

const char* ARG_HELP					= "--help";
const char* ARG_INPUT_FILE				= "--if";
const char* ARG_INPUTS_ARE_TEXT			= "--textinputs";
const char* ARG_OUTPUT_FILE				= "--of";
const char* ARG_CODE_TABLE_FILE			= "--ctf";
const char* ARG_ENCODE					= "--encode";
const char* ARG_DECODE					= "--decode";
const char* ARG_HELP_SHORT				= "-h";
const char* ARG_INPUT_FILE_SHORT		= "-i";
const char* ARG_INPUTS_ARE_TEXT_SHORT	= "-t";
const char* ARG_OUTPUT_FILE_SHORT		= "-o";
const char* ARG_CODE_TABLE_FILE_SHORT	= "-c";
const char* ARG_ENCODE_SHORT			= "-e";
const char* ARG_DECODE_SHORT			= "-d";

class CArgs {
public:
	CArgs() {
		PrintHelp = false;
		ParseErrorOnArgs = false;
		pOutputFileName = 0;
		pOutputCodeTableFileName = 0;
		Encode = true;
		TextInputs = false;
	}

	bool ParseArgs(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], ARG_HELP) == 0 || strcmp(argv[i], ARG_HELP_SHORT) == 0) {
				PrintHelp = true;
				break;
			} else if (strcmp(argv[i], ARG_ENCODE) == 0 || strcmp(argv[i], ARG_ENCODE_SHORT) == 0) {
				Encode = true;
			} else if (strcmp(argv[i], ARG_INPUTS_ARE_TEXT) == 0 || strcmp(argv[i], ARG_INPUTS_ARE_TEXT_SHORT) == 0) {
				TextInputs = true;
			} else if (strcmp(argv[i], ARG_DECODE) == 0 || strcmp(argv[i], ARG_DECODE_SHORT) == 0) {
				Encode = false;
			} else if (strcmp(argv[i], ARG_INPUT_FILE) == 0 || strcmp(argv[i], ARG_INPUT_FILE_SHORT) == 0) {
				i++;
				if (i < argc) {
					string Temp = argv[i];
					InputFileNames.push_back(Temp);
				} else {
					ParseErrorOnArgs = true;
				}
			} else if (strcmp(argv[i], ARG_OUTPUT_FILE) == 0 || strcmp(argv[i], ARG_OUTPUT_FILE_SHORT) == 0) {
				i++;
				if (i < argc) {
					pOutputFileName = argv[i];
				} else {
					ParseErrorOnArgs = true;
				}
			} else if (strcmp(argv[i], ARG_CODE_TABLE_FILE) == 0 || strcmp(argv[i], ARG_CODE_TABLE_FILE_SHORT) == 0) {
				i++;
				if (i < argc) {
					pOutputCodeTableFileName = argv[i];
				} else {
					ParseErrorOnArgs = true;
				}
			}
		}

		// Error and help handling
		if (ParseErrorOnArgs) {
			printf("Error parsing command line arguments.  Please read the help and try again.\n");
		}
		if (ParseErrorOnArgs || PrintHelp) {
			printf("Options:\n");
			printf("%s or %s to show help\n", ARG_HELP, ARG_HELP_SHORT);
			printf("%s or %s to run in encoding mode (default)\n", ARG_ENCODE, ARG_ENCODE_SHORT);
			printf("%s or %s to run in decoding mode\n", ARG_DECODE, ARG_DECODE_SHORT);
			printf("%s or %s to specify an input file\n", ARG_INPUT_FILE, ARG_INPUT_FILE_SHORT);
			printf("%s or %s to specify that encoding input file(s) is/are text (EOF cut at merging on linux)\n", ARG_INPUTS_ARE_TEXT, ARG_INPUTS_ARE_TEXT_SHORT);
			printf("%s or %s to specify an output file\n", ARG_OUTPUT_FILE, ARG_OUTPUT_FILE_SHORT);
			printf("%s or %s to specify the code table file\n", ARG_CODE_TABLE_FILE, ARG_CODE_TABLE_FILE_SHORT);
			printf("\nExamples:\n");
			printf("Huffman[.exe] -e -t -i IFile1 -i IFile2 -o OFile -c C.ctf\n");
			printf("Huffman[.exe] -d -i IFile -o OFile -c C.ctf\n");
			return false;
		}

		// Empty input / output file feedback
		if (InputFileNames.size() == 0) {
			printf("No input file name was specified therefore reading from stdin will be done.\n");
		}
		if (pOutputFileName == 0) {
			printf("No output file name was specified so encoded data will be put on stdout.\n");		}
		if (pOutputCodeTableFileName == 0) {
			printf("No code table file name was specified.\n");
		}

		return true;
	}

	bool PrintHelp;						// Is printing of help needed?
	bool ParseErrorOnArgs;				// Was there any error while parsing args?
	list<string> InputFileNames;		// List of input file names
	bool TextInputs;		// Are input files text (so ^Z can be discarded, for example)?
	list<string>::iterator InputFileIt;	// .. and its iterator
	char *pOutputFileName;				// File name for handled data
	char *pOutputCodeTableFileName;		// Code table (tree) file name
	bool Encode;						// true == encoding, false == decoding
};

// This function creates a test file with one character (0..255) each
bool CreateNastyTestFile(char *pFileName) {
	FILE *pFile = fopen(pFileName, "wb");
	if (!pFile) {
		printf("CreateNastyFile failed to create %s\n", pFileName);
		return true;
	}
	char Data[256];
	for (int i = 0; i < 256; i++) {
		Data[i] = i;
	}
	bool Success = true;
	if (fwrite(Data, 1, 256, pFile) != 256) {
		Success = false;
		printf("File write error: %s\n", pFileName);
	}
	fclose(pFile);

	return Success;
}

int main(int argc, char* argv[]) {
	//CreateNastyTestFile("C:\\dev\\src\\Huffman\\test\\nastytest.txt"); exit(0);

	CArgs Args;

	if (!Args.ParseArgs(argc, argv)) {
		return 0;
	}

	CHuffman Huffman;

	if (Args.Encode) {
		// Encoding is needed
		if (Args.InputFileNames.size() == 0) {
			/*
			 * Console input
			 */
			printf("Reading from the console starts now.  Please enter the string(s) to be encoded using CR terminator.  Empty input will stop the sequence.\n");

			// Accumulate inputs for encoding
			list<string> AllInputs;
			size_t TotalSize = 0;
			string Input;
			while (true) {
				getline(cin, Input);
				if (Input.empty()) {
					break;
				}
				AllInputs.push_back(Input);
				TotalSize += Input.size() + 1; // + 1 for the \0 terminator
			}

			// Check if there is anything to encode
			if (AllInputs.size() == 0) {
				printf("No input was received from the console.  There is nothing to do.\n");
				return 0;
			}

			// Accumulate inputs
			char *Accummulated = new char[TotalSize];
			size_t AccWritePos = 0;
			list<string>::iterator StringIt;
			for (StringIt = AllInputs.begin(); StringIt != AllInputs.end(); StringIt++) {
				memcpy(Accummulated + AccWritePos, (*StringIt).c_str(), (*StringIt).length());
				AccWritePos += (*StringIt).length() + 1;
				Accummulated[AccWritePos - 1] = '\n';
			}
			Accummulated[AccWritePos - 1] = 0;

			// Get ready for encoding
			bool Success = Huffman.PrepareForEncoding(Accummulated, strlen(Accummulated) + 1);
			if (!Success) {
				printf("Creation of the encoding table failed.\n");
				return -1;
			}
			// Encode
			char *pTargetEncoding = 0;
			size_t EncodedSize = Huffman.Encode(Accummulated, strlen(Accummulated) + 1, pTargetEncoding);
			if (EncodedSize == 0) {
				printf("Encoding failed.\n");
				return -1;
			}

			if (Args.pOutputFileName == 0) {
				// Print encoded string in case if there is no output file specified
				printf("Encoding table:\n");
				Huffman.PrintEncodingTable();
				//printf("EncodedSize: %d\n", EncodedSize);
				//printf("Encoded string: %s\n", pTargetEncoding);
				printf("\nEncoded string as binary:\n");
				printf("-------------------------\n");
				Huffman.PrintEncodedAsBinary(pTargetEncoding, EncodedSize);
				delete pTargetEncoding;
				return 0;
			}

			// Write encoded data into file
			FILE *pFile = fopen(Args.pOutputFileName, "wb");
			if (pFile == 0) {
				printf("Error opening output file: %s\n", Args.pOutputFileName);
				return -1;
			}
			if (EncodedSize != fwrite(pTargetEncoding, EncodedSize, 1, pFile)) {
				printf("Write error into file: %s\n", Args.pOutputFileName);
				fclose(pFile);
				return -1;
			}

			// Cleanup
			fclose(pFile);
			delete []Accummulated;
			delete pTargetEncoding;

			return 0;
		} else {
			/*
			 * Files input
			 */
			Huffman.Clear();
			for (Args.InputFileIt = Args.InputFileNames.begin(); Args.InputFileIt != Args.InputFileNames.end(); Args.InputFileIt++) {
				if (!Huffman.CountStringContentsFromFile((*Args.InputFileIt).c_str())) {
					printf("Input file error at file: %s", (*Args.InputFileIt).c_str());
					return -1;
				}
			}
			Huffman.CreateTree();
			Huffman.CreateEncodingTable();
			FILE *pOutput = 0;
			if (Args.pOutputFileName != 0) {
				pOutput = fopen(Args.pOutputFileName, "wb");
				if (pOutput == 0) {
					printf("Error opening file for writing: %s\n", Args.pOutputCodeTableFileName);
					printf("Output will be written to the console:\n");
				}
			}
			for (Args.InputFileIt = Args.InputFileNames.begin(); Args.InputFileIt != Args.InputFileNames.end(); Args.InputFileIt++) {
				if (!Huffman.EncodeFile((*Args.InputFileIt).c_str(), pOutput, Args.TextInputs)) {
					printf("Error during encoding file: %s\n", (*Args.InputFileIt).c_str());
					if (pOutput != 0) {
						fclose(pOutput);
						remove(Args.pOutputFileName);
					}
					return -1;
				} else {
					printf("Successfully encoded: %s\n", (*Args.InputFileIt).c_str());
				}
			}
		}

		// Save the encoding table if file name was specified for that
		if (Args.pOutputCodeTableFileName != 0) {
			Huffman.WriteCodeTree(Args.pOutputCodeTableFileName);
		} else {
			printf("No encoding table file was specified.  The table looks like:\n");
			Huffman.PrintEncodingTable();
		}
	} else {
		// Decoding
		// A table is mandatory therefore do load it
		if (Args.pOutputCodeTableFileName == 0) {
			printf("Error, cannot decode without a code table file name.\n");
			return -1;
		}
		if (!Huffman.ReadCodeTree(Args.pOutputCodeTableFileName)) {
			printf("Error reading code tree file: %s\n", Args.pOutputCodeTableFileName);
			return -1;
		}

		// We have a code tree by now
		if (Args.InputFileNames.size() != 1) {
			printf("Please provide one input file name for decoding.\n");
			return -1;
		}

		// Create output file if needed
		FILE *pFile = 0;
		if (Args.pOutputFileName != 0) {
			pFile = fopen(Args.pOutputFileName, "wb");
			if (pFile == 0) {
				printf("Error decode output file: %s; generating output to the console.\n", Args.pOutputFileName);
			}
		}
		if (pFile == 0) {
			printf("Decoded file:\n");
		}

		// Decode the file
		if (Huffman.DecodeFile(Args.InputFileNames.front().c_str(), pFile)) {
			printf("Decoding of file %s was successful.\n", Args.InputFileNames.front().c_str());
		} else {
			printf("Decoding of file %s was not successful.\n", Args.InputFileNames.front().c_str());
			return -1;
		}

		// Cleanup
		if (pFile != 0) {
			fclose(pFile);
		}

		return 0;
	}

	return 0;

}
