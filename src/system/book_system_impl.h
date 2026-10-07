#pragma once
// 实现在此，被 book_system.h 末尾包含（模板/内联风格）

// 构造函数：成员初始化列表，顺序必须与声明顺序一致
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

// ---------- 业务操作（占位，下轮实现）----------
inline bool BookSystem::createBook(const char* isbn, const char* name, const char* author,
                                   const char* keyword, double price) {
    // TODO(next): 校验 + 写主数据 + 写三个索引（保持原子性）
    (void)isbn; (void)name; (void)author; (void)keyword; (void)price;
    return false;
}

inline bool BookSystem::getByISBN(const char* isbn, BookData& out) {
    char buf[BOOK_VALUE_SIZE];
    if (!books_.find(isbn, buf)) return false;
    unpack(buf, out);
    return true;
}
