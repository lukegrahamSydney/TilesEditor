#ifndef GS1COMMON_BUFFERREADER_HPP
#define GS1COMMON_BUFFERREADER_HPP

#include <gs1/common/Util.hpp>

#include "Log.hpp"
#include <string>

namespace gs1
{
class BufferReader
{
public:
  BufferReader(const char *data, uint32_t len)
      : data(data), len(len), readPos(0)
  {
    // Check machine endianness
    char swapTest[2] = {1, 0};
    if (*(short *)swapTest == 1)
      isBigEndian = false;
    else
      isBigEndian = true;
  };

  int8_t Read8() { return data[readPos++]; };

  uint8_t ReadU8() { return data[readPos++]; };

  int16_t Read16()
  {
    char _value[4] = {data[readPos], data[readPos + 1]};
    int16_t value = *((int16_t *)&_value);

    if (isBigEndian)
      value = bswap_16(value);

    readPos += 2;

    return value;
  };

  uint16_t ReadU16()
  {
    char _value[4] = {data[readPos], data[readPos + 1]};
    uint16_t value = *((uint16_t *)&_value);

    if (isBigEndian)
      value = bswap_16(value);

    readPos += 2;

    return value;
  };

  int32_t Read32()
  {
    char _value[4] = {data[readPos], data[readPos + 1], data[readPos + 2],
                      data[readPos + 3]};
    int32_t value = *((int32_t *)&_value);

    if (isBigEndian)
      value = bswap_32(value);

    readPos += 4;

    return value;
  };

  uint32_t ReadU32()
  {
    char _value[4] = {data[readPos], data[readPos + 1], data[readPos + 2],
                      data[readPos + 3]};
    uint32_t value = *((uint32_t *)&_value);

    if (isBigEndian)
      value = bswap_32(value);

    readPos += 4;

    return value;
  };

  float ReadFloat()
  {
    char _value[4] = {data[readPos], data[readPos + 1], data[readPos + 2],
                      data[readPos + 3]};
    float value = *((float *)&_value);

    readPos += 4;

    return value;
  }

  std::string ReadString()
  {
    std::string output;

    uint32_t length = ReadU32();

    for (uint32_t i = 0; i < length; ++i)
      output += (char)ReadU8();

    return output;
  }

  bool Skip(uint32_t size)
  {
    if (readPos + size > len)
      return false;

    readPos += size;

    return true;
  };

  void Seek(uint32_t pos) { readPos = pos; };

private:
  bool isBigEndian;

  const char *data;

  uint32_t len;
  uint32_t readPos;
};
}

#endif