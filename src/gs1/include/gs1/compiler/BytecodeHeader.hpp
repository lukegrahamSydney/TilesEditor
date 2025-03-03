#ifndef GS1PARSE_BYTECODEHEADER_HPP
#define GS1PARSE_BYTECODEHEADER_HPP

#include <map>

#include <gs1/common/ByteBuffer.hpp>
#include <gs1/common/ConstantTable.hpp>
#include <gs1/common/Operation.hpp>
#include <gs1/common/PackedValue.hpp>

namespace gs1
{
class BytecodeHeader
{
public:
  BytecodeHeader();

  ~BytecodeHeader();

  std::shared_ptr<ConstantTable<std::string>> constStringTable;
  std::shared_ptr<ConstantTable<float>> constNumberTable;
  std::map<std::string, uint32_t> functionOffsetTable;

  ByteBuffer GetByteBuffer();

private:
};
}

#endif