#include <cstdarg>
#include <cstdio>
#include <gs1/common/Log.hpp>

using namespace gs1;

Log::Log() {}

Log::~Log() {}

Log &Log::Get()
{
  static Log singleton;

  return singleton;
}

void Log::SetLogCallback(std::function<void(LogLevel, char *)> callbackFunc)
{
  callback = callbackFunc;
}

void Log::Print(LogLevel level, const char *fmt, ...)
{
  va_list args;
  va_list args2;

  va_start(args, fmt);
  va_copy(args2, args);
  int size = std::vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char* buff = new char[size + 1];

  va_start(args2, fmt);
  std::vsprintf(buff, fmt, args2);
  va_end(args2);

  if (callback)
    callback(level, buff);
  delete buff;
}
