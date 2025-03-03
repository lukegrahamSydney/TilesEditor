#ifndef GS1PARSE_BYTECODEBODY_HPP
#define GS1PARSE_BYTECODEBODY_HPP

#include <gs1/common/ByteBuffer.hpp>
#include <gs1/common/Operation.hpp>
#include <gs1/common/PackedValue.hpp>

namespace gs1
{
struct Reservation {
  Reservation(ByteBuffer *byteBuffer, unsigned int offset)
      : byteBuffer(byteBuffer), offset(offset)
  {
  }

  ~Reservation(){};

  Reservation &Emit(Opcode op)
  {
    byteBuffer->Write8(op, offset++);
    return *this;
  };

  Reservation &Emit(int constant)
  {
    byteBuffer->Write32(constant, offset);

    offset += 4;

    return *this;
  };

  Reservation &Emit(unsigned int constant)
  {
    byteBuffer->WriteU32(constant, offset);

    offset += 4;

    return *this;
  };

  unsigned int GetPosition() { return offset; }

private:
  ByteBuffer *byteBuffer;
  unsigned int offset;
  unsigned int remaining;
};

class BytecodeBody
{
public:
  BytecodeBody();

  ~BytecodeBody();

  void Emit(Opcode op);

  void Emit(int constant);

  void Emit(unsigned int constant);

  void Emit(const PackedValue &value);

  Reservation BeginFunction(std::string name);

  void EndFunction(Reservation reservation);

  Reservation Reserve(unsigned int numBytes);

  unsigned int GetCurrentPosition();

  ByteBuffer GetByteBuffer();

private:
  ByteBuffer byteBuffer;
};
}

#endif
