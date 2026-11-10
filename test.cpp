#include "coalesced_hashmap.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace coalesced_hashmap;

int64_t random_int64() {
    int64_t r = ((int64_t) rand() << 48) | ((int64_t) rand() << 32) | ((int64_t) rand() << 16) | rand();
    return r == 0 ? 1 : r;
}

int test() {
    srand(time(NULL));
    int N = 1000000;
    CoalescedHashMap<std::string, int> map;
    CoalescedHashSet<std::string> set;

    for (int j = 0; j < 5; j++) {
        std::map<int64_t, int> test_number;

        for (int i = 0; i < N; i++) {
            auto key = random_int64();
            test_number[key] = i;
        }

        std::cout << "test number size:" << test_number.size() << std::endl;

        for (auto &i: test_number) {
            map.Insert(std::to_string(i.first), i.second);
            set.Insert(std::to_string(i.first));
        }

        std::cout << "size:" << map.Size() << std::endl;

        int value = 0;
        for (auto &i: test_number) {
            if (map.Find(std::to_string(i.first), value)) {
//                std::cout << "key: " << i.first << " value: " << value << std::endl;
            } else {
                std::cout << "key: " << i.first << " not found" << std::endl;
                return -1;
            }
            if (value != i.second) {
                std::cout << "key: " << i.first << " value: " << value << " not equal" << std::endl;
                return -1;
            }

            if (set.Contains(std::to_string(i.first))) {
//                std::cout << "key: " << i.first << std::endl;
            } else {
                std::cout << "key: " << i.first << " not found" << std::endl;
                return -1;
            }
        }

        std::cout << "size:" << map.Size() << std::endl;
        std::cout << "capacity:" << map.Capacity() << std::endl;
        std::cout << "main_position_size:" << map.MainPositionSize() << std::endl;
        auto chain_status = map.ChainStatus();
        for (auto &item: chain_status) {
            std::cout << "chain length: " << item.first << " count: " << item.second << std::endl;
        }

//        for (auto it = map.Begin(); it != map.End(); ++it) {
//            std::cout << "key: " << it.GetKey() << " value: " << it.GetValue() << std::endl;
//        }

        for (auto &i: test_number) {
            if (!map.Erase(std::to_string(i.first))) {
                std::cout << "erase key: " << i.first << " failed" << std::endl;
                return -1;
            }
            if (!set.Erase(std::to_string(i.first))) {
                std::cout << "erase key: " << i.first << " failed" << std::endl;
                return -1;
            }
        }
    }

    return 0;
}

std::unordered_set<int64_t> gData;

int init_test_data() {
    gData.clear();
    for (int i = 0; i < 1000 * 10000; i++) {
        auto key = random_int64();
        if (gData.find(key) != gData.end()) {
            i--;
            continue;
        }
        gData.insert(key);
    }
    return 0;
}

int benchmark() {
    init_test_data();
    std::cout << "--- CoalescedHashMap Benchmark ---" << std::endl;
    auto begin = std::chrono::high_resolution_clock::now();
    CoalescedHashMap<int64_t, int64_t> map;
    for (auto &i: gData) {
        map.Insert(i, i);
    }
    std::cout << "Insert time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;
    
    begin = std::chrono::high_resolution_clock::now();
    for (auto &i: gData) {
        int64_t value;
        if (!map.Find(i, value)) {
            std::cout << "Find key: " << i << " failed" << std::endl;
            return -1;
        }
    }
    std::cout << "Find time:   " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;
    std::cout << "Capacity:    " << map.Capacity() << std::endl;
    std::cout << std::endl;
    return 0;
}

int benchmark_unordered_map() {
    init_test_data();
    std::cout << "--- std::unordered_map Benchmark ---" << std::endl;
    auto begin = std::chrono::high_resolution_clock::now();
    std::unordered_map<int64_t, int64_t> map;
    for (auto &i: gData) {
        map[i] = i;
    }
    std::cout << "Insert time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;
    
    begin = std::chrono::high_resolution_clock::now();
    for (auto &i: gData) {
        auto it = map.find(i);
        if (it == map.end()) {
            std::cout << "Find key: " << i << " failed" << std::endl;
            return -1;
        }
    }
    std::cout << "Find time:   " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;
    std::cout << std::endl;
    return 0;
}

