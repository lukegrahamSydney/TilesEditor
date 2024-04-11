#include <gs1/parse/Source.hpp>

#include <cstdio>
#include <cstring>

using namespace gs1;

// ------------------------------------------------------------
// MemorySource implementation
// ------------------------------------------------------------

MemorySource::MemorySource(const char *beg, int size)
    : beg(beg), ptr(beg), currentLine(0)
{
  if (size == -1) {
    end = beg + strlen(beg);
  } else {
    end = beg + size;
  }
}

void MemorySource::Reset(Pos p) { ptr = beg + p.offset; }

Pos MemorySource::GetPos() const { return Pos(ptr - beg, currentLine); }

string MemorySource::GetRangeContents(Range r) const
{
  return string(&beg[r.beg.offset], r.end.offset - r.beg.offset);
}

void MemorySource::Advance()
{
  if (ptr < end) {
    if (*ptr == '\n')
      currentLine++;

    ptr++;
  }
}

uint32_t MemorySource::Current() { return (ptr < end) ? *ptr : '\0'; }

uint32_t MemorySource::Lookahead(size_t n)
{
  return (ptr + n < end) ? *(ptr + n) : '\0';
}

bool MemorySource::GetFinished() { return ptr >= end; }

// ------------------------------------------------------------
// FileSource implementation
// ------------------------------------------------------------

FileSource::FileSource(string filename) : offset(0), currentLine(0)
{
  auto file = fopen(filename.c_str(), "rb");
  if (!file) {
    throw Exception("couldn't open file: %s", filename.c_str());
  }

  fseek(file, 0, SEEK_END);
  auto size = ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer.resize(size);

  fread(&buffer[0], 1, size, file);
  fclose(file);
}

void FileSource::Reset(Pos p) { offset = p.offset; }

Pos FileSource::GetPos() const { return Pos(offset, currentLine); }

string FileSource::GetRangeContents(Range r) const
{
  if (r.end.offset == r.beg.offset)
    return "";

  return string(&buffer[r.beg.offset], r.end.offset - r.beg.offset);
}

void FileSource::Advance()
{
  if (offset < buffer.size()) {
    if (buffer[offset] == '\n')
      currentLine++;

    offset++;
  }
}

uint32_t FileSource::Current() { return (uint32_t)buffer[offset]; }

uint32_t FileSource::Lookahead(size_t n)
{
  if (offset + n < buffer.size()) {
    return (uint32_t)buffer[offset + n];
  } else {
    return '\0';
  }
}

bool FileSource::GetFinished() { return offset >= buffer.size(); }