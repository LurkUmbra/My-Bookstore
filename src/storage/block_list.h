#pragma once
#include <fstream>
#include <string>

// ================== 设计常量（全项目统一，改动要同步改文档）==================
constexpr int CAPACITY   = 64;   // 每个数据块的元素容量（B）
constexpr int VALUE_SIZE = 64;   // 每条 value 的字节数（定长）

// ================== 块结构（文件的最小读写单位，定长）==================
// 文件被划分为等长的块，块号从 0 开始：
//   0 号块 = 头块，其 next 字段存放"数据链表第一个数据块的块号"，-1 表示链表为空
//   1 号及以后 = 数据块，存储有序的 (key, value) 对，next 指向下一数据块
//
// 关键恒等式：第 b 号块的起始字节偏移 = b * sizeof(Block)
struct Block {
    int  count = 0;                        // 本块有效元素数（头块忽略此字段）
    int  next  = -1;                       // 下一块块号；-1 表示无
    int  keys[CAPACITY];                   // 按 key 升序排列
    char values[CAPACITY][VALUE_SIZE];     // 与 keys 一一对应；'\0' 结尾
};

class BlockList {
public:
    // 打开/创建文件。文件不存在（或为空）则初始化：写一个空头块（next = -1）
    explicit BlockList(const std::string& filename);
    ~BlockList();

    // 插入 (key, value)。key 已存在则返回 false（不覆盖）。
    bool insert(int key, const char* value);

    // 查找 key。找到则把 value 拷到 out（调用者保证 out 至少有 VALUE_SIZE 字节），返回 true。
    bool find(int key, char* out);

    // 删除 key。成功返回 true；不存在返回 false。
    bool erase(int key);

    // TODO(P1 第二阶段): traverse 按 key 升序遍历，供 show 使用。

private:
    std::fstream file_;
    int head_ = -1;    // 头块里 head 的内存缓存，避免每次操作都读 0 号块

    // ===== 内部工具（由你实现）=====
    // 读/写第 blockId 块。内部用 seekg/seekp + read/write，不要漏掉 reinterpret_cast。
    Block readBlock(int blockId);
    void  writeBlock(int blockId, const Block& b);

    // 在文件尾追加一个新块，返回其块号。
    // 提示：先 file_.seekp(0, std::ios::end) 得到文件大小，再据此算新块号。
    int   newBlock();

    // 更新 head_ 并写回 0 号块。
    void  updateHead(int newHead);
};
