#pragma once
#include "../common/constants.h"
#include "../storage/block_list.h"
#include <cstring>
#include <cstdio>

// One transaction: exactly one of income/expense is non-zero.
struct LogEntry {
    double income;
    double expense;
};

static_assert(sizeof(LogEntry) <= LOG_VALUE_SIZE,
              "LogEntry exceeds LOG_VALUE_SIZE, enlarge it in constants.h");

// Persistent financial log.
// Keys are zero-padded sequence numbers so lexicographic order matches
// numeric order.
class LogSystem {
public:
    LogSystem();

    void recordIncome(double amount);
    void recordExpense(double amount);

    // Sum income/expense over the last `count` transactions.
    // Returns false if count > totalCount(). count == 0 succeeds with (0, 0).
    bool showFinance(long long count, double& totalIncome, double& totalExpense);

    long long totalCount() const { return total_; }

private:
    BlockList<LOG_KEY_SIZE, LOG_VALUE_SIZE> log_;
    long long total_ = 0;

    void append(const LogEntry& e);
};

#include "log_system_impl.h"
