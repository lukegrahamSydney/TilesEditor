#ifndef GS1VM_OPERATIONDISPATCHER_HPP
#define GS1VM_OPERATIONDISPATCHER_HPP

#include <gs1/vm/Stack.hpp>

#include <gs1/common/BufferReader.hpp>
#include <gs1/common/Operation.hpp>

namespace gs1
{
class Context;

class OperationDispatcher
{
public:
  OperationDispatcher();
  ~OperationDispatcher();

  void Dispatch(Context *context, Bytecode &bytecode, const Opcode &op);

private:
};
};

#endif