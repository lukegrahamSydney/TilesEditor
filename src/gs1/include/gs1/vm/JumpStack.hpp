#ifndef GS1VM_JUMP_STACK_HPP
#define GS1VM_JUMP_STACK_HPP

/**
 * TODO:
 * This whole thing should be optimized. It's going to be used very
 * frequently and the extra copies done here are going to be costly.
 *
 * We should allocate a fixed region of memory on the heap. Use that
 * instead, just keep track of the endpoint and shift it, no need to
 * clear or copy variables unnessecarily like std::stack does.
 */

#include <stack>

namespace gs1
{
class Context;

class JumpStack
{
public:
  JumpStack(){};
  ~JumpStack(){};

  void Push(uint32_t offset) { stack.push(offset); };

  uint32_t Pop()
  {
    uint32_t value = stack.top();
    stack.pop();

    return value;
  };

  int Size() { return stack.size(); };

private:
  std::stack<uint32_t> stack;
};
};

#endif