#include "coalesced_hashmap.h"

using namespace coalesced_hashmap;

int test() {
    srand(time(NULL));
    int N = 100;
    CoalescedHashMap<std::string, int> map;
    std::set<int> test_number;

    for (int i = 0; i < N; i++) {
        test_number.insert(rand() % 10000000);
    }

    for (auto &i: test_number) {
        map.Insert(std::to_string(i), i);
    }

    int value = 0;
    for (auto &i: test_number) {
        if (map.Find(std::to_string(i), value)) {
            std::cout << "key: " << i << " value: " << value << std::endl;
        } else {
            std::cout << "key: " << i << " not found" << std::endl;
            return -1;
        }
        if (value != i) {
            std::cout << "key: " << i << " value: " << value << " not equal" << std::endl;
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

    for (auto &i: test_number) {
        if (!map.Erase(std::to_string(i))) {
            std::cout << "erase key: " << i << " failed" << std::endl;
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
