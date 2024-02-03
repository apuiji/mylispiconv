#include<cerrno>
#include<iconv.h>
#include<memory>
#include<stdlib>
#include"mylisp/maps.hh"
#include"mylisp/strings.hh"

using namespace std;
using namespace zlt;
using namespace zlt::mylisp;

static inline Value exp0rt();

extern "c" {
  Value mylispExport() {
    return exp0rt();
  }
}

static NativeFunction open;
static NativeFunction close;
static NativeFunction conv;
static NativeFunction encode;
static NativeFunction decode;

inline Value exp0rt() {
  Map::StrPool sp;
  sp[constring<'o', 'p', 'e', 'n'>] = open;
  sp[constring<'c', 'l', 'o', 's', 'e'>] = close;
  sp[constring<'c', 'o', 'n', 'v'>] = conv;
  sp[constring<'e', 'n', 'c', 'o', 'd', 'e'>] = encode;
  sp[constring<'d', 'e', 'c', 'o', 'd', 'e'>] = decode;
  auto m = neobj<Map>();
  m->strPool = std::move(sp);
  return m;
}

struct IconvObj: Object {
  iconv_t value;
  IconvObj(iconv_t value) noexcept: value(value) {}
  ~IconvObj() noexcept {
    iconv_close(value);
  }
};

template<class To, class From>
struct IconvObj1 final: IconvObj {
  using IconvObj::IconvObj;
};

/// @return 0 is iconv, 1 is strenc, 2 is strdec
static int open1(iconv_t &dest, const Value *it, const Value *end);

Value open(const Value *it, const Value *end) {
  iconv_t ic;
  switch (open1(ic, it, end)) {
    case 0: {
      return neobj<IconvObj1<char, char>>(ic);
    }
    case 1: {
      return neobj<IconvObj1<wchar_t, char>>(ic);
    }
    case 2: {
      return neobj<IconvObj1<char, wchar_t>>(ic);
    }
    default: {
      return Null();
    }
  }
}

static int charsetNames(wstring_view &to, wstring_view &from, const Value *it, const Value *end) noexcept;
static bool charsetName(unique_ptr<char[]> &dest, wstring_view src);

static inline bool open2(iconv_t &dest, const char *to, const char *from) {
  dest = iconv_open(from, to);
  return dest != (iconv_t) -1;
}

int open1(iconv_t &dest, const Value *it, const Value *end) {
  wstring_view to;
  wstring_view form;
  switch (charsetNames(to, from, it, end)) {
    case 0: {
      unique_ptr<char[]> to1;
      unique_ptr<char[]> from1;
      return charsetName(to1, to) && charsetName(from1, from) && open2(dest, to1.get(), from1.get()) ? 0 : -1;
    }
    case 1: {
      unique_ptr<char[]> from1;
      return charsetName(from1, from) && open2(dest, "WCHAR_T", from1.get()) ? 1 : -1;
    }
    case 2: {
      unique_ptr<char[]> to1;
      return charsetName(to1, to) && open2(dest, to1.get(), "WCHAR_T") ? 2 : -1;
    }
    default: {
      return -1;
    }
  }
}

/// @return 0 is given charset name, 1 is not
static int charsetName1(wstring_view &dest, const Value *it, const Value *end) noexcept;

int charsetNames(wstring_view &to, wstring_view &from, const Value *it, const Value *end) noexcept {
  if (it == end) [[unlikely]] {
    return -1;
  }
  int i = charsetName1(to, it, end);
  int j = charsetName1(from, it + 1, end);
  switch (i) {
    case 0: {
      switch (j) {
        case 0: {
          return 0;
        }
        case 1: {
          return 2;
        }
        default: {
          return -1;
        }
      }
    }
    case 1: {
      switch (j) {
        case 0: {
          return 1;
        }
        default: {
          return -1;
        }
      }
    }
    default: {
      return -1;
    }
  }
}

int charsetName1(wstring_view &dest, const Value *it, const Value *end) noexcept {
  if (it == end || !*it) [[unlikely]] {
    return 1;
  }
  if (dynamicast(dest, *it)) {
    return 0;
  }
  return -1;
}

static constexpr size_t charsetNameSize = 1 << 5;

bool charsetName(unique_ptr<char[]> &dest, wstring_view src) {
  dest.reset(new char[charsetNameSize]);
  size_t n = wcstombs(dest.get(), src.data(), charsetNameSize);
  return n != (size_t) -1;
}

Value close(const Value *it, const Value *end) {
  if (IconvObj *a; dynamicast(a, it, end)) {
    iconv_close(a->value);
    a->value = (iconv_t) -1;
  }
  return Null();
}

template<class C>
static bool conv2(basic_string<C> &dest, size_t n, iconv_t ic, const char *src, size_t m);

template<class To, class From>
static inline Value conv1(const Value *it, const Value *end) {
  IconvObj1<To, From> *ic;
  basic_string_view<From> src;
  if (!dynamicasts(make_tuple(&ic, &src), it, end)) [[unlikely]] {
    return Null();
  }
  basic_string<To> s;
  size_t m = src.size() * sizeof(From);
  size_t n = m / sizeof(To);
  if (!conv2(s, n, ic->value, (const char *) src.data(), m)) [[unlikely]] {
    return Null();
  }
  return neobj<BasicStringObj<To>>(std::move(s));
}

Value conv(const Value *it, const Value *end) {
  return conv1<char, char>(it, end);
}

Value encode(const Value *it, const Value *end) {
  return conv1<wchar_t, char>(it, end);
}

Value decode(const Value *it, const Value *end) {
  return conv1<char, wchar_t>(it, end);
}

static inline size_t conv3(char *dest, size_t n, iconv_t ic, const char *src, size_t m) noexcept {
  return iconv(ic, &dest, &n, (char **) &src, &m);
}

template<class C>
bool conv2(basic_string<C> &dest, size_t n, iconv_t ic, const char *src, size_t m) {
  dest.resize(n);
  errno = 0;
  size_t n1 = conv3((char *) dest.data(), n * sizeof(C), ic, src, m);
  switch (errno) {
    [[likely]] case 0: {
      dest.resize(n1 / sizeof(C));
      dest.shrink_to_fit();
      return true;
    }
    case E2BIG: {
      return conv2(dest, n << 1, ic, src, m);
    }
    default: {
      return false;
    }
  }
}
