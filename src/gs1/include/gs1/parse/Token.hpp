#ifndef GS1PARSE_TOKEN_HPP
#define GS1PARSE_TOKEN_HPP

#include <gs1/common/Util.hpp>

namespace gs1
{
enum TokenType {
  TokInvalid,
  TokEOF,
  TokWhitespace,
  TokNewline,
  TokComment,

  TokId,
  TokNumberLiteral,
  TokStringLiteral,

  TokKwIn,
  TokKwIf,
  TokKwElse,
  TokKwFor,
  TokKwWhile,
  TokKwBreak,
  TokKwContinue,
  TokKwFunction,
  TokKwReturn,

  TokDot,
  TokComma,
  TokColon,
  TokSemicolon,
  TokPipe,

  TokLeftParen,
  TokRightParen,
  TokLeftBrace,
  TokRightBrace,
  TokLeftBracket,
  TokRightBracket,

  TokOpTernary,

  TokOpNot,
  TokOpAnd,
  TokOpOr,
  TokOpEquals,
  TokOpNotEquals,
  TokOpLessThan,
  TokOpLessThanOrEqual,
  TokOpGreaterThan,
  TokOpGreaterThanOrEqual,

  TokOpAdd,
  TokOpSub,
  TokOpMul,
  TokOpDiv,
  TokOpPow,
  TokOpMod,

  TokOpAssign,
  TokOpAddAssign,
  TokOpSubAssign,
  TokOpMulAssign,
  TokOpDivAssign,
  TokOpPowAssign,
  TokOpModAssign,

  TokOpIncrement,
  TokOpDecrement,
  TokKwWith,
};

struct Token {
  Token();
  Token(Range range, string text, TokenType type);

  Range range;
  string text;
  TokenType type;
};

const char *GetTokenTypeName(TokenType type);
const char *GetTokenTypeSpelling(TokenType type);

int GetOperatorPrecedence(TokenType type);
bool IsBinaryOrTernaryOperator(TokenType type);

bool IsUnaryPrefix(TokenType type);
bool IsUnaryPostfix(TokenType type);
}

#endif