// Deterministic correctness tests. Checks always run (do not use assert):
// CMakeLists.txt compiles with -DNDEBUG, so assert() would be compiled out.
namespace {

int g_passed = 0;
int g_failed = 0;

bool test_check(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::cout << "CHECK failed at " << file << ":" << line
                  << " : " << expr << std::endl;
        return false;
    }
    return true;
}

// Single-line macro so CRLF line endings cannot break backslash continuation.
#define TEST_CHECK(cond) do { if (!test_check(static_cast<bool>(cond), #cond, __FILE__, __LINE__)) return false; } while (0)

struct ConstantHash {
    size_t operator()(int key) const {
        (void)key;
        return 1;
    }
};

struct IdentityHash {
    size_t operator()(int key) const {
        return static_cast<size_t>(key);
    }
};

template<typename Set>
std::vector<int> collect_set_keys(Set& set) {
    std::vector<int> keys;
    for (auto it = set.Begin(); it != set.End(); ++it) {
        keys.push_back(it.GetKey());
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

template<typename Map>
std::map<int, int> collect_map_entries(Map& map) {
    std::map<int, int> entries;
    for (auto it = map.Begin(); it != map.End(); ++it) {
        entries[it.GetKey()] = it.GetValue();
    }
    return entries;
}

int chain_status_total(const std::map<int, int>& status) {
    int total = 0;
    for (auto& item : status) {
        total += item.first * item.second;
    }
    return total;
}

bool expect_int_keys(const std::vector<int>& got, const std::vector<int>& expected_unsorted) {
    std::vector<int> expected = expected_unsorted;
    std::sort(expected.begin(), expected.end());
    return got == expected;
}

bool test_empty_container() {
    CoalescedHashMap<int, int> map;
    CoalescedHashSet<int> set;

    TEST_CHECK(map.Size() == 0);
    TEST_CHECK(set.Size() == 0);
    TEST_CHECK(map.Capacity() == 1);
    TEST_CHECK(set.Capacity() == 1);
    TEST_CHECK(map.MainPositionSize() == 0);
    TEST_CHECK(set.MainPositionSize() == 0);
    TEST_CHECK(map.ChainStatus().empty());
    TEST_CHECK(set.ChainStatus().empty());

    int value = 42;
    TEST_CHECK(!map.Find(1, value));
    TEST_CHECK(value == 42);
    TEST_CHECK(!set.Contains(1));
    int found_key = 42;
    TEST_CHECK(!set.Find(1, found_key));
    TEST_CHECK(found_key == 42);
    TEST_CHECK(!map.Erase(1));
    TEST_CHECK(!set.Erase(1));
    TEST_CHECK(!(map.Begin() != map.End()));
    TEST_CHECK(!(set.Begin() != set.End()));
    return true;
}

bool test_min_capacity_and_first_rehash() {
    CoalescedHashMap<int, int> map(1);
    CoalescedHashSet<int> set(1);

    TEST_CHECK(map.Capacity() == 1);
    TEST_CHECK(set.Capacity() == 1);
    // Free slots must not look occupied: an uninitialized Node::key of a
    // primitive Key type used to make Size() nonzero and, at capacity 1,
    // Insert of a second key walked a self-cycle in next.
    TEST_CHECK(map.Size() == 0);
    TEST_CHECK(set.Size() == 0);

    map.Insert(7, 70);
    set.Insert(7);
    TEST_CHECK(map.Size() == 1);
    TEST_CHECK(set.Size() == 1);
    TEST_CHECK(map.Capacity() == 1);
    TEST_CHECK(set.Capacity() == 1);

    int value = 0;
    TEST_CHECK(map.Find(7, value));
    TEST_CHECK(value == 70);
    TEST_CHECK(set.Contains(7));

    // Second insert on a full table of capacity 1 must rehash.
    // FindNextCapacity(1 * 2 + 1) == FindNextCapacity(3) == 5.
    map.Insert(8, 80);
    set.Insert(8);
    TEST_CHECK(map.Size() == 2);
    TEST_CHECK(set.Size() == 2);
    TEST_CHECK(map.Capacity() == 5);
    TEST_CHECK(set.Capacity() == 5);
    TEST_CHECK(map.Find(7, value) && value == 70);
    TEST_CHECK(map.Find(8, value) && value == 80);
    TEST_CHECK(set.Contains(7));
    TEST_CHECK(set.Contains(8));
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {7, 8}));

    std::map<int, int> entries = collect_map_entries(map);
    TEST_CHECK(entries.size() == 2);
    TEST_CHECK(entries[7] == 70);
    TEST_CHECK(entries[8] == 80);
    return true;
}

