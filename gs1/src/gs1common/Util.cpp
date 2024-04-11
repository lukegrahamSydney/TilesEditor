#include <gs1/common/Util.hpp>

#include <cstdarg>
#include <cstdio>
#include <string.h>

using namespace gs1;

Exception::Exception(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(message, bufferSize, fmt, args);
  va_end(args);
}

const char *Exception::what() const noexcept { return message; }

bool gs1::HasPrefix(const std::string &str, const std::string &prefix)
{
  return memcmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}
