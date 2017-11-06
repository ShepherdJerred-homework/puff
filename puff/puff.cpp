// puff.cpp
//By Jerred Shepherd and Mack Peters
//This program takes a file that has been previously compressed by its corresponding
//huff version and decompresses it to produce a lossless version of what has been compressed.

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>
#include <ctime>

using std::map;
using std::vector;
using std::string;

using std::ofstream;
using std::ifstream;
using std::ios;

using std::cin;
using std::cout;
using std::endl;

//Defines how an entry should look in the huffman table
struct huffTableEntry {
    int glyph = -1;
    int leftPointer = -1;
    int rightPointer = -1;
};

//Contains the contents of the hufFile, as well as some pieces of information that pertain
//to its decoding. hufTable is generated based on the contents of the file and is what allows
//the file to be decoded. byteCodes is generated once hufTable is completed.
struct pufFile {
    int realNameLength = 0;
    string realName = "";
    string pufName;
    int numberOfGlyphs = 0;
    huffTableEntry hufTable[513];
    ifstream compressedFile;
    double compressedFileLength;
    map<string, int> byteCodes;
};

void loadFileContents(pufFile &pufFile) {
    pufFile.compressedFile = ifstream(pufFile.pufName, ios::in | ios::binary);
}

//Determines whether a bit is on or off and returns the result
bool determineBitOrientation(unsigned char byte, int bitPosition) {
    return (byte & (short) pow(2.0, bitPosition)) != 0;
}

//Determines how long the file name is and then gets the file name
void getFileNameLengthAndName(pufFile &pufFile) {
    unsigned char c;
    int bitTracker = 0;

    //loop four times to get 4 bytes of name length
    for (int i = 0; i < 4; i++) {
        pufFile.compressedFile.read((char *)&c, 1);
        pufFile.compressedFile.seekg(0, 1);

        for (int j = 0; j < 8; j++) {
            pufFile.realNameLength += determineBitOrientation(c, j) ? (short) pow(2.0, bitTracker) : 0;
            bitTracker++;
        }
    }

    //loop until we've gotten the whole length of the file
    for (int i = 0; i < pufFile.realNameLength; i++) {
        pufFile.compressedFile.read((char *)&c, 1);
        pufFile.compressedFile.seekg(0, 1);

        pufFile.realName += c;
    }
}

//Generates the huffman table from the contents of file. First gets the length
//of the huffman table, then loops to get all the values and fill the table.
void getHuffmanTable(pufFile &pufFile) {
    unsigned char c;
    int bitTracker = 0;
    int bitValue = 0;

    //loops through 4 bytes to get huffman length
    for (int i = 0; i < 4; i++) {
        pufFile.compressedFile.read((char *)&c, 1);
        pufFile.compressedFile.seekg(0, 1);

        for (int j = 0; j < 8; j++) {
            pufFile.numberOfGlyphs += determineBitOrientation(c, j) ? (short) pow(2.0, bitTracker) : 0;
            bitTracker++;
        }
    }

    // first loop is for the slot in huffman table array, second loop is for glyph(first) vs left pointer(second) vs right pointer(third),
    //third loop is for the values gotten from file, fourth is for bit orientation.
    for (int i = 0; i < pufFile.numberOfGlyphs; i++) {
        for (int j = 0; j < 3; j++) {
            bitTracker = 0;
            bitValue = 0;
            for (int k = 0; k < 4; k++) {
                pufFile.compressedFile.read((char *)&c, 1);
                pufFile.compressedFile.seekg(0, 1);

                for (int l = 0; l < 8; l++) {
                    bitValue += determineBitOrientation(c, l) ? (short) pow(2.0, bitTracker) : 0;
                    bitTracker++;
                }
            }
            if (j == 0)
                pufFile.hufTable[i].glyph = bitValue;
            else if (j == 1)
                pufFile.hufTable[i].leftPointer = bitValue;
            else
                pufFile.hufTable[i].rightPointer = bitValue;
        }
    }

}