bool test_duplicate_keys_overwrite() {
    CoalescedHashMap<std::string, int> map;
    CoalescedHashSet<std::string> set;

    map.Insert("alpha", 1);
    map.Insert("alpha", 2);
    set.Insert("alpha");
    set.Insert("alpha");

    TEST_CHECK(map.Size() == 1);
    TEST_CHECK(set.Size() == 1);
    int value = 0;
    TEST_CHECK(map.Find("alpha", value));
    TEST_CHECK(value == 2);
    TEST_CHECK(set.Contains("alpha"));
    TEST_CHECK(!map.Find("missing", value));
    return true;
}

bool test_erase_missing_and_double_erase() {
    CoalescedHashMap<int, int> map;
    CoalescedHashSet<int> set;
    map.Insert(3, 30);
    set.Insert(3);

    TEST_CHECK(!map.Erase(4));
    TEST_CHECK(!set.Erase(4));
    TEST_CHECK(map.Size() == 1);
    TEST_CHECK(set.Size() == 1);
    TEST_CHECK(map.Erase(3));
    TEST_CHECK(set.Erase(3));
    TEST_CHECK(map.Size() == 0);
    TEST_CHECK(set.Size() == 0);
    TEST_CHECK(!map.Erase(3));
    TEST_CHECK(!set.Erase(3));
    int value = 99;
    TEST_CHECK(!map.Find(3, value));
    TEST_CHECK(value == 99);
    TEST_CHECK(!set.Contains(3));
    return true;
}

bool test_zero_value_allowed() {
    // Default-constructed keys are empty-slot markers and must not be inserted,
    // but a zero *value* is a normal payload.
    CoalescedHashMap<int, int> map;
    map.Insert(11, 0);
    int value = 1;
    TEST_CHECK(map.Find(11, value));
    TEST_CHECK(value == 0);
    TEST_CHECK(map.Size() == 1);
    return true;
}

