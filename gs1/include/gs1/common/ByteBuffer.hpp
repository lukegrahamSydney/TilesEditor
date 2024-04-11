#ifndef GS1COMMON_BYTEBUFFER_HPP
#define GS1COMMON_BYTEBUFFER_HPP

#include <stdint.h>
#include <string>

#define BYTEBUFFER_OFFSET_NONE -128

namespace gs1
{
class ByteBuffer
{
public:
  ByteBuffer();
  ByteBuffer(const ByteBuffer &other);

  ~ByteBuffer();

  void Write8(const int8_t &byte, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void WriteU8(const uint8_t &byte, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void Write16(const int16_t &value, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void WriteU16(const uint16_t &value, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void Write32(const int32_t &value, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void WriteU32(const uint32_t &value, int32_t offset = BYTEBUFFER_OFFSET_NONE);

  void WriteString(const std::string &value);

  void WriteBytes(const char *bytes, const unsigned int &length);

  unsigned int Reserve(const uint32_t &size);

  char *GetBytes();

  unsigned int GetLength();

  friend void swap(ByteBuffer &first, ByteBuffer &other)
  {
    using std::swap;

    std::swap(first.len, other.len);
    std::swap(first.maxLen, other.maxLen);
    std::swap(first.isBigEndian, other.isBigEndian);
    std::swap(first.bytes, other.bytes);
  }

  ByteBuffer &operator=(ByteBuffer other)
  {
    swap(*this, other);

    return (*this);
  };

private:
  bool isBigEndian;
  char *bytes;
  unsigned int len;
  unsigned int maxLen;
};
}

#endif
