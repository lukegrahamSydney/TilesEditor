#include <gs1/parse/Lexer.hpp>

using namespace gs1;

Lexer::Lexer(DiagBuilder &diag, ISource &source)
    : diag(diag), source(source), stringNext(false)
{
  ch = source.Current();
  pk = source.Lookahead();
}

void Lexer::FlagNextAsString(bool inCall, bool lastArg)
{
  stringNext = true;
  stringInCall = inCall;
  stringLastArg = lastArg;

  if (!tokens.empty()) {
    source.Reset(tokens.front().range.beg);
    tokens.clear();

    ch = source.Current();
    pk = source.Lookahead();
  }
}

void Lexer::Advance()
{
  if (tokens.empty()) {
    FetchToken();
  }
  tokens.pop_front();
}

Token Lexer::Current()
{
  if (tokens.empty()) {
    FetchToken();
  }
  return tokens.front();
}

Token Lexer::Lookahead(size_t n)
{
  while (tokens.size() <= n) {
    FetchToken();
  }
  return tokens[n];
}

bool Lexer::IsAlpha(uint32_t c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::IsAlphaNum(uint32_t c) { return IsAlpha(c) || IsDecDigit(c); }

bool Lexer::IsWhitespace(uint32_t c) { return c == ' ' || c == '\t'; }

bool Lexer::IsBinDigit(uint32_t c) { return c == '0' || c == '1'; }

bool Lexer::IsDecDigit(uint32_t c) { return c >= '0' && c <= '9'; }

bool Lexer::IsHexDigit(uint32_t c)
{
  return IsDecDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

void Lexer::NextChar()
{
  source.Advance();

  if (source.GetFinished()) {
    ch = pk = 0;
    return;
  }

  ch = source.Current();
  pk = source.Lookahead();
}

void Lexer::FetchToken()
{
  start = source.GetPos();

  if (ch == '\0') {
    HandleEOF();
  } else if (IsWhitespace(ch)) {
    HandleWhitespace();
  } else if (ch == '\n' || (ch == '\r' && pk == '\n')) {
    HandleNewline();
  } else if (stringNext) {
    HandleStringLiteral();
  } else if (ch == '/' && pk == '/') {
    HandleLineComment();
  } else if (ch == '/' && pk == '*') {
    HandleBlockComment();
  } else if (ch == '_' || IsAlpha(ch)) {
    HandleId();
  } else if (IsDecDigit(ch) || (ch == '.' && IsDecDigit(pk))) {
    HandleNumberLiteral();
  } else if (!HandleSymbol()) {
    // TODO: diag, but it might be inside a string..
    NextChar();
    PushToken(TokInvalid);
  }
}

void Lexer::PushToken(TokenType type)
{
  auto range = Range(start, source.GetPos());
  auto text = source.GetRangeContents(range);
  tokens.push_back(Token(range, text, type));
}

void Lexer::HandleEOF() { PushToken(TokEOF); }

void Lexer::HandleWhitespace()
{
  do {
    NextChar();
  } while (IsWhitespace(ch));

  PushToken(TokWhitespace);
}

void Lexer::HandleNewline()
{
  if (ch == '\r') {
    NextChar();
  }

  NextChar();
  PushToken(TokNewline);
}

void Lexer::HandleLineComment()
{
  do {
    NextChar();
  } while (ch != '\0' && ch != '\n' && !(ch == '\r' && pk == '\n'));

  PushToken(TokComment);
}

void Lexer::HandleBlockComment()
{
  do {
    NextChar();
  } while (ch != '\0' && !(ch == '*' && pk == '/'));

  if (ch == '\0') {
    diag.Error(Pos(), Range(start, source.GetPos()),
               "unterminated block comment");
  } else {
    NextChar();
    NextChar();
  }

  PushToken(TokComment);
}

void Lexer::HandleId()
{
  do {
    NextChar();
  } while (ch == '_' || ch == '.' || IsAlphaNum(ch));

  auto text = source.GetRangeContents(Range(start, source.GetPos()));

  auto it = keywords.find(text);
  if (it != keywords.end()) {
    PushToken(it->second);
  } else {
    PushToken(TokId);
  }
}

void Lexer::HandleNumberLiteral()
{
  int base = 10;
  bool hasDot = false;

  if (ch == '0') {
    if (pk == 'b' || pk == 'B') {
      NextChar();
      NextChar();
      base = 2;
    } else if (pk == 'x' || pk == 'X') {
      NextChar();
      NextChar();
      base = 16;
    }
  }

  if (base == 2) {
    while (IsBinDigit(ch)) {
      NextChar();
    }
  } else if (base == 10) {
    do {
      if (ch == '.') {
        if (hasDot) {
          break;
        } else {
          hasDot = true;
        }
      }
      NextChar();
    } while (ch == '.' || IsDecDigit(ch));

    if ((hasDot && ch == 'e') || ch == 'E') {
      NextChar();

      if (ch == '+' || ch == '-') {
        NextChar();
      }

      do {
        NextChar();
      } while (IsDecDigit(ch));
    }

    if (ch == 'f') {
      NextChar();
    }
  } else if (base == 16) {
    while (IsHexDigit(ch)) {
      NextChar();
    }
  }

  PushToken(TokNumberLiteral);
}

void Lexer::HandleStringLiteral()
{
  stringNext = false;

  int nestLevel = 0;

  do {
    if (stringInCall && stringLastArg && ch == ')' && !nestLevel) {
      break;
    } else if (stringInCall && !stringLastArg && ch == ',' && !nestLevel) {
      break;
    } else if (!stringInCall && stringLastArg && (ch == ';' || ch=='}') && !nestLevel) {
      break;
    } else if (!stringInCall && !stringLastArg && ch == ',' && !nestLevel) {
      break;
    }

    // TODO: only do for string interpolation
    if (ch == '(') {
      nestLevel++;
    } else if (ch == ')') {
      nestLevel--;
    }

    NextChar();
  } while (ch != '\0');

  PushToken(TokStringLiteral);
}

bool Lexer::HandleSymbol()
{
  string op2;
  op2 += ch;
  op2 += pk;

  auto it = symbols.find(op2);
  if (it != symbols.end()) {
    NextChar();
    NextChar();
    PushToken(it->second);
    return true;
  }

  string op1;
  op1 += ch;

  it = symbols.find(op1);
  if (it != symbols.end()) {
    NextChar();
    PushToken(it->second);
    return true;
  }

  return false;
}

unordered_map<string, TokenType> Lexer::keywords = {
    {"in", TokKwIn},
    {"if", TokKwIf},
    {"else", TokKwElse},
    {"for", TokKwFor},
    {"while", TokKwWhile},
    {"break", TokKwBreak},
    {"continue", TokKwContinue},
    {"function", TokKwFunction},
    {"return", TokKwReturn},
};

unordered_map<string, TokenType> Lexer::symbols = {
    {".", TokDot},
    {",", TokComma},
    {":", TokColon},
    {";", TokSemicolon},
    {"|", TokPipe},

    {"(", TokLeftParen},
    {")", TokRightParen},
    {"{", TokLeftBrace},
    {"}", TokRightBrace},
    {"[", TokLeftBracket},
    {"]", TokRightBracket},

    {"?", TokOpTernary},

    {"!", TokOpNot},
    {"&&", TokOpAnd},
    {"||", TokOpOr},
    {"==", TokOpEquals},
    {"!=", TokOpNotEquals},
    {"<", TokOpLessThan},
    {"<=", TokOpLessThanOrEqual},
    {"=<", TokOpGreaterThanOrEqual},
    {">", TokOpGreaterThan},
    {">=", TokOpGreaterThanOrEqual},
    {"=>", TokOpGreaterThanOrEqual},

    {"+", TokOpAdd},
    {"-", TokOpSub},
    {"*", TokOpMul},
    {"/", TokOpDiv},
    {"^", TokOpPow},
    {"%", TokOpMod},

    {"=", TokOpAssign},
    {"+=", TokOpAddAssign},
    {"-=", TokOpSubAssign},
    {"*=", TokOpMulAssign},
    {"/=", TokOpDivAssign},
    {"^=", TokOpPowAssign},
    {"%=", TokOpModAssign},

    {"++", TokOpIncrement},
    {"--", TokOpDecrement},
};
