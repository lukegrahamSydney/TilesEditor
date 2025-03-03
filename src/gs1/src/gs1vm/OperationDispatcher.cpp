#include <gs1/common/Log.hpp>
#include <gs1/vm/Context.hpp>

using namespace gs1;

static bool g_initialized = false;
static std::function<void(Context *context)> operationHandlers[OP_NUM_OPS];

static inline int32_t readOffset(const char *data)
{
  char value[4] = {data[0], data[1], data[2], data[3]};

  return *((int32_t *)&value);
}

OperationDispatcher::OperationDispatcher()
{
  if (g_initialized)
    return;

  g_initialized = true;

  operationHandlers[OP_PUSH] = [&](Context *context) {
    PackedValue &pLValue = *(PackedValue *)context->instructionPointer;
    context->instructionPointer += sizeof(PackedValue);

    GValue lValue = context->UnpackValue(pLValue);
    context->stack.Push(lValue);

    if (lValue.GetValueType() == GVALUETYPE_NUMBER)
      Log::Get().Print(LOGLEVEL_VERBOSE, "push %f\n", lValue.GetNumber());
    else if (lValue.GetValueType() == GVALUETYPE_FLAG)
      Log::Get().Print(LOGLEVEL_VERBOSE, "push %s\n",
                       lValue.GetFlag() ? "true" : " false");
    else if (lValue.GetValueType() == GVALUETYPE_GVARIABLE)
      Log::Get().Print(LOGLEVEL_VERBOSE, "push %s : %s\n",
                       lValue.GetVariable()->name.c_str(),
                       lValue.GetVariable()->DebugString().c_str());
  };

  operationHandlers[OP_ASSIGN] = [&](Context *context) {
    GValue rValue = context->stack.Pop();
    std::string varName = context->stack.Pop().GetVariable()->name;

    switch (rValue.GetValueType()) {
    case GVALUETYPE_NUMBER:
      context->SetVariable(varName, GVARTYPE_NUMBER, rValue);

      Log::Get().Print(LOGLEVEL_VERBOSE, "%s = Number: %f\n", varName.c_str(),
                       rValue.GetNumber());
      break;

    case GVALUETYPE_FLAG:
      context->SetVariable(varName, GVARTYPE_FLAG, rValue);

      Log::Get().Print(LOGLEVEL_VERBOSE, "%s = Bool: %s\n", varName.c_str(),
                       rValue.GetFlag() ? "true" : "false");
      break;

    case GVALUETYPE_GVARIABLE:
      switch (rValue.GetVariable()->GetVarType()) {
      case GVARTYPE_ARRAY:
        context->SetVariable(varName, GVARTYPE_ARRAY, rValue);

        Log::Get().Print(
            LOGLEVEL_VERBOSE, "%s = Array: size %u\n", varName.c_str(),
            (uint32_t)((GArrayVariable *)rValue.GetVariable())->values.size());
        break;

      case GVARTYPE_NUMBER:
        context->SetVariable(varName, GVARTYPE_NUMBER, rValue);

        Log::Get().Print(LOGLEVEL_VERBOSE, "%s = Number: %f\n", varName.c_str(),
                         rValue.GetNumber());
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }
  };

  operationHandlers[OP_ARR_SET] = [&](Context *context) {
    GValue rValue = context->stack.Pop();
    GValue index = context->stack.Pop();
    std::string arrName = context->stack.Pop().GetVariable()->name;

    GArrayVariable *array =
        (GArrayVariable *)context->GetVariable(arrName, GVARTYPE_ARRAY);
    array->values[(uint32_t)index.GetNumber()] = rValue.GetNumber();

    Log::Get().Print(LOGLEVEL_VERBOSE, "Array set: %s[%u] = %f\n",
                     arrName.c_str(), (uint32_t)index.GetNumber(),
                     rValue.GetNumber());
  };

  operationHandlers[OP_ARR_GET] = [&](Context *context) {
    GValue index = context->stack.Pop();
    std::string arrName = context->stack.Pop().GetVariable()->name;

    GArrayVariable *array =
        (GArrayVariable *)context->GetVariable(arrName, GVARTYPE_ARRAY);
    GValue value = array->values[(uint32_t)index.GetNumber()];

    Log::Get().Print(LOGLEVEL_VERBOSE, "Array lookup: %s[%u], Push %f\n",
                     arrName.c_str(), (uint32_t)index.GetNumber(),
                     value.GetNumber());

    context->stack.Push(value);
  };

  operationHandlers[OP_ADD] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Add the two values and push the result
    context->stack.Push(GValue(lValue + rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f + %f = %f\n", lValue, rValue,
                     lValue + rValue);
  };

  operationHandlers[OP_SUB] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Subtract the two values and push the result
    context->stack.Push(GValue(lValue - rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f - %f = %f\n", lValue, rValue,
                     lValue - rValue);
  };

  operationHandlers[OP_MUL] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(lValue * rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f * %f = %f\n", lValue, rValue,
                     lValue * rValue);
  };

  operationHandlers[OP_DIV] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(lValue / rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f / %f = %f\n", lValue, rValue,
                     lValue / rValue);
  };

  operationHandlers[OP_MOD] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(fmod(lValue, rValue)));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f %% %f = %f\n", lValue, rValue,
                     fmod(lValue, rValue));
  };

  operationHandlers[OP_POW] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(powf(lValue, rValue)));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f ^ %f = %f\n", lValue, rValue,
                     powf(lValue, rValue));
  };

  operationHandlers[OP_INC] = [&](Context *context) {
    GValue value = context->stack.Pop();

    ((GNumberVariable *)value.GetVariable())->number += 1.0f;

    // Increment the value on the varstore
    context->SetVariable(value.GetVariable()->name, GVARTYPE_NUMBER, value);

    Log::Get().Print(LOGLEVEL_VERBOSE, "%s++ = %f\n",
                     value.GetVariable()->name.c_str(), value.GetNumber());
  };

  operationHandlers[OP_INCPUSH] = [&](Context *context) {
    GValue value = context->stack.Pop();

    // Push the value back onto the stack
    context->stack.Push(GValue(value.GetNumber()));

    ((GNumberVariable *)value.GetVariable())->number += 1.0f;

    // Increment the value on the varstore
    context->SetVariable(value.GetVariable()->name, GVARTYPE_NUMBER, value);

    Log::Get().Print(LOGLEVEL_VERBOSE, "%s++ = %f PUSH\n",
                     value.GetVariable()->name.c_str(), value.GetNumber());
  };

  operationHandlers[OP_DEC] = [&](Context *context) {
    GValue value = context->stack.Pop();

    ((GNumberVariable *)value.GetVariable())->number -= 1.0f;

    // Increment the value on the varstore
    context->SetVariable(value.GetVariable()->name.c_str(), GVARTYPE_NUMBER,
                         value);

    Log::Get().Print(LOGLEVEL_VERBOSE, "%s-- = %f\n",
                     value.GetVariable()->name.c_str(), value.GetNumber());
  };

  operationHandlers[OP_DECPUSH] = [&](Context *context) {
    GValue value = context->stack.Pop();

    // Push the value back onto the stack
    context->stack.Push(GValue(value.GetNumber()));

    ((GNumberVariable *)value.GetVariable())->number -= 1.0f;

    // Increment the value on the varstore
    context->SetVariable(value.GetVariable()->name.c_str(), GVARTYPE_NUMBER,
                         value);

    Log::Get().Print(LOGLEVEL_VERBOSE, "%s-- = %f PUSH\n",
                     value.GetVariable()->name.c_str(), value.GetNumber());
  };

  operationHandlers[OP_CALL] = [&](Context *context) {
    PackedValue &packedCommandName =
        *(PackedValue *)context->instructionPointer;
    context->instructionPointer += sizeof(PackedValue);

    std::string funcName =
        ((GStringVariable *)context->UnpackValue(packedCommandName)
             .GetVariable())
            ->string;

    Log::Get().Print(LOGLEVEL_VERBOSE, "FUNC_CALL: %s\n", funcName.c_str());

    context->CallFunction(funcName);
  };

  operationHandlers[OP_CMD_CALL] = [&](Context *context) {
    PackedValue &packedCommandName =
        *(PackedValue *)context->instructionPointer;
    context->instructionPointer += sizeof(PackedValue);

    std::string commandName =
        ((GStringVariable *)context->UnpackValue(packedCommandName)
             .GetVariable())
            ->string;

    Log::Get().Print(LOGLEVEL_VERBOSE, "CMD_CALL: %s\n", commandName.c_str());

    context->CallCommand(commandName);
  };

  operationHandlers[OP_JMP] = [&](Context *context) {
    // Get byte offset
    int32_t offset = readOffset(context->instructionPointer);

    // Jump to offset
    context->instructionPointer += offset;

    Log::Get().Print(LOGLEVEL_VERBOSE, "JMP: Jumping by %d\n", offset);
  };

  operationHandlers[OP_JAL] = [&](Context *context) {
    // Get byte offset
    int32_t offset = readOffset(context->instructionPointer);

    // Jump to offset and link
    context->BranchAndLink((context->instructionPointer + offset) -
                           context->currentBytecode->GetBody());

    Log::Get().Print(LOGLEVEL_VERBOSE, "JAL: Jump + linking by %d\n", offset);
  };

  operationHandlers[OP_RET] = [&](Context *context) {
    // Return
    context->Return();

    Log::Get().Print(LOGLEVEL_VERBOSE, "Return\n");
  };

  operationHandlers[OP_EQ] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(lValue == rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f == %f = %d\n", lValue, rValue,
                     lValue == rValue);
  };

  operationHandlers[OP_LT] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Compare the two values and push the result
    context->stack.Push(GValue(lValue < rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f < %f = %d\n", lValue, rValue,
                     lValue < rValue);
  };

  operationHandlers[OP_GT] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Compare the two values and push the result
    context->stack.Push(GValue(lValue > rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f > %f = %d\n", lValue, rValue,
                     lValue > rValue);
  };

  operationHandlers[OP_LTE] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(lValue <= rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f <= %f = %d\n", lValue, rValue,
                     lValue <= rValue);
  };

  operationHandlers[OP_GTE] = [&](Context *context) {
    double rValue = context->stack.Pop().GetNumber();
    double lValue = context->stack.Pop().GetNumber();

    // Multiply the two values and push the result
    context->stack.Push(GValue(lValue >= rValue));

    Log::Get().Print(LOGLEVEL_VERBOSE, "%f >= %f = %d\n", lValue, rValue,
                     lValue >= rValue);
  };

  operationHandlers[OP_JEZ] = [&](Context *context) {
    bool value = context->stack.Pop().GetFlag();

    // Get byte offset
    int32_t offset = readOffset(context->instructionPointer);

    // Jump to offset
    if (!value) {
      Log::Get().Print(LOGLEVEL_VERBOSE, "JEZ: 0 == 0, Jumping by %d\n",
                       offset);

      context->instructionPointer += offset;
    } else {
      Log::Get().Print(LOGLEVEL_VERBOSE, "JEZ: %d != 0, Ignoring jump by %d\n",
                       value, offset);

      context->instructionPointer += sizeof(int32_t);
    }
  };

  operationHandlers[OP_JNZ] = [&](Context *context) {
    bool value = context->stack.Pop().GetFlag();

    // Get byte offset
    int32_t offset = readOffset(context->instructionPointer);

    // Jump to offset
    if (value) {
      Log::Get().Print(LOGLEVEL_VERBOSE, "JNZ: 0 != 0, Jumping by %d\n",
                       offset);

      context->instructionPointer += offset;
    } else {
      Log::Get().Print(LOGLEVEL_VERBOSE, "JNZ: %d == 0, Ignoring jump by %d\n",
                       value, offset);

      context->instructionPointer += sizeof(int32_t);
    }
  };

  operationHandlers[OP_NOT] = [&](Context *context) {
    bool value = context->stack.Pop().GetFlag();

    // Multiply the two values and push the result
    context->stack.Push(GValue(value ? false : true));

    Log::Get().Print(LOGLEVEL_VERBOSE, "!%d = %d\n", value,
                     value ? false : true);
  };

  operationHandlers[OP_STOP] = [&](Context *context) {
    // Halts the context's execution
    context->Halt();
  };

  // TODO:
  // Do logical AND and OR need operators? Not sure.
  // The short-circuit implementation seems to fix that..

  /*
      OP_AND,     //  PUSH (S(1) && S(0))
      OP_OR,      //  PUSH (S(1) || S(0))

      OP_EOF,     //  End of file
  */
}

OperationDispatcher::~OperationDispatcher() {}

void OperationDispatcher::Dispatch(Context *context, Bytecode &bytecode,
                                   const Opcode &op)
{
  operationHandlers[op](context);
}
