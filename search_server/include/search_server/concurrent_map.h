#pragma once

#include <map>
#include <mutex>
#include <vector>


template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket {
        std::mutex Mutex;
        std::map<Key, Value> Map;
    };

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> Guard;
        Value& RefToValue;

        Access(const Key& key, Bucket& bucket)
            : Guard(bucket.Mutex),
            RefToValue(bucket.Map[key])
        {}
    };

    explicit ConcurrentMap(size_t bucketCount) : m_buckets(bucketCount) {}

    Access operator[](const Key& key) {
        auto& bucket = m_buckets[static_cast<uint64_t>(key) % m_buckets.size()];
        return { key, bucket };
    }

    size_t erase(const Key& key) {
        size_t result = 0;
        for (auto& [mutex, map] : m_buckets) {
            std::lock_guard g(mutex);
            result = map.erase(key);
        }
        return result;
    }

    inline auto begin() {
        return m_buckets.cbegin();
    }

    inline auto end() {
        return m_buckets.cend();
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mutex, map] : m_buckets) {
            std::lock_guard g(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private:
    std::vector<Bucket> m_buckets;
};
