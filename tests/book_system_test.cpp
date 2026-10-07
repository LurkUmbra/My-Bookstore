#include "../src/system/book_system.h"
#include <cstdio>
#include <cstring>
#include <string>

static int passed = 0, failed = 0;
#define CHECK(cond, msg) do { \
    if (cond) { printf("[PASS] %s\n", msg); passed++; } \
    else      { printf("[FAIL] %s\n", msg); failed++; } \
} while (0)

static void clean() {
    std::remove("books.dat");
    std::remove("name.idx");
    std::remove("author.idx");
    std::remove("keyword.idx");
}

int main() {
    clean();
    {
        BookSystem sys;
        CHECK(sys.createBook("ISBN-001", "Math", "Wu", "sci|algo", 45.0), "create book 1");
        CHECK(sys.createBook("ISBN-002", "Physics", "Wu", "sci", 60.0), "create book 2");
        CHECK(!sys.createBook("ISBN-001", "Dup", "X", "y", 1.0), "duplicate ISBN -> false");

        BookData d;
        CHECK(sys.getByISBN("ISBN-001", d), "getByISBN found");
        CHECK(std::strcmp(d.name, "Math") == 0, "name correct");
        CHECK(std::strcmp(d.author, "Wu") == 0, "author correct");
        CHECK(std::strcmp(d.keyword, "sci|algo") == 0, "keyword correct");
        CHECK(d.price == 45.0, "price correct");
        CHECK(d.stock == 0, "new book stock = 0");

        CHECK(!sys.getByISBN("NOPE", d), "getByISBN not found");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