//Recursive function that generates byte codes. If the table entry is a merge node (-1 as glyph), we then look for
//left and right pointers. If the pointer goes left, add a 0 to the byte code, if it goes right, add a 1. Once we hit
//a leaf node (glyph not -1) then attach the string to that glyph. Stored in a map of <string, int> where the string is the
//bytecode and the int is the glyph. Ints are necessary because 256 is EOF and cannot be represented as a char.
void generateByteCodes(huffTableEntry hufTable[], map<string, int> &byteCodes, int position, string byteCode) {
    if (hufTable[position].leftPointer != -1 || hufTable[position].rightPointer != -1) {
        if (hufTable[position].leftPointer != -1) {
            generateByteCodes(hufTable, byteCodes, hufTable[position].leftPointer, (byteCode + "0"));
        }
        if (hufTable[position].rightPointer != -1) {
            generateByteCodes(hufTable, byteCodes, hufTable[position].rightPointer, (byteCode + "1"));
        }
    } else {
        byteCodes[byteCode] = hufTable[position].glyph;
    }
}

//Acts as an entry point to generate the byte codes. Implemented in order to keep other functions more organized
void generateByteCodeTable(pufFile &pufFile) {
    string byteCode;

    generateByteCodes(pufFile.hufTable, pufFile.byteCodes, 0, byteCode);
}

//Gets the contents of file as a long string of bits to be read later and decoded
string getEncodedMessage(pufFile &pufFile) {
    //Get the current position in the file, get the file length, then go back to the place that was left from
    int currentPosition = pufFile.compressedFile.tellg();
    pufFile.compressedFile.seekg(0, pufFile.compressedFile.end);
    pufFile.compressedFileLength = pufFile.compressedFile.tellg();
    pufFile.compressedFile.seekg(currentPosition);

    string s = "";

    for (int i = currentPosition; i < pufFile.compressedFileLength; i++) {
        unsigned char c;
        pufFile.compressedFile.read((char *)&c, 1);
        pufFile.compressedFile.seekg(0, 1);

        for (int j = 0; j < 8; j++) {
            s += determineBitOrientation(c, j) ? "1" : "0";
        }
    }
    return s;
}

string getDecodedMessage(pufFile &pufFile, string message) {
    int messageLength = message.size();
    string comparisonString;
    string decodedString;
    for (int i = 0; i < messageLength; i++) {
        comparisonString += message[i];
        auto search = pufFile.byteCodes.find(comparisonString);
        if (search != pufFile.byteCodes.end()) {
            if (search->second == 256) {
                i = messageLength;
            } else {
                decodedString += (unsigned char) search->second;
                comparisonString = "";
            }
        }
    }
    return decodedString;
}

void generateOutputFile(pufFile &pufFile, string decodedMessage) {
    ofstream decodedFile(pufFile.realName, ios::out|ios::binary);

    decodedFile.write(decodedMessage.c_str(), decodedMessage.size());
    decodedFile.close();
}

void decompressFile(string fileToDecompress) {
    pufFile pufFile;
    pufFile.pufName = fileToDecompress;

    loadFileContents(pufFile);
    getFileNameLengthAndName(pufFile);
    getHuffmanTable(pufFile);
    generateByteCodeTable(pufFile);
    string message = getEncodedMessage(pufFile);
    string decodedMessage = getDecodedMessage(pufFile, message);
    generateOutputFile(pufFile, decodedMessage);
}

void main() {
    string fileToDecompress;
    cout << "Enter the name of the .huf file to be decompressed: ";
    getline(cin, fileToDecompress);
	clock_t start, end;
	start = clock();
    decompressFile(fileToDecompress);
	end = clock();
	cout << std::setprecision(1) << std::fixed;
	cout << "The time was " << (double(end - start) / CLOCKS_PER_SEC) << " seconds." << endl;
}
