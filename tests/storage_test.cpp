#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <string>
#include "../src/storage/block_list.h"

static int passed = 0, failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { std::cout << "[PASS]" << msg << std::endl; passed++; } \
    else      { std::cout << "[FAIL]" << msg << std::endl; failed++; } \
} while (0)

// 测试用尺寸：key 21，value 64
constexpr int KS = 21;
constexpr int VS = 64;
using BL = BlockList<KS, VS>;

// 把 C 字符串打包成定长 VS 缓冲区
static void packStr(const char* s, char* buf) {
    std::memset(buf, 0, VS);
    std::strncpy(buf, s, VS - 1);
}

// 插入辅助：自动把字符串 value 打包成定长
static bool insStr(BL& bl, const char* key, const char* val) {
    char buf[VS];
    packStr(val, buf);
    return bl.insert(key, buf);
}

// 整型 key 辅助
static std::string k(int i) { return std::to_string(i); }

void testEmptyList() {
    std::remove("t_empty.bin");
    BL bl("t_empty.bin");
    char buf[VS];
    CHECK(!bl.find("1", buf), "EmptyList: find should return false");
}

void testInsertAndFind() {
    std::remove("t_data.bin");
    {
        BL bl("t_data.bin");
        CHECK(insStr(bl, "20", "twenty"), "insert 20");
        CHECK(insStr(bl, "10", "ten"),    "insert 10");
        CHECK(insStr(bl, "30", "thirty"), "insert 30");

        char buf[VS];
        CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "find 20 -> twenty");
        CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "find 10 -> ten");
        CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "find 30 -> thirty");
        CHECK(!bl.find("25", buf), "find 25 -> not found");
    }
    {
        BL bl("t_data.bin");
        char buf[VS];
        CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "reopen: find 20");
    }
}

void testInsertBasic() {
    std::remove("t_ins.bin");
    {
        BL bl("t_ins.bin");
        CHECK(insStr(bl, "20", "twenty"), "insert 20");
        CHECK(insStr(bl, "10", "ten"),    "insert 10");
        CHECK(insStr(bl, "30", "thirty"), "insert 30");
        CHECK(!insStr(bl, "20", "dup"),   "duplicate 20 -> false");

        char buf[VS];
        CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "after insert, find 10");
        CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "after insert, find 30");
    }
    {
        BL bl("t_ins.bin");
        char buf[VS];
        CHECK(bl.find("10", buf) && std::strcmp(buf, "ten") == 0,    "reopen: find 10");
        CHECK(bl.find("20", buf) && std::strcmp(buf, "twenty") == 0, "reopen: find 20");
        CHECK(bl.find("30", buf) && std::strcmp(buf, "thirty") == 0, "reopen: find 30");
    }
}

void testInsertSplit() {
    std::remove("t_split.bin");
    int n = CAPACITY + 5;
    {
        BL bl("t_split.bin");
        bool allIns = true;
        for (int i = 1; i <= n; i++) {
            char val[32];
            std::snprintf(val, sizeof(val), "val_%d", i);
            if (!insStr(bl, k(i).c_str(), val)) { allIns = false; break; }
        }
        CHECK(allIns, "split: insert CAPACITY+5 keys all succeed");
    }
    {
        BL bl("t_split.bin");
        char buf[VS];
        bool allFound = true;
        for (int i = 1; i <= n; i++) {
            if (!bl.find(k(i).c_str(), buf)) { allFound = false; break; }
        }
        CHECK(allFound, "split: reopen, all keys found");
        CHECK(bl.find("1", buf) && std::strcmp(buf, "val_1") == 0, "split: find(1) -> val_1");
        CHECK(bl.find(k(n).c_str(), buf), "split: find(last) ok");
    }
}

void testInsertReverse() {
    std::remove("t_rev.bin");
    int n = CAPACITY + 10;
    {
        BL bl("t_rev.bin");
        bool ok = true;
        for (int i = n; i >= 1; i--) {
            char val[32];
            std::snprintf(val, sizeof(val), "v%d", i);
            if (!insStr(bl, k(i).c_str(), val)) { ok = false; break; }
        }
        CHECK(ok, "reverse: all inserted");
    }
    {
        BL bl("t_rev.bin");
        char buf[VS];
        bool allFound = true;
        for (int i = 1; i <= n; i++) {
            if (!bl.find(k(i).c_str(), buf)) { allFound = false; break; }
        }
        CHECK(allFound, "reverse: reopen, all found");
    }
}

