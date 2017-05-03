#include "first.h"

int cacheSize;
int blockSize;
int associativity;
const char * associativityType;
FILE * traceFile;
int numBlockBits;
int numIndexBits;
int numTagBits;
int numSets;

int main(int argc, char const *argv[]) {
	// Parameter Check
	if (argc != 5) {
		printf("Wrong Usage: Example \"./first <cache size> <associativity> <block size> <trace file>\"\n");
		exit(1);
	}
	cacheSize = atoi(argv[1]);
	blockSize = atoi(argv[3]);
	associativity = 0;
	associativityType = argv[2];
	traceFile = fopen(argv[4], "r");
	// Error Checking
	//**********************************************************
	int numErrors = 0;
	if(isPowerOfTwo(cacheSize) == 0){
		printf("Error: <cache size> needs to be a power of 2.\n");
		numErrors++;
	}
	if(strcmp(associativityType, "direct") == 0) {
		associativity = 1;
	} else if(strcmp(associativityType, "assoc") == 0) {
		associativity = cacheSize/blockSize;
	} else {
		int assoc = checkAssocN((char*)associativityType);
		if(assoc > 0) {
			associativity = assoc;
		} else {
			printf("Error: <associativity> is one of:\n\t– direct - simulate a direct mapped cache.\n\t– assoc - simulate a fully associative cache.\n\t– assoc:n - simulate an n − way associative cache. n will be a power of 2.\n");
			numErrors++;
		}
	}
	if(isPowerOfTwo(blockSize) == 0){
		printf("Error: <block size> needs to be a power of 2.\n");
		numErrors++;
	}
	if (!traceFile){
		printf("Error: <trace file> could NOT be read or found.\n");
		numErrors++;
	}
	if(numErrors > 0) {
		exit(1);
	}
	
	//Calculate Bits
	numSets = cacheSize / (associativity * blockSize);
	numBlockBits = logTwo(blockSize);
	numIndexBits = logTwo(numSets);
	numTagBits = (48 - (numBlockBits + numIndexBits));
	
	//Initialize typeA cache
	Cache * cacheA = malloc(sizeof(Cache));
	cacheA -> reads = 0;
	cacheA -> writes = 0;
	cacheA -> hits = 0;
	cacheA -> misses = 0;
	cacheA -> sets = malloc(sizeof(Set)*numSets);
	cacheA -> type = 1;
	//
	Cache * cacheB = malloc(sizeof(Cache));
	cacheB -> reads = 0;
	cacheB -> writes = 0;
	cacheB -> hits = 0;
	cacheB -> misses = 0;
	cacheB -> sets = malloc(sizeof(Set*)*numSets);
	cacheB -> type = 2;
	//
	int k;
	for (k = 0; k < numSets; k++){
		cacheA->sets[k] = malloc(sizeof(Set));
		cacheB->sets[k] = malloc(sizeof(Set));
		cacheA->sets[k]->headBlock = NULL;
		cacheB->sets[k]->headBlock = NULL;
	}
	
	char tmp[16];
	char act[2];
	char address[16];
	while(fscanf(traceFile, "%s %s %s", tmp, act, address) != EOF && strcmp(tmp, "#eof") != 0){
		//printf("not used bit: %s\taction: %s\taddress: %s\n", tmp, act, address);
		
		char * bin;
		bin = hexToBinary(address);
		if (strcmp(act, "R") == 0){
			if (associativity == cacheSize/blockSize){
				fullyAssocRead(cacheA, bin);
				fullyAssocRead(cacheB, bin);
			} else if (associativity == 1){
				directRead(cacheA, bin);
				directRead(cacheB, bin);
			} else {
				readFromCache(cacheA, bin);
				readFromCache(cacheB, bin);
			}
		} else {
			if (associativity == cacheSize/blockSize){
				fullyAssocWrite(cacheA, bin);
				fullyAssocWrite(cacheB, bin);
			} else if (associativity == 1){
				directWrite(cacheA, bin);
				directWrite(cacheB, bin);
			}  else {
				writeToCache(cacheA, bin);
				writeToCache(cacheB, bin);
			}
		}
	}
	
	// Print Output
	printf("cache A\n");
	printf("Memory reads: %d\n", cacheA->reads);
	printf("Memory writes: %d\n", cacheA->writes);
	printf("Cache hits: %d\n", cacheA->hits);
	printf("Cache misses: %d\n", cacheA->misses);
	printf("cache B\n");
	printf("Memory reads: %d\n", cacheB->reads);
	printf("Memory writes: %d\n", cacheB->writes);
	printf("Cache hits: %d\n", cacheB->hits);
	printf("Cache misses: %d\n", cacheB->misses);
	return 0;
}

