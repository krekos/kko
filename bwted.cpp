/* Jmeno: Petr Kubat
 * Login: xkubat11
 * Datum: 11.05.2014
 * Nazev: bwted.cpp
 * Popis: Implementace knihovny pro kompresi a dekompresi dat pomoci
 * 		  algoritmu BWT, MTF a RLE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>

#include "bwted.hpp"

using namespace std;

// general buffer
char buff[BLOCK_LEN+1];
// aray used in MTF
char front[256];

bool compare(const strItem a, const strItem b) {
    return (string) a.x < (string) b.x;
}

bool compareChar(const strItem a, const strItem b) {
    return a.x[0] < b.x[0];
}

/* bwted – záznam o kódování
inputFile – vstupní soubor (nekódovaný)
outputFile – výstupní soubor (kódovaný)
návratová hodnota – 0 kódování proběhlo v pořádku,-1 při kódování
nastala chyba */
int BWTEncoding(tBWTED *bwted, FILE *inputFile, FILE *outputFile) {
	strItem suffArr[BLOCK_LEN+1]; //suffix array
    FILE *fp = inputFile;
	FILE *fo = outputFile;
    string inputstr;
    string newlol; // string used to hold BWT and MTF strings
	newlol.reserve(BLOCK_LEN+1);
	string newnewlol = ""; // string used to hold RLE
	newnewlol.reserve(BLOCK_LEN+1);
    size_t n = 0;
	// read block of input data
    while ((n = fread(&buff, sizeof(char), BLOCK_LEN, fp)) != 0) {
	    // log input data
	    bwted->uncodedSize = bwted->uncodedSize + n;
	    newnewlol = "";
	    buff[n] = '\0';
	    inputstr = buff;
	    inputstr = inputstr + '$';
	    uint32_t len = inputstr.length();
	    // create suffix array
	    for (uint32_t i = 0; i < len; i++) {
	        suffArr[i].x = &inputstr[i];
			suffArr[i].pos = i;
		}

		sort(suffArr, suffArr+len, compare);
		// create the BWT string from the suffix array
	    for (uint32_t i = 0; i < len; i++) {
	        if (suffArr[i].pos != 0) {
				newlol[i] = inputstr[(suffArr[i].pos - 1)];
			}
			else {
				newlol[i] = inputstr[len-1];
			}
		}
		newlol[len] = '\0';
		uint32_t index;
		// we need to save the index of the original input msg
	    for (uint32_t i = 0; i < len; i++) {
	        if (suffArr[i].pos == 0)
				index = i;
	    }

		//encode MTF
		for (int counter = 0; counter < 256; counter++)
	        front[counter] = (char) counter;
	    int ind;
	    for (uint32_t i = 0; i < len; i++) {
		    for (ind = 0; ; ind++)
		        if (newlol[i] == front[ind])
		                break;
		    char tmp = newlol[i];
		    newlol[i] = ind;
		    for (; ind!=0; --ind)
		        front[ind]=front[ind-1];
			front[0] = tmp;
	    }
	    newlol[len] = '\0';

	    // encode RLE
	    // AAABCC to AA1BCC0
	    for (uint32_t i = 0; i < len; i++) {
			uint8_t charcnt = 1;
			char tmp = newlol[i];
			while (((i+1) < len) && newlol[i] == newlol[i+1]) {
				charcnt++;
				i++;
			}
			if (charcnt == 1)
				newnewlol = newnewlol + tmp;
			else
				newnewlol = newnewlol + tmp + tmp + (char)(charcnt-2);
		}
		// add delimiter
		newnewlol = newnewlol + (char)255;
		// log output size
		bwted->codedSize = bwted->codedSize + newnewlol.length()+4;
		// write index to file
		fwrite(&index, sizeof(index), 1, fo);
		// write coded data to file
		fwrite(newnewlol.c_str(), 1, newnewlol.length(), fo);
		// reset buffer
		memset(buff, 0, sizeof(char)*BLOCK_LEN+1);
	}
	return 0;
}

/* bwted – záznam o dekódování
inputFile – vstupní soubor (kódovaný)
outputFile – výstupní soubor (dekódovaný)
návratová hodnota – 0 dekódování proběhlo v pořádku,-1 při dekódování
nastala chyba */
int BWTDecoding(tBWTED *bwted, FILE *inputFile, FILE *outputFile) {
    strItem decArr[BLOCK_LEN+1]; // BWT decoding suffix array
    char charArr[BLOCK_LEN+1];
	char c;
	string newlol; // string used to hold BWT and MTF strings
    newlol.reserve(BLOCK_LEN+1);
    string newnewlol = ""; // string used to hold RLE
    newnewlol.reserve(BLOCK_LEN+1);

    uint32_t index = 0;

	size_t n = 0;
	// read the index
    while ((n = fread(&index, sizeof(uint32_t), 1, inputFile)) != 0) {
		// log coded size
		bwted->codedSize = bwted->codedSize + n;
		// reset strings
		newnewlol = "";
		newlol = "";
		uint32_t iter = 0;
		// fill buffer until the delimiter is found
		while ((c = fgetc(inputFile)) != (char)255) {
			buff[iter] = c;
			iter++;
		}
		// log coded size
		bwted->codedSize = bwted->codedSize + iter+4;
		// get data from the buffer
		for (uint32_t i = 0; i < iter; i++)
			newnewlol = newnewlol + buff[i];
		newnewlol[iter] = '\0';

		// decode RLE
		for (uint32_t i = 0; i < newnewlol.length(); i++ ) {
			if ((i+1 < newnewlol.length()) && (newnewlol[i] == newnewlol[i+1])) {
				for (int charcnt = ((int)(newnewlol[i+2]))+2; charcnt != 0; charcnt--) {
					newlol = newlol + newnewlol[i];
				}
				i = i+2;
			}
			else {
				newlol = newlol + newnewlol[i];
			}
		}

		uint32_t len = newlol.length();

	    // decode MTF
	    for (int counter = 0; counter!=256; counter++)
	        front[counter] = counter;

		for (uint32_t i = 0; i < len; i++) {
			char tmp = newlol[i];
			newlol[i] = front[(int)tmp];
			int ind = tmp;
			for (; ind != 0; --ind)
				front[ind] = front[ind-1];
			front[0] = newlol[i];
		}
		// decode BWT
	    for (uint32_t i = 0; i < len; i++) {
			decArr[i].pos = i;
			charArr[i] = newlol[i];
			decArr[i].x = &charArr[i];
		}
		stable_sort(decArr, decArr+len, compareChar);
		for(uint32_t i = 0; i < len; i++ ) {
			index = decArr[index].pos;
			buff[i] = newlol[index];
		}
		buff[len-1] = '\0';
		// log size
		bwted->uncodedSize = bwted->uncodedSize + newlol.length()-1;
		fwrite(buff, 1, newlol.length()-1, outputFile);
		// reset buffer
		memset(buff, 0, sizeof(char)*BLOCK_LEN+1);

	}
	return 0;
}
