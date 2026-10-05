#include "block_list.h"
#include <filesystem>
#include <cstring>
namespace fs = std::filesystem;

BlockList::BlockList(const std::string& filename) {
    bool needInit = !fs::exists(filename) || fs::file_size(filename) == 0;
    if (needInit) {
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        Block head{};
        writeBlock(0, head);
        head_ = head.next;
    } else {
        file_.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        Block h = readBlock(0);
        head_ = h.next;
    }
}

BlockList::~BlockList() {
    file_.close();
}

bool BlockList::insert(int key, const char* value) {
    // empty list
    if (head_ == -1) {
        int newHead = newBlock();
        updateHead(newHead);
        Block nb{};
        nb.count++;
        nb.keys[0] = key;
        std::strcpy(nb.values[0], value);
        writeBlock(newHead, nb);
        return true;
    }
    
    // decide which block to insert
    int cur = head_, targetID;
    Block b;
    bool found = false;
    while (cur != -1) {
        targetID = cur;
        b = readBlock(cur);
        if (key <= b.keys[b.count - 1]) {
            found = true;
            break;
        }
        cur = b.next;
    }

    if (!found) { // insert to the end of the list
        b.keys[b.count] = key;
        std::strcpy(b.values[b.count], value);
        b.count++;
    } else { // binary search inside the block to find the pos to insert 
        int lo = 0, hi = b.count;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (key < b.keys[mid]) {
                hi = mid;
            } else if (key > b.keys[mid]) {
                lo = mid + 1;
            } else {
                return false; // duplicate
            }
        }
        // insert to mid
        int pos = lo;
        std::memmove(&b.keys[pos + 1], &b.keys[pos], (b.count - pos) * sizeof(b.keys[0]));
        std::memmove(&b.values[pos + 1], &b.values[pos], (b.count - pos) * sizeof(b.values[0]));
        b.keys[pos] = key;
        std::strcpy(b.values[pos], value);
        b.count++;
    }

    // check division
    if (b.count > CAPACITY) {
        int pos = b.count / 2;
        Block newBlock{};
        newBlock.next = b.next;
        b.next = BlockList::newBlock();

        for (int i = pos; i < b.count; i++) {
            newBlock.keys[i - pos] = b.keys[i];
            std::strcpy(newBlock.values[i - pos], b.values[i]);
        }
        newBlock.count = b.count - pos; 
        b.count = pos;
        writeBlock(b.next, newBlock);
    }

    writeBlock(targetID, b);
    return true;
}

bool BlockList::find(int key, char* out) {
    int cur = head_;
    Block b;

    while (cur != -1) {
        b = readBlock(cur);
        if (b.count == 0) { cur = b.next; continue; }

        if (key < b.keys[0]) {
            return false;
        }

        int lo = 0, hi = b.count;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (b.keys[mid] < key) lo = mid + 1;
            else if (b.keys[mid] > key) hi = mid;
            else {
                std::strcpy(out, b.values[mid]);
                return true;
            }
        }

        cur = b.next;
    }
    return false;
}

bool BlockList::erase(int key) {
    return false; // TODO
}

Block BlockList::readBlock(int blockId) {
    file_.seekg(sizeof(Block) * blockId, std::ios::beg);
    Block b{};
    file_.read(reinterpret_cast<char*>(&b), sizeof(Block));
    return b;
}

void BlockList::writeBlock(int blockId, const Block& b) {
    file_.seekp(sizeof(Block) * blockId, std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&b), sizeof(Block));
}

int BlockList::newBlock() {
    file_.seekp(0, std::ios::end);
    std::streampos endPos = file_.tellp();
    std::streamoff size = static_cast<std::streamoff>(endPos);
    int newId = size / sizeof(Block);
    return newId;
}

void BlockList::updateHead(int newHead) {
    head_ = newHead;
    Block h = readBlock(0);
    h.next = newHead;
    writeBlock(0, h);
}