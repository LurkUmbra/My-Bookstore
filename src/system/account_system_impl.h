#pragma once
#include <cstring>
#include <cstdio>

inline AccountSystem::AccountSystem() : accounts_("accounts.dat") {
    // Bootstrap the root account on first run.
    AccountData tmp;
    if (!findAccount("root", tmp)) {
        AccountData root{};
        std::strncpy(root.password, "sjtu", MAX_PASSWD);
        std::strncpy(root.username, "root", MAX_USERNAME);
        root.privilege = 7;
        char buf[ACC_VALUE_SIZE];
        std::memset(buf, 0, ACC_VALUE_SIZE);
        std::memcpy(buf, &root, sizeof(AccountData));
        accounts_.insert("root", buf);
    }
}

inline bool AccountSystem::findAccount(const char* userid, AccountData& out) const {
    char buf[ACC_VALUE_SIZE];
    if (!accounts_.find(userid, buf)) return false;
    std::memcpy(&out, buf, sizeof(AccountData));
    return true;
}

inline bool AccountSystem::isInLoginStack(const char* userid) const {
    for (const auto& id : loginStack_) {
        if (id == userid) return true;
    }
    return false;
}

inline int AccountSystem::currentPrivilege() const {
    if (loginStack_.empty()) return 0;
    AccountData d;
    if (!findAccount(loginStack_.back().c_str(), d)) return 0;
    return d.privilege;
}

inline bool AccountSystem::isLoggedIn() const {
    return !loginStack_.empty();
}

inline const char* AccountSystem::currentUser() const {
    return loginStack_.empty() ? nullptr : loginStack_.back().c_str();
}

inline bool AccountSystem::registerUser(const char* userid, const char* password, const char* username) {
    AccountData tmp;
    if (findAccount(userid, tmp)) return false;
    AccountData d{};
    std::strncpy(d.password, password, MAX_PASSWD);
    std::strncpy(d.username, username, MAX_USERNAME);
    d.privilege = 1;
    char buf[ACC_VALUE_SIZE];
    std::memset(buf, 0, ACC_VALUE_SIZE);
    std::memcpy(buf, &d, sizeof(AccountData));
    return accounts_.insert(userid, buf);
}

inline bool AccountSystem::useradd(const char* userid, const char* password, int privilege, const char* username) {
    if (currentPrivilege() < 3) return false;
    if (privilege >= currentPrivilege()) return false;
    AccountData tmp;
    if (findAccount(userid, tmp)) return false;
    AccountData d{};
    std::strncpy(d.password, password, MAX_PASSWD);
    std::strncpy(d.username, username, MAX_USERNAME);
    d.privilege = privilege;
    char buf[ACC_VALUE_SIZE];
    std::memset(buf, 0, ACC_VALUE_SIZE);
    std::memcpy(buf, &d, sizeof(AccountData));
    return accounts_.insert(userid, buf);
}

inline bool AccountSystem::deleteUser(const char* userid) {
    if (currentPrivilege() != 7) return false;
    AccountData tmp;
    if (!findAccount(userid, tmp)) return false;
    if (isInLoginStack(userid)) return false;
    return accounts_.erase(userid);
}

inline bool AccountSystem::login(const char* userid, const char* password) {
    AccountData d;
    if (!findAccount(userid, d)) return false;
    if (password != nullptr) {
        if (std::strcmp(d.password, password) != 0) return false;
    } else {
        if (currentPrivilege() <= d.privilege) return false;
    }
    loginStack_.push_back(userid);
    return true;
}

inline bool AccountSystem::logout() {
    if (loginStack_.empty()) return false;
    loginStack_.pop_back();
    return true;
}

inline bool AccountSystem::changePassword(const char* userid, const char* currentPassword, const char* newPassword) {
    if (currentPrivilege() < 1) return false;
    AccountData d;
    if (!findAccount(userid, d)) return false;
    if (currentPrivilege() != 7) {
        if (currentPassword == nullptr) return false;
        if (std::strcmp(d.password, currentPassword) != 0) return false;
    }
    std::strncpy(d.password, newPassword, MAX_PASSWD);
    d.password[MAX_PASSWD] = '\0';
    char buf[ACC_VALUE_SIZE];
    std::memset(buf, 0, ACC_VALUE_SIZE);
    std::memcpy(buf, &d, sizeof(AccountData));
    accounts_.erase(userid);
    return accounts_.insert(userid, buf);
}
