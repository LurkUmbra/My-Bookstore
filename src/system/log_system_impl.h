#pragma once
#include <cstdio>
#include <cstring>
#include <string>

inline LogSystem::LogSystem() : log_("finance.log"), ops_("operation.log") {
    long long maxSeq = -1;
    log_.traverse([&](const char* key, const char*) {
        long long seq = std::stoll(key);
        if (seq > maxSeq) maxSeq = seq;
        return true;
    });
    total_ = maxSeq + 1;

    long long maxOp = -1;
    ops_.traverse([&](const char* key, const char*) {
        long long seq = std::stoll(key);
        if (seq > maxOp) maxOp = seq;
        return true;
    });
    opTotal_ = maxOp + 1;
}

inline void LogSystem::append(const LogEntry& e) {
    char key[LOG_KEY_SIZE];
    std::snprintf(key, LOG_KEY_SIZE, "%020lld", total_);
    char buf[LOG_VALUE_SIZE];
    std::memset(buf, 0, LOG_VALUE_SIZE);
    std::memcpy(buf, &e, sizeof(LogEntry));
    log_.insert(key, buf);
    total_++;
}

inline void LogSystem::recordIncome(double amount) {
    LogEntry e{amount, 0.0};
    append(e);
}

inline void LogSystem::recordExpense(double amount) {
    LogEntry e{0.0, amount};
    append(e);
}

inline void LogSystem::appendOp(const OpEntry& e) {
    char key[OP_KEY_SIZE];
    std::snprintf(key, OP_KEY_SIZE, "%020lld", opTotal_);
    char buf[OP_VALUE_SIZE];
    std::memset(buf, 0, OP_VALUE_SIZE);
    std::memcpy(buf, &e, sizeof(OpEntry));
    ops_.insert(key, buf);
    opTotal_++;
}

inline void LogSystem::recordOperation(const char* userid, const char* action) {
    OpEntry e{};
    std::strncpy(e.userid, userid ? userid : "(guest)", MAX_USERID);
    std::strncpy(e.action, action, sizeof(e.action) - 1);
    appendOp(e);
}

template <typename Func>
inline void LogSystem::traverseTransactions(Func fn) const {
    log_.traverse([&](const char* key, const char* value) {
        LogEntry e;
        std::memcpy(&e, value, sizeof(LogEntry));
        return fn(key, e);
    });
}

template <typename Func>
inline void LogSystem::traverseOperations(Func fn) const {
    ops_.traverse([&](const char* key, const char* value) {
        OpEntry e;
        std::memcpy(&e, value, sizeof(OpEntry));
        return fn(key, e);
    });
}

inline bool LogSystem::showFinance(long long count, double& totalIncome, double& totalExpense) const {
    totalIncome = 0;
    totalExpense = 0;
    if (count < 0 || count > total_) return false;
    if (count == 0) return true;

    long long startSeq = total_ - count;
    char lo[LOG_KEY_SIZE], hi[LOG_KEY_SIZE];
    std::snprintf(lo, LOG_KEY_SIZE, "%020lld", startSeq);
    std::snprintf(hi, LOG_KEY_SIZE, "%020lld", total_);

    log_.traverseRange(lo, hi, [&](const char*, const char* value) {
        LogEntry e;
        std::memcpy(&e, value, sizeof(LogEntry));
        totalIncome += e.income;
        totalExpense += e.expense;
        return true;
    });
    return true;
}
