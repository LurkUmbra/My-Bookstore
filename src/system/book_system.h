#pragma once
#include "../common/constants.h"
#include "../storage/block_list.h"
#include <cstring>
#include <string>

// Persistent record for a book (POD, memcpy-able into BlockList value).
//
// WARNING: this is a private file format. Reordering or resizing fields
// breaks compatibility with existing books.dat files.
//
// char[] instead of std::string: POD can be written as raw bytes, while
// std::string holds internal pointers that must not be persisted.
struct BookData {
    char   name[MAX_NAME + 1];
    char   author[MAX_AUTHOR + 1];
    char   keyword[MAX_KEYWORD + 1];   // multiple keywords joined by '|'
    double price;
    int    stock;
};

static_assert(sizeof(BookData) <= BOOK_VALUE_SIZE,
              "BookData exceeds BOOK_VALUE_SIZE, enlarge it in constants.h");

// Book system: primary data plus three secondary indexes.
class BookSystem {
public:
    BookSystem();

    // Serialization helpers.
    static void pack(const BookData& d, char* out);
    static void unpack(const char* buf, BookData& d);

    // Create a new book with full information.
    bool createBook(const char* isbn, const char* name, const char* author,
                    const char* keyword, double price);

    // Fetch a book by ISBN.
    bool getByISBN(const char* isbn, BookData& out);

    template <typename Func>
    void showByName(const char* name, Func fn);

    template <typename Func>
    void showByAuthor(const char* author, Func fn);

    template <typename Func>
    void showByKeyword(const char* keyword, Func fn);

    // Fields to update in modifyBook. A null pointer means "leave unchanged".
    struct ModifyFields {
        const char*   name    = nullptr;
        const char*   author  = nullptr;
        const char*   keyword = nullptr;
        const double* price   = nullptr;   // pointer so 0 is distinguishable
    };
    bool modifyBook(const char* isbn, const ModifyFields& fields);

private:
    BlockList<BOOK_KEY_SIZE, BOOK_VALUE_SIZE> books_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> nameIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> authorIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> keywordIdx_;

    // Build an index key of the form "field|isbn".
    static void buildIdxKey(char* out, int outSize, const char* field, const char* isbn);
};

#include "book_system_impl.h"
