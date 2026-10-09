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
        CHECK(sys.createBook("ISBN-001", "Math",    "Wu", "sci",  45.0), "create Math #1");
        CHECK(sys.createBook("ISBN-002", "Math",    "Li", "algo", 50.0), "create Math #2");
        CHECK(sys.createBook("ISBN-003", "Physics", "Wu", "sci",  60.0), "create Physics");

        // ================= showByName =================
        int n = 0;
        sys.showByName("Math", [&](const char*, const BookData& d) { n++; (void)d; return true; });
        CHECK(n == 2, "showByName(Math) -> 2 books");

        int m = 0;
        sys.showByName("Physics", [&](const char*, const BookData& d) { m++; (void)d; return true; });
        CHECK(m == 1, "showByName(Physics) -> 1 book");

        int q = 0;
        sys.showByName("Nope", [&](const char*, const BookData&) { q++; return true; });
        CHECK(q == 0, "showByName(Nope) -> 0 books");

        int k = 0;
        sys.showByName("Math", [&](const char*, const BookData&) { return ++k < 1; });
        CHECK(k == 1, "showByName(Math) early stop at 1");

        double price = 0;
        sys.showByName("Physics", [&](const char*, const BookData& d) { price = d.price; return true; });
        CHECK(price == 60.0, "showByName(Physics) price = 60");

        // ================= showByAuthor =================
        int wa = 0;
        sys.showByAuthor("Wu", [&](const char*, const BookData& d) { wa++; (void)d; return true; });
        CHECK(wa == 2, "showByAuthor(Wu) -> 2 books");

        int la = 0;
        sys.showByAuthor("Li", [&](const char*, const BookData& d) { la++; (void)d; return true; });
        CHECK(la == 1, "showByAuthor(Li) -> 1 book");

        int za = 0;
        sys.showByAuthor("Zhao", [&](const char*, const BookData& d) { za++; (void)d; return true; });
        CHECK(za == 0, "showByAuthor(Zhao) -> 0 books");

        // ================= showByKeyword =================
        int ks = 0;
        sys.showByKeyword("sci", [&](const char*, const BookData& d) { ks++; (void)d; return true; });
        CHECK(ks == 2, "showByKeyword(sci) -> 2 books");

        int ka = 0;
        sys.showByKeyword("algo", [&](const char*, const BookData& d) { ka++; (void)d; return true; });
        CHECK(ka == 1, "showByKeyword(algo) -> 1 book");

        int kc = 0;
        sys.showByKeyword("sci|algo", [&](const char*, const BookData& d) { kc++; (void)d; return true; });
        CHECK(kc == 0, "showByKeyword(sci|algo) -> 0 (not indexed as whole)");

        int kn = 0;
        sys.showByKeyword("nope", [&](const char*, const BookData& d) { kn++; (void)d; return true; });
        CHECK(kn == 0, "showByKeyword(nope) -> 0 books");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