void testInsertRandom() {
    std::remove("t_rand.bin");
    int n = CAPACITY * 3 + 7;
    int keys[300];
    for (int i = 0; i < n; i++) keys[i] = i * 2;
    for (int i = n - 1; i > 0; i--) {
        int j = (i * 7 + 3) % (i + 1);
        int t = keys[i]; keys[i] = keys[j]; keys[j] = t;
    }
    {
        BL bl("t_rand.bin");
        bool ok = true;
        for (int i = 0; i < n; i++) {
            char val[32];
            std::snprintf(val, sizeof(val), "k%d", keys[i]);
            if (!insStr(bl, k(keys[i]).c_str(), val)) { ok = false; break; }
        }
        CHECK(ok, "random: all inserted");
    }
    {
        BL bl("t_rand.bin");
        char buf[VS];
        bool allFound = true;
        for (int i = 0; i < n; i++) {
            if (!bl.find(k(keys[i]).c_str(), buf)) { allFound = false; break; }
        }
        CHECK(allFound, "random: reopen, all found");
    }
}

void testErase() {
    std::remove("t_erase.bin");
    {
        BL bl("t_erase.bin");
        for (int i = 1; i <= 10; i++) {
            char v[16];
            std::snprintf(v, sizeof(v), "v%d", i);
            insStr(bl, k(i).c_str(), v);
        }
        CHECK(bl.erase("5"), "erase 5 -> true");
        CHECK(!bl.erase("5"), "erase 5 again -> false");
        CHECK(!bl.erase("999"), "erase 999 -> false");
        CHECK(!bl.erase("0"), "erase 0 (below min) -> false");

        char buf[VS];
        CHECK(!bl.find("5", buf), "5 gone after erase");
        CHECK(bl.find("4", buf) && std::strcmp(buf, "v4") == 0, "4 still there");
        CHECK(bl.find("6", buf) && std::strcmp(buf, "v6") == 0, "6 still there");
    }
    {
        BL bl("t_erase.bin");
        char buf[VS];
        CHECK(!bl.find("5", buf), "reopen: 5 still gone");
        CHECK(bl.find("6", buf),  "reopen: 6 still there");
    }
}

void testEraseUntilEmpty() {
    std::remove("t_erase2.bin");
    {
        BL bl("t_erase2.bin");
        for (int i = 1; i <= 10; i++) {
            char v[16];
            std::snprintf(v, sizeof(v), "v%d", i);
            insStr(bl, k(i).c_str(), v);
        }
        bool allErased = true;
        for (int i = 1; i <= 10; i++) {
            if (!bl.erase(k(i).c_str())) { allErased = false; break; }
        }
        CHECK(allErased, "erase all 10");
        CHECK(!bl.erase("1"), "erase after empty -> false");
    }
}

// 通用 insert 辅助（不同尺寸）
template <int KS_, int VS_>
static bool insStrT(BlockList<KS_, VS_>& bl, const char* key, const char* val) {
    char buf[VS_];
    std::memset(buf, 0, VS_);
    std::strncpy(buf, val, VS_ - 1);
    return bl.insert(key, buf);
}

void testDifferentTypes() {
    std::remove("t_a.bin");
    std::remove("t_b.bin");
    BlockList<21, 64>  small("t_a.bin");
    BlockList<31, 128> large("t_b.bin");

    insStrT(small, "k", "v");
    insStrT(large, "a-much-longer-key", "a-much-longer-value-here");

    char buf[128];
    CHECK(small.find("k", buf) && std::strcmp(buf, "v") == 0, "small type: find k");
    CHECK(large.find("a-much-longer-key", buf)
          && std::strcmp(buf, "a-much-longer-value-here") == 0, "large type: find long key");
}

void testTraverse() {
    std::remove("t_trav.bin");
    BL bl("t_trav.bin");
    for (int i = 1; i <= 10; i++) {
        char v[16];
        std::snprintf(v, sizeof(v), "%d", i * 100);
        insStr(bl, k(i).c_str(), v);
    }

    int cnt = 0;
    std::string all;
    bl.traverse([&](const char* key, const char*) {
        all += key; all += ",";
        cnt++;
        return true;
    });
    CHECK(cnt == 10, "traverse: visited 10 entries");
    CHECK(all == "1,10,2,3,4,5,6,7,8,9,", "traverse: keys in lexicographic order");

    int n3 = 0;
    bl.traverse([&](const char*, const char*) {
        return ++n3 < 3;
    });
    CHECK(n3 == 3, "traverse: early stop after 3");

    std::string range;
    bl.traverseRange("3", "7", [&](const char* key, const char*) {
        range += key; range += ",";
        return true;
    });
    CHECK(range == "3,4,5,6,", "traverseRange[3,7): got 3,4,5,6");

    std::string pfx;
    bl.traverseRange("10", "11", [&](const char* key, const char*) {
        pfx += key; pfx += ",";
        return true;
    });
    CHECK(pfx == "10,", "traverseRange[10,11): only key '10'");
}

int main() {
    testEmptyList();
    testInsertAndFind();
    testInsertBasic();
    testInsertSplit();
    testInsertReverse();
    testInsertRandom();
    testErase();
    testEraseUntilEmpty();
    testDifferentTypes();
    testTraverse();

    std::cout << "\n==== Passed: " << passed
              << ", Failed: " << failed << "====\n";
    return failed == 0 ? 0 : 1;
}
