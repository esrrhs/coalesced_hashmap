#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <atomic>
#include <vector>
#include <map>
#include <sys/types.h>
#include <ctime>
#include <numeric>
#include <string>
#include <cstddef>
#include <unistd.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <functional>
#include <chrono>
#include <iterator>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>
#include <queue>
#include <iostream>
#include <type_traits>

namespace coalesced_hashmap {

template<typename Key, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct DefaultIsValid {
    bool operator()(const Key& key) const {
        return !Equal()(key, Key());
    }
};

static const int primes[] = {
    2, 5, 7, 11, 17, 23, 37, 53, 79, 113, 167, 251, 373, 557, 839, 1259, 1889,
    2833, 4243, 6361, 9533, 14249, 21373, 32059, 48089, 72131, 108197, 162293,
    243439, 365159, 547739, 821609, 1232413, 1848619, 2772929, 4159393, 6239089,
    9358633, 14037949, 21056923, 31585387, 47378081, 71067121, 106600683,
    159901019, 239851529, 359777293, 539665939, 809498909, 1214247359,
    1821371039
};

template<typename Key, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename IsValid = DefaultIsValid<Key, Hash, Equal>>
class CoalescedHashSet {
public:
    CoalescedHashSet(int size = 1) {
        m_nodes = new Node[size];
        m_size = size;
        InitFreeList();
    }

    ~CoalescedHashSet() {
        delete[] m_nodes;
    }

    void Insert(const Key& key) {
        size_t h = Hash()(key);
        auto mp = h % m_size;
        if (Valid(mp)) {
            auto cur = mp;
            while (cur != -1) {
                if (Equal()(m_nodes[cur].key, key)) {
                    m_nodes[cur].key = key;
                    return;
                }
                cur = m_nodes[cur].next;
            }

            auto f = GetFreePosition();
            if (f < 0) {
                Rehash();
                return Insert(key);
            }
            
            auto othern = Hash()(m_nodes[mp].key) % m_size;
            if (othern != mp) {
                auto pre = m_nodes[mp].pre;
                assert(pre != -1);
                auto next = m_nodes[mp].next;
                m_nodes[pre].next = f;
                if (next != -1) {
                    m_nodes[next].pre = f;
                }
                m_nodes[f] = m_nodes[mp];
                m_nodes[mp].Clear();
            } else {
                auto next = m_nodes[mp].next;
                if (next != -1) {
                    m_nodes[f].next = next;
                    m_nodes[next].pre = f;
                }
                m_nodes[mp].next = f;
                m_nodes[f].pre = mp;
                mp = f;
            }
        } else {
            auto pre = m_nodes[mp].pre;
            auto next = m_nodes[mp].next;
            if (pre != -1) m_nodes[pre].next = next;
            if (next != -1) m_nodes[next].pre = pre;
            if (m_free == mp) m_free = next;
            m_nodes[mp].Clear();
        }
        m_nodes[mp].key = key;
    }

    template<typename OtherKey>
    bool Find(OtherKey other_key, Key& key) {
        if (m_size == 0) return false;
        auto mp = Hash()(other_key) % m_size;
        while (mp != -1) {
            if (Equal()(m_nodes[mp].key, other_key)) {
                key = m_nodes[mp].key;
                return true;
            }
            mp = m_nodes[mp].next;
        }
        return false;
    }

    template<typename OtherKey, typename Func>
    bool FindAndExecute(const OtherKey& other_key, Func func) {
        if (m_size == 0) return false;
        auto mp = Hash()(other_key) % m_size;
        while (mp != -1) {
            if (Equal()(m_nodes[mp].key, other_key)) {
                func(m_nodes[mp].key);
                return true;
            }
            mp = m_nodes[mp].next;
        }
        return false;
    }

    bool Contains(const Key& key) {
        if (m_size == 0) return false;
        auto mp = Hash()(key) % m_size;
        while (mp != -1) {
            if (Equal()(m_nodes[mp].key, key)) {
                return true;
            }
            mp = m_nodes[mp].next;
        }
        return false;
    }

    template<typename OtherKey>
    bool Erase(const OtherKey& other_key) {
        if (m_size == 0) return false;
        auto mp = Hash()(other_key) % m_size;
        auto cur = mp;
        while (cur != -1) {
            if (Equal()(m_nodes[cur].key, other_key)) {
                auto clear_pos = cur;
                auto pre = m_nodes[cur].pre;
                auto next = m_nodes[cur].next;
                if (pre != -1) {
                    m_nodes[pre].next = next;
                }
                if (next != -1) {
                    m_nodes[next].pre = pre;
                    if (mp == cur) {
                        auto next_next = m_nodes[next].next;
                        m_nodes[cur] = m_nodes[next];
                        m_nodes[cur].pre = -1;
                        if (next_next != -1) {
                            m_nodes[next_next].pre = cur;
                        }
                        clear_pos = next;
                    }
                }
                m_nodes[clear_pos].Clear();
                m_nodes[clear_pos].next = m_free;
                m_nodes[clear_pos].pre = -1;
                if (m_free != -1) {
                    m_nodes[m_free].pre = clear_pos;
                }
                m_free = clear_pos;
                return true;
            }
            cur = m_nodes[cur].next;
        }
        return false;
    }

    int Capacity() const {
        return m_size;
    }

    int Size() const {
        int ret = 0;
        for (int i = 0; i < m_size; i++) {
            if (Valid(i)) {
                ret++;
            }
        }
        return ret;
    }

    int MainPositionSize() const {
        int ret = 0;
        for (int i = 0; i < m_size; i++) {
            if (Valid(i)) {
                size_t h = Hash()(m_nodes[i].key);
                if (h % m_size == (size_t)i) {
                    ret++;
                }
            }
        }
        return ret;
    }