// ********************** //
// ** Helper Functions ** //
// ********************** //

// Checks if x is a power of 2
int isPowerOfTwo(int x){
	if (x == 0){
		return 0;
	}
	while (x != 1){
		if (x % 2 != 0){
			return 0;
		}
		x = x / 2;
	}
	return 1;
}

// Return log base two of num
int logTwo(int num){
	int ans = 0;
	int l = num;
	while(l / 2 >= 1) {
		ans++;
		l = l/2;
	}
	//printf("log2(%d) = %d\n", num, ans);
	//exit(0);
	return ans;
}

// gets n for associativity from input
int checkAssocN(char * buff){
	int i = 0;
	char * t = strtok(buff, ":");
   char * array[2];
   
   while (t != NULL){
   	if(i > 1){
   		return 0;
   	}
   	array[i++] = t;
   	t = strtok (NULL, ":");
   }
   
   if(strcmp(array[0], "assoc") != 0){
   	return 0;
   }else{
   	int n = atoi(array[1]);
   	if(isPowerOfTwo(n) == 0){
   		return 0;
   	} else {
   		return n;
   	}
   }
}

// Converts Hex to Binary
char * hexToBinary(char* hex){
	int hexLen = strlen(hex);
	char * binary = (char *)malloc(sizeof(char)*49);
	binary[48] = '\0';
	char * newHex = (char *)malloc(sizeof(char)*(hexLen-1));
	newHex[hexLen - 2] = '\0';
	int n = 0;
	while(n < hexLen-2){
		newHex[n] = hex[n+2];
		n++;
	}
	int numZeros = 48 - (n*4);
	int i = 0;
	while (i < numZeros){
		binary[i] = '0';
		i++;
	}
	int j = -1;
	for(i = numZeros; i < 48; i += 4){
		j++;
		switch(newHex[j]) {
			case '0' : 
			binary[i] = '0';
			binary[i+1] = '0';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
			case '1' : 
			binary[i] = '0';
			binary[i+1] = '0';
			binary[i+2] = '0';
			binary[i+3] = '1';
			break;
			case '2' : 
			binary[i] = '0';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case '3' : 
			binary[i] = '0';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			case '4' : 
			binary[i] = '0';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
			case '5' : 
			binary[i] = '0';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '1';
			break;
			case '6' : 
			binary[i] = '0';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case '7' : 
			binary[i] = '0';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			case '8' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
			case '9' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '0';
			binary[i+3] = '1';
			break;
			case 'a' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case 'b' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			case 'c' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
			case 'd' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '1';
			break;
			case 'e' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case 'f' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			case 'A' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case 'B' : 
			binary[i] = '1';
			binary[i+1] = '0';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			case 'C' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
			case 'D' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '0';
			binary[i+3] = '1';
			break;
			case 'E' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '0';
			break;
			case 'F' : 
			binary[i] = '1';
			binary[i+1] = '1';
			binary[i+2] = '1';
			binary[i+3] = '1';
			break;
			default : 
			binary[i] = '0';
			binary[i+1] = '0';
			binary[i+2] = '0';
			binary[i+3] = '0';
			break;
		}
	}
	//printf("HEX: %s\t BIN: %s\n", newHex, binary);
	return binary;
}

