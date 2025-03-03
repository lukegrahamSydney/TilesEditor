#include <gs1/parse/Token.hpp>

using namespace gs1;

Token::Token() : type(TokInvalid) {}

Token::Token(Range range, string text, TokenType type)
    : range(range), text(text), type(type)
{
}

const char *gs1::GetTokenTypeName(TokenType type)
{
  static const char *spelling[] = {
      "TokInvalid",
      "TokEOF",
      "TokWhitespace",
      "TokNewline",
      "TokComment",

      "TokId",
      "TokNumberLiteral",
      "TokStringLiteral",

      "TokKwIn",
      "TokKwIf",
      "TokKwElse",
      "TokKwFor",
      "TokKwWhile",
      "TokKwBreak",
      "TokKwContinue",
      "TokKwFunction",
      "TokKwReturn",

      "TokComma",
      "TokColon",
      "TokSemicolon",
      "TokPipe",

      "TokLeftParen",
      "TokRightParen",
      "TokLeftBrace",
      "TokRightBrace",
      "TokLeftBracket",
      "TokRightBracket",

      "TokOpTernary",

      "TokOpNot",
      "TokOpAnd",
      "TokOpOr",
      "TokOpEquals",
      "TokOpNotEquals",
      "TokOpLessThan",
      "TokOpLessThanOrEqual",
      "TokOpGreaterThan",
      "TokOpGreaterThanOrEqual",

      "TokOpAdd",
      "TokOpSub",
      "TokOpMul",
      "TokOpDiv",
      "TokOpPow",
      "TokOpMod",

      "TokOpAssign",
      "TokOpAddAssign",
      "TokOpSubAssign",
      "TokOpMulAssign",
      "TokOpDivAssign",
      "TokOpPowAssign",
      "TokOpModAssign",

      "TokOpIncrement",
      "TokOpDecrement",
      "TokKwWith"
  };

  if (type >= TokInvalid && type <= TokKwWith) {
    return spelling[type];
  }

  throw Exception("name requested for unknown token type: %u", type);
}

const char *gs1::GetTokenTypeSpelling(TokenType type)
{
  static const char *spelling[] = {
      "invalid",  "end of file", "whitespace", "newline", "comment",

      "id",       "number",      "string",

      "in",       "if",          "else",       "for",     "while",   "break",
      "continue", "function",    "return",

      ",",        ":",           ";",          "|",

      "(",        ")",           "{",          "}",       "[",       "]",

      "?",

      "!",        "&&",          "||",         "==",      "!=",      "<",
      "<=",       ">",           ">=",

      "+",        "-",           "*",          "/",       "^",       "%",

      "=",        "+=",          "-=",         "*=",      "/=",      "^=",
      "%=",

      "++",       "--", "with"
  };

  if (type >= TokInvalid && type <= TokKwWith) {
    return spelling[type];
  }

  throw Exception("spelling requested for unknown token type: %u", type);
}

int gs1::GetOperatorPrecedence(TokenType type)
{
  switch (type) {
  case TokKwIn:
    // TODO: is this is the correct precedence for 'in'?
    return 15;
  case TokOpMul:
  case TokOpDiv:
  case TokOpPow:
  case TokOpMod:
    return 14;
  case TokOpAdd:
  case TokOpSub:
    return 13;
  case TokOpLessThan:
  case TokOpLessThanOrEqual:
  case TokOpGreaterThan:
  case TokOpGreaterThanOrEqual:
    return 12;
  case TokOpEquals:
  case TokOpNotEquals:
    return 11;
  case TokOpAnd:
    return 10;
  case TokOpOr:
    return 9;
  case TokOpTernary:
    return 8;
  case TokOpAssign:
  case TokOpAddAssign:
  case TokOpSubAssign:
  case TokOpMulAssign:
  case TokOpDivAssign:
  case TokOpPowAssign:
  case TokOpModAssign:
    return 7;
  default:
    throw Exception("unreachable in gs1::GetOperatorPrecedence");
  }
}

bool gs1::IsBinaryOrTernaryOperator(TokenType type)
{
  switch (type) {
  case TokKwIn:

  case TokOpTernary:

  case TokOpAnd:
  case TokOpOr:
  case TokOpEquals:
  case TokOpNotEquals:
  case TokOpLessThan:
  case TokOpLessThanOrEqual:
  case TokOpGreaterThan:
  case TokOpGreaterThanOrEqual:

  case TokOpAdd:
  case TokOpSub:
  case TokOpMul:
  case TokOpDiv:
  case TokOpPow:
  case TokOpMod:

  case TokOpAssign:
  case TokOpAddAssign:
  case TokOpSubAssign:
  case TokOpMulAssign:
  case TokOpDivAssign:
  case TokOpPowAssign:
  case TokOpModAssign:
    return true;

  default:
    return false;
  }
}

bool gs1::IsUnaryPrefix(TokenType type)
{
  switch (type) {
  case TokOpNot:
  case TokOpSub:
  case TokOpIncrement:
  case TokOpDecrement:
    return true;
  default:
    return false;
  }
}

bool gs1::IsUnaryPostfix(TokenType type)
{
  switch (type) {
  case TokOpIncrement:
  case TokOpDecrement:
    return true;
  default:
    return false;
  }
}
