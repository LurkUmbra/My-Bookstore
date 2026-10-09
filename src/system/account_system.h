#pragma once
#include "../common/constants.h"
#include "../storage/block_list.h"
#include <string>
#include <vector>

// Persistent record for a user account.
//
// WARNING: private file format; reordering fields breaks compatibility
// with existing accounts.dat files.
struct AccountData {
    char password[MAX_PASSWD + 1];
    char username[MAX_USERNAME + 1];
    int  privilege;                  // 1, 3 or 7
};

static_assert(sizeof(AccountData) <= ACC_VALUE_SIZE,
              "AccountData exceeds ACC_VALUE_SIZE, enlarge it in constants.h");

// Account system with a nested login stack.
//
// The login stack is in-memory only: it is intentionally reset on startup
// (quitting the program logs out every account, per spec).
class AccountSystem {
public:
    AccountSystem();

    // ---- account management ----
    // Register a privilege-1 account. Requires no login. Fails on duplicate id.
    bool registerUser(const char* userid, const char* password, const char* username);

    // Create an account with the given privilege.
    // Requires current privilege >= 3 and new privilege < current privilege.
    bool useradd(const char* userid, const char* password, int privilege, const char* username);

    // Delete an account. Requires current privilege == 7.
    // Fails if target does not exist or is currently logged in.
    bool deleteUser(const char* userid);

    // ---- login / logout ----
    // Log in as userid. password may be nullptr, in which case the current
    // privilege must be strictly greater than the target account privilege.
    bool login(const char* userid, const char* password);

    // Pop the top of the login stack. Requires a non-empty stack.
    bool logout();

    // Change password. Requires current privilege >= 1.
    // If current privilege == 7, currentPassword may be nullptr;
    // otherwise currentPassword must match the target account password.
    bool changePassword(const char* userid, const char* currentPassword, const char* newPassword);

    // ---- state queries ----
    int  currentPrivilege() const;   // 0 when no account is logged in
    bool isLoggedIn() const;
    const char* currentUser() const; // nullptr when not logged in

private:
    BlockList<ACC_KEY_SIZE, ACC_VALUE_SIZE> accounts_;
    std::vector<std::string> loginStack_;

    bool findAccount(const char* userid, AccountData& out);
    bool isInLoginStack(const char* userid) const;
};

#include "account_system_impl.h"
