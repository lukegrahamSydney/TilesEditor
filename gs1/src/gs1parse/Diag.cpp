#include <gs1/parse/Diag.hpp>

#include <cstdarg>

using namespace gs1;

DiagBuilder::DiagBuilder(DiagObserver observer) : observer(observer) {}

void DiagBuilder::Info(Pos p, Range r, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Build(Diag::Info, p, r, fmt, args);
  va_end(args);
}

void DiagBuilder::Warn(Pos p, Range r, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Build(Diag::Warning, p, r, fmt, args);
  va_end(args);
}

void DiagBuilder::Error(Pos p, Range r, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Build(Diag::Error, p, r, fmt, args);
  va_end(args);
}

void DiagBuilder::Build(Diag::Severity s, Pos p, Range r, const char *fmt,
                        va_list args)
{
  static char buffer[1024];
  vsnprintf(buffer, 1024, fmt, args);

  Diag d;
  d.pos = p;
  d.range = r;
  d.message = buffer;
  d.severity = s;
  observer(d);
}