bool test_hash_collisions_chain_delete_and_reinsert() {
    // All keys share one main position so we can target head / middle / tail.
    const int cap = 7;
    CoalescedHashMap<int, int, ConstantHash> map(cap);
    CoalescedHashSet<int, ConstantHash> set(cap);

    const int keys[] = {2, 4, 6, 8, 10};
    for (int i = 0; i < 5; i++) {
        map.Insert(keys[i], keys[i] * 10);
        set.Insert(keys[i]);
    }

    TEST_CHECK(map.Size() == 5);
    TEST_CHECK(set.Size() == 5);
    TEST_CHECK(map.Capacity() == cap);
    TEST_CHECK(set.Capacity() == cap);
    TEST_CHECK(map.MainPositionSize() == 1);
    TEST_CHECK(set.MainPositionSize() == 1);
    TEST_CHECK(chain_status_total(map.ChainStatus()) == 5);
    TEST_CHECK(map.ChainStatus()[5] == 1);
    TEST_CHECK(set.ChainStatus()[5] == 1);

    for (int i = 0; i < 5; i++) {
        int value = 0;
        TEST_CHECK(map.Find(keys[i], value));
        TEST_CHECK(value == keys[i] * 10);
        TEST_CHECK(set.Contains(keys[i]));
        // A key that collides but was never inserted must not be found.
        TEST_CHECK(!map.Find(keys[i] + 1, value));
        TEST_CHECK(!set.Contains(keys[i] + 1));
    }

    // Delete a middle node of the chain (insertion places later nodes after the
    // head, so 6 is neither the original head 2 nor the last appended key).
    TEST_CHECK(map.Erase(6));
    TEST_CHECK(set.Erase(6));
    TEST_CHECK(map.Size() == 4);
    TEST_CHECK(set.Size() == 4);
    TEST_CHECK(!map.Erase(6));
    int value = 0;
    TEST_CHECK(!map.Find(6, value));
    TEST_CHECK(!set.Contains(6));
    for (int key : {2, 4, 8, 10}) {
        TEST_CHECK(map.Find(key, value) && value == key * 10);
        TEST_CHECK(set.Contains(key));
    }
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {2, 4, 8, 10}));
    TEST_CHECK(chain_status_total(map.ChainStatus()) == 4);

    // Re-insert the deleted key with a new value.
    map.Insert(6, 66);
    set.Insert(6);
    TEST_CHECK(map.Size() == 5);
    TEST_CHECK(set.Size() == 5);
    TEST_CHECK(map.Capacity() == cap);
    TEST_CHECK(set.Capacity() == cap);
    TEST_CHECK(map.Find(6, value) && value == 66);
    TEST_CHECK(set.Contains(6));
    TEST_CHECK(map.ChainStatus()[5] == 1);

    // Delete the original chain head (2), then a later node, checking
    // survivors after each erase. Deleting the head covers the main-position
    // move-next-into-slot path.
    TEST_CHECK(set.MainPositionSize() == 1);
    int to_delete[] = {2, 10, 4};
    std::vector<int> live = {2, 4, 6, 8, 10};
    for (int del : to_delete) {
        TEST_CHECK(map.Erase(del));
        TEST_CHECK(set.Erase(del));
        live.erase(std::remove(live.begin(), live.end(), del), live.end());
        TEST_CHECK(!map.Find(del, value));
        TEST_CHECK(!set.Contains(del));
        TEST_CHECK(map.Size() == (int)live.size());
        TEST_CHECK(expect_int_keys(collect_set_keys(set), live));
        for (int key : live) {
            int expected = (key == 6) ? 66 : key * 10;
            TEST_CHECK(map.Find(key, value) && value == expected);
            TEST_CHECK(set.Contains(key));
        }
        TEST_CHECK(chain_status_total(set.ChainStatus()) == (int)live.size());
        if (!live.empty()) {
            TEST_CHECK(set.MainPositionSize() == 1);
        }
    }

    TEST_CHECK(expect_int_keys(collect_set_keys(set), {6, 8}));
    // Re-insert a previously deleted head and tail.
    map.Insert(2, 20);
    set.Insert(2);
    map.Insert(10, 100);
    set.Insert(10);
    TEST_CHECK(map.Size() == 4);
    TEST_CHECK(set.Capacity() == cap);
    TEST_CHECK(map.Find(2, value) && value == 20);
    TEST_CHECK(map.Find(6, value) && value == 66);
    TEST_CHECK(map.Find(8, value) && value == 80);
    TEST_CHECK(map.Find(10, value) && value == 100);
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {2, 6, 8, 10}));
    return true;
}

