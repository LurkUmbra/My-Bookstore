#include "block_list.h"
#include <filesystem>
#include <cstring>
namespace fs = std::filesystem;

template<int KEY_SIZE, int VALUE_SIZE>
BlockList<KEY_SIZE, VALUE_SIZE>::BlockList(const std::string &filename)
{
    bool needInit = !fs::exists(filename) || fs::file_size(filename) == 0;
    if (needInit)
    {
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        Block head{};
        writeBlock(0, head);
        head_ = head.next;
    }
    else
    {
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        Block h = readBlock(0);
        head_ = h.next;
    }
}

template<int KEY_SIZE, int VALUE_SIZE>
BlockList<KEY_SIZE, VALUE_SIZE>::~BlockList()
{
    file_.close();
}

template<int KEY_SIZE, int VALUE_SIZE>
bool BlockList<KEY_SIZE, VALUE_SIZE>::insert(const char* key, const char *value)
{
    // empty list
    if (head_ == -1)
    {
        int newHead = newBlock();
        updateHead(newHead);
        Block nb{};
        nb.count++;
        std::strcpy(nb.keys[0], key);
        std::strcpy(nb.values[0], value);
        writeBlock(newHead, nb);
        return true;
    }

    // decide which block to insert
    int cur = head_, targetID;
    Block b;
    bool found = false;
    while (cur != -1)
    {
        targetID = cur;
        b = readBlock(cur);
        if (std::strcmp(key, b.keys[b.count - 1]) <= 0)
        {
            found = true;
            break;
        }
        cur = b.next;
    }

    if (!found)
    { // insert to the end of the list
        std::strcpy(b.keys[b.count], key);
        std::strcpy(b.values[b.count], value);
        b.count++;
    }
    else
    { // binary search inside the block to find the pos to insert
        int lo = 0, hi = b.count;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            if (std::strcmp(key, b.keys[mid]) < 0)
            {
                hi = mid;
            }
            else if (std::strcmp(key, b.keys[mid]) > 0)
            {
                lo = mid + 1;
            }
            else
            {
                return false; // duplicate
            }
        }
        // insert to mid
        int pos = lo;
        std::memmove(&b.keys[pos + 1], &b.keys[pos], (b.count - pos) * sizeof(b.keys[0]));
        std::memmove(&b.values[pos + 1], &b.values[pos], (b.count - pos) * sizeof(b.values[0]));
        std::strcpy(b.keys[pos], key);
        std::strcpy(b.values[pos], value);
        b.count++;
    }

    // check division
    if (b.count > CAPACITY)
    {
        int pos = b.count / 2;
        Block newBlock{};
        newBlock.next = b.next;
        b.next = BlockList::newBlock();

        for (int i = pos; i < b.count; i++)
        {
            std::strcpy(newBlock.keys[i - pos], b.keys[i]);
            std::strcpy(newBlock.values[i - pos], b.values[i]);
        }
        newBlock.count = b.count - pos;
        b.count = pos;
        writeBlock(b.next, newBlock);
    }

    writeBlock(targetID, b);
    return true;
}

template<int KEY_SIZE, int VALUE_SIZE>
bool BlockList<KEY_SIZE, VALUE_SIZE>::find(const char* key, char *out)
{
    int cur = head_;
    Block b;

    while (cur != -1)
    {
        b = readBlock(cur);
        if (b.count == 0)
        {
            cur = b.next;
            continue;
        }

        if (std::strcmp(key, b.keys[0]) < 0)
        {
            return false;
        }

        int lo = 0, hi = b.count;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            if (std::strcmp(key, b.keys[mid]) > 0)
                lo = mid + 1;
            else if (std::strcmp(key, b.keys[mid]) < 0)
                hi = mid;
            else
            {
                std::strcpy(out, b.values[mid]);
                return true;
            }
        }

        cur = b.next;
    }
    return false;
}

template<int KEY_SIZE, int VALUE_SIZE>
bool BlockList<KEY_SIZE, VALUE_SIZE>::erase(const char* key)
{
    int cur = head_;
    Block b;
    while (cur != -1)
    {
        b = readBlock(cur);
        if (b.count == 0)
        {
            cur = b.next;
            continue;
        }
        if (std::strcmp(key, b.keys[0]) < 0)
            return false;
        if (std::strcmp(key, b.keys[b.count - 1]) > 0)
        {
            cur = b.next;
            continue;
        }

        int lo = 0, hi = b.count, pos = -1;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            if (std::strcmp(key, b.keys[mid]) > 0)
                lo = mid + 1;
            else if (std::strcmp(key, b.keys[mid]) < 0)
                hi = mid;
            else
            {
                pos = mid;
                break;
            }
        }

        if (pos == -1)
            return false;
        std::memmove(&b.keys[pos], &b.keys[pos + 1], (b.count - pos - 1) * sizeof(b.keys[0]));
        std::memmove(&b.values[pos], &b.values[pos + 1], (b.count - pos - 1) * sizeof(b.values[0]));
        b.count--;
        writeBlock(cur, b);
        return true;
    }
    return false;
}

template<int KEY_SIZE, int VALUE_SIZE>
typename BlockList<KEY_SIZE, VALUE_SIZE>::Block
BlockList<KEY_SIZE, VALUE_SIZE>::readBlock(int blockId)
{
    file_.seekg(sizeof(Block) * blockId, std::ios::beg);
    Block b{};
    file_.read(reinterpret_cast<char *>(&b), sizeof(Block));
    return b;
}

template<int KEY_SIZE, int VALUE_SIZE>
void BlockList<KEY_SIZE, VALUE_SIZE>::writeBlock(int blockId, const Block &b)
{
    file_.seekp(sizeof(Block) * blockId, std::ios::beg);
    file_.write(reinterpret_cast<const char *>(&b), sizeof(Block));
}

template<int KEY_SIZE, int VALUE_SIZE>
int BlockList<KEY_SIZE, VALUE_SIZE>::newBlock()
{
    file_.seekp(0, std::ios::end);
    std::streampos endPos = file_.tellp();
    std::streamoff size = static_cast<std::streamoff>(endPos);
    int newId = size / sizeof(Block);
    return newId;
}

template<int KEY_SIZE, int VALUE_SIZE>
void BlockList<KEY_SIZE, VALUE_SIZE>::updateHead(int newHead)
{
    head_ = newHead;
    Block h = readBlock(0);
    h.next = newHead;
    writeBlock(0, h);
}