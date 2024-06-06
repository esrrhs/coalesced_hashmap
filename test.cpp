#include "coalesced_hashmap.h"

using namespace coalesced_hashmap;

int test() {
    srand(time(NULL));
    int N = 100;
    CoalescedHashMap<std::string, int> map;
    std::map<int, int> test_number;

    for (int i = 0; i < N; i++) {
        auto key = rand() % 10000000;
        auto it = test_number.find(key);
        if (it != test_number.end()) {
            i--;
            continue;
        }
        test_number[key] = i;
    }

    for (auto &i: test_number) {
        map.Insert(std::to_string(i.first), i.second);
    }

    int value = 0;
    for (auto &i: test_number) {
        if (map.Find(std::to_string(i.first), value)) {
            std::cout << "key: " << i.first << " value: " << value << std::endl;
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

    //std::cout << map.Dump() << std::endl;

    for (auto it = map.Begin(); it != map.End(); ++it) {
        std::cout << "key: " << it.GetKey() << " value: " << it.GetValue() << std::endl;
    }

    for (auto &i: test_number) {
        if (!map.Erase(std::to_string(i.first))) {
            std::cout << "erase key: " << i.first << " failed" << std::endl;
            return -1;
        }
    }
    return 0;
}

int64_t random_int64() {
    return (int64_t) rand() << 32 | rand();
}

int benchmark() {
    srand(time(NULL));
    CoalescedHashMap<int64_t, int64_t> map;
    for (int i = 0; i < 1000 * 10000; i++) {
        map.Insert(random_int64(), i);
    }
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
    srand(time(NULL));
    std::unordered_map<int64_t, int64_t> map;
    for (int i = 0; i < 1000 * 10000; i++) {
        map.insert({random_int64(), i});
    }
    std::cout << "size:" << map.size() << std::endl;
    char c;
    std::cin >> c;
    return 0;
}

int main() {
    test();
    return 0;
}
