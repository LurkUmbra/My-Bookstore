#pragma once
#include "../common/constants.h"
#include "../storage/block_list.h"
#include <cstring>
#include <string>

// ============================================================
// 图书的持久化记录结构（POD，可整体 memcpy 到 BlockList 的 value 里）
// ============================================================
// 警告：这是本项目的私有文件格式。
//   改动字段顺序 / 类型 / 数组长度，会导致旧文件不兼容！
//
// 为什么用 char[] 而不是 std::string？
//   char[] 是 POD，能整体读写；std::string 内含指针，写进文件就是垃圾。
struct BookData {
    char   name[MAX_NAME + 1];       // 61 字节（含 '\0'）
    char   author[MAX_AUTHOR + 1];   // 61
    char   keyword[MAX_KEYWORD + 1]; // 61，多关键词以 '|' 分隔
    double price;                    // 单价（输出精度 2 位小数）
    int    stock;                    // 库存数量
};

static_assert(sizeof(BookData) <= BOOK_VALUE_SIZE,
              "BookData exceeds BOOK_VALUE_SIZE, enlarge it in constants.h");

// ============================================================
// 图书系统：主数据 + 3 个二级索引
// ============================================================
class BookSystem {
public:
    BookSystem();

    // ========== 序列化 / 反序列化 ==========
    // 把 BookData 打包进定长字节缓冲（存之前）
    static void pack(const BookData& d, char* out);
    // 从定长字节缓冲还原 BookData（读之后）
    static void unpack(const char* buf, BookData& d);

    // ========== 业务操作（本轮先声明，下轮实现）==========
    // 新建图书（首次录入完整信息）
    bool createBook(const char* isbn, const char* name, const char* author,
                    const char* keyword, double price);
    // 按 ISBN 查，找到则填充 out
    bool getByISBN(const char* isbn, BookData& out);

private:
    BlockList<BOOK_KEY_SIZE, BOOK_VALUE_SIZE> books_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> nameIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> authorIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> keywordIdx_;

    // 构造索引 key："字段值|ISBN"
    static void buildIdxKey(char* out, int outSize, const char* field, const char* isbn);
};

#include "book_system_impl.h"
