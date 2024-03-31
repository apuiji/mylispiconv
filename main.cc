#include<filesystem>
#include<iostream>
#include"myccutils/xyz.hh"
#include"mylisp/compile.hh"
#include"mylisp/eval.hh"
#include"mylisp/rte.hh"

using namespace std;
using namespace zlt;
using namespace zlt::mylisp;

static const char *indexFile;
static const char **processArgsBegin;
static const char **processArgsEnd;
static size_t mainCoroutineValuekSize = 1 << 21;

static int parseOptions(const char **it, const char **end);

struct ParseOptionBad {
  string what;
  ParseOptionBad(string &&what) noexcept: what(std::move(what)) {}
};

int main(int argc, char **argv, char **envp) {
  try {
    parseOptions(const_cast<const char **>(argv + 1), const_cast<const char **>(argv + argc));
  } catch (ParseOptionBad bad) {
    cerr << bad.what;
    return 0;
  }
  rte::init();
  if (indexFile) {
    ast::UNodes a;
    try {
      ast::Ast {}(a, filesystem::path(indexFile));
    } catch (ast::AstBad bad) {
      cerr << bad.code;
      return 0;
    }
    string s;
    compile(s, a.begin(), a.end());
    {
      unique_ptr<ValueStack> k(new(mainCoroutineValuekSize) ValueStack);
      rte::coroutines.push_back(Coroutine(std::move(k)));
    }
    rte::itCoroutine = rte::coroutines.begin();
    return eval(s.data(), s.data() + s.size());
  } else {
    /// TODO: repl
    return 0;
  }
}

int parseOptions(const char **it, const char **end) {
  if (it == end) [[unlikely]] {
    return 0;
  }
  indexFile = *it;
  processArgsBegin = it + 1;
  processArgsEnd = end;
  return 0;
}
