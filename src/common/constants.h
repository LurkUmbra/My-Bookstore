#pragma once

// Maximum field lengths (excluding the terminating '\0').

constexpr int MAX_ISBN     = 20;   // ISBN: printable ASCII
constexpr int MAX_NAME     = 60;   // book name
constexpr int MAX_AUTHOR   = 60;   // author name
constexpr int MAX_KEYWORD  = 60;   // keywords separated by '|'
constexpr int MAX_USERID   = 30;   // user id (alnum + underscore)
constexpr int MAX_PASSWD   = 30;   // password
constexpr int MAX_USERNAME = 30;   // display name

// BlockList template parameters (KEY_SIZE / VALUE_SIZE include '\0').

constexpr int BOOK_KEY_SIZE   = MAX_ISBN + 1;    // 21
constexpr int BOOK_VALUE_SIZE = 208;             // >= sizeof(BookData)

constexpr int IDX_KEY_SIZE   = 128;              // "field|ISBN"
constexpr int IDX_VALUE_SIZE = MAX_ISBN + 1;     // 21

constexpr int ACC_KEY_SIZE   = MAX_USERID + 1;   // 31
constexpr int ACC_VALUE_SIZE = 128;

constexpr int LOG_KEY_SIZE   = 21;
constexpr int LOG_VALUE_SIZE = 24;

constexpr int OP_KEY_SIZE   = 21;
constexpr int OP_VALUE_SIZE = 128;
