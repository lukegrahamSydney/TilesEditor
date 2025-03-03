#ifndef GS1PARSE_LEXER_HPP
#define GS1PARSE_LEXER_HPP

#include <gs1/parse/Diag.hpp>
#include <gs1/parse/Source.hpp>
#include <gs1/parse/Token.hpp>

namespace gs1
{
class Lexer
{
public:
  Lexer(DiagBuilder &diag, ISource &source);

  void FlagNextAsString(bool inCall, bool lastArg);

  void Advance();
  Token Current();
  Token Lookahead(size_t n = 1);

private:
  bool IsAlpha(uint32_t c);
  bool IsAlphaNum(uint32_t c);
  bool IsWhitespace(uint32_t c);
  bool IsBinDigit(uint32_t c);
  bool IsDecDigit(uint32_t c);
  bool IsHexDigit(uint32_t c);

  void NextChar();
  void FetchToken();
  void PushToken(TokenType type);

  void HandleEOF();
  void HandleWhitespace();
  void HandleNewline();
  void HandleLineComment();
  void HandleBlockComment();
  void HandleId();
  void HandleNumberLiteral();
  void HandleStringLiteral();
  bool HandleSymbol();

  ISource &source;
  DiagBuilder &diag;
  Pos start;
  bool stringNext;
  bool stringInCall;
  bool stringLastArg;
  uint32_t ch;
  uint32_t pk;
  deque<Token> tokens;

  static unordered_map<string, TokenType> keywords;
  static unordered_map<string, TokenType> symbols;
};
}

#endif
