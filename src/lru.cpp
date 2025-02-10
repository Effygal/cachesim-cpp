#include <vector>
#include <cassert>
#include <optional>
#include <cstdint>
#include <iostream>

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

class LRU {
    int C = 0;
    std::vector<int> m_map;
    int tail = 0, head = 0;
    std::vector<int> cache;
    std::vector<int> ref;
    std::vector<int> enter;
    int n_evict = 0;
    double s_evict_ref = 0;
    double s2_evict_ref = 0;
    int len = 0;
    int n_cachefill = 0;
    int n_access = 0;
    int n_miss = 0;
public:
    LRU(int _C) : C(_C) {
        m_map.resize(100000, -1);
        enter.resize(100000, -1);
        ref.resize(100000, -1);
        cache.resize(2 * _C, -1);
    }
    ~LRU() = default;

    void verify() const {
        for (int i = tail; i < head; i++)
            assert(cache[i] == -1 || m_map[cache[i]] == i);
        for (unsigned int i = 0; i < m_map.size(); i++) {
            if (m_map[i] != -1) {
                assert(cache[m_map[i]] == static_cast<int>(i));
                assert(m_map[i] < head);
            }
        }
    }

    std::vector<int> contents() const {
        std::vector<int> out;
        for (int i = head - 1; i >= 0; i--)
            if (cache[i] != -1)
                out.push_back(cache[i]);
        return out;
    }

    void pull(int addr) {
        assert(m_map[addr] != -1);
        cache[m_map[addr]] = -1;
        m_map[addr] = -1;
        len--;
    }

    int pop() {
        while (tail < head && cache[tail] == -1)
            tail++;
        assert(tail < head);
        int addr = cache[tail];
        pull(addr);
        return addr;
    }

    void push(int addr) {
        if (head >= 2 * C) {
            int j = 0;
            for (int i = 0; i < head; i++)
                if (cache[i] != -1) {
                    m_map[cache[i]] = j;
                    cache[j++] = cache[i];
                }
            tail = 0;
            head = j;
        }
        m_map[addr] = head;
        cache[head++] = addr;
        len++;
    }

    void access(unsigned int addr) {
        n_access++;
        if (len < C)
            n_cachefill = n_access;
        if (addr >= m_map.size())
            m_map.resize(addr * 3 / 2, -1);
        if (m_map[addr] == -1)
            n_miss++;
        else
            pull(addr);
        push(addr);
        if (len > C)
            pop();
    }

    struct Verbose {
        int miss;
        int evictee;
        int last_ref;
        int entered;
    };

    Verbose access_verbose(unsigned int addr) {
        n_access++;
        if (len < C)
            n_cachefill = n_access;
        if (addr >= m_map.size()) {
            int n = addr * 3 / 2;
            m_map.resize(n, -1);
            ref.resize(n, 0);
            enter.resize(n, 0);
        }
        int miss = 0;
        if (m_map[addr] == -1) {
            n_miss++;
            miss = 1;
            enter[addr] = n_access;
            ref[addr] = n_access;
        } else {
            ref[addr] = n_access;
            pull(addr);
        }
        push(addr);
        Verbose res {0, -1, 0, 0};
        if (len > C) {
            int e = pop();
            res.miss = 1;
            res.evictee = e;
            res.last_ref = n_access - ref[e];
            res.entered = n_access - enter[e];
            n_evict++;
            s_evict_ref += res.last_ref;
            s2_evict_ref += 1.0 * res.last_ref * res.last_ref;
        }
        return res;
    }

    struct QueueStats {
        int n_evict;
        double s_evict_ref;
        double s2_evict_ref;
    };

    QueueStats queue_stats() const {
        return {n_evict, s_evict_ref, s2_evict_ref};
    }

    void multi_access(const std::vector<int>& addrs) {
        for (int a : addrs)
            access(a);
    }

    std::vector<Verbose> multi_access_age(const std::vector<int>& addrs) {
        std::vector<Verbose> results;
        results.reserve(addrs.size());
        for (int a : addrs)
            results.push_back(access_verbose(a));
        return results;
    }

    double hit_rate() const {
        int effective = n_access - n_cachefill;
        if (effective <= 0)
            return 1.0;
        double miss_rate = (n_miss - C) * 1.0 / effective;
        return 1.0 - miss_rate;
    }

    void data(int& _access, int& _miss, int& _cachefill) const {
        _access = n_access;
        _miss = n_miss;
        _cachefill = n_cachefill;
    }
};