bool test_displaced_node_and_main_position_takeover() {
    // Capacity 5, IdentityHash: key 5 lives at slot 0, key 10 collides and is
    // stored off its main position, then key 1 claims slot 1 and must displace 10.
    const int cap = 5;
    CoalescedHashMap<int, int, IdentityHash> map(cap);
    CoalescedHashSet<int, IdentityHash> set(cap);

    map.Insert(5, 50);
    set.Insert(5);
    map.Insert(10, 100);
    set.Insert(10);
    TEST_CHECK(map.Size() == 2);
    TEST_CHECK(set.MainPositionSize() == 1);
    TEST_CHECK(set.ChainStatus()[2] == 1);

    map.Insert(1, 10);
    set.Insert(1);
    TEST_CHECK(map.Size() == 3);
    TEST_CHECK(set.Size() == 3);
    TEST_CHECK(map.Capacity() == cap);

    int value = 0;
    TEST_CHECK(map.Find(5, value) && value == 50);
    TEST_CHECK(map.Find(10, value) && value == 100);
    TEST_CHECK(map.Find(1, value) && value == 10);
    TEST_CHECK(set.Contains(5) && set.Contains(10) && set.Contains(1));
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {1, 5, 10}));

    // 5 and 1 occupy their own main positions; 10 remains chained under 5.
    TEST_CHECK(set.MainPositionSize() == 2);
    TEST_CHECK(map.MainPositionSize() == 2);
    TEST_CHECK(chain_status_total(set.ChainStatus()) == 3);
    TEST_CHECK(set.ChainStatus()[1] == 1);
    TEST_CHECK(set.ChainStatus()[2] == 1);

    // Key 6 should chain off 1 (same main position as 1), without disturbing 5/10.
    map.Insert(6, 60);
    set.Insert(6);
    TEST_CHECK(map.Find(6, value) && value == 60);
    TEST_CHECK(map.Find(1, value) && value == 10);
    TEST_CHECK(map.Find(5, value) && value == 50);
    TEST_CHECK(map.Find(10, value) && value == 100);
    TEST_CHECK(set.Size() == 4);
    TEST_CHECK(set.MainPositionSize() == 2);

    // Delete the displaced key 10, then re-insert it.
    TEST_CHECK(map.Erase(10));
    TEST_CHECK(set.Erase(10));
    TEST_CHECK(!map.Find(10, value));
    TEST_CHECK(!set.Contains(10));
    TEST_CHECK(map.Find(5, value) && value == 50);
    TEST_CHECK(map.Find(1, value) && value == 10);
    TEST_CHECK(map.Find(6, value) && value == 60);
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {1, 5, 6}));

    map.Insert(10, 101);
    set.Insert(10);
    TEST_CHECK(map.Find(10, value) && value == 101);
    TEST_CHECK(map.Size() == 4);
    TEST_CHECK(map.Capacity() == cap);

    // Delete chain head 5; 10 must remain findable, 1 and 6 untouched.
    TEST_CHECK(map.Erase(5));
    TEST_CHECK(set.Erase(5));
    TEST_CHECK(!map.Find(5, value));
    TEST_CHECK(map.Find(10, value) && value == 101);
    TEST_CHECK(map.Find(1, value) && value == 10);
    TEST_CHECK(map.Find(6, value) && value == 60);
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {1, 6, 10}));
    TEST_CHECK(chain_status_total(set.ChainStatus()) == 3);

    map.Insert(5, 55);
    set.Insert(5);
    TEST_CHECK(map.Find(5, value) && value == 55);
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {1, 5, 6, 10}));
    return true;
}

bool test_rehash_preserves_key_values() {
    CoalescedHashMap<int, int> map(1);
    CoalescedHashSet<int> set(1);
    const int n = 80;
    for (int i = 1; i <= n; i++) {
        map.Insert(i, i * 3 + 1);
        set.Insert(i);
    }

    TEST_CHECK(map.Size() == n);
    TEST_CHECK(set.Size() == n);
    TEST_CHECK(map.Capacity() > 1);
    TEST_CHECK(set.Capacity() == map.Capacity());
    // From capacity 1, growth is next prime >= cap*2+1: 5, 11, 23, 53, 113, ...
    TEST_CHECK(map.Capacity() == 113);
    TEST_CHECK(chain_status_total(map.ChainStatus()) == n);
    TEST_CHECK(chain_status_total(set.ChainStatus()) == n);
    TEST_CHECK(collect_set_keys(set).size() == (size_t)n);

    for (int i = 1; i <= n; i++) {
        int value = 0;
        TEST_CHECK(map.Find(i, value));
        TEST_CHECK(value == i * 3 + 1);
        TEST_CHECK(set.Contains(i));
        TEST_CHECK(!map.Find(i + n, value));
        TEST_CHECK(!set.Contains(i + n));
    }

    std::map<int, int> entries = collect_map_entries(map);
    TEST_CHECK((int)entries.size() == n);
    for (int i = 1; i <= n; i++) {
        TEST_CHECK(entries[i] == i * 3 + 1);
    }

    // Crossing another rehash after already holding a full table.
    for (int i = n + 1; i <= 114; i++) {
        map.Insert(i, i * 3 + 1);
        set.Insert(i);
    }
    TEST_CHECK(map.Size() == 114);
    // Full at 113, next capacity is FindNextCapacity(113 * 2 + 1) ==
    // FindNextCapacity(227) == 251 (167 is skipped by this growth sequence).
    TEST_CHECK(map.Capacity() == 251);
    for (int i = 1; i <= 114; i++) {
        int value = 0;
        TEST_CHECK(map.Find(i, value) && value == i * 3 + 1);
        TEST_CHECK(set.Contains(i));
    }
    TEST_CHECK(chain_status_total(map.ChainStatus()) == 114);
    return true;
}

