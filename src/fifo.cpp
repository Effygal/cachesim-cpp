#include <vector>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <utility>

class fifo {
    int C;
    std::vector<int> cache;
    std::vector<int> enter;
    int n_evict;
    double s_evict;
    double s2_evict;
    std::vector<bool> m;
    int in_idx, out_idx;
    int n_cf;
    int n_acc;
    int n_miss;
public:
    fifo(int _C) : C(_C), n_evict(0), s_evict(0), s2_evict(0), in_idx(0), out_idx(0), n_cf(0), n_acc(0), n_miss(0) {
        cache.resize(C + 1);
        enter.resize(C + 1, 0);
        m.resize(100000, false);
    }
    ~fifo() = default;

    void access(int32_t addr) {
        n_acc++;
        if (addr >= static_cast<int>(m.size()))
            m.resize(addr * 3 / 2, false);
        assert(addr < static_cast<int>(m.size()));
        if (!m[addr]) {
            n_miss++;
            if ((in_idx + 1) % (C + 1) != out_idx)
                n_cf = n_acc;
            else {
                int ev = cache[out_idx];
                int age = n_acc - enter[out_idx];
                n_evict++;
                s_evict += age;
                s2_evict += 1.0 * age * age;
                out_idx = (out_idx + 1) % (C + 1);
                m[ev] = false;
            }
            cache[in_idx] = addr;
            enter[in_idx] = n_acc;
            m[addr] = true;
            in_idx = (in_idx + 1) % (C + 1);
        }
        assert(in_idx >= 0 && in_idx < C + 1 && out_idx >= 0 && out_idx < C + 1);
    }

    std::vector<int> contents() const {
        std::vector<int> res;
        int i = (in_idx + C) % (C + 1);
        do {
            res.push_back(cache[i]);
            if (i == out_idx)
                break;
            i = (i + C) % (C + 1);
        } while (true);
        return res;
    }

    void multi_access(const std::vector<int32_t>& addrs) {
        for (auto a : addrs)
            access(a);
    }

    std::vector<int> multi_access_verbose(const std::vector<int32_t>& addrs) {
        std::vector<int> res(addrs.size(), 0);
        for (size_t i = 0; i < addrs.size(); i++) {
            int prev = n_miss;
            access(addrs[i]);
            if (n_miss != prev)
                res[i] = 1;
        }
        return res;
    }

    std::pair<std::vector<int>, std::vector<int>> multi_access_age(const std::vector<int32_t>& addrs) {
        std::vector<int> misses(addrs.size(), 0);
        std::vector<int> ages(addrs.size(), 0);
        std::vector<int> times(C, 0);
        int t = 1;
        for (size_t i = 0; i < addrs.size(); i++, t++) {
            n_acc++;
            int32_t addr = addrs[i];
            if (addr >= static_cast<int>(m.size()))
                m.resize(addr * 3 / 2, false);
            if (!m[addr]) {
                n_miss++;
                misses[i] = 1;
                if ((in_idx + 1) % (C + 1) != out_idx)
                    n_cf = n_acc;
                else {
                    int ev = cache[out_idx];
                    ages[i] = t - times[out_idx];
                    out_idx = (out_idx + 1) % (C + 1);
                    m[ev] = false;
                }
                cache[in_idx] = addr;
                times[in_idx] = t;
                m[addr] = true;
                in_idx = (in_idx + 1) % (C + 1);
            }
        }
        return {misses, ages};
    }

    double hit_rate() const {
        int effective = n_acc - n_cf;
        if (effective <= 0)
            return 1.0;
        double miss_rate = (n_miss - C) * 1.0 / effective;
        return 1.0 - miss_rate;
    }

    struct Stats {
        int n_evict;
        double s_evict;
        double s2_evict;
    };

    Stats queue_stats() const {
        return {n_evict, s_evict, s2_evict};
    }

    void data(int &acc, int &miss, int &cf) const {
        acc = n_acc;
        miss = n_miss;
        cf = n_cf;
    }
};

#ifdef TEST_FIFO
int main() {
    fifo f(3);
    std::vector<int32_t> addrs = {1, 2, 3, 4, 1, 5, 2};
    for (auto a : addrs)
        f.access(a);
    auto cont = f.contents();
    std::cout << "FIFO contents: ";
    for (auto v : cont)
        std::cout << v << " ";
    std::cout << "\nHit rate: " << f.hit_rate() << "\n";
    auto st = f.queue_stats();
    std::cout << "Evictions: " << st.n_evict << ", s_evict: " << st.s_evict << ", s2_evict: " << st.s2_evict << "\n";
    return 0;
}
#endif
