#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cassert>
#include "../src/storage/block_list.h"

static int passed = 0, failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { std::cout << "[PASS]" << msg << std::endl; passed++; } \
    else      { std::cout << "[FAIL]" << msg << std::endl; failed++; } \
} while (0)

void prepareTestFile(const std::string& filename) {
    std::fstream f(filename, std::ios::out | std::ios::binary | std::ios::trunc);

    Block head{};
    head.next = 1;
    f.write(reinterpret_cast<const char*>(&head), sizeof(Block));

    Block data{};
    data.count = 3;
    data.next = -1;
    data.keys[0] = 10; data.keys[1] = 20; data.keys[2] = 30;
    std::strcpy(data.values[0], "ten");
    std::strcpy(data.values[1], "twenty");
    std::strcpy(data.values[2], "thirty");
    f.write(reinterpret_cast<const char*>(&data), sizeof(Block));

    f.close();
}

void testEmptyList() {
    std::remove("t_empty.bin");
    BlockList bl("t_empty.bin");
    char buf[VALUE_SIZE];
    CHECK(!bl.find(1, buf), "EmptyList: find should return false");
}

void testFind() {
    prepareTestFile("t_data.bin");
    BlockList bl("t_data.bin");
    char buf[VALUE_SIZE];

    CHECK(bl.find(20, buf) && std::strcmp(buf, "twenty") == 0, "find(20) -> twenty");
    CHECK(bl.find(10, buf) && std::strcmp(buf, "ten")    == 0, "find(10) -> ten");
    CHECK(bl.find(30, buf) && std::strcmp(buf, "thirty") == 0, "find(30) -> thirty");
    CHECK(!bl.find(25, buf), "find(25) -> 不存在");
    CHECK(!bl.find(5,  buf), "find(5)  -> 不存在");
}

void testPersistence() {
    prepareTestFile("t_persist.bin");
    {
        BlockList bl("t_persist.bin");
        char buf[VALUE_SIZE];
        CHECK(bl.find(20, buf), "重开前: 能查到 20");
    }   // ← bl destruct, f.close()
    {
        BlockList bl("t_persist.bin");
        char buf[VALUE_SIZE];
        CHECK(bl.find(20, buf) && std::strcmp(buf, "twenty") == 0,
            "重开后: 能查到 20 -> twenty");
    }    
}

int main() {
    testEmptyList();
    testFind();
    testPersistence();

    std::cout << "\n==== Passed: " << passed
              << ", Failed: " << failed << "====\n";
    return failed == 0 ? 0 : 1;
}