bool test_capacity_reused_after_erase() {
    CoalescedHashMap<int, int> map(5);
    for (int i = 1; i <= 5; i++) {
        map.Insert(i, i);
    }
    TEST_CHECK(map.Size() == 5);
    TEST_CHECK(map.Capacity() == 5);

    TEST_CHECK(map.Erase(2));
    TEST_CHECK(map.Erase(4));
    TEST_CHECK(map.Size() == 3);

    map.Insert(6, 6);
    map.Insert(7, 7);
    TEST_CHECK(map.Size() == 5);
    TEST_CHECK(map.Capacity() == 5);

    int value = 0;
    for (int key : {1, 3, 5, 6, 7}) {
        TEST_CHECK(map.Find(key, value) && value == key);
    }
    TEST_CHECK(!map.Find(2, value));
    TEST_CHECK(!map.Find(4, value));

    // One more insert must grow: 5*2+1 == 11.
    map.Insert(8, 8);
    TEST_CHECK(map.Size() == 6);
    TEST_CHECK(map.Capacity() == 11);
    TEST_CHECK(map.Find(8, value) && value == 8);
    TEST_CHECK(map.Find(1, value) && value == 1);
    return true;
}

bool test_erase_all_then_reinsert() {
    CoalescedHashMap<std::string, int> map(11);
    CoalescedHashSet<std::string> set(11);
    for (int i = 1; i <= 8; i++) {
        map.Insert(std::to_string(i), i);
        set.Insert(std::to_string(i));
    }
    const int cap = map.Capacity();
    for (int i = 1; i <= 8; i++) {
        TEST_CHECK(map.Erase(std::to_string(i)));
        TEST_CHECK(set.Erase(std::to_string(i)));
    }
    TEST_CHECK(map.Size() == 0);
    TEST_CHECK(set.Size() == 0);
    TEST_CHECK(!(map.Begin() != map.End()));
    TEST_CHECK(map.Capacity() == cap);

    for (int i = 1; i <= 8; i++) {
        map.Insert(std::to_string(i), i + 100);
        set.Insert(std::to_string(i));
    }
    TEST_CHECK(map.Size() == 8);
    TEST_CHECK(map.Capacity() == cap);
    for (int i = 1; i <= 8; i++) {
        int value = 0;
        TEST_CHECK(map.Find(std::to_string(i), value) && value == i + 100);
        TEST_CHECK(set.Contains(std::to_string(i)));
    }
    return true;
}

bool test_iterators_and_find_and_execute() {
    CoalescedHashSet<int> set;
    for (int i = 1; i <= 15; i++) {
        set.Insert(i);
    }
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));

    int seen = 0;
    TEST_CHECK(set.FindAndExecute(9, [&](const int& key) {
        seen = key;
    }));
    TEST_CHECK(seen == 9);
    seen = 0;
    TEST_CHECK(!set.FindAndExecute(16, [&](const int& key) {
        seen = key;
    }));
    TEST_CHECK(seen == 0);

    int out = 0;
    TEST_CHECK(set.Find(12, out) && out == 12);

    TEST_CHECK(set.Erase(1));
    TEST_CHECK(set.Erase(15));
    TEST_CHECK(set.Erase(8));
    TEST_CHECK(expect_int_keys(collect_set_keys(set), {2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14}));
    return true;
}

