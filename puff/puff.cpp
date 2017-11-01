// puff.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cmath>

using std::map;
using std::vector;
using std::string;

using std::ofstream;
using std::ifstream;
using std::ios;

using std::cin;
using std::cout;
using std::endl;


struct huffTableEntry {
	int glyph = -1;
	int leftPointer = -1;
	int rightPointer = -1;
};

struct pufFile {
	int realNameLength = 0;
	string realName = "";
	string pufName;
	int numberOfGlyphs = 0;
	huffTableEntry hufTable[513];
	ifstream compressedFile;
	double compressedFileLength;
};

// TODO close file
// TODO map file into memory (https://stackoverflow.com/questions/15138353/how-to-read-a-binary-file-into-a-vector-of-unsigned-chars)
void loadFileContents(pufFile &pufFile)
{
	pufFile.compressedFile = ifstream(pufFile.pufName, ios::in | ios::binary);

	pufFile.compressedFile.seekg(0, pufFile.compressedFile.end);
	pufFile.compressedFileLength = pufFile.compressedFile.tellg();
	pufFile.compressedFile.seekg(0, pufFile.compressedFile.beg);

}

bool determineBitOrientation(unsigned char byte, int bitPosition)
{
	if (byte & (short)pow(2.0, bitPosition))
		return true;
	else
		return false;
}

// TODO Figure out how to determine 2 or 4 byte integers. Assuming 4 for now based on coach's test .huf file
void getFileNameLengthAndName(pufFile &pufFile)
{
	unsigned char c[1];
	int bitTracker = 0;
	//loop four times to get 4 bytes of name length (based on however long we decide our integers will be. Coach's are 4 bytes)
	for (int i = 0; i < 4; i++)
	{
		pufFile.compressedFile.read((char*)c, 1);
		pufFile.compressedFile.seekg(0, 1);

		for (int j = 0; j < 8; j++)
		{
			pufFile.realNameLength += determineBitOrientation(c[0], j) ? (short)pow(2.0, bitTracker) : 0;
			bitTracker++;
		}
	}

	//loop until we've gotten the whole length of the file
	for (int i = 0; i < pufFile.realNameLength; i++)
	{
		pufFile.compressedFile.read((char*)c, 1);
		pufFile.compressedFile.seekg(0, 1);

		pufFile.realName += c[0];
	}
}

void getHuffmanTable(pufFile &pufFile)
{
	unsigned char c[1];
	int bitTracker = 0;
	int bitValue = 0;

	//loops through 4 bytes to get huffman length
	for (int i = 0; i < 4; i++)
	{
		pufFile.compressedFile.read((char*)c, 1);
		pufFile.compressedFile.seekg(0, 1);

		for (int j = 0; j < 8; j++)
		{
			pufFile.numberOfGlyphs += determineBitOrientation(c[0], j) ? (short)pow(2.0, bitTracker) : 0;
			bitTracker++;
		}
	}

	//this is getting crazy and can be done better, but first loop is for the
	//slot in huffman table array, second loop is for glyph vs left pointer vs right pointer,
	//third loop is for the values gotten from file, fourth is for bit orientation.
	for (int i = 0; i < pufFile.numberOfGlyphs; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			bitTracker = 0;
			bitValue = 0;
			for (int k = 0; k < 4; k++)
			{
				pufFile.compressedFile.read((char*)c, 1);
				pufFile.compressedFile.seekg(0, 1);

				for (int l = 0; l < 8; l++)
				{
					bitValue += determineBitOrientation(c[0], l) ? (short)pow(2.0, bitTracker) : 0;
					bitTracker++;
				}
			}
			if (bitValue > 256)
				bitValue = -1;
			if (j == 0)
				pufFile.hufTable[i].glyph = bitValue;
			else if (j == 1)
				pufFile.hufTable[i].leftPointer = bitValue;
			else
				pufFile.hufTable[i].rightPointer = bitValue;
		}
		cout << (char)pufFile.hufTable[i].glyph << ' ' << pufFile.hufTable[i].leftPointer << ' ' << pufFile.hufTable[i].rightPointer << endl;
	}

}

void decompressFile(string fileToDecompress)
{
	pufFile pufFile;
	pufFile.pufName = fileToDecompress;

	loadFileContents(pufFile);
	getFileNameLengthAndName(pufFile);
	getHuffmanTable(pufFile);
}

void main()
{
	string fileToDecompress;
	cout << "Enter the name of the .huf file to be decompressed: ";
	getline(cin, fileToDecompress);
	decompressFile(fileToDecompress);
}
