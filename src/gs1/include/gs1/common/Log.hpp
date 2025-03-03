#ifndef GS1COMMON_LOG_HPP
#define GS1COMMON_LOG_HPP

#include <functional>

namespace gs1
{
enum LogLevel {
  LOGLEVEL_ASSERT,
  LOGLEVEL_DEBUG,
  LOGLEVEL_WARNING,
  LOGLEVEL_ERROR,
  LOGLEVEL_INFO,
  LOGLEVEL_VERBOSE
};

class Log
{
public:
  Log();
  ~Log();

  static Log &Get();

  void SetLogCallback(std::function<void(LogLevel, char *message)> callback);
  void Print(LogLevel logLevel, const char *fmt, ...);

private:
  std::function<void(LogLevel, char *message)> callback;
};
}

#endif