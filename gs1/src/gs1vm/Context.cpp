#include <gs1/compiler/CompileVisitor.hpp>
#include <gs1/parse/Parser.hpp>
#include <gs1/vm/Context.hpp>
#include <gs1/vm/Device.hpp>

#include <functional>

using namespace gs1;

Context::Context(Device *device, std::shared_ptr<GVarStore> primaryVarStore)
    : device(device), primaryVarStore(primaryVarStore), eventFlags(nullptr),
      stringFormatter(new GStringFormatter()), halted(false)
{
}

Context::~Context() {}

void Context::LinkBytecode(std::shared_ptr<Bytecode> bytecode)
{
  ContextLinkedBytecode cb(bytecode);
  linkedBytecode.push_back(cb);
}

void Context::LinkVarStore(std::shared_ptr<GVarStore> varstore,
                           std::string prefix)
{
  linkedVarstores.push_back(ContextLinkedVarstore(varstore, prefix));
}

void Context::LinkLibrary(std::shared_ptr<GLibrary> library)
{
  linkedLibraries.push_back(ContextLinkedLibrary(library));
}

GValue Context::UnpackValue(const PackedValue &value,
                            const UnpackType unpackType)
{
  switch (value.valueType) {
  case PACKVALUE_CONST_NUMBER:
    return GValue(
        currentBytecode->numberConstants.GetConstant(value.value).val);

  case PACKVALUE_CONST_STRING: {
    GStringVariable *sv = new GStringVariable();

    sv->string = currentBytecode->stringConstants.GetConstant(value.value).val;

    return GValue((GVariable *)sv);
  }

  case PACKVALUE_CONST_ARRAY: {
    GArrayVariable *array = new GArrayVariable();

    uint32_t size =
        currentBytecode->numberConstants.GetConstant(value.value).val;

    array->values.resize(size);

    for (uint32_t i = 0; i < size; ++i)
      array->values[size - i - 1] = stack.Pop();

    return GValue((GVariable *)array);
  }

  case PACKVALUE_NAMED: {
    std::string varName =
        currentBytecode->stringConstants.GetConstant(value.value).val;

    switch (unpackType) {
    case UNPACK_NUMBER:
      return GetVariableValue(varName, GVARTYPE_NUMBER);

    case UNPACK_FLAG:
      return GetVariableValue(varName, GVARTYPE_FLAG);

    case UNPACK_STRING:
      return GetVariableValue(varName, GVARTYPE_STRING);

    case UNPACK_ARRAY:
      return GetVariableValue(varName, GVARTYPE_ARRAY);

    case UNPACK_ANY: {
      GVariable *var;

      if ((var = GetVariable(varName, GVARTYPE_FLAG)))
        return GValue(*var);

      if ((var = GetVariable(varName, GVARTYPE_NUMBER)))
        return GValue(*var);

      if ((var = GetVariable(varName, GVARTYPE_STRING)))
        return GValue(*var);

      if ((var = GetVariable(varName, GVARTYPE_ARRAY)))
        return GValue(*var);

      // Variable wasn't found..
      // Return a temporary named value
      return GetVariableValue(varName, GVARTYPE_NUMBER);
    }
    }
  }

  default:
    return GValue();
  }
}

GValue Context::GetVariableValue(const std::string &name, const GVarType &type)
{
  // Check if this variable's prefix is owned
  for (auto &clv : linkedVarstores) {
    if (HasPrefix(name, clv.GetPrefix())) {
      auto varStore = clv.GetVarstore();

      if (varStore->HasValue(name, type))
        return varStore->GetValue(name, type);
    }
  }

  // Otherwise, use the unprefixed, primary varstore
  if (primaryVarStore->HasValue(name, type))
    return primaryVarStore->GetValue(name, type);

  // Return a GValue with a name
  // TODO: Maybe separate this into a GetNamedValue?
  GVariable *var;

  switch (type) {
  case GVARTYPE_NUMBER:
    var = new GNumberVariable();
    var->name = name;

    return GValue(var);

  case GVARTYPE_FLAG:
    var = new GFlagVariable();
    var->name = name;

    return GValue(var);

  case GVARTYPE_STRING:
    var = new GStringVariable();
    var->name = name;

    return GValue(var);

  case GVARTYPE_ARRAY:
    var = new GArrayVariable();
    var->name = name;

    return GValue(var);
  }
}

