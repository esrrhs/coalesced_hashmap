#include "coalesced_hashmap.h"

using namespace coalesced_hashmap;

int64_t random_int64() {
    return (int64_t) rand() << 32 | rand();
}

int test() {
    srand(time(NULL));
    int N = 1000000;
    CoalescedHashMap<std::string, int> map;

    while (1) {
        std::map<int64_t, int> test_number;

        for (int i = 0; i < N; i++) {
            auto key = random_int64();
            test_number[key] = i;
        }

        std::cout << "test number size:" << test_number.size() << std::endl;

        for (auto &i: test_number) {
            map.Insert(std::to_string(i.first), i.second);
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
        }

        std::cout << "size:" << map.Size() << std::endl;
        std::cout << "capacity:" << map.Capacity() << std::endl;
        std::cout << "main_position_size:" << map.MainPositionSize() << std::endl;
        auto chain_status = map.ChainStatus();
        for (auto &item: chain_status) {
            std::cout << "chain length: " << item.first << " count: " << item.second << std::endl;
        }

//        std::cout << map.Dump() << std::endl;

//        for (auto it = map.Begin(); it != map.End(); ++it) {
//            std::cout << "key: " << it.GetKey() << " value: " << it.GetValue() << std::endl;
//        }

        for (auto &i: test_number) {
            if (!map.Erase(std::to_string(i.first))) {
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
    auto begin = std::chrono::high_resolution_clock::now();
    CoalescedHashMap<int64_t, int64_t> map;
    for (auto &i: gData) {
        map.Insert(i, i);
    }
    std::cout << "insert time:" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << std::endl;
    begin = std::chrono::high_resolution_clock::now();
    for (auto &i: gData) {
        int64_t value;
        if (!map.Find(i, value)) {
            std::cout << "find key: " << i << " failed" << std::endl;
            return -1;
        }
        if (value != i) {
            std::cout << "find key: " << i << " value: " << value << " not equal" << std::endl;
            return -1;
        }
    }
    std::cout << "find time:" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << std::endl;
    std::cout << "size:" << map.Size() << std::endl;
    std::cout << "capacity:" << map.Capacity() << std::endl;
    std::cout << "main_position_size:" << map.MainPositionSize() << std::endl;
    auto chain_status = map.ChainStatus();
    for (auto &item: chain_status) {
        std::cout << "chain length: " << item.first << " count: " << item.second << std::endl;
    }
    std::cout << "end" << std::endl;
    char c;
    std::cin >> c;
    return 0;
}

int benchmark_unordered_map() {
    init_test_data();
    auto begin = std::chrono::high_resolution_clock::now();
    std::unordered_map<int64_t, int64_t> map;
    for (auto &i: gData) {
        map[i] = i;
    }
    std::cout << "insert time:" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << std::endl;
    begin = std::chrono::high_resolution_clock::now();
    for (auto &i: gData) {
        auto it = map.find(i);
        if (it == map.end()) {
            std::cout << "find key: " << i << " failed" << std::endl;
            return -1;
        }
        if (it->second != i) {
            std::cout << "find key: " << i << " value: " << it->second << " not equal" << std::endl;
            return -1;
        }
    }
    std::cout << "find time:" << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - begin).count() << std::endl;
    std::cout << "size:" << map.size() << std::endl;
    char c;
    std::cin >> c;
    return 0;
}

int main() {
    benchmark();
    return 0;
}
