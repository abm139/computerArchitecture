#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "first.h"

int main(int argc, char** argv) 
{
	if (argc!= 6)
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
	char* cSize = argv[1];
	char* Assoc = argv[2];
	char* cPolicy = argv[3];
	char* inputBytesPerBlock = argv[4];
	char* inputTraceFile = argv[5];
	
	if(isPower2(cSize) != 1 || isPower2(inputBytesPerBlock) != 1)
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
	
	if(strcmp(cPolicy,"fifo")==0)
		policy = 0;
	else if(strcmp(cPolicy,"lru")==0)
		policy = 1;
	else
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
	
	setCache(cSize, Assoc, cPolicy, inputBytesPerBlock, inputTraceFile );
	
	struct cBlock* totalBlocks = malloc(sizeof(struct cBlock) * tBlockCount);
	struct cSet* ourCache = malloc(sizeof(struct cSet) * tSetCount);
	
	rCount = 0;
	wCount = 0;
	hCount = 0;
	mCount = 0;
	resetCache(ourCache, totalBlocks);
	runSim(ourCache, 0);
	printf("no-prefetch\n");
	printf("Memory reads: %d\n", rCount);
	printf("Memory writes: %d\n", wCount);
	printf("Cache hits: %d\n", hCount);
	printf("Cache misses: %d\n", mCount);

	rCount = 0;
	wCount = 0;
	hCount = 0;
	mCount = 0;
	resetCache(ourCache, totalBlocks);
	runSim(ourCache, 1);
	printf("with-prefetch\n");
	printf("Memory reads: %d\n", rCount);
	printf("Memory writes: %d\n", wCount);
	printf("Cache hits: %d\n", hCount);
	printf("Cache misses: %d\n", mCount);

	free(totalBlocks);
	free(ourCache);
	return 0;
}

int isPower2(char* num)
{
	int i;
	int len = strlen(num);
	for(i = 0; i < len; i++)
	{
		if(!isdigit(num[i]))
			return 0;
	}
	
	int input = atoi(num);
	for(i = 1; i <= input; i = i*2)
	{
		if(i == input)
			return 1;
	}
	return 0;
}


void checkAssoc(char* Assoc)
{
	if (strstr(Assoc, "assoc:") != NULL) 
	{
		int len = strlen("assoc:");
		if(!isPower2(Assoc+len))
		{
			fprintf(stderr,"error\n");
			exit(1);
		}
		tBlocksPerSet = atoi(Assoc+len);
	}
	else 
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
}

int getLI(struct cSet* Set, int recentIndex){
	int i = 0;	
	while(i < tBlocksPerSet)
	{
		struct cBlock* Block = Set->setBlocks + i;
		if(i != recentIndex)
		{
			if(Block->LRUtracker == -1)
				return i;
			else
				Block->LRUtracker = Block->LRUtracker + 1;
		}
		else
			Block->LRUtracker  = 0;
			
		i++;	
	}
	int LRUindex = 0;
	int LRUvalue = 0;
	i = 0;
	while(i < tBlocksPerSet)
	{
		struct cBlock* Block = Set->setBlocks + i;
		if(Block->LRUtracker > LRUvalue)
		{
				LRUvalue = Block->LRUtracker;
				LRUindex = i;
		}
		i++;
	}
	return LRUindex;
}

void setCache(char* cSize, char* Assoc, char* cPolicy, char* inputBytesPerBlock, char* inputTraceFile) 
{
	tBytesInCache = atoi(cSize);
	tBytesPerBlock	= atoi(inputBytesPerBlock);
	tBlockCount = tBytesInCache/tBytesPerBlock;
	iStart = logTwo(tBytesPerBlock);
	traceFile = inputTraceFile;
	
	if (strcmp(Assoc,"direct") == 0)
		tBlocksPerSet = 1;
	else if (strcmp(Assoc, "assoc")==0) 
		tBlocksPerSet = tBlockCount;
	else
		checkAssoc(Assoc);
	
	tSetCount = tBlockCount / tBlocksPerSet;
	tLength = 48 - logTwo(tBytesPerBlock) - logTwo(tSetCount);		
	iLength = logTwo(tSetCount);
	tStart = logTwo(tBytesPerBlock) + logTwo(tSetCount);

	if (tBytesInCache <= 0 || tBytesPerBlock <= 0 || tBlocksPerSet <=0 || tBlockCount <= 0 || tSetCount <= 0){
		fprintf(stderr,"error\n");
		exit(1);
	}
	if(tBytesInCache != tBytesPerBlock * tBlockCount)
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
	if(tBytesInCache != tBytesPerBlock * tBlocksPerSet * tSetCount) 
	{
		fprintf(stderr,"error\n");
		exit(1);
	}
	
}

