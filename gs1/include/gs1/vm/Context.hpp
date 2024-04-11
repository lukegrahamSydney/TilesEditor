#ifndef GS1VM_CONTEXT_HPP
#define GS1VM_CONTEXT_HPP

#include <gs1/common/ConstantTable.hpp>
#include <gs1/vm/Bytecode.hpp>
#include <gs1/vm/GLibrary.hpp>
#include <gs1/vm/GStringFormatter.hpp>
#include <gs1/vm/GVarStore.hpp>
#include <gs1/vm/JumpStack.hpp>
#include <gs1/vm/OperationDispatcher.hpp>
#include <gs1/vm/Stack.hpp>

#include <memory>
#include <vector>

namespace gs1
{
class Device;

enum UnpackType {
  UNPACK_NUMBER,
  UNPACK_FLAG,
  UNPACK_STRING,
  UNPACK_ARRAY,
  UNPACK_ANY
};

class ContextLinkedBytecode
{
  friend class Context;

public:
  virtual ~ContextLinkedBytecode(){};

  std::shared_ptr<Bytecode> GetBytecode() { return bytecode; };

private:
  ContextLinkedBytecode(std::shared_ptr<Bytecode> bytecode)
  {
    this->bytecode = bytecode;
  }

  std::shared_ptr<Bytecode> bytecode;
};

class ContextLinkedVarstore
{
  friend class Context;

public:
  virtual ~ContextLinkedVarstore(){};

  std::string GetPrefix() { return prefix; };
  std::shared_ptr<GVarStore> GetVarstore() { return varstore; };

private:
  ContextLinkedVarstore(std::shared_ptr<GVarStore> varstore, std::string prefix)
  {
    this->varstore = varstore;
    this->prefix = prefix;
  };

  std::string prefix;
  std::shared_ptr<GVarStore> varstore;
};

class ContextLinkedLibrary
{
  friend class Context;

public:
  virtual ~ContextLinkedLibrary(){};

  std::shared_ptr<GLibrary> GetLibrary() { return library; };

private:
  ContextLinkedLibrary(std::shared_ptr<GLibrary> library)
  {
    this->library = library;
  };

  std::shared_ptr<GLibrary> library;
};

/**
 * A context holds bytecode and variable store.
 */
class Context
{
  friend class GLibrary;
  friend class OperationDispatcher;
  friend class Device;

public:
  Context(Device *device, std::shared_ptr<GVarStore> primaryVarStore);
  virtual ~Context();

  void LinkBytecode(std::shared_ptr<Bytecode> bytecode);
  void LinkVarStore(std::shared_ptr<GVarStore> varstore, std::string prefix);
  void LinkLibrary(std::shared_ptr<GLibrary> library);

  GValue UnpackValue(const PackedValue &value,
                     const UnpackType type = UNPACK_ANY);

  GValue GetVariableValue(const std::string &name, const GVarType &type);
  GVariable *GetVariable(const std::string &name, const GVarType &type);
  void SetVariable(const std::string &name, const GVarType &type,
                   const GValue &value);

  std::string InterpolateString(std::string string);

  void CallCommand(const std::string &name);
  void CallFunction(const std::string &name);

  void BranchAndLink(const uint32_t &offset);
  void Return();

  void Eval(const std::string &code, const Stack &stack);
  void Run(GVarStore *eventFlags = nullptr);

  void Halt();

  // Stack used for operations
  Stack stack;

private:
  // The bytecode currently being ran
  std::shared_ptr<Bytecode> currentBytecode;

  // Stack used for branching and linking
  JumpStack jumpStack;

  const char *instructionPointer;

  Device *device;

  bool halted;

  std::shared_ptr<GVarStore> primaryVarStore;

  OperationDispatcher operationDispatcher;

  std::vector<ContextLinkedBytecode> linkedBytecode;
  std::vector<ContextLinkedVarstore> linkedVarstores;
  std::vector<ContextLinkedLibrary> linkedLibraries;

  GStringFormatter *stringFormatter;

  GVarStore *eventFlags;
};
}

#endif