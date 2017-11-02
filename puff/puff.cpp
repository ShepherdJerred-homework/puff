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
	map<string, int> byteCodes;
};

// TODO close file
// TODO map file into memory (https://stackoverflow.com/questions/15138353/how-to-read-a-binary-file-into-a-vector-of-unsigned-chars)
void loadFileContents(pufFile &pufFile)
{
	pufFile.compressedFile = ifstream(pufFile.pufName, ios::in | ios::binary);
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
			if (bitValue == 65535)
				bitValue = -1;
			if (j == 0)
				pufFile.hufTable[i].glyph = bitValue;
			else if (j == 1)
				pufFile.hufTable[i].leftPointer = bitValue;
			else
				pufFile.hufTable[i].rightPointer = bitValue;
		}
		//Debug statement
		/*cout << pufFile.hufTable[i].glyph << ' ' << pufFile.hufTable[i].leftPointer << ' ' << pufFile.hufTable[i].rightPointer << endl;*/
	}

}

void generateByteCodes(huffTableEntry hufTable[], map<string, int> &byteCodes, int position, string byteCode)
{
	if (hufTable[position].leftPointer != -1 || hufTable[position].rightPointer != -1) {
		if (hufTable[position].leftPointer != -1) {
			generateByteCodes(hufTable, byteCodes, hufTable[position].leftPointer, (byteCode + "0"));
		}
		if (hufTable[position].rightPointer != -1) {
			generateByteCodes(hufTable, byteCodes, hufTable[position].rightPointer, (byteCode + "1"));
		}
	}
	else {
		byteCodes[byteCode] = hufTable[position].glyph;
	}
}

void generateByteCodeTable(pufFile &pufFile)
{
	string byteCode;

	generateByteCodes(pufFile.hufTable, pufFile.byteCodes, 0, byteCode);

	//Debug Statement
	/*for (auto elem : pufFile.byteCodes) {
		std::cout << elem.first << " " << std::setfill('0') << std::setw(2) << std::uppercase << elem.second
			<< "\n";
	}*/
}

string getEncodedMessage(pufFile &pufFile)
{
	//Get the current position in the file, get the file length, then go back to the place that was left from
	int currentPosition = pufFile.compressedFile.tellg();
	pufFile.compressedFile.seekg(0, pufFile.compressedFile.end);
	pufFile.compressedFileLength = pufFile.compressedFile.tellg();
	pufFile.compressedFile.seekg(currentPosition);
	
	string s = "";
	int bitTracker = 0;

	for (int i = currentPosition; i < pufFile.compressedFileLength; i++)
	{
		unsigned char c[1];
		pufFile.compressedFile.read((char *)c, 1);
		pufFile.compressedFile.seekg(0, 1);

		for (int j = 0; j < 8; j++)
		{
			s += determineBitOrientation(c[0], j) ? "1" : "0";
			bitTracker++;
		}

		bitTracker = 0;
	}
	return s;
}

string getDecodedMessage(pufFile &pufFile, string message)
{
	int messageLength = message.size();
	string comparisonString = "";
	string decodedString = "";
	for (int i = 0; i < messageLength; i++)
	{
		comparisonString += message[i];
		auto search = pufFile.byteCodes.find(comparisonString);
		if (search != pufFile.byteCodes.end())
		{
			if (search->second == 256)
			{
				i = messageLength;
			}
			else
			{
				decodedString += (char)search->second;
				comparisonString = "";
			}
		}
	}
	return decodedString;
}

void generateOutputFile(pufFile &pufFile, string decodedMessage)
{
	ofstream decodedFile(pufFile.realName, ios::out);

	decodedFile << decodedMessage;

	decodedFile.close();
}

void decompressFile(string fileToDecompress)
{
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

void main()
{
	string fileToDecompress;
	cout << "Enter the name of the .huf file to be decompressed: ";
	getline(cin, fileToDecompress);
	decompressFile(fileToDecompress);
}
