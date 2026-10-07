#pragma once
#include <sstream>
#include <cstdio>
#include <set>

inline BookSystem::BookSystem()
    : books_("books.dat")
    , nameIdx_("name.idx")
    , authorIdx_("author.idx")
    , keywordIdx_("keyword.idx")
{}

inline void BookSystem::pack(const BookData& d, char* out) {
    std::memset(out, 0, BOOK_VALUE_SIZE);
    std::memcpy(out, &d, sizeof(BookData));
}

inline void BookSystem::unpack(const char* buf, BookData& d) {
    std::memcpy(&d, buf, sizeof(BookData));
}

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

inline bool BookSystem::modifyBook(const char* isbn, const ModifyFields& fields) {
    if (!isbn || !*isbn) return false;

    BookData oldData;
    if (!getByISBN(isbn, oldData)) return false;

    BookData newData = oldData;
    char oldKey[IDX_KEY_SIZE], newKey[IDX_KEY_SIZE];

    // name: single-value index, erase old key then insert new key.
    if (fields.name != nullptr && std::strcmp(oldData.name, fields.name) != 0) {
        buildIdxKey(oldKey, IDX_KEY_SIZE, oldData.name, isbn);
        nameIdx_.erase(oldKey);
        buildIdxKey(newKey, IDX_KEY_SIZE, fields.name, isbn);
        nameIdx_.insert(newKey, isbn);
        std::strncpy(newData.name, fields.name, MAX_NAME);
        newData.name[MAX_NAME] = '\0';
    }

    // author: same pattern as name.
    if (fields.author != nullptr && std::strcmp(oldData.author, fields.author) != 0) {
        buildIdxKey(oldKey, IDX_KEY_SIZE, oldData.author, isbn);
        authorIdx_.erase(oldKey);
        buildIdxKey(newKey, IDX_KEY_SIZE, fields.author, isbn);
        authorIdx_.insert(newKey, isbn);
        std::strncpy(newData.author, fields.author, MAX_AUTHOR);
        newData.author[MAX_AUTHOR] = '\0';
    }

    // price: not indexed, update in place.
    if (fields.price != nullptr) {
        newData.price = *fields.price;
    }

    // keyword: multi-value index, apply set difference between old and new.
    if (fields.keyword != nullptr && std::strcmp(oldData.keyword, fields.keyword) != 0) {
        std::set<std::string> oldKw, newKw;
        {
            std::istringstream iss(oldData.keyword);
            std::string s;
            while (std::getline(iss, s, '|')) oldKw.insert(s);
        }
        {
            std::istringstream iss(fields.keyword);
            std::string s;
            while (std::getline(iss, s, '|')) newKw.insert(s);
        }
        for (const auto& kw : oldKw) {
            if (newKw.find(kw) == newKw.end()) {
                buildIdxKey(oldKey, IDX_KEY_SIZE, kw.c_str(), isbn);
                keywordIdx_.erase(oldKey);
            }
        }
        for (const auto& kw : newKw) {
            if (oldKw.find(kw) == oldKw.end()) {
                buildIdxKey(newKey, IDX_KEY_SIZE, kw.c_str(), isbn);
                keywordIdx_.insert(newKey, isbn);
            }
        }
        std::strncpy(newData.keyword, fields.keyword, MAX_KEYWORD);
        newData.keyword[MAX_KEYWORD] = '\0';
    }

    // Rewrite the primary record (value changed, so erase + insert).
    char packed[BOOK_VALUE_SIZE];
    pack(newData, packed);
    books_.erase(isbn);
    books_.insert(isbn, packed);
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
