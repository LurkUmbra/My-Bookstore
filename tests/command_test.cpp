// End-to-end tests for the command layer.
//
// Strategy: for each case, write a script of commands to a temp input file,
// run code.exe with that input, capture stdout, and compare against the
// expected output.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static int passed = 0, failed = 0;

static std::string runCLI(const std::string& input) {
    FILE* f = std::fopen("_cmd_in.tmp", "w");
    std::fwrite(input.c_str(), 1, input.size(), f);
    std::fclose(f);
    std::system("code.exe < _cmd_in.tmp > _cmd_out.tmp 2>&1");
    FILE* g = std::fopen("_cmd_out.tmp", "rb");
    std::string out;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), g)) > 0) out.append(buf, n);
    std::fclose(g);
    std::remove("_cmd_in.tmp");
    std::remove("_cmd_out.tmp");
    return out;
}

static void cleanData() {
    const char* files[] = {
        "accounts.dat", "books.dat", "name.idx", "author.idx",
        "keyword.idx", "finance.log", "operation.log"
    };
    for (const char* f : files) std::remove(f);
}

// Normalize: strip trailing whitespace on each line, drop trailing blank lines.
static std::string normalize(const std::string& s) {
    std::vector<std::string> lines;
    size_t i = 0;
    while (i < s.size()) {
        size_t j = s.find('\n', i);
        if (j == std::string::npos) j = s.size();
        std::string line = s.substr(i, j - i);
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        lines.push_back(line);
        i = j + 1;
    }
    while (!lines.empty() && lines.back().empty()) lines.pop_back();
    std::string out;
    for (size_t k = 0; k < lines.size(); ++k) {
        if (k) out += '\n';
        out += lines[k];
    }
    return out;
}

static void expect(const char* name, const std::string& input, const std::string& expected) {
    cleanData();
    std::string got = normalize(runCLI(input));
    std::string exp = normalize(expected);
    if (got == exp) {
        std::printf("[PASS] %s\n", name);
        passed++;
    } else {
        std::printf("[FAIL] %s\n", name);
        std::printf("  expected:\n%s\n", exp.c_str());
        std::printf("  got:\n%s\n", got.c_str());
        failed++;
    }
}

