#include "block_list.h"
#include <filesystem>
#include <cstring>
namespace fs = std::filesystem;

BlockList::BlockList(const std::string& filename) {
    bool needInit = !fs::exists(filename) || fs::file_size(filename) == 0;
    if (needInit) {
        file_.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
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

bool BlockList::find(int key, char* out) {
    int cur = head_;
    while (cur != -1) {
        Block b = readBlock(cur);

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