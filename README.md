# coalesced_hashmap
C++ [Coalesced_hashing](https://en.wikipedia.org/wiki/Coalesced_hashing), inspired by Lua table implementation.
header only, no dependencies.

## Usage
```cpp
CoalescedHashMap<std::string, int> map;
map.Insert("key", 1);
int value = 0;
if (map.Find("key", value)) {
    std::cout << "found" << std::endl;
}
for (auto it = map.Begin(); it != map.End(); ++it) {
    std::cout << "key: " << it.GetKey() << " value: " << it.GetValue() << std::endl;
}
if (map.Erase("key")) {
    std::cout << "erased" << std::endl;
}
```

## Build
```bash
mkdir build
cd build
cmake ..
make
```
