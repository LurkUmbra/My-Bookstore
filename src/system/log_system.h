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

// One operation record: who did what.
struct OpEntry {
    char userid[MAX_USERID + 1];
    char action[96];
};

static_assert(sizeof(LogEntry) <= LOG_VALUE_SIZE,
              "LogEntry exceeds LOG_VALUE_SIZE, enlarge it in constants.h");
static_assert(sizeof(OpEntry) <= OP_VALUE_SIZE,
              "OpEntry exceeds OP_VALUE_SIZE, enlarge it in constants.h");

// Persistent financial log.
// Keys are zero-padded sequence numbers so lexicographic order matches
// numeric order.
class LogSystem {
public:
    LogSystem();

    void recordIncome(double amount);
    void recordExpense(double amount);
    // Record a user operation: "userid did action".
    void recordOperation(const char* userid, const char* action);

    // Sum income/expense over the last `count` transactions.
    // Returns false if count > totalCount(). count == 0 succeeds with (0, 0).
    bool showFinance(long long count, double& totalIncome, double& totalExpense) const;

    long long totalCount() const { return total_; }
    long long opCount() const { return opTotal_; }

    // Iterate over transactions/operations in insertion order.
    // fn(key, entry) returns false to stop early.
    template <typename Func> void traverseTransactions(Func fn) const;
    template <typename Func> void traverseOperations(Func fn) const;

private:
    BlockList<LOG_KEY_SIZE, LOG_VALUE_SIZE> log_;
    BlockList<OP_KEY_SIZE,   OP_VALUE_SIZE> ops_;
    long long total_   = 0;
    long long opTotal_ = 0;

    void append(const LogEntry& e);
    void appendOp(const OpEntry& e);
};

#include "log_system_impl.h"
