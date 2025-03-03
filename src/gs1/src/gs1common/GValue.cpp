#include <gs1/common/GValue.hpp>
#include <gs1/common/GVariable.hpp>

#include <cmath>
#include <string>

using namespace gs1;

GValue::GValue() { nanbox = nanbox_empty(); }

GValue::GValue(bool const value) { nanbox = nanbox_from_boolean(value); }

GValue::GValue(double const value) { nanbox = nanbox_from_double(value); }

GValue::GValue(GVariable const &var)
{
  nanbox = nanbox_from_pointer(var.Clone());
}

GValue::GValue(GVariable const *var)
{
  nanbox = nanbox_from_pointer((void *)var);
}

GValue::GValue(GValue const &other)
{
  if (other.GetValueType() == GVALUETYPE_GVARIABLE) {
    // Copy the GVariable
    nanbox = nanbox_from_pointer(other.GetVariable()->Clone());
  } else {
    nanbox = other.nanbox;
  }
}

GValue::~GValue()
{
  if (GetValueType() == GVALUETYPE_GVARIABLE)
    delete ((GVariable *)nanbox_to_pointer(nanbox));
}

GValueType GValue::GetValueType() const
{
  if (nanbox_is_number(nanbox))
    return GVALUETYPE_NUMBER;
  else if (nanbox_is_pointer(nanbox))
    return GVALUETYPE_GVARIABLE;
  else if (nanbox_is_boolean(nanbox))
    return GVALUETYPE_FLAG;
  else
    return GVALUETYPE_EMPTY;
}

double GValue::GetNumber() const
{
  if (nanbox_is_number(nanbox))
    return nanbox_to_double(nanbox);
  else if (nanbox_is_pointer(nanbox)) {
    switch (GetVariable()->GetVarType()) {
    case GVARTYPE_NUMBER:
      return ((GNumberVariable *)GetVariable())->number;

    default:
      return NAN;
    }
  } else
    return NAN;
};

bool GValue::GetFlag() const
{
  if (nanbox_is_boolean(nanbox))
    return nanbox_to_boolean(nanbox);
  else if (nanbox_is_double(nanbox))
    return nanbox_to_double(nanbox) != 0.0f;
  else if (nanbox_is_pointer(nanbox)) {
    switch (GetVariable()->GetVarType()) {
    case GVARTYPE_FLAG:
      return ((GFlagVariable *)GetVariable())->flag;

    case GVARTYPE_NUMBER:
      return ((GNumberVariable *)GetVariable())->number != 0.0f;

    default:
      return false;
    }
  }

  return false;
};

std::string GValue::GetString() const
{
  GVariable *variable = GetVariable();

  if (!variable)
    return nullptr;

  switch (variable->GetVarType()) {
  case GVARTYPE_STRING:
    return ((GStringVariable *)variable)->string;

  default:
    return "";
  };
};

GVariable *GValue::GetVariable() const
{
  if (nanbox_is_pointer(nanbox))
    return (GVariable *)nanbox_to_pointer(nanbox);

  return nullptr;
};

void GValue::swap(GValue &first, GValue &other)
{
  using std::swap;

  std::swap(first.nanbox, other.nanbox);
};

GValue &GValue::operator=(GValue other)
{
  swap(*this, other);

  return (*this);
};