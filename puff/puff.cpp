// puff.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>

using std::map;
using std::vector;
using std::string;

using std::ofstream;
using std::ifstream;
using std::ios;

using std::cin;
using std::cout;
using std::endl;


struct pufFile {
	int realNameLength;
	string realName;
	string pufName;
	int numberOfGlyphs;
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

// TODO Figure out how to determine 2 or 4 byte integers. Assuming 4 for now based on coach's test .huf file
void getFileNameLengthAndName(pufFile &pufFile)
{

}

void decompressFile(string fileToDecompress)
{
	pufFile pufFile;
	pufFile.pufName = fileToDecompress;

	loadFileContents(pufFile);
	getFileNameLengthAndName(pufFile);
}

void main()
{
	string fileToDecompress;
	cout << "Enter the name of the .huf file to be decompressed: ";
	getline(cin, fileToDecompress);
	decompressFile(fileToDecompress);
}