bool test_mirror_unordered_map_and_set() {
    // Fixed-seed mixed operations compared against the STL containers.
    const unsigned seed = 20261110u;
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> key_dist(-200, 200);
    std::uniform_int_distribution<int> val_dist(-10000, 10000);
    std::uniform_int_distribution<int> op_dist(0, 99);

    CoalescedHashMap<int, int> map(1);
    CoalescedHashSet<int> set;
    std::unordered_map<int, int> shadow_map;
    std::unordered_set<int> shadow_set;

    auto still_in_sync = [&]() -> bool {
        if (map.Size() != (int)shadow_map.size()) {
            std::cout << "map size mismatch: " << map.Size() << " vs " << shadow_map.size() << std::endl;
            return false;
        }
        if (set.Size() != (int)shadow_set.size()) {
            std::cout << "set size mismatch: " << set.Size() << " vs " << shadow_set.size() << std::endl;
            return false;
        }
        if (chain_status_total(map.ChainStatus()) != map.Size()) {
            std::cout << "map chain status total mismatch" << std::endl;
            return false;
        }
        if (chain_status_total(set.ChainStatus()) != set.Size()) {
            std::cout << "set chain status total mismatch" << std::endl;
            return false;
        }
        for (auto& kv : shadow_map) {
            int value = 0;
            if (!map.Find(kv.first, value) || value != kv.second) {
                std::cout << "map missing or wrong value for key " << kv.first << std::endl;
                return false;
            }
        }
        std::map<int, int> entries = collect_map_entries(map);
        if (entries.size() != shadow_map.size()) {
            std::cout << "map iterator count mismatch" << std::endl;
            return false;
        }
        for (auto& kv : entries) {
            auto it = shadow_map.find(kv.first);
            if (it == shadow_map.end() || it->second != kv.second) {
                std::cout << "map iterator extra or wrong key " << kv.first << std::endl;
                return false;
            }
        }
        for (int key : shadow_set) {
            if (!set.Contains(key)) {
                std::cout << "set missing key " << key << std::endl;
                return false;
            }
        }
        std::vector<int> set_keys = collect_set_keys(set);
        if (set_keys.size() != shadow_set.size()) {
            std::cout << "set iterator count mismatch" << std::endl;
            return false;
        }
        for (int key : set_keys) {
            if (shadow_set.count(key) == 0) {
                std::cout << "set iterator extra key " << key << std::endl;
                return false;
            }
        }
        return true;
    };

    for (int step = 0; step < 4000; step++) {
        int key = key_dist(rng);
        // Default-constructed keys are reserved as empty slots; skip 0.
        if (key == 0) {
            key = 201;
        }
        int op = op_dist(rng);
        if (op < 55) {
            int value = val_dist(rng);
            map.Insert(key, value);
            shadow_map[key] = value;
            set.Insert(key);
            shadow_set.insert(key);
        } else if (op < 80) {
            bool erased_map = map.Erase(key);
            bool erased_shadow = shadow_map.erase(key) > 0;
            if (erased_map != erased_shadow) {
                std::cout << "map erase mismatch at step " << step << " key " << key << std::endl;
                return false;
            }
            bool erased_set = set.Erase(key);
            bool erased_shadow_set = shadow_set.erase(key) > 0;
            if (erased_set != erased_shadow_set) {
                std::cout << "set erase mismatch at step " << step << " key " << key << std::endl;
                return false;
            }
        } else {
            int value = 0;
            bool found = map.Find(key, value);
            auto it = shadow_map.find(key);
            if (found != (it != shadow_map.end())) {
                std::cout << "map find presence mismatch at step " << step << " key " << key << std::endl;
                return false;
            }
            if (found && value != it->second) {
                std::cout << "map find value mismatch at step " << step << " key " << key << std::endl;
                return false;
            }
            if (set.Contains(key) != (shadow_set.count(key) > 0)) {
                std::cout << "set contains mismatch at step " << step << " key " << key << std::endl;
                return false;
            }
        }
    }

    TEST_CHECK(still_in_sync());

    // Delete every remaining key, then re-insert a dense range to force rehash
    // and confirm the table still matches the shadow containers.
    std::vector<int> remaining(shadow_set.begin(), shadow_set.end());
    for (int key : remaining) {
        TEST_CHECK(map.Erase(key) == true);
        TEST_CHECK(set.Erase(key) == true);
        shadow_map.erase(key);
        shadow_set.erase(key);
    }
    TEST_CHECK(map.Size() == 0);
    TEST_CHECK(set.Size() == 0);
    TEST_CHECK(still_in_sync());

    for (int key = -120; key <= 120; key++) {
        if (key == 0) {
            continue;
        }
        map.Insert(key, key * 7);
        shadow_map[key] = key * 7;
        set.Insert(key);
        shadow_set.insert(key);
    }
    TEST_CHECK(still_in_sync());
    TEST_CHECK(map.Size() == 240);
    TEST_CHECK(map.Capacity() >= 240);
    return true;
}

