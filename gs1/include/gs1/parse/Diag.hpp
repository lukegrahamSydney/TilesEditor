#ifndef GS1PARSE_DIAG_HPP
#define GS1PARSE_DIAG_HPP

#include <gs1/common/Util.hpp>

namespace gs1
{
struct Diag {
  enum Severity { Info, Warning, Error };
  Pos pos;
  Range range;
  string message;
  Severity severity;
};

typedef function<void(const Diag &)> DiagObserver;

class DiagBuilder
{
public:
  DiagBuilder(DiagObserver observer);

  void Info(Pos p, Range r, const char *fmt, ...);
  void Warn(Pos p, Range r, const char *fmt, ...);
  void Error(Pos p, Range r, const char *fmt, ...);

private:
  void Build(Diag::Severity s, Pos p, Range r, const char *fmt, va_list args);

  DiagObserver observer;
};
}

#endif
