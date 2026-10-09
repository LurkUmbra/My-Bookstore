#pragma once
#include "executor.h"
#include "tokenizer.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

inline Executor::Executor(AccountSystem& acc, BookSystem& book, LogSystem& log)
    : acc_(acc), book_(book), log_(log) {}

inline bool Executor::isPrivilegeAtLeast(int required) const {
    return acc_.currentPrivilege() >= required;
}

inline void Executor::printInvalid() const {
    std::printf("Invalid\n");
}

inline void Executor::recordOp(const char* action) {
    const char* u = acc_.currentUser();
    log_.recordOperation(u ? u : "(guest)", action);
}

inline void Executor::printBook(const char* isbn, const BookData& d) const {
    std::printf("%s\t%s\t%s\t%s\t%.2f\t%d\n",
                isbn, d.name, d.author, d.keyword, d.price, d.stock);
}

// Strip a leading/trailing double quote if both present.
static std::string unquote(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

inline void Executor::execute(const std::string& line) {
    auto t = tokenize(line);
    if (t.empty()) return;
    const std::string& cmd = t[0];

    if (cmd == "quit" || cmd == "exit")      { shouldQuit_ = true; return; }
    if (cmd == "su")                         { cmdSu(t); return; }
    if (cmd == "logout")                     { cmdLogout(t); return; }
    if (cmd == "register")                   { cmdRegister(t); return; }
    if (cmd == "passwd")                     { cmdPasswd(t); return; }
    if (cmd == "useradd")                    { cmdUseradd(t); return; }
    if (cmd == "delete")                     { cmdDelete(t); return; }
    if (cmd == "show") {
        if (t.size() >= 2 && t[1] == "finance") { cmdShowFinance(t); return; }
        cmdShow(t); return;
    }
    if (cmd == "buy")                        { cmdBuy(t); return; }
    if (cmd == "select")                     { cmdSelect(t); return; }
    if (cmd == "modify")                     { cmdModify(t); return; }
    if (cmd == "import")                     { cmdImport(t); return; }
    if (cmd == "log")                        { cmdLog(t); return; }
    if (cmd == "report") {
        if (t.size() == 2 && t[1] == "finance")  { cmdReportFinance(t); return; }
        if (t.size() == 2 && t[1] == "employee") { cmdReportEmployee(t); return; }
    }
    printInvalid();
}

// ---------- account commands ----------

inline void Executor::cmdSu(const std::vector<std::string>& t) {
    if (t.size() != 2 && t.size() != 3) { printInvalid(); return; }
    const char* userid = t[1].c_str();
    const char* pw = (t.size() == 3) ? t[2].c_str() : nullptr;
    if (!acc_.login(userid, pw)) printInvalid();
}

inline void Executor::cmdLogout(const std::vector<std::string>& t) {
    if (t.size() != 1) { printInvalid(); return; }
    if (!acc_.logout()) printInvalid();
}

inline void Executor::cmdRegister(const std::vector<std::string>& t) {
    if (t.size() != 4) { printInvalid(); return; }
    if (!acc_.registerUser(t[1].c_str(), t[2].c_str(), t[3].c_str())) { printInvalid(); return; }
    std::string act = "register "; act += t[1];
    recordOp(act.c_str());
}

inline void Executor::cmdPasswd(const std::vector<std::string>& t) {
    if (t.size() != 3 && t.size() != 4) { printInvalid(); return; }
    const char* userid = t[1].c_str();
    const char* cur = (t.size() == 4) ? t[2].c_str() : nullptr;
    const char* nw  = (t.size() == 4) ? t[3].c_str() : t[2].c_str();
    if (!acc_.changePassword(userid, cur, nw)) { printInvalid(); return; }
    std::string act = "passwd "; act += userid;
    recordOp(act.c_str());
}

inline void Executor::cmdUseradd(const std::vector<std::string>& t) {
    if (t.size() != 5) { printInvalid(); return; }
    if (t[3].size() != 1 || t[3][0] < '0' || t[3][0] > '9') { printInvalid(); return; }
    int priv = t[3][0] - '0';
    if (!acc_.useradd(t[1].c_str(), t[2].c_str(), priv, t[4].c_str())) { printInvalid(); return; }
    std::string act = "useradd "; act += t[1];
    recordOp(act.c_str());
}

inline void Executor::cmdDelete(const std::vector<std::string>& t) {
    if (t.size() != 2) { printInvalid(); return; }
    if (!acc_.deleteUser(t[1].c_str())) { printInvalid(); return; }
    std::string act = "delete "; act += t[1];
    recordOp(act.c_str());
}

// ---------- book commands ----------

inline void Executor::cmdShow(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(1)) { printInvalid(); return; }

    bool any = false;
    auto printer = [&](const char* isbn, const BookData& d) {
        printBook(isbn, d); any = true; return true;
    };

    // show            -> all
    if (t.size() == 1) {
        book_.showAll(printer);
        if (!any) std::printf("\n");
        return;
    }
    if (t.size() != 2) { printInvalid(); return; }

    const std::string& arg = t[1];
    if (arg.rfind("-ISBN=", 0) == 0) {
        std::string v = arg.substr(6);
        if (v.empty()) { printInvalid(); return; }
        book_.showByISBN(v.c_str(), printer);
    } else if (arg.rfind("-name=", 0) == 0) {
        std::string v = unquote(arg.substr(6));
        if (v.empty()) { printInvalid(); return; }
        book_.showByName(v.c_str(), printer);
    } else if (arg.rfind("-author=", 0) == 0) {
        std::string v = unquote(arg.substr(8));
        if (v.empty()) { printInvalid(); return; }
        book_.showByAuthor(v.c_str(), printer);
    } else if (arg.rfind("-keyword=", 0) == 0) {
        std::string v = unquote(arg.substr(9));
        if (v.empty()) { printInvalid(); return; }
        if (v.find('|') != std::string::npos) { printInvalid(); return; }
        book_.showByKeyword(v.c_str(), printer);
    } else {
        printInvalid(); return;
    }
    if (!any) std::printf("\n");
}

