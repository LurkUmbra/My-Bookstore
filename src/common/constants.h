#pragma once

// 全局常量：所有"最大长度"都指"不含结尾 '\0' 的字符数"

// ===== 业务字段最大长度（不含 '\0'）=====
constexpr int MAX_ISBN     = 20;   // ISBN：除不可见字符外 ASCII
constexpr int MAX_NAME     = 60;   // 书名
constexpr int MAX_AUTHOR   = 60;   // 作者
constexpr int MAX_KEYWORD  = 60;   // 关键词（多段用 '|' 分隔，总长 ≤ 60）
constexpr int MAX_USERID   = 30;   // 用户 ID（数字/字母/下划线）
constexpr int MAX_PASSWD   = 30;   // 密码
constexpr int MAX_USERNAME = 30;   // 用户名（除不可见字符外 ASCII）

// ===== BlockList 模板尺寸参数 =====
// 约定：所有 KEY_SIZE / VALUE_SIZE 均 "含结尾 '\0'"

// 图书主数据文件 books.dat
constexpr int BOOK_KEY_SIZE   = MAX_ISBN + 1;    // 21
constexpr int BOOK_VALUE_SIZE = 208;             // >= sizeof(BookData)=200，留余量

// 索引文件（key = "字段值|ISBN"，value = ISBN）
// 最坏情况：字段值 60 + '|' + ISBN 20 + '\0' = 82，留余量取 128
constexpr int IDX_KEY_SIZE   = 128;
constexpr int IDX_VALUE_SIZE = MAX_ISBN + 1;     // 21

// 账户数据文件 accounts.dat
constexpr int ACC_KEY_SIZE   = MAX_USERID + 1;   // 31
constexpr int ACC_VALUE_SIZE = 128;              // 留余量（AccountData 约 68）
