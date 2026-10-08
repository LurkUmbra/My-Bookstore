#include "../src/system/book_system.h"
#include "../src/system/log_system.h"
#include <cstdio>
#include <cstring>
#include <cmath>

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
    std::remove("finance.log");
}

static bool nearEq(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    clean();

    // ---------- LogSystem ----------
    {
        LogSystem log;
        double inc, exp;
        CHECK(!log.showFinance(-1, inc, exp), "showFinance(-1) -> false");
        CHECK(log.showFinance(0, inc, exp) && nearEq(inc, 0) && nearEq(exp, 0),
              "showFinance(0) -> (0, 0)");
        CHECK(!log.showFinance(1, inc, exp), "showFinance(1) on empty -> false");
    }

    clean();
    {
        LogSystem log;
        log.recordIncome(10.5);
        log.recordExpense(3.25);
        log.recordIncome(2.0);

        double inc, exp;
        CHECK(log.totalCount() == 3, "log count = 3");
        CHECK(log.showFinance(3, inc, exp) && nearEq(inc, 12.5) && nearEq(exp, 3.25),
              "showFinance(3) -> 12.5 / 3.25");
        CHECK(log.showFinance(2, inc, exp) && nearEq(inc, 2.0) && nearEq(exp, 3.25),
              "showFinance(2) -> 2.0 / 3.25");
        CHECK(log.showFinance(1, inc, exp) && nearEq(inc, 2.0) && nearEq(exp, 0),
              "showFinance(1) -> 2.0 / 0");
        CHECK(!log.showFinance(4, inc, exp), "showFinance(4) -> false");
    }

    clean();
    {
        LogSystem log;
        log.recordIncome(1.0);
        log.recordExpense(2.0);
    }
    {
        LogSystem log;
        CHECK(log.totalCount() == 2, "reopen: count = 2");
        double inc, exp;
        CHECK(log.showFinance(2, inc, exp) && nearEq(inc, 1.0) && nearEq(exp, 2.0),
              "reopen: showFinance(2) correct");
        log.recordIncome(3.0);
        CHECK(log.totalCount() == 3, "reopen: append -> count 3");
    }

    // ---------- buyBook / importBook ----------
    clean();
    {
        BookSystem sys;
        sys.createBook("ISBN-001", "Math", "Wu", "sci", 45.0);

        double cost = 0;
        CHECK(sys.buyBook("ISBN-001", 3, cost) && nearEq(cost, 135.0), "buy 3 -> 135.0");

        BookData d;
        sys.getByISBN("ISBN-001", d);
        CHECK(d.stock == -3, "stock after buy 3 = -3");

        CHECK(!sys.buyBook("ISBN-001", 0, cost), "buy 0 -> false");
        CHECK(!sys.buyBook("ISBN-001", -1, cost), "buy -1 -> false");
        CHECK(!sys.buyBook("NOPE", 1, cost), "buy NOPE -> false");

        CHECK(sys.importBook("ISBN-001", 10, 400.0), "import 10/400 -> true");
        sys.getByISBN("ISBN-001", d);
        CHECK(d.stock == 7, "stock after import 10 = 7");

        CHECK(!sys.importBook("ISBN-001", 0, 100.0), "import qty=0 -> false");
        CHECK(!sys.importBook("ISBN-001", 5, 0.0),   "import cost=0 -> false");
        CHECK(!sys.importBook("ISBN-001", 5, -1.0),  "import cost=-1 -> false");
        CHECK(!sys.importBook("NOPE", 5, 100.0),     "import NOPE -> false");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
