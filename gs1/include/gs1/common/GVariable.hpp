#ifndef GS1COMMON_GVARIABLE_HPP
#define GS1COMMON_GVARIABLE_HPP

#include <gs1/common/GValue.hpp>

#include <string>
#include <vector>

namespace gs1
{
enum GVarType {
  GVARTYPE_NUMBER,
  GVARTYPE_FLAG,
  GVARTYPE_STRING,
  GVARTYPE_ARRAY
};

struct GVariable {
  GVariable(){};
  virtual ~GVariable(){};

  virtual GVariable *Clone() const = 0;

  virtual GVarType GetVarType() const = 0;
  virtual std::string DebugString() const = 0;

  std::string name;
};

struct GNumberVariable : public GVariable {
  GNumberVariable(){};
  GNumberVariable(float number) : number(number){};

  ~GNumberVariable(){};

  GVariable *Clone() const
  {
    GNumberVariable *copy = new GNumberVariable();

    copy->name = name;
    copy->number = number;

    return copy;
  };

  std::string DebugString() const
  {
    return std::string("GNumberVar: ") + std::to_string(number);
  };

  GVarType GetVarType() const { return GVARTYPE_NUMBER; };

  float number;
};

struct GFlagVariable : public GVariable {
  GFlagVariable(){};
  GFlagVariable(bool flag) : flag(flag){};

  ~GFlagVariable(){};

  GVariable *Clone() const
  {
    GFlagVariable *copy = new GFlagVariable();

    copy->name = name;
    copy->flag = flag;

    return copy;
  };

  GVarType GetVarType() const { return GVARTYPE_FLAG; };

  std::string DebugString() const
  {
    return flag ? "GFlagVariable: true" : "GFlagVariable: false";
  };

  bool flag;
};

struct GStringVariable : public GVariable {
  GStringVariable(){};
  GStringVariable(std::string string) : string(string){};

  ~GStringVariable(){};

  GVariable *Clone() const
  {
    GStringVariable *copy = new GStringVariable();

    copy->name = name;
    copy->string = string;

    return copy;
  };

  GVarType GetVarType() const { return GVARTYPE_STRING; };

  std::string DebugString() const
  {
    return std::string("GStringVar: ") + string;
  };

  std::string string;
};

struct GArrayVariable : public GVariable {
  GArrayVariable(){};
  GArrayVariable(std::vector<GValue> values) : values(values){};

  ~GArrayVariable(){};

  GVariable *Clone() const
  {
    GArrayVariable *copy = new GArrayVariable();

    copy->name = name;
    copy->values = values;

    return copy;
  };

  GVarType GetVarType() const { return GVARTYPE_ARRAY; };

  std::string DebugString() const
  {
    return std::string("GArrayVar: Size: ") + std::to_string(values.size());
  };

  std::vector<GValue> values;

  uint32_t size() { return values.size(); }
};
}

#endif