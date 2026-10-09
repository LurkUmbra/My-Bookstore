#pragma once
#include "../system/account_system.h"
#include "../system/book_system.h"
#include "../system/log_system.h"
#include <string>
#include <vector>

// Routes a command line to the right system and prints results to stdout.
class Executor {
public:
    Executor(AccountSystem& acc, BookSystem& book, LogSystem& log);

    // Execute one command line. Never throws; unknown/invalid input prints "Invalid".
    void execute(const std::string& line);

    // True after a quit/exit command has been issued.
    bool shouldQuit() const { return shouldQuit_; }

private:
    AccountSystem& acc_;
    BookSystem&    book_;
    LogSystem&     log_;
    bool           shouldQuit_ = false;

    bool isPrivilegeAtLeast(int required) const;

    void cmdSu(const std::vector<std::string>& t);
    void cmdLogout(const std::vector<std::string>& t);
    void cmdRegister(const std::vector<std::string>& t);
    void cmdPasswd(const std::vector<std::string>& t);
    void cmdUseradd(const std::vector<std::string>& t);
    void cmdDelete(const std::vector<std::string>& t);
    void cmdShow(const std::vector<std::string>& t);
    void cmdBuy(const std::vector<std::string>& t);
    void cmdSelect(const std::vector<std::string>& t);
    void cmdModify(const std::vector<std::string>& t);
    void cmdImport(const std::vector<std::string>& t);
    void cmdShowFinance(const std::vector<std::string>& t);
    void cmdLog(const std::vector<std::string>& t);
    void cmdReportFinance(const std::vector<std::string>& t);
    void cmdReportEmployee(const std::vector<std::string>& t);

    void printBook(const char* isbn, const BookData& d) const;
    void printInvalid() const;

    // Record an operation on behalf of the current user (or "(guest)").
    void recordOp(const char* action);

    // "select"ed book per the spec is per-session; tracked here.
    std::string selectedISBN_;
};

#include "executor_impl.h"
