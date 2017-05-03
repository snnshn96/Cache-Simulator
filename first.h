#ifndef first_h
#define first_h

#include "first.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Block{
	char* tag;
	struct _Block * nextBlock;
} Block;

typedef struct _Set{
	Block *headBlock;
} Set;

typedef struct _Cache{
	int reads;
	int writes;
	int hits;
	int misses;
	int type; // 1 or 2 means Type A or B respectively
	Set ** sets;
} Cache;

int isPowerOfTwo(int x);
int logTwo(int num);
int checkAssocN(char * buff);
char * hexToBinary(char* hex);
void readFromCache(Cache * cache, char* address);
void writeToCache(Cache * cache, char* address);
void fullyAssocRead(Cache * cache, char* address);
void fullyAssocWrite(Cache * cache, char* address);
void directRead(Cache * cache, char* address);
void directWrite(Cache * cache, char* address);
int powerF(int x, int y);

#endif
