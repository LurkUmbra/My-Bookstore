#pragma once
#include "tokenizer.h"
#include <cctype>

inline std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool inQuote = false;
    bool hasToken = false;

    for (char c : line) {
        if (inQuote) {
            cur.push_back(c);
            if (c == '"') inQuote = false;
            continue;
        }
        if (c == '"') {
            inQuote = true;
            hasToken = true;
            cur.push_back(c);
            continue;
        }
        if (c == ' ') {
            if (hasToken) { out.push_back(cur); cur.clear(); hasToken = false; }
            continue;
        }
        hasToken = true;
        cur.push_back(c);
    }
    if (hasToken) out.push_back(cur);
    return out;
}