inline void Executor::cmdBuy(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(1)) { printInvalid(); return; }
    if (t.size() != 3) { printInvalid(); return; }
    int qty = std::atoi(t[2].c_str());
    if (qty <= 0) { printInvalid(); return; }
    double cost = 0;
    if (!book_.buyBook(t[1].c_str(), qty, cost)) { printInvalid(); return; }
    log_.recordIncome(cost);
    std::string act = "buy "; act += t[1];
    recordOp(act.c_str());
    std::printf("%.2f\n", cost);
}

inline void Executor::cmdSelect(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(3)) { printInvalid(); return; }
    if (t.size() != 2) { printInvalid(); return; }
    if (!book_.ensureBook(t[1].c_str())) { printInvalid(); return; }
    selectedISBN_ = t[1];
    std::string act = "select "; act += t[1];
    recordOp(act.c_str());
}

inline void Executor::cmdModify(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(3)) { printInvalid(); return; }
    if (selectedISBN_.empty())  { printInvalid(); return; }
    if (t.size() < 2)           { printInvalid(); return; }

    BookSystem::ModifyFields f;
    double priceVal = 0;
    bool hasName = false, hasAuthor = false, hasKeyword = false, hasPrice = false;
    std::string nameV, authorV, keywordV;

    for (size_t i = 1; i < t.size(); ++i) {
        const std::string& a = t[i];
        if (a.rfind("-ISBN=", 0) == 0) {
            // ISBN change: treat as invalid for simplicity (not in spec examples as required change)
            printInvalid(); return;
        } else if (a.rfind("-name=", 0) == 0) {
            if (hasName) { printInvalid(); return; }
            nameV = unquote(a.substr(6)); hasName = true;
            if (nameV.empty()) { printInvalid(); return; }
        } else if (a.rfind("-author=", 0) == 0) {
            if (hasAuthor) { printInvalid(); return; }
            authorV = unquote(a.substr(8)); hasAuthor = true;
            if (authorV.empty()) { printInvalid(); return; }
        } else if (a.rfind("-keyword=", 0) == 0) {
            if (hasKeyword) { printInvalid(); return; }
            keywordV = unquote(a.substr(9)); hasKeyword = true;
            if (keywordV.empty()) { printInvalid(); return; }
        } else if (a.rfind("-price=", 0) == 0) {
            if (hasPrice) { printInvalid(); return; }
            priceVal = std::atof(a.substr(7).c_str()); hasPrice = true;
        } else {
            printInvalid(); return;
        }
    }
    if (!hasName && !hasAuthor && !hasKeyword && !hasPrice) { printInvalid(); return; }

    if (hasName)    f.name = nameV.c_str();
    if (hasAuthor)  f.author = authorV.c_str();
    if (hasKeyword) f.keyword = keywordV.c_str();
    if (hasPrice)   f.price = &priceVal;

    if (!book_.modifyBook(selectedISBN_.c_str(), f)) { printInvalid(); return; }
    std::string act = "modify "; act += selectedISBN_;
    recordOp(act.c_str());
}