// Converts Binary as a String to decimal
long long binaryToInt(char * bin){
	//int ans = (int) strtol(bin, NULL, 2);
	long long ans = 0;
	int pos = strlen(bin) - 1;
	long long powers = 1;
	long long mult = 0;
	while(pos >= 0) {
		if ('1' == bin[pos]){
			mult = 1;
		} else {
			mult = 0;
		}
		//printf("%d", mult);
		ans += mult * powers;
		pos --;
		powers *= 2;
	}
   //printf("Bin(%d): %s\tDec: %lld\n", (int)strlen(bin), bin, ans);
   return ans;
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void readFromCache(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * indexBits = (char*)malloc(sizeof(char) * numIndexBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * numTagBits + 1);
	blockBits[numBlockBits]	= '\0';
	indexBits[numIndexBits]	= '\0';
	tagBits[numTagBits]	= '\0';
	
	int i = 0;
	if (cache->type == 1){
		// tag|index|block //
		while(i < numTagBits) {
			tagBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numIndexBits) {
			indexBits[i] = address[i+numTagBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	} else {
		// index|tag|block //
		while(i < numIndexBits) {
			indexBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numTagBits) {
			tagBits[i] = address[i+numIndexBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	}
	
	//printf("Address: %s\n", address);
	//printf("Tag(%d): %s\tIndex(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numIndexBits, indexBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[binaryToInt(indexBits)%(numSets)];
	int b = 0;
	int didHit = 0;
	Block * currentBlock = currentSet->headBlock;
	while(currentBlock != NULL && b < associativity){
		if (strcmp(currentBlock->tag, tagBits) == 0){
			didHit = 1;
			break;
		}
		currentBlock = currentBlock->nextBlock;
		b++;
	}
	
	if (didHit == 0){
		cache->misses++;
		cache->reads++;
		b = 0;
		currentBlock = currentSet->headBlock;
		if (currentBlock == NULL){
			Block * temp = malloc(sizeof(Block));
			temp->tag = tagBits;
			temp->nextBlock = NULL;
			currentSet->headBlock = temp;
		} else {
			while(b < associativity) {
				b++;
				if(b == associativity) {
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					currentSet->headBlock = currentSet->headBlock->nextBlock;
					break;
				} else if(currentBlock->nextBlock == NULL){
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					break;
				} 
				currentBlock = currentBlock->nextBlock;
			}
		}
	} else {
		cache->hits++;
	}
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void writeToCache(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * indexBits = (char*)malloc(sizeof(char) * numIndexBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * numTagBits + 1);
	blockBits[numBlockBits]	= '\0';
	indexBits[numIndexBits]	= '\0';
	tagBits[numTagBits]	= '\0';
	
	int i = 0;
	if (cache->type == 1){
		// tag|index|block //
		while(i < numTagBits) {
			tagBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numIndexBits) {
			indexBits[i] = address[i+numTagBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	} else {
		// index|tag|block //
		while(i < numIndexBits) {
			indexBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numTagBits) {
			tagBits[i] = address[i+numIndexBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	}
	
	//printf("Address: %s\n", address);
	//printf("Tag(%d): %s\tIndex(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numIndexBits, indexBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[binaryToInt(indexBits)%(numSets)];
	
	int b = 0;
	int didHit = 0;
	Block * currentBlock = currentSet->headBlock;
	while(currentBlock != NULL && b < associativity){
		if (strcmp(currentBlock->tag, tagBits) == 0){
			didHit = 1;
			break;
		}
		currentBlock = currentBlock->nextBlock;
		b++;
	}
	
	if (didHit == 0){
		cache->misses++;
		cache->writes++;
		cache->reads++;
		b = 0;
		currentBlock = currentSet->headBlock;
		if (currentBlock == NULL){
			Block * temp = malloc(sizeof(Block));
			temp->tag = tagBits;
			temp->nextBlock = NULL;
			currentSet->headBlock = temp;
		} else {
			while(b < associativity) {
				b++;
				if(b == associativity) {
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					currentSet->headBlock = currentSet->headBlock->nextBlock;
				} else if(currentBlock->nextBlock == NULL){
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					break;
				} 
				currentBlock = currentBlock->nextBlock;
			}
		}
	} else {
		cache->hits++;
		cache->writes++;
	}
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void fullyAssocRead(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * (48-numBlockBits) + 1);
	blockBits[numBlockBits]	= '\0';
	tagBits[(48-numBlockBits)]	= '\0';
	
	int i = 0;
	// tag|block //
	while(i < (48-numBlockBits)) {
		tagBits[i] = address[i];
		i++;
	}
	i = 0;
	while(i < numBlockBits) {
		blockBits[i] = address[i+(48-numBlockBits)];
		i++;
	}
	
	//printf("Address: %s\n", address);	
	//printf("Tag(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[0];
	int b = 0;
	int didHit = 0;
	Block * currentBlock = currentSet->headBlock;
	while(currentBlock != NULL && b < associativity){
		if (strcmp(currentBlock->tag, tagBits) == 0){
			didHit = 1;
			break;
		}
		currentBlock = currentBlock->nextBlock;
		b++;
	}
	
	if (didHit == 0){
		cache->misses++;
		cache->reads++;
		b = 0;
		currentBlock = currentSet->headBlock;
		if (currentBlock == NULL){
			Block * temp = malloc(sizeof(Block));
			temp->tag = tagBits;
			temp->nextBlock = NULL;
			currentSet->headBlock = temp;
		} else {
			while(b < associativity) {
				b++;
				if(b == associativity) {
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					currentSet->headBlock = currentSet->headBlock->nextBlock;
					break;
				} else if(currentBlock->nextBlock == NULL){
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					break;
				} 
				currentBlock = currentBlock->nextBlock;
			}
		}
	} else {
		cache->hits++;
	}
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void fullyAssocWrite(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * (48-numBlockBits) + 1);
	blockBits[numBlockBits]	= '\0';
	tagBits[(48-numBlockBits)]	= '\0';
	
	int i = 0;
	// tag|block //
	while(i < (48-numBlockBits)) {
		tagBits[i] = address[i];
		i++;
	}
	i = 0;
	while(i < numBlockBits) {
		blockBits[i] = address[i+(48-numBlockBits)];
		i++;
	}
	
	
	//printf("Address: %s\n", address);
	//printf("Tag(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[0];
	int b = 0;
	int didHit = 0;
	Block * currentBlock = currentSet->headBlock;
	while(currentBlock != NULL && b < associativity){
		if (strcmp(currentBlock->tag, tagBits) == 0){
			didHit = 1;
			break;
		}
		currentBlock = currentBlock->nextBlock;
		b++;
	}
	
	if (didHit == 0){
		cache->misses++;
		cache->writes++;
		cache->reads++;
		b = 0;
		currentBlock = currentSet->headBlock;
		if (currentBlock == NULL){
			Block * temp = malloc(sizeof(Block));
			temp->tag = tagBits;
			temp->nextBlock = NULL;
			currentSet->headBlock = temp;
		} else {
			while(b < associativity) {
				b++;
				if(b == associativity) {
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					currentSet->headBlock = currentSet->headBlock->nextBlock;
				} else if(currentBlock->nextBlock == NULL){
					Block * temp = malloc(sizeof(Block));
					temp->tag = tagBits;
					temp->nextBlock = NULL;
					currentBlock->nextBlock = temp;
					break;
				} 
				currentBlock = currentBlock->nextBlock;
			}
		}
	} else {
		cache->hits++;
		cache->writes++;
	}
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void directRead(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * indexBits = (char*)malloc(sizeof(char) * numIndexBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * numTagBits + 1);
	blockBits[numBlockBits]	= '\0';
	indexBits[numIndexBits]	= '\0';
	tagBits[numTagBits]	= '\0';
	
	int i = 0;
	if (cache->type == 1){
		// tag|index|block //
		while(i < numTagBits) {
			tagBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numIndexBits) {
			indexBits[i] = address[i+numTagBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	} else {
		// index|tag|block //
		while(i < numIndexBits) {
			indexBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numTagBits) {
			tagBits[i] = address[i+numIndexBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	}
	
	//printf("Address: %s\n", address);
	//printf("Tag(%d): %s\tIndex(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numIndexBits, indexBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[binaryToInt(indexBits)%(numSets)];
	Block * currentBlock = currentSet->headBlock;
	if (currentBlock != NULL && strcmp(currentBlock->tag, tagBits) == 0){
		cache->hits++;
	} else {
		cache->misses++;
		cache->reads++;
		currentBlock = currentSet->headBlock; 
		Block * temp = malloc(sizeof(Block));
		temp->tag = tagBits;
		temp->nextBlock = NULL;
		currentSet->headBlock = temp;
	}
}

//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************

void directWrite(Cache * cache, char* address){
	char * blockBits = (char*)malloc(sizeof(char) * numBlockBits + 1);
	char * indexBits = (char*)malloc(sizeof(char) * numIndexBits + 1);
	char * tagBits = (char*)malloc(sizeof(char) * numTagBits + 1);
	blockBits[numBlockBits]	= '\0';
	indexBits[numIndexBits]	= '\0';
	tagBits[numTagBits]	= '\0';
	
	int i = 0;
	if (cache->type == 1){
		// tag|index|block //
		while(i < numTagBits) {
			tagBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numIndexBits) {
			indexBits[i] = address[i+numTagBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	} else {
		// index|tag|block //
		while(i < numIndexBits) {
			indexBits[i] = address[i];
			i++;
		}
		i = 0;
		while(i < numTagBits) {
			tagBits[i] = address[i+numIndexBits];
			i++;
		}
		i = 0;
		while(i < numBlockBits) {
			blockBits[i] = address[i+numTagBits+numIndexBits];
			i++;
		}
	}
	
	//printf("Address: %s\n", address);
	//printf("[W] Tag(%d): %s\tIndex(%d): %s\tBlock(%d): %s\n",numTagBits, tagBits, numIndexBits, indexBits, numBlockBits, blockBits);
	Set * currentSet = cache->sets[binaryToInt(indexBits)%(numSets)];
	Block * currentBlock = currentSet->headBlock;
	if (currentBlock != NULL && strcmp(currentBlock->tag, tagBits) == 0){
		cache->hits++;
		cache->writes++;
	} else {
		cache->misses++;
		cache->reads++;
		cache->writes++;
		currentBlock = currentSet->headBlock;
		Block * temp = malloc(sizeof(Block));
		temp->tag = tagBits;
		temp->nextBlock = NULL;
		currentSet->headBlock = temp;
	}
}