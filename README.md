# Cache-Simulator
PA4: Cache Simulation
by Sinan Sahin
This program implements a cache simulator to evaluate different configurations of caches.
Usage:
./first <cache size> <associativity> <block size> <trace file>
where:
A) < cachesize > is the total size of the cache in
bytes. This number will be a power of 2.
B) < associativity > is one of:
  direct - simulate a direct mapped cache.
  assoc - simulate a fully associative cache.
  assoc:n - simulate an n âˆ’ way associative cache. n will be a power of 2.
C) < blocksize > is a power of 2 integer that speci
fies the size of the cache block in bytes.
D) < tracefile > is the name of the trace file.

The structure of my cache is as follows:
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

Explanation of Cache Data Structure:
Program first declares a pointer cache which holds the meta data and an array of Sets. 
The number of Sets are determined by the inputs to the program. 
Then each set holds a pointer to a Block node which more blocks are chained to as needed. 
Up to n block can be chained together; n is determined by the associativity.
