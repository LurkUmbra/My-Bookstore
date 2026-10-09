#include "../src/system/account_system.h"
#include <cstdio>
#include <cstring>

static int passed = 0, failed = 0;
#define CHECK(cond, msg) do { \
    if (cond) { printf("[PASS] %s\n", msg); passed++; } \
    else      { printf("[FAIL] %s\n", msg); failed++; } \
} while (0)

static void clean() { std::remove("accounts.dat"); }

int main() {
    clean();
    {
        AccountSystem sys;
        // Bootstrap: root auto-created
        CHECK(!sys.isLoggedIn(), "initial: not logged in");
        CHECK(sys.currentPrivilege() == 0, "initial: privilege 0");
        CHECK(sys.currentUser() == nullptr, "initial: currentUser nullptr");

        // Register
        CHECK(sys.registerUser("alice", "pw1", "Alice"), "register alice");
        CHECK(!sys.registerUser("alice", "x", "Dup"), "register duplicate -> false");
        CHECK(!sys.registerUser("root", "x", "Dup"), "register root -> false");

        // Login (no prior privilege)
        CHECK(!sys.login("alice", nullptr), "login alice no pw (guest) -> false");
        CHECK(!sys.login("alice", "wrong"), "login alice wrong pw -> false");
        CHECK(sys.login("alice", "pw1"), "login alice correct pw");
        CHECK(sys.currentPrivilege() == 1, "after login: privilege 1");
        CHECK(std::strcmp(sys.currentUser(), "alice") == 0, "after login: currentUser = alice");

        // Alice cannot useradd (priv 1)
        CHECK(!sys.useradd("bob", "pw", 1, "Bob"), "alice useradd -> false");

        // Login root (nested)
        CHECK(sys.login("root", "sjtu"), "login root");
        CHECK(sys.currentPrivilege() == 7, "after root: privilege 7");

        // Root can useradd
        CHECK(sys.useradd("bob", "pw2", 3, "Bob"), "root useradd bob(3)");
        CHECK(!sys.useradd("carol", "pw", 7, "Carol"), "root useradd priv 7 -> false");
        CHECK(sys.useradd("dave", "pw", 3, "Dave"), "root useradd dave(3)");


        // Logout twice: back to alice, then guest
        CHECK(sys.logout(), "logout root");
        CHECK(sys.currentPrivilege() == 1, "back to alice (priv 1)");
        CHECK(sys.logout(), "logout alice");
        CHECK(!sys.isLoggedIn(), "logged out entirely");
        CHECK(!sys.logout(), "logout empty -> false");

        // Login root and change alice password
        sys.login("root", "sjtu");
        CHECK(sys.changePassword("alice", nullptr, "newpw"), "root change alice pw (no current)");
        CHECK(!sys.changePassword("nobody", nullptr, "x"), "change pw of nonexistent -> false");

        // Delete alice
        CHECK(sys.deleteUser("alice"), "delete alice");
        CHECK(!sys.deleteUser("alice"), "delete alice again -> false");
        CHECK(sys.deleteUser("bob"), "delete bob (not logged in, succeeds)");

        // Cannot delete root while logged in
        CHECK(!sys.deleteUser("root"), "delete root while logged in -> false");
    }

    // Reopen: only persistence of accounts_ matters; login stack resets
    {
        AccountSystem sys;
        CHECK(!sys.isLoggedIn(), "reopen: not logged in");
        CHECK(!sys.login("bob", "pw2"), "reopen: bob deleted, login fails");
    }

    clean();
    printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