    std::map<int, int> ChainStatus() const {
        std::map<int, int> ret;
        for (int i = 0; i < m_size; i++) {
            if (Valid(i)) {
                if (m_nodes[i].pre == -1) {
                    int count = 0;
                    int next = i;
                    while (next != -1) {
                        count++;
                        next = m_nodes[next].next;
                    }
                    ret[count]++;
                }
            }
        }
        return ret;
    }

    class Iterator {
    public:
        Iterator(CoalescedHashSet* map) : m_map(map) {
            m_index = 0;
            while (m_index < m_map->m_size && !m_map->Valid(m_index)) {
                m_index++;
            }
        }

        Iterator(CoalescedHashSet* map, int index) : m_map(map), m_index(index) {
        }

        const Key& GetKey() const {
            return m_map->m_nodes[m_index].key;
        }

        Iterator& operator++() {
            m_index++;
            while (m_index < m_map->m_size && !m_map->Valid(m_index)) {
                m_index++;
            }
            return *this;
        }

        bool operator!=(const Iterator& other) {
            return m_index != other.m_index;
        }

    private:
        CoalescedHashSet* m_map;
        int m_index;
    };

    Iterator Begin() {
        return Iterator(this);
    }

    Iterator End() {
        return Iterator(this, m_size);
    }

private:
    struct Node {
        Key key;
        int pre = -1;
        int next = -1;

        void Clear() {
            key = Key();
            pre = -1;
            next = -1;
        }
    };

    bool Valid(int index) const {
        return IsValid()(m_nodes[index].key);
    }

    int GetFreePosition() {
        if (m_free == -1) {
            return -1;
        }
        auto ret = m_free;
        auto next = m_nodes[m_free].next;
        if (next != -1) {
            m_nodes[next].pre = -1;
        }
        m_free = next;
        m_nodes[ret].Clear();
        return ret;
    }

    int FindNextCapacity(int n) {
        int size = sizeof(primes) / sizeof(primes[0]);
        int left = 0;
        int right = size - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (primes[mid] == n) {
                return primes[mid];
            } else if (primes[mid] < n) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return (left < size) ? primes[left] : -1;
    }

    void InitFreeList() {
        for (int i = 0; i < m_size - 1; i++) {
            m_nodes[i + 1].pre = i;
            m_nodes[i].next = i + 1;
        }
        m_nodes[0].pre = -1;
        m_nodes[m_size - 1].next = -1;
        m_free = 0;
    }

    int Rehash() {
        auto size = FindNextCapacity(Capacity() * 2 + 1);
        if (size == -1) return -1;
        auto oldnodes = m_nodes;
        auto oldsize = m_size;
        m_nodes = new Node[size];
        m_size = size;
        InitFreeList();
        for (int i = 0; i < oldsize; i++) {
            if (IsValid()(oldnodes[i].key)) {
                Insert(oldnodes[i].key);
            }
        }
        delete[] oldnodes;
        return 0;
    }

private:
    int m_free = 0; // free list head
    int m_size = 0;
    Node* m_nodes;
};

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename IsValid = DefaultIsValid<Key, Hash, Equal>>
class CoalescedHashMap {
private:
    struct KeyValue {
        Key key;
        Value value;
        KeyValue() : key(Key()), value(Value()) {}
        KeyValue(const Key& k, const Value& v) : key(k), value(v) {}
    };

    struct KeyValueHash {
        size_t operator()(const KeyValue& kv) const {
            return Hash()(kv.key);
        }

        size_t operator()(const Key& k) const {
            return Hash()(k);
        }
    };

    struct KeyValueEqual {
        bool operator()(const KeyValue& lhs, const KeyValue& rhs) const {
            return Equal()(lhs.key, rhs.key);
        }

        bool operator()(const KeyValue& lhs, const Key& rhs) const {
            return Equal()(lhs.key, rhs);
        }
    };

    struct KeyValueIsValid {
        bool operator()(const KeyValue& kv) const {
            return IsValid()(kv.key);
        }
    };

    typedef CoalescedHashSet<KeyValue, KeyValueHash, KeyValueEqual, KeyValueIsValid> CoalescedHashSetType;
    CoalescedHashSetType m_set;

public:
    CoalescedHashMap(int size = 1) : m_set(size) {
    }

    ~CoalescedHashMap() {
    }

    void Insert(const Key& key, const Value& value) {
        m_set.Insert({key, value});
    }

    bool Find(const Key& key, Value& value) {
        return m_set.FindAndExecute(key, [&](const KeyValue& kv) {
            value = kv.value;
        });
    }

    bool Erase(const Key& key) {
        return m_set.Erase(key);
    }

    int Capacity() const {
        return m_set.Capacity();
    }

    int Size() const {
        return m_set.Size();
    }

    int MainPositionSize() const {
        return m_set.MainPositionSize();
    }

    std::map<int, int> ChainStatus() const {
        return m_set.ChainStatus();
    }

    class Iterator {
    public:
        Iterator(typename CoalescedHashSetType::Iterator it) : m_set_iter(it) {
        }

        const Key& GetKey() const {
            return m_set_iter.GetKey().key;
        }

        const Value& GetValue() const {
            return m_set_iter.GetKey().value;
        }

        Iterator& operator++() {
            ++m_set_iter;
            return *this;
        }

        bool operator!=(const Iterator& other) {
            return m_set_iter != other.m_set_iter;
        }

    private:
        typename CoalescedHashSetType::Iterator m_set_iter;
    };

    Iterator Begin() {
        return Iterator(m_set.Begin());
    }

    Iterator End() {
        return Iterator(m_set.End());
    }
};
}
