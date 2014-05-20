/* Jmeno: Petr Kubat
 * Login: xkubat11
 * Datum: 11.05.2014
 * Nazev: main.cpp
 * Popis: Cast programu slouzici pro volani funkci pro kompresi a
 * 		  dekompresi souboru.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <fstream>

#include "bwted.hpp"

using namespace std;

int main(int argc, char **argv) {
    string inputf = "";
    string outputf = "";
    string logf = "";
    string helpstr;
    helpstr = "Usage: bwted [-i inputfile] [-o outputfile] [-l logfile] ";
    helpstr += "-c/-x [-h]\n";
    helpstr += "Arguments: \n";
    helpstr += "    -i: inputfile, if not used will read from stdin\n";
    helpstr += "    -o: outputfile, if not used will write to stdout\n";
    helpstr += "    -l: logfile for log output, if not used will skip\n";
    helpstr += "    -c: the appliaction will compress input\n";
    helpstr += "    -x: the appliaction will extract intput\n";
    helpstr += "    -h: prints this help message\n";
    tBWTED logstruct;
    logstruct.uncodedSize = 0;
    logstruct.codedSize = 0;
    int opt;
    // compression/extraction flags
    uint8_t comp = 0;
    uint8_t ext = 0;
    FILE *fp;
    FILE *fo;
    // get args
    while ((opt = getopt(argc, argv, "i:o:l:cxh")) != -1) {
        switch (opt) {
            case 'i':
                inputf = optarg;
                break;
            case 'o':
                outputf = optarg;
                break;
            case 'l':
                logf = optarg;
                break;
            case 'c':
                comp = 1;
                break;
            case 'x':
                ext = 1;
                break;
            case 'h':
				cout << helpstr;
				return (EXIT_SUCCESS);
            default:
                cout << helpstr;
                return (EXIT_FAILURE);
        }
    }
    // cannot compress and extract at the same time
    if (comp == ext) {
		cout << helpstr;
        return (EXIT_FAILURE);
    }
    if (inputf == "")
		fp = stdin;
	else
		fp = fopen(inputf.c_str(), "rb");
	
	if (outputf == "")
		fo = stdout;
	else
		fo = fopen(outputf.c_str(), "wb");

	if (comp)
		BWTEncoding(&logstruct, fp, fo);
	else
		BWTDecoding(&logstruct, fp, fo);
	
	// write log into file
	if (logf != "") {
		ofstream logs;
		logs.open (logf);
		logs << "login = xkubat11" << endl;
		logs << "unCodedSize = " << logstruct.uncodedSize << endl;
		logs << "codedSize = " << logstruct.codedSize << endl;
		logs.close();
	}
	fclose (fp);
	fclose (fo);
    return 0;
}
