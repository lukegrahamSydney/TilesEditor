#ifndef GS1VM_STACK_HPP
#define GS1VM_STACK_HPP

/**
 * TODO:
 * This whole thing should be optimized. It's going to be used very
 * frequently and the extra copies done here are going to be costly.
 *
 * We should allocate a fixed region of memory on the heap. Use that
 * instead, just keep track of the endpoint and shift it, no need to
 * clear or copy variables unnessecarily like std::stack does.
 */

#include <gs1/common/GValue.hpp>

#include <stack>

namespace gs1
{
class Context;

class Stack
{
public:
  Stack(){};
  ~Stack(){};

  void Push(GValue value) { stack.push(value); };

  GValue Pop()
  {
    GValue value = stack.top();
    stack.pop();

    return value;
  };

  int Size() { return stack.size(); };

private:
  std::stack<GValue> stack;
};
};

#endif