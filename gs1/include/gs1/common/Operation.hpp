#ifndef GS1COMMON_OPERATION_HPP
#define GS1COMMON_OPERATION_HPP

#include <string>

namespace gs1
{
// Legend:
// A(x) - Argument value by index into bytecode
// S(x) - Stack value by reverse offset index from the top

enum Opcode {

  // @formatter:off

  /*  NAME            SEMANTIC                        */
  /* ------------------------------------------------ */
  OP_PUSH, //  PUSH N(0, sizeof(PackedValue))

  OP_ASSIGN, //  S(1) = S(0)

  OP_ARR_SET, //  S(1) =
  OP_ARR_GET, //  S(1) =

  OP_ADD, //  PUSH (S(1) + S(0))
  OP_SUB, //  PUSH (S(1) - S(0))
  OP_MUL, //  PUSH (S(1) * S(0))
  OP_DIV, //  PUSH (S(1) / S(0))
  OP_MOD, //  PUSH (S(1) % S(0))
  OP_POW, //  PUSH (S(1) ^ S(0))

  OP_INC,     //  SET (S(0) = S(0) + 1)
  OP_INCPUSH, //  PUSH (S(0)), SET (S(0) = S(0) + 1)

  OP_DEC,     //  SET (S(0) = S(0) - 1)
  OP_DECPUSH, //  PUSH (S(0)), SET (S(0) = S(0) - 1)

  OP_CALL,     //  Pushes # of args, followed by args
  OP_CMD_CALL, //  Pushes # of args, followed by args

  OP_JMP, //  JUMP to N(0, 4) by byte offset unconditionally
  OP_JAL, //  JUMP as per prior semantic and pass last location to jump stack
  OP_RET, //  Return to location on top of on jump stack

  OP_EQ,  //  PUSH (S(1) == S(0))
  OP_LT,  //  PUSH (S(1) < S(0))
  OP_GT,  //  PUSH (S(1) > S(0))
  OP_LTE, //  PUSH (S(1) <= S(0))
  OP_GTE, //  PUSH (S(1) >= S(0))

  OP_NOT, //  PUSH (!S(0))
  OP_AND, //  PUSH (S(1) && S(0))
  OP_OR,  //  PUSH (S(1) || S(0))

  OP_JEZ, //  JUMP to N(0, 4) if S(0) == 0
  OP_JNZ, //  JUMP to N(0, 4) if S(0) != 0

  OP_STOP, // Stops execution

  OP_DBG_OUT, //  Debug output

  OP_NUM_OPS //  This is to get the number of operations

  // @formatter:on

};

std::string OpcodeToString(Opcode opcode);
}

#endif