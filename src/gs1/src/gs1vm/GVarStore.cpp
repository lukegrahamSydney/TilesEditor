#include <gs1/common/GValue.hpp>
#include <gs1/vm/Context.hpp>
#include <gs1/vm/GVarStore.hpp>

using namespace gs1;

GVarStore::GVarStore()
{
  // Create each type bank
  typeBanks[GVARTYPE_NUMBER] = new TypeBank();
  typeBanks[GVARTYPE_STRING] = new TypeBank();
  typeBanks[GVARTYPE_FLAG] = new TypeBank();
  typeBanks[GVARTYPE_ARRAY] = new TypeBank();
}

GVarStore::~GVarStore()
{
  for (auto &typeBank : typeBanks)
    delete typeBank.second;
}

bool GVarStore::HasValue(const std::string &name, GVarType type)
{
  TypeBank *bank = typeBanks[type];

  return bank->variables.find(name) != bank->variables.end();
}

GVariable *GVarStore::GetVariable(const std::string &name, GVarType type)
{
  TypeBank *bank = typeBanks[type];

  auto var = bank->variables.find(name);
  if (var != bank->variables.end())
    return var->second;

  return nullptr;
}

GValue GVarStore::GetValue(const std::string &name, GVarType type)
{
  auto var = GetVariable(name, type);

  if (var != nullptr)
    return GValue(var);
  else
    return GValue();
}

void GVarStore::SetValue(const std::string &name, const GVarType type,
                         const GValue &value)
{
  TypeBank *bank = typeBanks[type];
  GVariable *var = bank->variables[name];

  if (var != nullptr)
    delete var;

  switch (value.GetValueType()) {
  case GVALUETYPE_NUMBER: {
    GVariable *numberVar = new GNumberVariable(value.GetNumber());
    numberVar->name = name;

    bank->variables[name] = numberVar;
    break;
  }

  case GVALUETYPE_FLAG: {
    GVariable *flagVar = new GFlagVariable(value.GetFlag());
    flagVar->name = name;

    bank->variables[name] = flagVar;
    break;
  }

  case GVALUETYPE_GVARIABLE: {
    GVariable *copy = value.GetVariable()->Clone();
    copy->name = name;

    bank->variables[name] = copy;
    break;
  }

  default:
    break;
  }
}