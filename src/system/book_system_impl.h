#pragma once
#include <sstream>
#include <cstdio>

inline BookSystem::BookSystem()
    : books_("books.dat")
    , nameIdx_("name.idx")
    , authorIdx_("author.idx")
    , keywordIdx_("keyword.idx")
{}

// ---------- 序列化 / 反序列化 ----------
inline void BookSystem::pack(const BookData& d, char* out) {
    std::memset(out, 0, BOOK_VALUE_SIZE);
    std::memcpy(out, &d, sizeof(BookData));
}

inline void BookSystem::unpack(const char* buf, BookData& d) {
    std::memcpy(&d, buf, sizeof(BookData));
}

// ---------- 索引 key 拼接 ----------
inline void BookSystem::buildIdxKey(char* out, int outSize, const char* field, const char* isbn) {
    std::snprintf(out, outSize, "%s|%s", field, isbn);
}

inline bool BookSystem::createBook(const char* isbn, const char* name, const char* author,
                                   const char* keyword, double price) {
    if (!isbn || !*isbn)       return false;
    if (!name || !*name)       return false;
    if (!author || !*author)   return false;
    if (!keyword || !*keyword) return false;
    if (price < 0)             return false;

    char tmp[BOOK_VALUE_SIZE];
    if (books_.find(isbn, tmp)) return false;

    BookData d{};
    std::strncpy(d.name,    name,    MAX_NAME);
    std::strncpy(d.author,  author,  MAX_AUTHOR);
    std::strncpy(d.keyword, keyword, MAX_KEYWORD);
    d.price = price;
    d.stock = 0;

    char packed[BOOK_VALUE_SIZE];
    pack(d, packed);
    books_.insert(isbn, packed);

    char idxKey[IDX_KEY_SIZE];

    buildIdxKey(idxKey, IDX_KEY_SIZE, name, isbn);
    nameIdx_.insert(idxKey, isbn);

    buildIdxKey(idxKey, IDX_KEY_SIZE, author, isbn);
    authorIdx_.insert(idxKey, isbn);

    std::string kwStr(keyword);
    std::istringstream iss(kwStr);
    std::string seg;
    while (std::getline(iss, seg, '|')) {
        buildIdxKey(idxKey, IDX_KEY_SIZE, seg.c_str(), isbn);
        keywordIdx_.insert(idxKey, isbn);
    }

    return true;
}

inline bool BookSystem::getByISBN(const char* isbn, BookData& out) {
    char buf[BOOK_VALUE_SIZE];
    if (!books_.find(isbn, buf)) return false;
    unpack(buf, out);
    return true;
}

template <typename Func>
inline void BookSystem::showByName(const char* name, Func fn) {
    char lo[IDX_KEY_SIZE], hi[IDX_KEY_SIZE];
    std::snprintf(lo, IDX_KEY_SIZE, "%s|", name);
    std::snprintf(hi, IDX_KEY_SIZE, "%s}", name);

    nameIdx_.traverseRange(lo, hi, [&](const char* /*idxKey*/, const char* isbn) {
        BookData d;
        if (getByISBN(isbn, d)) {
            return fn(d);
        }
        return true;
    });
}

template <typename Func>
inline void BookSystem::showByAuthor(const char* author, Func fn) {
    char lo[IDX_KEY_SIZE], hi[IDX_KEY_SIZE];
    std::snprintf(lo, IDX_KEY_SIZE, "%s|", author);
    std::snprintf(hi, IDX_KEY_SIZE, "%s}", author);

    authorIdx_.traverseRange(lo, hi, [&](const char* /*idxKey*/, const char* isbn) {
        BookData d;
        if (getByISBN(isbn, d)) {
            return fn(d);
        }
        return true;
    });
}

template <typename Func>
inline void BookSystem::showByKeyword(const char* keyword, Func fn) {
    char lo[IDX_KEY_SIZE], hi[IDX_KEY_SIZE];
    std::snprintf(lo, IDX_KEY_SIZE, "%s|", keyword);
    std::snprintf(hi, IDX_KEY_SIZE, "%s}", keyword);
    
    keywordIdx_.traverseRange(lo, hi, [&](const char* /*idxKey*/, const char* isbn) {
        BookData d;
        if (getByISBN(isbn, d)) {
            return fn(d);
        }
        return true;
    });
}