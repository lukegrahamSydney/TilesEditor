#include <gs1/compiler/BytecodeHeader.hpp>

using namespace gs1;

BytecodeHeader::BytecodeHeader()
    : constStringTable(new ConstantTable<std::string>()),
      constNumberTable(new ConstantTable<float>())
{
}

BytecodeHeader::~BytecodeHeader() {}

ByteBuffer BytecodeHeader::GetByteBuffer()
{
  ByteBuffer buffer;

  auto bodyOffsetReservation = buffer.Reserve(4);

  // Write string constants
  buffer.WriteU32(constStringTable->constants.size());

  for (auto &key : constStringTable->constants)
    buffer.WriteString(key.val);

  // Write numeric constants
  buffer.WriteU32(constNumberTable->constants.size());

  for (auto &key : constNumberTable->constants)
    buffer.WriteU32(*reinterpret_cast<uint32_t *>(&key.val));

  buffer.WriteU32(buffer.GetLength(), bodyOffsetReservation);

  // Write function names and offsets

  return buffer;
}