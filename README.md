# cachesim

A simple library implementing three cache replacement algorithms: **LRU**, **FIFO**, and **CLOCK**. Assume all integer items.

## Cache Algorithms

### LRU 2C-array Algorithm
We introduce an LRU 2C-array algorithm; it is an efficient method for implementing a cache eviction policy using a fixed-size array; it is particularly suitable for scenarios where memory constraints prevent the use of linked lists.
```

/*
  +-+-+-+-+-+-+-+-+-+-+-+-+-+
  | | |x| |x| |x| | | | | | |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+
   0   ^         ^
	   |         head ->
	   tail ->
  head points to first unoccupied entry
  tail may point to empty entry
 */
 ```
 - Instead of using a linked list, the algorithm keeps an array A of size 2C, where C represents the cache size.
- Array A stores items (e.g. pointers) in the cache and NULL values to indicate empty slots.
- Keeps a hash table H to store integer indexes of the items in A.
- `head` points to the first unoccupied entry in A, and tail points to the oldest item in A, might be NULL.
#### Operations
```
Initialize head = tail = 0

Insert (i.e. item not in cache):
    A[head] = item
    H[item] = head
    head++

Evict (if |A!=NULL| == C, do before inserting):
    while A[tail] == NULL
        tail++
    item = A[tail]
    delete H[item]
    A[tail++] = NULL

Access (item already in cache):
    i = H[item]
    A[i] = NULL
    insert(item)

If (`head' reaches the end of A):
    copy all non-null items to the bottom of A
    now tail == 0
        head <= C
```
### CLOCK Circular Buffer Algorithm
```
CLOCK algorithm using a circular buffer and vector mapping:

        +-----+-----+-----+-----+-----+-----+
        |  0  |  1  |  2  |  3  | ... |  C  |
        +-----+-----+-----+-----+-----+-----+
        ^                           ^
        |                           |
        outIdx                      inIdx
```
- The circular buffer (cache[]) holds cache entries (addresses).
- "inIdx" pouint32_ts to the next free slot for a new entry.
- "outIdx" pouint32_ts to the next candidate for eviction.
- Each slot has an associated second-chance flag (abit) stored in a vector.
- A separate vector mapping (inCache) tracks which addresses are present.
- When the cache is full, the algorithm examines the entry at outIdx:
        • If its second-chance flag is set, the flag is cleared and the entry
        is given another chance by re-inserting it (i.e. moved to the inIdx side).
        • Otherwise, that entry is evicted.

## Building and Testing

### Prerequisites
- [Meson](https://mesonbuild.com/) and [Ninja](https://ninja-build.org/)
- A C++20-compliant compiler (e.g. GCC 10+, Clang 10+)

### Build
```bash
meson setup build
meson compile -C build
```

### Test
```bash
./build/ctest <test trace> <cache size>
```
Assuming the input test trace is in SPC format. You would see below output:
```
=== Cache Simulation Statistics ===

LRU:
  Accesses: 10000, Misses: 8878, Cache fill time: 30
  Hit rate: 0.112538

FIFO:
  Accesses: 10000, Misses: 7919, Cache fill time: 30
  Hit rate: 0.208726

CLOCK:
  Accesses: 10000, Misses: 8820, Cache fill time: 30, Recycles: 1164
  Hit rate: 0.118355
```