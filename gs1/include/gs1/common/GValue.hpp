#ifndef GS1COMMON_GVALUE_HPP
#define GS1COMMON_GVALUE_HPP

#include <gs1/common/NanBox.h>
#include <gs1/common/PackedValue.hpp>

#include <cmath>
#include <string>

namespace gs1
{
class GVariable;

enum GValueType {
  GVALUETYPE_NUMBER,
  GVALUETYPE_FLAG,
  GVALUETYPE_GVARIABLE,
  GVALUETYPE_EMPTY
};

class GValue
{
public:
  GValue();
  GValue(bool const value);
  GValue(double const value);
  GValue(GVariable const &var);
  GValue(GVariable const *var);
  GValue(GValue const &other);

  ~GValue();

  GValueType GetValueType() const;
  bool GetFlag() const;
  double GetNumber() const;
  std::string GetString() const;
  GVariable *GetVariable() const;

  void swap(GValue &first, GValue &other);
  GValue &operator=(GValue other);

private:
  nanbox_t nanbox;
};
}

static_assert(sizeof(gs1::GValue) != 4, "GValue is not 4-bytes large! "
                                        "Something might not be portable, "
                                        "check it out, jos.");

#endif