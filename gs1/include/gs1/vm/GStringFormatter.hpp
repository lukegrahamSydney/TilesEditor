#ifndef GS1VM_GSTRING_FORMATTER_HPP
#define GS1VM_GSTRING_FORMATTER_HPP

#include <string>

namespace gs1
{
class Context;

class GStringFormatter
{
public:
  GStringFormatter(){};
  virtual ~GStringFormatter(){};

  // Returns the size of the format specifier
  virtual int NeedsFormatting(const char *str);
  virtual std::string Format(Context *context, const std::string &type,
                             const std::string &param);
};
};

#endif