bool test_string_collision_style_ops() {
    CoalescedHashMap<std::string, int> map(5);
    CoalescedHashSet<std::string> set(5);
    const char* keys[] = {"a", "b", "c", "d", "e", "f", "g"};
    for (int i = 0; i < 7; i++) {
        map.Insert(keys[i], i + 1);
        set.Insert(keys[i]);
    }
    TEST_CHECK(map.Size() == 7);
    TEST_CHECK(set.Size() == 7);
    TEST_CHECK(map.Capacity() == 11);

    int value = 0;
    for (int i = 0; i < 7; i++) {
        TEST_CHECK(map.Find(keys[i], value) && value == i + 1);
        TEST_CHECK(set.Contains(keys[i]));
    }

    TEST_CHECK(map.Erase("c"));
    TEST_CHECK(set.Erase("c"));
    TEST_CHECK(map.Erase("a"));
    TEST_CHECK(set.Erase("a"));
    TEST_CHECK(!map.Find("c", value));
    TEST_CHECK(!set.Contains("a"));
    map.Insert("c", 30);
    set.Insert("c");
    map.Insert("a", 10);
    set.Insert("a");
    TEST_CHECK(map.Find("c", value) && value == 30);
    TEST_CHECK(map.Find("a", value) && value == 10);
    TEST_CHECK(set.Contains("a") && set.Contains("c"));
    TEST_CHECK(map.Size() == 7);

    std::vector<std::string> got;
    for (auto it = set.Begin(); it != set.End(); ++it) {
        got.push_back(it.GetKey());
    }
    std::sort(got.begin(), got.end());
    std::vector<std::string> expected = {"a", "b", "c", "d", "e", "f", "g"};
    TEST_CHECK(got == expected);
    return true;
}

int run_test(const char* name, bool (*fn)()) {
    std::cout << "[RUN]  " << name << std::endl;
    if (fn()) {
        std::cout << "[PASS] " << name << std::endl;
        g_passed++;
        return 0;
    }
    std::cout << "[FAIL] " << name << std::endl;
    g_failed++;
    return -1;
}

}  // namespace

int correctness_tests() {
    g_passed = 0;
    g_failed = 0;
    std::cout << "--- Deterministic correctness tests ---" << std::endl;
    run_test("empty_container", test_empty_container);
    run_test("min_capacity_and_first_rehash", test_min_capacity_and_first_rehash);
    run_test("duplicate_keys_overwrite", test_duplicate_keys_overwrite);
    run_test("erase_missing_and_double_erase", test_erase_missing_and_double_erase);
    run_test("zero_value_allowed", test_zero_value_allowed);
    run_test("hash_collisions_chain_delete_and_reinsert", test_hash_collisions_chain_delete_and_reinsert);
    run_test("displaced_node_and_main_position_takeover", test_displaced_node_and_main_position_takeover);
    run_test("rehash_preserves_key_values", test_rehash_preserves_key_values);
    run_test("capacity_reused_after_erase", test_capacity_reused_after_erase);
    run_test("erase_all_then_reinsert", test_erase_all_then_reinsert);
    run_test("iterators_and_find_and_execute", test_iterators_and_find_and_execute);
    run_test("mirror_unordered_map_and_set", test_mirror_unordered_map_and_set);
    run_test("string_collision_style_ops", test_string_collision_style_ops);
    std::cout << "Deterministic tests: " << g_passed << " passed, " << g_failed
              << " failed" << std::endl;
    return g_failed == 0 ? 0 : -1;
}

int main(int argc, char** argv) {
    bool correctness_only = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--correctness-only") {
            correctness_only = true;
        }
    }

    if (correctness_tests() != 0) {
        std::cout << "Deterministic correctness tests failed!" << std::endl;
        return 1;
    }
    std::cout << "Deterministic correctness tests passed!" << std::endl << std::endl;
    if (correctness_only) {
        return 0;
    }

    if (test() != 0) {
        std::cout << "Random correctness test failed!" << std::endl;
        return 1;
    }
    std::cout << "Random correctness test passed!" << std::endl << std::endl;

    if (benchmark() != 0) {
        return 1;
    }
    if (benchmark_unordered_map() != 0) {
        return 1;
    }
    return 0;
}
