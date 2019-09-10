int policy; // fifo = 0, lru = 1
int readWrite; //read = 0, write =1
char* traceFile;
int	tBytesInCache;
int	tBlockCount;	
int	tSetCount;
int	tBytesPerBlock;
int	tBlocksPerSet;
int tStart;								
int tLength;
int iStart;
int iLength;
unsigned int hCount;
unsigned int mCount;
unsigned int rCount;
unsigned int wCount;


struct cBlock{
	unsigned long tag;
	int valid;
	int LRUtracker;
};

struct cSet {
	struct cBlock* setBlocks;
	int	overwriteIndex;
};

int isPower2(char* num);
void checkAssoc(char* Assoc);
int getLRUIndex(struct cSet* curSet, int recentIndex);
void setCache(char* cSize, char* Assoc, char* cPolicy, char* inputBytesPerBlock, char* inputTraceFile);
int logTwo(int input);
void resetCache(struct cSet* ourCache, struct cBlock* totalBlocks);
unsigned long getPartFromLong(unsigned long input, int start, int length);
int readOrWrite(struct cSet* ourCache, char line[128], int pre, int adj);
void runSim(struct cSet* ourCache, int pre);
unsigned long xToNum(char* in); 