int main() {
    // ---------- basic ----------
    expect("empty line produces nothing",
           "\n\nquit\n", "");

    expect("unknown command prints Invalid",
           "foobar\nquit\n", "Invalid");

    // ---------- su / logout ----------
    expect("su root with wrong password -> Invalid",
           "su root wrong\nquit\n", "Invalid");

    expect("su root sjtu succeeds silently",
           "su root sjtu\nquit\n", "");

    expect("logout on empty stack -> Invalid",
           "logout\nquit\n", "Invalid");

    expect("su then logout ok",
           "su root sjtu\nlogout\nquit\n", "");

    expect("nested login and logout",
           "su root sjtu\nsu root sjtu\nlogout\nlogout\nlogout\nquit\n",
           "Invalid");   // third logout fails

    // ---------- register ----------
    expect("register then login",
           "register alice pw Alice\nsu alice pw\nquit\n", "");

    expect("duplicate register -> Invalid",
           "register alice pw Alice\nregister alice x X\nquit\n", "Invalid");

    expect("register root -> Invalid",
           "register root x X\nquit\n", "Invalid");

    // ---------- useradd ----------
    expect("guest cannot useradd",
           "useradd bob pw 3 Bob\nquit\n", "Invalid");

    expect("alice(1) cannot useradd",
           "register alice pw Alice\nsu alice pw\nuseradd bob pw 1 Bob\nquit\n", "Invalid");

    expect("root can useradd priv 3",
           "su root sjtu\nuseradd bob pw 3 Bob\nquit\n", "");

    expect("root cannot useradd priv 7",
           "su root sjtu\nuseradd x pw 7 X\nquit\n", "Invalid");

    // ---------- delete ----------
    expect("delete nonexistent -> Invalid",
           "su root sjtu\ndelete nope\nquit\n", "Invalid");

    expect("delete logged-in user fails",
           "su root sjtu\ndelete root\nquit\n", "Invalid");

    expect("delete another user ok",
           "su root sjtu\nregister bob p Bob\ndelete bob\nquit\n", "");

    // ---------- show (empty) ----------
    expect("show with no books prints a blank line",
           "su root sjtu\nshow\nquit\n", "");

    expect("show -name=Nothing prints blank line",
           "su root sjtu\nshow -name=\"Nothing\"\nquit\n", "");

    // ---------- select + modify + show ----------
    expect("select new isbn then modify then show",
           "su root sjtu\nselect B1\nmodify -name=\"Math\" -author=\"Wu\" -keyword=\"sci|algo\" -price=45\nshow\nquit\n",
           "B1\tMath\tWu\tsci|algo\t45.00\t0");

    expect("show -keyword with multiple keywords -> Invalid",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"a|b\" -price=1\nshow -keyword=\"a|b\"\nquit\n",
           "Invalid");

    // ---------- buy ----------
    expect("buy nonexistent -> Invalid",
           "su root sjtu\nbuy NOPE 1\nquit\n", "Invalid");

    expect("buy zero qty -> Invalid",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nbuy B1 0\nquit\n",
           "Invalid");

    expect("buy 3 -> prints cost",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nbuy B1 3\nquit\n",
           "30.00");

    // ---------- import ----------
    expect("import ok adds stock",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nimport 5 100\nshow\nquit\n",
           "B1\tM\tW\tk\t10.00\t5");

    expect("import zero cost -> Invalid",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nimport 5 0\nquit\n",
           "Invalid");

    // ---------- show finance ----------
    expect("show finance with no transactions -> 0 0",
           "su root sjtu\nshow finance\nquit\n", "+ 0.00 - 0.00");

    expect("show finance 0 -> blank line",
           "su root sjtu\nshow finance 0\nquit\n", "");

    expect("show finance 1 with no transactions -> Invalid",
           "su root sjtu\nshow finance 1\nquit\n", "Invalid");

    expect("show finance after buy",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nbuy B1 3\nshow finance\nquit\n",
           "30.00\n+ 30.00 - 0.00");

    // ---------- privilege ----------
    expect("guest cannot show",
           "show\nquit\n", "Invalid");

    expect("guest cannot buy",
           "buy X 1\nquit\n", "Invalid");

    expect("priv 1 cannot select",
           "register a p A\nsu a p\nselect B1\nquit\n", "Invalid");

    expect("guest cannot show finance",
           "show finance\nquit\n", "Invalid");

    // ---------- cross-system: buy records finance ----------
    expect("buy records a finance entry, import records expense",
           "su root sjtu\nselect B1\nmodify -name=\"M\" -author=\"W\" -keyword=\"k\" -price=10\nbuy B1 2\nimport 3 60\nshow finance\nquit\n",
           "20.00\n+ 20.00 - 60.00");


    // ---------- argument validation ----------
    expect("userid with illegal char -> Invalid",
           "register alice! pw Alice\nquit\n", "Invalid");
    expect("userid too long -> Invalid",
           "register aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa pw Alice\nquit\n", "Invalid");
    expect("username with control char -> Invalid",
           "register alice pw Ali\tce\nquit\n", "Invalid");
    expect("isbn too long -> Invalid",
           "su root sjtu\nselect aaaaaaaaaaaaaaaaaaaaaaaaaaaa\nquit\n", "Invalid");
    expect("quantity with letters -> Invalid",
           "su root sjtu\nbuy X abc\nquit\n", "Invalid");
    expect("quantity overflow -> Invalid",
           "su root sjtu\nbuy X 99999999999\nquit\n", "Invalid");
    expect("price with letters -> Invalid",
           "su root sjtu\nselect B1\nmodify -price=12a\nquit\n", "Invalid");
    expect("price with two dots -> Invalid",
           "su root sjtu\nselect B1\nmodify -price=1.2.3\nquit\n", "Invalid");
    expect("keyword duplicate segment -> Invalid",
           "su root sjtu\nselect B1\nmodify -keyword=\"a|a\"\nquit\n", "Invalid");
    expect("show name without quotes -> Invalid",
           "su root sjtu\nshow -name=Math\nquit\n", "Invalid");
    expect("modify duplicate -name -> Invalid",
           "su root sjtu\nselect B1\nmodify -name=\"A\" -name=\"B\"\nquit\n", "Invalid");
    expect("useradd priv 2 -> Invalid",
           "su root sjtu\nuseradd x pw 2 X\nquit\n", "Invalid");

    cleanData();
    std::printf("\n==== Passed: %d, Failed: %d ====\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
