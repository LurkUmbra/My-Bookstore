#include "../src/system/book_system.h"
#include <cstdio>
#include <cstring>
#include <string>

static int passed = 0, failed = 0;
#define CHECK(cond, msg) do { \
    if (cond) { printf("[PASS] %s\n", msg); passed++; } \
    else      { printf("[FAIL] %s\n", msg); failed++; } \
} while (0)

int main() {
    std::remove("books.dat");
    std::remove("name.idx");
    std::remove("author.idx");
    std::remove("keyword.idx");

    BookSystem sys;

    // pack / unpack 往返测试
    BookData d{};
    std::strncpy(d.name, "Math", sizeof(d.name) - 1);
    std::strncpy(d.author, "Wu", sizeof(d.author) - 1);
    std::strncpy(d.keyword, "sci", sizeof(d.keyword) - 1);
    d.price = 45.0;
    d.stock = 10;

    char buf[BOOK_VALUE_SIZE];
    BookSystem::pack(d, buf);

    BookData d2{};
    BookSystem::unpack(buf, d2);

    CHECK(std::strcmp(d2.name, "Math") == 0, "pack/unpack: name");
    CHECK(std::strcmp(d2.author, "Wu") == 0, "pack/unpack: author");
    CHECK(std::strcmp(d2.keyword, "sci") == 0, "pack/unpack: keyword");
    CHECK(d2.price == 45.0, "pack/unpack: price");
    CHECK(d2.stock == 10, "pack/unpack: stock");

    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
