#include <vector>
#include <cstdint>
#include <cassert>

/*
   CLOCK algorithm using a circular buffer and vector mapping:

         +-----+-----+-----+-----+-----+-----+
         |  0  |  1  |  2  |  3  | ... |  C  |
         +-----+-----+-----+-----+-----+-----+
           ^                           ^
           |                           |
         outIdx                      inIdx

   - The circular buffer (cache[]) holds cache entries (addresses).
   - "inIdx" pouint32_ts to the next free slot for a new entry.
   - "outIdx" pouint32_ts to the next candidate for eviction.
   - Each slot has an associated second-chance flag (abit) stored in a vector.
   - A separate vector mapping (inCache) tracks which addresses are present.
   - When the cache is full, the algorithm examines the entry at outIdx:
         • If its second-chance flag is set, the flag is cleared and the entry
           is given another chance by re-inserting it (i.e. moved to the inIdx side).
         • Otherwise, that entry is evicted.
*/

class Clock {
    uint32_t cap;
    std::vector<uint32_t> cache;
    std::vector<uint32_t> tEnt;
    std::vector<int> abit;
    std::vector<bool> inCache;
    std::vector<uint32_t> ent;
    std::vector<uint32_t> ref;
    uint32_t nTop = 0;
    double sumTop = 0, sumTop2 = 0;
    uint32_t inIdx = 0, outIdx = 0;
    uint32_t nFill = 0;
    uint32_t nAcc = 0;
    uint32_t nMiss = 0;
    uint32_t nRecycle = 0;
    uint32_t nExam = 0;
    uint32_t sumAbit = 0;
public:
    struct Verbose {
        int32_t evict;
        uint32_t miss;
        uint32_t refAge;
        uint32_t entAge;
    };

    Clock(uint32_t _cap) : cap(_cap) {
        cache.resize(cap + 1);
        tEnt.resize(cap + 1);
        abit.resize(100000, 0);
        inCache.resize(100000, false);
        ent.resize(100000, 0);
        ref.resize(100000, 0);
    }

    bool full() const {
        return (inIdx + 1) % (cap + 1) == outIdx;
    }

    uint32_t pop() {
        uint32_t a = cache[outIdx];
        uint32_t age = nAcc - tEnt[outIdx];
        outIdx = (outIdx + 1) % (cap + 1);
        inCache[a] = false;
        sumTop += age;
        sumTop2 += static_cast<double>(age) * age;
        nTop++;
        return a;
    }

    void push(uint32_t a) {
        cache[inIdx] = a;
        tEnt[inIdx] = nAcc;
        inIdx = (inIdx + 1) % (cap + 1);
        inCache[a] = true;
        abit[a] = 0;
    }

    std::vector<uint32_t> contents() const {
        std::vector<uint32_t> res;
        uint32_t start = (inIdx + cap) % (cap + 1);
        uint32_t i = start;
        do {
            res.push_back(cache[i]);
            if (i == outIdx) break;
            i = (i + cap) % (cap + 1);
        } while (true);
        return res;
    }

    void access(uint32_t a) {
        nAcc++;
        if (a >= static_cast<uint32_t>(inCache.size())) {
            uint32_t n = a * 3 / 2;
            inCache.resize(n, false);
            abit.resize(n, 0);
            ent.resize(n, 0);
            ref.resize(n, 0);
        }
        if (!inCache[a]) {
            nMiss++;
            if (!full())
                nFill = nAcc;
            else {
                while (true) {
                    uint32_t ev = pop();
                    nExam++;
                    if (abit[ev]) {
                        sumAbit += abit[ev];
                        abit[ev] = 0;
                        push(ev);
                        nRecycle++;
                    } else break;
                }
            }
            push(a);
            ent[a] = nAcc;
            ref[a] = nAcc;
        } else {
            abit[a] = 1;
            ref[a] = nAcc;
        }
    }

    Verbose accessVerbose(uint32_t a) {
        Verbose v { -1, 0, 0, 0 };
        nAcc++;
        if (a >= static_cast<uint32_t>(inCache.size())) {
            uint32_t n = a * 3 / 2;
            inCache.resize(n, false);
            abit.resize(n, 0);
            ent.resize(n, 0);
            ref.resize(n, 0);
        }
        if (!inCache[a]) {
            nMiss++;
            if (!full())
                nFill = nAcc;
            else {
                while (true) {
                    uint32_t ev = pop();
                    if (abit[ev]) {
                        abit[ev] = 0;
                        push(ev);
                        nRecycle++;
                    } else {
                        v.miss = 1;
                        v.evict = ev;
                        v.refAge = nAcc - ref[ev];
                        v.entAge = nAcc - ent[ev];
                        break;
                    }
                }
            }
            push(a);
            ent[a] = nAcc;
            ref[a] = nAcc;
        } else {
            abit[a] = 1;
            ref[a] = nAcc;
        }
        return v;
    }

    void multiAccess(const std::vector<uint32_t>& addrs) {
        for (auto a : addrs)
            access(a);
    }

    std::vector<Verbose> multiAccessVerbose(const std::vector<uint32_t>& addrs) {
        std::vector<Verbose> res;
        res.reserve(addrs.size());
        for (auto a : addrs)
            res.push_back(accessVerbose(a));
        return res;
    }

    double hitRate() const {
        uint32_t effective = nAcc - nFill;
        if (effective <= 0)
            return 1.0;
        double missRate = (nMiss - cap) * 1.0 / effective;
        return 1 - missRate;
    }

    struct Stats {
        uint32_t nAcc;
        uint32_t nMiss;
        uint32_t nFill;
        uint32_t nRecycle;
        uint32_t nExam;
        uint32_t sumAbit;
    };

    Stats data() const {
        return { nAcc, nMiss, nFill, nRecycle, nExam, sumAbit };
    }

    struct QStats {
        uint32_t nTop;
        double sumTop;
        double sumTop2;
    };

    QStats queueStats() const {
        return { nTop, sumTop, sumTop2 };
    }
};
