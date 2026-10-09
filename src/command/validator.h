#pragma once
#include <string>

// Character-set and length rules for command arguments, per the assignment spec.
// Every function returns false for invalid input.

// [UserID] / [Password] / [CurrentPassword] / [NewPassword]
//   charset: [A-Za-z0-9_]; length <= 30
bool isValidUserID(const std::string& s);
bool isValidPassword(const std::string& s);

// [Username]: visible ASCII; length <= 30
bool isValidUsername(const std::string& s);

// [ISBN]: visible ASCII; length <= 20
bool isValidISBN(const std::string& s);

// [BookName] / [Author] / [Keyword] segment: visible ASCII minus '"'; length <= 60
bool isValidBookField(const std::string& s);

// [Privilege]: single digit among 1, 3, 7. On success writes the value.
bool isValidPrivilegeStr(const std::string& s, int& out);

// [Quantity] / [Count]: digits; length <= 10; value <= 2^31-1
bool isValidQuantityStr(const std::string& s);

// [Price] / [TotalCost]: digits and '.'; length <= 13; at most one '.'
bool isValidPriceStr(const std::string& s);

// Split "prefix\"value\"" into value, requiring both quotes.
// Example: splitQuoted("\"-name=\"Math\"", "-name=", out) -> out = "Math"
bool splitQuoted(const std::string& token, const char* prefix, std::string& out);

#include "validator_impl.h"
