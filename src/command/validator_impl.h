#pragma once
#include "validator.h"
#include <cctype>
#include <cstring>

inline bool v_allAlnumUnderscore(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        unsigned char u = (unsigned char)c;
        if (!(std::isalnum(u) || c == '_')) return false;
    }
    return true;
}

inline bool v_allVisibleASCII(const std::string& s) {
    for (char c : s) {
        unsigned char u = (unsigned char)c;
        if (u < 0x20 || u > 0x7E) return false;
    }
    return true;
}

inline bool isValidUserID(const std::string& s) {
    if (s.size() > 30) return false;
    return v_allAlnumUnderscore(s);
}

inline bool isValidPassword(const std::string& s) {
    return isValidUserID(s);
}

inline bool isValidUsername(const std::string& s) {
    if (s.empty() || s.size() > 30) return false;
    return v_allVisibleASCII(s);
}

inline bool isValidISBN(const std::string& s) {
    if (s.empty() || s.size() > 20) return false;
    return v_allVisibleASCII(s);
}

inline bool isValidBookField(const std::string& s) {
    if (s.empty() || s.size() > 60) return false;
    if (!v_allVisibleASCII(s)) return false;
    if (s.find('"') != std::string::npos) return false;
    return true;
}

inline bool isValidPrivilegeStr(const std::string& s, int& out) {
    if (s.size() != 1) return false;
    if (s[0] < '0' || s[0] > '9') return false;
    int v = s[0] - '0';
    if (v != 1 && v != 3 && v != 7) return false;
    out = v;
    return true;
}

inline bool isValidQuantityStr(const std::string& s) {
    if (s.empty() || s.size() > 10) return false;
    long long v = 0;
    for (char c : s) {
        if (!std::isdigit((unsigned char)c)) return false;
        v = v * 10 + (c - '0');
    }
    return v <= 2147483647LL;
}

inline bool isValidPriceStr(const std::string& s) {
    if (s.empty() || s.size() > 13) return false;
    int dots = 0;
    for (char c : s) {
        if (c == '.') { if (++dots > 1) return false; }
        else if (!std::isdigit((unsigned char)c)) return false;
    }
    return true;
}

inline bool splitQuoted(const std::string& token, const char* prefix, std::string& out) {
    size_t plen = std::strlen(prefix);
    if (token.size() < plen + 2) return false;
    if (token.compare(0, plen, prefix) != 0) return false;
    if (token[plen] != '"') return false;
    if (token.back() != '"') return false;
    out = token.substr(plen + 1, token.size() - plen - 2);
    return true;
}
