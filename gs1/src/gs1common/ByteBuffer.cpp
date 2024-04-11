#include <gs1/common/ByteBuffer.hpp>
#include <gs1/common/Util.hpp>

#include <stdlib.h>
#include <string.h>

#define MAX_LEN_INITIAL 4096

using namespace gs1;

/**
 * This is a byte-buffer that enforces little-endian
 */

ByteBuffer::ByteBuffer() : len(0), maxLen(MAX_LEN_INITIAL)
{
  bytes = (char *)malloc(maxLen * sizeof(char));

  // Check machine endianness
  char swapTest[2] = {1, 0};
  if (*(short *)swapTest == 1)
    isBigEndian = false;
  else
    isBigEndian = true;
}

ByteBuffer::ByteBuffer(const ByteBuffer &other)
    : len(other.len), maxLen(other.maxLen)
{
  bytes = (char *)malloc(maxLen * sizeof(char));

  memcpy(bytes, other.bytes, len);
}

ByteBuffer::~ByteBuffer() { free(bytes); }

void ByteBuffer::Write8(const int8_t &value, int32_t offset)
{
  if (offset < 0) {
    if (len < maxLen)
      bytes[len++] = value;
    else {
      maxLen *= 2;

      bytes = (char *)realloc(bytes, maxLen * sizeof(char));

      Write8(value);
    }
  } else {
    bytes[offset] = value;
  }
}

void ByteBuffer::WriteU8(const uint8_t &value, int32_t offset)
{
  if (offset < 0) {
    if (len < maxLen)
      bytes[len++] = value;
    else {
      maxLen *= 2;

      bytes = (char *)realloc(bytes, maxLen * sizeof(char));

      WriteU8(value);
    }
  } else {
    bytes[offset] = value;
  }
}

void ByteBuffer::Write16(const int16_t &value, int32_t offset)
{
  if (isBigEndian) {
#ifdef _MSC_VER
    int16_t swap = _byteswap_ushort(value);
#else
    int16_t swap = bswap_16(value);
#endif

    Write8(((char *)&swap)[0], offset);
    Write8(((char *)&swap)[1], offset + 1);
  } else {
    Write8(((char *)&value)[0], offset);
    Write8(((char *)&value)[1], offset + 1);
  }
}

void ByteBuffer::WriteU16(const uint16_t &value, int32_t offset)
{
  if (isBigEndian) {
#ifdef _MSC_VER
    int16_t swap = _byteswap_ushort(value);
#else
    int16_t swap = bswap_16(value);
#endif

    WriteU8(((unsigned char *)&swap)[0], offset);
    WriteU8(((unsigned char *)&swap)[1], offset + 1);
  } else {
    WriteU8(((unsigned char *)&value)[0], offset);
    WriteU8(((unsigned char *)&value)[1], offset + 1);
  }
}

void ByteBuffer::Write32(const int32_t &value, int32_t offset)
{
  if (isBigEndian) {
#ifdef _MSC_VER
    int16_t swap = _byteswap_ulong(value);
#else
    int16_t swap = bswap_32(value);
#endif

    Write8(((char *)&swap)[0], offset);
    Write8(((char *)&swap)[1], offset + 1);
    Write8(((char *)&swap)[2], offset + 2);
    Write8(((char *)&swap)[3], offset + 3);
  } else {
    Write8(((char *)&value)[0], offset);
    Write8(((char *)&value)[1], offset + 1);
    Write8(((char *)&value)[2], offset + 2);
    Write8(((char *)&value)[3], offset + 3);
  }
}

void ByteBuffer::WriteU32(const uint32_t &value, int32_t offset)
{
  if (isBigEndian) {
#ifdef _MSC_VER
    int16_t swap = _byteswap_ulong(value);
#else
    int16_t swap = bswap_32(value);
#endif

    WriteU8(((unsigned char *)&swap)[0], offset);
    WriteU8(((unsigned char *)&swap)[1], offset + 1);
    WriteU8(((unsigned char *)&swap)[2], offset + 2);
    WriteU8(((unsigned char *)&swap)[3], offset + 3);
  } else {
    WriteU8(((unsigned char *)&value)[0], offset);
    WriteU8(((unsigned char *)&value)[1], offset + 1);
    WriteU8(((unsigned char *)&value)[2], offset + 2);
    WriteU8(((unsigned char *)&value)[3], offset + 3);
  }
}

void ByteBuffer::WriteString(const std::string &value)
{
  WriteU32(value.length());

  for (int i = 0; i < value.length(); ++i)
    WriteU8(value[i]);
}

void ByteBuffer::WriteBytes(const char *bytes, const unsigned int &length)
{
  for (int i = 0; i < length; ++i)
    WriteU8(bytes[i]);
}

unsigned int ByteBuffer::Reserve(const uint32_t &size)
{
  unsigned int offset = len;

  for (unsigned int i = 0; i < size; i++)
    WriteU8(0);

  return offset;
}

char *ByteBuffer::GetBytes() { return bytes; }

unsigned int ByteBuffer::GetLength() { return len; }