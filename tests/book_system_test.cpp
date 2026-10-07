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
        // 两本同名书 + 一本不同名书
        CHECK(sys.createBook("ISBN-001", "Math",   "Wu", "sci",  45.0), "create Math #1");
        CHECK(sys.createBook("ISBN-002", "Math",   "Li", "algo", 50.0), "create Math #2");
        CHECK(sys.createBook("ISBN-003", "Physics","Wu", "sci",  60.0), "create Physics");

        // ---- showByName("Math") 应返回 2 本 ----
        int n = 0;
        sys.showByName("Math", [&](const BookData& d) {
            n++;
            (void)d;
            return true;
        });
        CHECK(n == 2, "showByName(Math) -> 2 books");

        // ---- showByName("Physics") 应返回 1 本 ----
        int m = 0;
        sys.showByName("Physics", [&](const BookData& d) {
            m++;
            (void)d;
            return true;
        });
        CHECK(m == 1, "showByName(Physics) -> 1 book");

        // ---- 不存在的名字 ----
        int q = 0;
        sys.showByName("Nope", [&](const BookData&) { q++; return true; });
        CHECK(q == 0, "showByName(Nope) -> 0 books");

        // ---- 提前停止：只取 1 本 ----
        int k = 0;
        sys.showByName("Math", [&](const BookData&) {
            return ++k < 1;   // 取到 1 本就返回 false
        });
        CHECK(k == 1, "showByName(Math) early stop at 1");

        // ---- 校验内容：Math 那一本 price = 45 或 50 ----
        double price = 0;
        sys.showByName("Physics", [&](const BookData& d) {
            price = d.price;
            return true;
        });
        CHECK(price == 60.0, "showByName(Physics) price = 60");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
