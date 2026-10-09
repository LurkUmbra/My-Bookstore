#include "../src/system/book_system.h"
#include <cstdio>
#include <cstring>

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

static int countByName(BookSystem& sys, const char* name) {
    int n = 0;
    sys.showByName(name, [&](const char*, const BookData&) { n++; return true; });
    return n;
}

static int countByAuthor(BookSystem& sys, const char* author) {
    int n = 0;
    sys.showByAuthor(author, [&](const char*, const BookData&) { n++; return true; });
    return n;
}

static int countByKeyword(BookSystem& sys, const char* kw) {
    int n = 0;
    sys.showByKeyword(kw, [&](const char*, const BookData&) { n++; return true; });
    return n;
}

int main() {
    clean();
    {
        BookSystem sys;
        sys.createBook("ISBN-001", "Math", "Wu", "sci|algo", 45.0);
        sys.createBook("ISBN-002", "Physics", "Li", "sci", 60.0);

        // ---------- change name ----------
        BookSystem::ModifyFields f1;
        f1.name = "Mathematics";
        CHECK(sys.modifyBook("ISBN-001", f1), "modify name -> true");
        CHECK(countByName(sys, "Math") == 0, "old name 'Math' -> 0");
        CHECK(countByName(sys, "Mathematics") == 1, "new name 'Mathematics' -> 1");

        BookData d;
        sys.getByISBN("ISBN-001", d);
        CHECK(std::strcmp(d.name, "Mathematics") == 0, "primary name updated");

        // ---------- change author ----------
        BookSystem::ModifyFields f2;
        f2.author = "Zhao";
        CHECK(sys.modifyBook("ISBN-001", f2), "modify author -> true");
        CHECK(countByAuthor(sys, "Wu") == 0, "old author 'Wu' -> 0");
        CHECK(countByAuthor(sys, "Zhao") == 1, "new author 'Zhao' -> 1");

        // ---------- change price only ----------
        double newPrice = 99.5;
        BookSystem::ModifyFields f3;
        f3.price = &newPrice;
        CHECK(sys.modifyBook("ISBN-001", f3), "modify price -> true");
        sys.getByISBN("ISBN-001", d);
        CHECK(d.price == 99.5, "price updated to 99.5");
        CHECK(countByName(sys, "Mathematics") == 1, "name index untouched by price change");

        // ---------- change keyword: remove algo, add magic, keep sci ----------
        BookSystem::ModifyFields f4;
        f4.keyword = "sci|magic";
        CHECK(sys.modifyBook("ISBN-001", f4), "modify keyword -> true");
        CHECK(countByKeyword(sys, "sci") == 2, "keyword 'sci' still on 2 books");
        CHECK(countByKeyword(sys, "algo") == 0, "keyword 'algo' removed");
        CHECK(countByKeyword(sys, "magic") == 1, "keyword 'magic' added");

        // ---------- modify nonexistent ----------
        BookSystem::ModifyFields f5;
        f5.name = "X";
        CHECK(!sys.modifyBook("NOPE", f5), "modify nonexistent -> false");

        // ---------- no-op: same name ----------
        BookSystem::ModifyFields f6;
        f6.name = "Mathematics";
        CHECK(sys.modifyBook("ISBN-001", f6), "modify same name -> true");
        CHECK(countByName(sys, "Mathematics") == 1, "same-name no-op keeps index intact");
    }

    // ---------- reopen: verify persistence ----------
    {
        BookSystem sys;
        BookData d;
        CHECK(sys.getByISBN("ISBN-001", d), "reopen: find ISBN-001");
        CHECK(std::strcmp(d.name, "Mathematics") == 0, "reopen: name = Mathematics");
        CHECK(std::strcmp(d.author, "Zhao") == 0, "reopen: author = Zhao");
        CHECK(std::strcmp(d.keyword, "sci|magic") == 0, "reopen: keyword = sci|magic");
        CHECK(d.price == 99.5, "reopen: price = 99.5");

        CHECK(countByName(sys, "Mathematics") == 1, "reopen: index name ok");
        CHECK(countByAuthor(sys, "Zhao") == 1, "reopen: index author ok");
        CHECK(countByKeyword(sys, "magic") == 1, "reopen: index keyword magic ok");
        CHECK(countByKeyword(sys, "algo") == 0, "reopen: old keyword algo gone");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
