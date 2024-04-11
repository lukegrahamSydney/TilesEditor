#ifndef GS1VM_BYTECODE_HPP
#define GS1VM_BYTECODE_HPP

#include <gs1/common/ConstantTable.hpp>

#include <functional>

namespace gs1
{
class Bytecode
{
  friend class Context;
  friend class Device;

public:
  Bytecode(const char *data, int len);
  virtual ~Bytecode();

  virtual const char *GetBody();
  virtual unsigned int GetBodyLen();

  virtual const char *GetData();
  virtual unsigned int GetLen();

private:
  char *body;
  unsigned int bodyLen;

  char *data;
  unsigned int len;

  ConstantTable<std::string> stringConstants;
  ConstantTable<float> numberConstants;
};
}

#endif