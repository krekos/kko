/* Jmeno: Petr Kubat
 * Login: xkubat11
 * Datum: 11.05.2014
 * Nazev: bwted.hpp
 * Popis: Hlavickovy soubor k implementaci knihovny bwted
 */

// encoding block length
#define BLOCK_LEN 10000

// suffix array item
struct strItem {
    char *x;
    uint32_t pos;
};

// log struct
typedef struct{
	int64_t uncodedSize;
	int64_t codedSize;
} tBWTED;

int BWTEncoding(tBWTED *bwted, FILE *inputFile, FILE *outputFile);
int BWTDecoding(tBWTED *bwted, FILE *inputFile, FILE *outputFile);
