#ifndef GS1COMMON_UTIL_HPP
#define GS1COMMON_UTIL_HPP

#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define bswap_16(value) ((((value)&0xff) << 8) | ((value) >> 8))

#define bswap_32(value)                                                        \
  (((uint32_t)bswap_16((uint16_t)((value)&0xffff)) << 16) |                    \
   (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value)                                                        \
  (((uint64_t)bswap_32((uint32_t)((value)&0xffffffff)) << 32) |                \
   (uint64_t)bswap_32((uint32_t)((value) >> 32)))

namespace gs1
{
using std::deque;
using std::string;
using std::vector;
using std::function;
using std::shared_ptr;
using std::unordered_map;

class Exception : public std::exception
{
public:
  Exception(const char *fmt, ...);

  const char *what() const noexcept;

private:
  static const int bufferSize = 1024;
  char message[bufferSize];
};

struct Pos {
  Pos() : offset(-1) {}
  Pos(size_t offset) : offset(offset) {}
  Pos(size_t offset, int32_t line) : offset((int)offset), line(line) {}

  int offset = 0; 
  int32_t line = 0;
};

struct Range {
  Range() {}
  Range(Pos beg, Pos end) : beg(beg), end(end) {}

  Pos beg;
  Pos end;
};

bool HasPrefix(const std::string &str, const std::string &prefix);
}

#endif
