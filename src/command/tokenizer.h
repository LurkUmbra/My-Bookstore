#pragma once
#include <string>
#include <vector>

// Split a command line into tokens.
//
// Rules:
//   - whitespace separates tokens;
//   - a segment enclosed in double quotes is one token;
//   - quotes are kept in the token (the parser decides whether they matter);
//   - a line of only whitespace yields an empty vector.
std::vector<std::string> tokenize(const std::string& line);

#include "tokenizer_impl.h"
