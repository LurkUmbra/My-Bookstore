#include "command/executor.h"
#include "command/tokenizer.h"
#include "system/account_system.h"
#include "system/book_system.h"
#include "system/log_system.h"
#include <iostream>
#include <string>

int main() {
    AccountSystem acc;
    BookSystem    book;
    LogSystem     log;
    Executor      executor(acc, book, log);

    std::string line;
    while (std::getline(std::cin, line)) {
        executor.execute(line);
        if (executor.shouldQuit()) break;
    }
    return 0;
}
