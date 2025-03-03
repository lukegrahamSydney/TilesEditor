#include <gs1/common/Operation.hpp>

std::string gs1::OpcodeToString(Opcode opcode)
{
  switch (opcode) {
  case OP_PUSH:
    return "OP_PUSH";

  case OP_ASSIGN:
    return "OP_ASSIGN";

  case OP_ARR_SET:
    return "OP_ARR_SET";

  case OP_ARR_GET:
    return "OP_ARR_GET";

  case OP_ADD:
    return "OP_ADD";

  case OP_SUB:
    return "OP_SUB";

  case OP_MUL:
    return "OP_MUL";

  case OP_DIV:
    return "OP_DIV";

  case OP_MOD:
    return "OP_MOD";

  case OP_POW:
    return "OP_POW";

  case OP_INC:
    return "OP_INC";

  case OP_INCPUSH:
    return "OP_INCPUSH";

  case OP_DEC:
    return "OP_DEC";

  case OP_DECPUSH:
    return "OP_DECPUSH";

  case OP_CALL:
    return "OP_CALL";

  case OP_CMD_CALL:
    return "OP_CMD_CALL";

  case OP_JMP:
    return "OP_JMP";

  case OP_JAL:
    return "OP_JAL";

  case OP_RET:
    return "OP_RET";

  case OP_EQ:
    return "OP_EQ";

  case OP_LT:
    return "OP_LT";

  case OP_GT:
    return "OP_GT";

  case OP_LTE:
    return "OP_LTE";

  case OP_GTE:
    return "OP_GTE";

  case OP_NOT:
    return "OP_NOT";

  case OP_AND:
    return "OP_AND";

  case OP_OR:
    return "OP_OR";

  case OP_JEZ:
    return "OP_JEZ";

  case OP_JNZ:
    return "OP_JNZ";

  case OP_STOP:
    return "OP_STOP";

  case OP_DBG_OUT:
    return "OP_DBG_OUT";

  default:
    return "";
  }

  return "Unknown opcode";
}