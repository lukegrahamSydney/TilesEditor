#include <gs1/common/BufferReader.hpp>
#include <gs1/vm/Bytecode.hpp>

#include <gs1/common/Log.hpp>
#include <stdlib.h>
#include <string.h>

using namespace gs1;

Bytecode::Bytecode(const char *data, int len) : len(len)
{
  this->data = (char *)malloc(len);

  memcpy(this->data, data, len);

  // Set offset to body
  BufferReader reader(data, len);

  // Load the offset to the bytecode body
  uint32_t bodyOffset = reader.ReadU32();

  body = (char *)data + bodyOffset;
  bodyLen = len - (body - data);

  // Load the constant tables

  // String constants
  uint32_t numStrConstants = reader.ReadU32();

  for (uint32_t i = 0; i < numStrConstants; ++i) {
    std::string value = reader.ReadString();

    stringConstants.GetKey(value);
  }

  // Number constants
  uint32_t numNumConstants = reader.ReadU32();

  for (uint32_t i = 0; i < numNumConstants; ++i) {
    float num = reader.ReadFloat();

    numberConstants.GetKey(num);
  }
}

Bytecode::~Bytecode() { free(data); }

const char *Bytecode::GetBody() { return body; }

unsigned int Bytecode::GetBodyLen() { return bodyLen; }

const char *Bytecode::GetData() { return data; }

unsigned int Bytecode::GetLen() { return len; }