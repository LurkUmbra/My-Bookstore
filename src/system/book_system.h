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
    bool getByISBN(const char* isbn, BookData& out) const;

    // Ensure a book with the given ISBN exists. If not, create an empty one
    // (only ISBN set; other fields zero). Used by "select".
    bool ensureBook(const char* isbn);

    // Each showBy* invokes fn(isbn, book) for every match, in ISBN order.
    // fn returns false to stop early.
    template <typename Func>
    void showAll(Func fn) const;

    template <typename Func>
    void showByName(const char* name, Func fn) const;

    template <typename Func>
    void showByAuthor(const char* author, Func fn) const;

    template <typename Func>
    void showByKeyword(const char* keyword, Func fn) const;

    // Look up one book by ISBN and invoke fn exactly once on hit.
    template <typename Func>
    void showByISBN(const char* isbn, Func fn) const;

    // Fields to update in modifyBook. A null pointer means "leave unchanged".
    struct ModifyFields {
        const char*   name    = nullptr;
        const char*   author  = nullptr;
        const char*   keyword = nullptr;
        const double* price   = nullptr;   // pointer so 0 is distinguishable
    };
    bool modifyBook(const char* isbn, const ModifyFields& fields);

    // Reduce stock by quantity; totalCost = price * quantity.
    bool buyBook(const char* isbn, int quantity, double& totalCost);

    // Increase stock by quantity.
    bool importBook(const char* isbn, int quantity, double totalCost);

private:
    BlockList<BOOK_KEY_SIZE, BOOK_VALUE_SIZE> books_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> nameIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> authorIdx_;
    BlockList<IDX_KEY_SIZE,   IDX_VALUE_SIZE> keywordIdx_;

    // Build an index key of the form "field|isbn".
    static void buildIdxKey(char* out, int outSize, const char* field, const char* isbn);
};

#include "book_system_impl.h"
