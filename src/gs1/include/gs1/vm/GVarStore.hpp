#ifndef GS1VM_GVARSTORE_HPP
#define GS1VM_GVARSTORE_HPP

#include <gs1/common/ConstantTable.hpp>
#include <gs1/common/GVariable.hpp>
#include <gs1/common/PackedValue.hpp>

#include <cmath>
#include <string>
#include <unordered_map>

namespace gs1
{
class GValue;
class ContextLinkedBytecode;

struct TypeBank {
  TypeBank(){};

  ~TypeBank()
  {
    for (auto &var : variables)
      delete var.second;
  };

  std::unordered_map<std::string, GVariable *> variables;
};

class GVarStore
{
  friend class Context;

public:
  GVarStore();
  ~GVarStore();

  bool HasValue(const std::string &name, const GVarType type);
  GVariable *GetVariable(const std::string &name, const GVarType type);
  GValue GetValue(const std::string &name, const GVarType type);
  void SetValue(const std::string &name, const GVarType type,
                const GValue &value);

private:
  std::unordered_map<int, TypeBank *> typeBanks;
};
};

#endif