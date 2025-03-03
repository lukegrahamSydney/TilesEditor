#ifndef GS1PARSE_SOURCE_HPP
#define GS1PARSE_SOURCE_HPP

#include <gs1/common/Util.hpp>

namespace gs1
{
class ISource
{
public:
  virtual ~ISource() {}

  virtual void Reset(Pos p) = 0;

  virtual Pos GetPos() const = 0;
  virtual string GetRangeContents(Range r) const = 0;

  virtual void Advance() = 0;
  virtual uint32_t Current() = 0;
  virtual uint32_t Lookahead(size_t n = 1) = 0;

  virtual bool GetFinished() = 0;
};

class MemorySource : public ISource
{
public:
  MemorySource(const char *beg, int size = -1);

  void Reset(Pos p);

  Pos GetPos() const;
  string GetRangeContents(Range r) const;

  void Advance();
  uint32_t Current();
  uint32_t Lookahead(size_t n = 1);

  bool GetFinished();

private:
  const char *beg;
  const char *ptr;
  const char *end;
  uint32_t currentLine;
};

class FileSource : public ISource
{
public:
  FileSource(string filename);

  void Reset(Pos p);

  Pos GetPos() const;
  string GetRangeContents(Range r) const;

  void Advance();
  uint32_t Current();
  uint32_t Lookahead(size_t n = 1);

  bool GetFinished();

private:
  size_t offset;
  vector<char> buffer;
  uint32_t currentLine;
};
}

#endif