inline void Executor::cmdImport(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(3)) { printInvalid(); return; }
    if (selectedISBN_.empty())  { printInvalid(); return; }
    if (t.size() != 3)          { printInvalid(); return; }
    int qty = std::atoi(t[1].c_str());
    double cost = std::atof(t[2].c_str());
    if (qty <= 0 || cost <= 0) { printInvalid(); return; }
    if (!book_.importBook(selectedISBN_.c_str(), qty, cost)) { printInvalid(); return; }
    log_.recordExpense(cost);
    std::string act = "import "; act += selectedISBN_;
    recordOp(act.c_str());
}

inline void Executor::cmdShowFinance(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(7)) { printInvalid(); return; }
    double inc = 0, exp = 0;

    if (t.size() == 2) {
        // No Count: sum of all transactions (0.00 0.00 when none).
        log_.showFinance(log_.totalCount(), inc, exp);
        std::printf("+ %.2f - %.2f\n", inc, exp);
        return;
    }
    if (t.size() == 3) {
        long long count = std::atoll(t[2].c_str());
        if (count == 0) { std::printf("\n"); return; }   // spec: Count 0 -> blank line
        if (!log_.showFinance(count, inc, exp)) { printInvalid(); return; }
        std::printf("+ %.2f - %.2f\n", inc, exp);
        return;
    }
    printInvalid();
}

inline void Executor::cmdLog(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(7)) { printInvalid(); return; }
    if (t.size() != 1) { printInvalid(); return; }

    log_.traverseOperations([](const char*, const OpEntry& e) {
        std::printf("%s\t%s\n", e.userid, e.action);
        return true;
    });
    int i = 0;
    log_.traverseTransactions([&](const char*, const LogEntry& e) {
        std::printf("%d\t+ %.2f - %.2f\n", i++, e.income, e.expense);
        return true;
    });
}

inline void Executor::cmdReportFinance(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(7)) { printInvalid(); return; }
    if (t.size() != 2) { printInvalid(); return; }

    long long n = log_.totalCount();
    double inc = 0, exp = 0;
    log_.showFinance(n, inc, exp);
    std::printf("transactions\t%lld\n", n);
    std::printf("income\t%.2f\n", inc);
    std::printf("expense\t%.2f\n", exp);
    std::printf("profit\t%.2f\n", inc - exp);
}

inline void Executor::cmdReportEmployee(const std::vector<std::string>& t) {
    if (!isPrivilegeAtLeast(7)) { printInvalid(); return; }
    if (t.size() != 2) { printInvalid(); return; }

    if (log_.opCount() == 0) { std::printf("\n"); return; }
    log_.traverseOperations([](const char*, const OpEntry& e) {
        std::printf("%s\t%s\n", e.userid, e.action);
        return true;
    });
}
