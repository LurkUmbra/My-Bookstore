#pragma once
#include <fstream>
#include <string>

// Elements per block (block capacity, not including the overflow slot).
constexpr int CAPACITY = 64;

// Fixed-size block, the unit of file I/O. Blocks are numbered from 0:
//   block 0     - header block; its next field stores the head of the data list
//                 (-1 means the list is empty)
//   block 1..N  - data blocks; store sorted (key, value) pairs
//
// Identity: byte offset of block b = b * sizeof(Block)
template <int KEY_SIZE, int VALUE_SIZE>
class BlockList {
    struct Block {
        int  count = 0;
        int  next  = -1;
        char keys  [CAPACITY + 1][KEY_SIZE];
        char values[CAPACITY + 1][VALUE_SIZE];
    };

public:
    explicit BlockList(const std::string& filename);
    ~BlockList();

    // Insert (key, value). Returns false if key already exists.
    // value must point to at least VALUE_SIZE bytes.
    bool insert(const char* key, const char* value);

    // Find key. Copies value into out (at least VALUE_SIZE bytes). Returns true on hit.
    bool find(const char* key, char* out) const;

    // Erase key. Returns true if found and removed.
    bool erase(const char* key);

    // Visit every (key, value) in ascending key order.
    // fn returns false to stop early.
    template <typename Func>
    void traverse(Func fn) const;

    // Visit every (key, value) with lo <= key < hi.
    // fn returns false to stop early.
    template <typename Func>
    void traverseRange(const char* lo, const char* hi, Func fn) const;

private:
    mutable std::fstream file_;   // mutable: read methods move the stream position
    int head_ = -1;

    Block readBlock(int blockId) const;
    void  writeBlock(int blockId, const Block& b);

    // Append a fresh block at the end of the file and return its block id.
    int   newBlock();

    // Update head_ and persist it into the header block.
    void  updateHead(int newHead);
};

#include "block_list_impl.h"