int logTwo(int input) {
	int index = 0;
	while(index < 31) 
	{
		if ((input >> index) == 1)	
			break;
		index++;
	}
	return index;
}

void resetCache(struct cSet* ourCache, struct cBlock* totalBlocks) {
	int i;
	for (i=0; i < tBlockCount; i++) 
	{
		struct cBlock* Block = totalBlocks + i;
		Block->tag	= 0;
		Block->valid	= 0;
		Block->LRUtracker = -1;
	}

	for (i=0; i < tSetCount; i++) 
	{
		struct cSet* Set = ourCache + i;
		Set->setBlocks	= totalBlocks + i * tBlocksPerSet;
		Set->overwriteIndex = 0;
	}
}

unsigned long getPiece(unsigned long input, int start, int length) {
	unsigned long output;
	int longBytes = 64;
	int lstart = length + start;
	
	if (length == 0) 
	{
		output = 0;
	}
	else 
	{
		output  = input << (longBytes  - lstart);
		output  = output >> (longBytes  - length);
	}

	return output;
}

int readOrWrite(struct cSet* ourCache, char line[128], int pre, int adj) {
	char rW;
	char sAddress[32];
	char pC[32];
	
	sscanf(line, "%s %c %s", pC, &rW, sAddress);
	
	if (strcmp(pC, "#eof")==0) 
		return 0;
	
	unsigned long address = xToNum(sAddress);
	if(adj == 1)
		address += tBytesPerBlock;
	
	unsigned long tag = getPiece(address, tStart, tLength);
	int index = (int)getPiece(address, iStart, iLength);
	struct cSet* Set = ourCache + index;

	int hit = 0;
	int i = 0;
	while(i < tBlocksPerSet) {
		struct cBlock* Block = (Set->setBlocks + i);
		if (Block->valid == 1 && Block->tag == tag ) 
		{
			if(adj == 0)
			{
				hCount++;
				if(policy == 1)
					Set->overwriteIndex = getLI(Set, i);
				
				if (rW == 'w' || rW == 'W')
					wCount++;
			}
			hit = 1;
			break;
		}
		i++;
	}

	if (hit == 0) 
	{
		struct cBlock* Block = (Set->setBlocks + Set->overwriteIndex);
		
		if(policy == 1)
			Set->overwriteIndex = getLI(Set, Set->overwriteIndex);
		else
		{
			Set->overwriteIndex++;
			Set->overwriteIndex = Set->overwriteIndex % tBlocksPerSet;
		}		

		rCount++;

		Block->tag = tag;
		Block->valid = 1;
		if(adj == 0)
		{
			mCount++;
			if (rW == 'w' || rW == 'W') 
				wCount++;
		}

		if(pre == 1)
			readOrWrite(ourCache, line, 0, 1);
	}
	return 1;
}

void runSim(struct cSet* ourCache, int pre) {
    FILE *file = fopen(traceFile, "r");
	if (file == NULL)
	{
		fprintf(stderr, "error\n");
		exit(1);
	}
	else 
	{
		char line[128];
		while (fgets(line, 128, file) != NULL) 
		{
			if(readOrWrite(ourCache, line, pre, 0) == 0)
				break;
		}
 	}
	if (file) {
		fclose(file);
	}
}

unsigned long xToNum(char* in) {
	unsigned long ans = 0;
	int index = 0;

	if (*(in + index) == '0')
		index = index + 1;
		
	if (*(in + index + 1) == 'x' || *(in + index + 1) == 'X') 
		index = index + 1;

	for(;*(in + index) != 0;index++) 
	{
		ans = ans * 16;
		char one = *(in + index);

		if (one >= 'A' && one <= 'F') 
			ans = ans + (one - 'A') + 10;
		else if (one > '0' && one <= '9' ) 
			ans = ans + (one - '0');
		else if (one >= 'a' && one <= 'f' )
			ans = ans + (one - 'a') + 10;

	}
	return ans;
}