GVariable *Context::GetVariable(const std::string &name, const GVarType &type)
{
  // Event flags are read only and have top priority
  if (eventFlags != nullptr) {
    if (eventFlags->HasValue(name, type))
      return eventFlags->GetVariable(name, type);
  }

  // Check if this variable's prefix is owned
  for (auto &clv : linkedVarstores) {
    if (HasPrefix(name, clv.GetPrefix())) {
      auto varStore = clv.GetVarstore();

      if (varStore->HasValue(name, type))
        return varStore->GetVariable(name, type);
    }
  }

  // Otherwise, use the unprefixed, primary varstore
  if (primaryVarStore->HasValue(name, type))
    return primaryVarStore->GetVariable(name, type);

  return nullptr;
}

void Context::SetVariable(const std::string &name, const GVarType &type,
                          const GValue &value)
{
  // Check if this variable's prefix is owned
  for (auto &clv : linkedVarstores) {
    if (HasPrefix(name, clv.GetPrefix())) {
      auto varStore = clv.GetVarstore();

      varStore->SetValue(name, type, value);

      return;
    }
  }

  // Otherwise, use the unprefixed, primary varstore
  primaryVarStore->SetValue(name, type, value);
}

void Context::CallCommand(const std::string &name)
{
  // Call command by library
  std::function<void(Context * context)> *func;

  for (auto &lib : linkedLibraries) {
    if ((func = lib.GetLibrary()->GetCommand(name))) {
      (*func)(this);

      break;
    }
  }
}

void Context::CallFunction(const std::string &name)
{
  // Call function by library
  std::function<void(Context * context)> *func;

  for (auto &lib : linkedLibraries) {
    if ((func = lib.GetLibrary()->GetFunction(name))) {
      (*func)(this);

      break;
    }
  }
}

void Context::BranchAndLink(const uint32_t &offset)
{
  jumpStack.Push(currentBytecode->body - instructionPointer);

  instructionPointer = currentBytecode->body + offset;
}

void Context::Return()
{
  if (jumpStack.Size() > 0) {
    instructionPointer = currentBytecode->body + jumpStack.Pop();
  } else {
    Halt();
  }
}

// TODO:
// Clean this up..
// This should be part of AST generation, perhaps
std::string Context::InterpolateString(std::string inputString)
{
  std::string outputString;

  // Find format specifiers
  const char *c = inputString.c_str();

  while (*c != '\0') {
    if (*c == '#') {
      // Check if needs formatting
      int len;
      if ((len = stringFormatter->NeedsFormatting(++c))) {
        std::string specifier = std::string(c, len);
        c += specifier.length();

        // Get the parameters
        // Is as follows:
        // Look for uninterrupted whitespace followed by (
        // If there's a (, go until matching )
        // If no (, there are no parameters
        std::string param;
        const char *p = c;

        // Skip whitespace until '('
        while (*p == ' ' && *p != '(') {
          ++p;
          continue;
        }

        // Check if there's a parameter
        if (*p == '(') {
          int depth = 1;

          // Skip the leading bracket
          p++;

          while (true) {
            if (*p == '(')
              depth++;

            if (*p == ')')
              depth--;

            if (depth <= 0)
              break;

            param += *p++;
          }
        }

        c = p + 1;

        outputString += stringFormatter->Format(this, specifier, param);
      }
    }

    outputString += *c++;
  }

  return outputString;
}

void Context::Eval(const std::string &code, const Stack &stack) {}

void Context::Run(GVarStore *eventFlags)
{
  // Set the current event flags
  this->eventFlags = eventFlags;

  // Run each linked bytecode
  for (auto &clb : linkedBytecode) {
    currentBytecode = clb.GetBytecode();

    const char *startPos = currentBytecode->GetBody();
    unsigned int len = currentBytecode->GetBodyLen();

    instructionPointer = startPos;

    while (instructionPointer < startPos + len) {
      const Opcode op = static_cast<const Opcode>(*instructionPointer);
      instructionPointer++;

      operationDispatcher.Dispatch(this, *currentBytecode, op);

      if (halted) {
        halted = false;
        break;
      }
    }
  }
}

void Context::Halt() { halted = true; }
