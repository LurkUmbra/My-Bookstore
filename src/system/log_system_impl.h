#pragma once
#include <cstdio>
#include <cstring>
#include <string>

inline LogSystem::LogSystem() : log_("finance.log") {
    long long maxSeq = -1;
    log_.traverse([&](const char* key, const char*) {
        long long seq = std::stoll(key);
        if (seq > maxSeq) maxSeq = seq;
        return true;
    });
    total_ = maxSeq + 1;
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

inline bool LogSystem::showFinance(long long count, double& totalIncome, double& totalExpense) {
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
