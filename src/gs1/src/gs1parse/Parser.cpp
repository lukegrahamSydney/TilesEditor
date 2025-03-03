#include <gs1/parse/Parser.hpp>

#include <climits>
using namespace gs1;

Parser::Parser(DiagBuilder &diag, Lexer &lexer, const PrototypeMap &commands,
               const PrototypeMap &functions)
    : diag(diag), lexer(lexer), token(terminal.token.type), commands(commands),
      functions(functions)
{
  EatTerminal();
}


shared_ptr<SyntaxNode> Parser::Parse()
{
  auto node = new StmtBlock;

  PushNode(node);
  {
    while (true) {
      auto stmt = ParseStmt(true);
      if (!stmt) {
        break;
      }

      node->statements.push_back(stmt);
    }

    if (token == TokEOF) {
      EatTerminal();
    } else {
      diag.Warn(terminal.token.range.beg, Range(), "expected end of file");
    }
  }
  PopNode();

  return shared_ptr<SyntaxNode>(node);
}

void Parser::EatTerminal(SyntaxTerminal **ref)
{
  if (!stack.empty()) {
    auto node = stack.back();
    auto term = new SyntaxTerminal(terminal);

    if (ref) {
      *ref = term;
    }

    term->parent = node;
    node->children.push_back(shared_ptr<SyntaxNodeOrTerminal>(term));
  }

  auto tok = lexer.Current();
  terminal.leadingTrivia.clear();
  terminal.trailingTrivia.clear();

  while (tok.type == TokWhitespace || tok.type == TokNewline ||
         tok.type == TokComment) {
    terminal.leadingTrivia.push_back(tok);
    lexer.Advance();
    tok = lexer.Current();
  }

  terminal.token = tok;
  lexer.Advance();
  tok = lexer.Current();

  while (tok.type == TokWhitespace || tok.type == TokComment) {
    terminal.trailingTrivia.push_back(tok);
    lexer.Advance();
    tok = lexer.Current();
  }
}

void Parser::EatTerminal(TokenType type, SyntaxTerminal **ref)
{
  if (token == type) {
    EatTerminal(ref);
  } else {
    auto node = stack.back();
    auto term = new SyntaxTerminal;
    term->token.type = type;
    term->token.range =
        Range(terminal.token.range.beg, terminal.token.range.beg);

    term->parent = node;
    node->children.push_back(shared_ptr<SyntaxNodeOrTerminal>(term));

    diag.Error(terminal.token.range.beg, Range(), "expected '%s' got '%s'",
               GetTokenTypeSpelling(type),
               GetTokenTypeSpelling(terminal.token.type));
  }
}

void Parser::AdoptNode(SyntaxNode *child, SyntaxNode *newParent)
{
  auto oldParent = child->parent;
  child->parent = newParent;

  for (auto it = oldParent->children.begin(); it != oldParent->children.end();
       it++) {
    if (it->get() == child) {
      auto ptr = *it;
      oldParent->children.erase(it);
      newParent->children.push_back(ptr);
      break;
    }
  }
}

void Parser::PushNode(SyntaxNode *node)
{
  if (!stack.empty()) {
    auto parent = stack.back();
    node->parent = parent;
    parent->children.push_back(shared_ptr<SyntaxNodeOrTerminal>(node));
  } else {
    node->parent = nullptr;
  }

  stack.push_back(node);
}

void Parser::PopNode() { stack.pop_back(); }

// --------------------------------------------------
// Statements
// --------------------------------------------------

Stmt *Parser::ParseStmt(bool optional)
{
  switch (token) {
  case TokSemicolon:
    return ParseStmtEmpty();
  case TokLeftBrace:
    return ParseStmtBlock();
  case TokKwIf:
    return ParseStmtIf();
  case TokKwFor:
    return ParseStmtFor();
  case TokKwWhile:
    return ParseStmtWhile();
  case TokKwBreak:
    return ParseStmtBreak();
  case TokKwContinue:
    return ParseStmtContinue();
  case TokKwReturn:
    return ParseStmtReturn();

  case TokKwWith:
      return ParseStmtWith();

  case TokKwFunction: {
    auto node = ParseStmtFunctionDecl();
    if (stack.size() != 1) {
      diag.Error(Pos(), node->GetRange(),
                 "function declaration not at top level");
    }
    return node;
  }
  default:
    break;
  }

  if (token == TokId) {
    auto it = commands.find(terminal.token.text);
    if (it != commands.end()) {
      return ParseStmtCommand(it->second);
    }
  }

  auto expr = ParseExpr(true);
  if (expr) {
    // TODO: last expr/stmt doesnt need semi
    EatTerminal(TokSemicolon);
    return expr;
  }

  if (!optional) {
    diag.Error(terminal.token.range.beg, Range(), "expected statement");
  }

  return nullptr;
}

Stmt *Parser::ParseStmtEmpty()
{
  auto node = new StmtEmpty;
  PushNode(node);
  {
    EatTerminal(TokSemicolon);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtBlock()
{
  auto node = new StmtBlock;
  PushNode(node);
  {
    EatTerminal(TokLeftBrace);

    while (token != TokRightBrace) {
      auto stmt = ParseStmt();
      if (stmt) {
        node->statements.push_back(stmt);
      } else {
        break;
      }
    }

    EatTerminal(TokRightBrace);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtIf()
{
  auto node = new StmtIf;
  PushNode(node);
  {
    EatTerminal(TokKwIf);
    EatTerminal(TokLeftParen);
    node->cond = ParseExpr();

    EatTerminal(TokRightParen);
    node->thenBody = ParseStmt();

    if (token == TokKwElse) {
      EatTerminal();
      node->elseBody = ParseStmt();
    } else {
      node->elseBody = nullptr;
    }
  }
  PopNode();
  return node;
}

Stmt* gs1::Parser::ParseStmtWith()
{
    auto node = new StmtWith;
    PushNode(node);
    {
        EatTerminal(TokKwWith);
        EatTerminal(TokLeftParen);
        node->init = ParseStmt();

        EatTerminal(TokRightParen);
        node->body = ParseStmt();
    }
    PopNode();
    return node;
}

Stmt *Parser::ParseStmtFor()
{
  auto node = new StmtFor;
  PushNode(node);
  {
    EatTerminal(TokKwFor);
    EatTerminal(TokLeftParen);
    {
      node->init = ParseExpr(true);
      EatTerminal(TokSemicolon);
      node->cond = ParseExpr(true);
      EatTerminal(TokSemicolon);
      node->step = ParseExpr(true);
      if(token == TokSemicolon)
          EatTerminal(TokSemicolon);
    }
    EatTerminal(TokRightParen);
    node->body = ParseStmt();
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtWhile()
{
  auto node = new StmtWhile;
  PushNode(node);
  {
    EatTerminal(TokKwWhile);
    EatTerminal(TokLeftParen);
    node->cond = ParseExpr();
    EatTerminal(TokRightParen);
    node->body = ParseStmt();
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtBreak()
{
  auto node = new StmtBreak;
  PushNode(node);
  {
    EatTerminal(TokKwBreak);
    EatTerminal(TokSemicolon);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtContinue()
{
  auto node = new StmtContinue;
  PushNode(node);
  {
    EatTerminal(TokKwContinue);
    EatTerminal(TokSemicolon);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtReturn()
{
  auto node = new StmtReturn();
  PushNode(node);
  {
    EatTerminal(TokKwReturn);
    EatTerminal(TokSemicolon);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtCommand(const vector<bool> &prototype)
{
  auto node = new StmtCommand;
  PushNode(node);
  {
    if (!prototype.empty() && prototype[0]) {
      lexer.FlagNextAsString(false, prototype.size() == 1);
    }

    EatTerminal(TokId, &node->name);

    for (size_t i = 0; i < prototype.size(); i++) {
      if (i != 0) {
        if (prototype[i]) {
          lexer.FlagNextAsString(false, i == prototype.size() - 1);
        }

        EatTerminal(TokComma);
      }
      
      auto arg = ParseExpr();
      node->args.push_back(arg);
    }

    // TODO: last expr/stmt doesnt need semi, unknown if this applies in this
    // scenario
    EatTerminal(TokSemicolon);
  }
  PopNode();
  return node;
}

Stmt *Parser::ParseStmtFunctionDecl()
{
  auto node = new StmtFunctionDecl;
  PushNode(node);
  {
    EatTerminal(TokKwFunction);
    EatTerminal(TokId, &node->name);
    EatTerminal(TokLeftParen);
    EatTerminal(TokRightParen);
    node->body = ParseStmt();
  }
  PopNode();
  return node;
}

// --------------------------------------------------
// Expressions
// --------------------------------------------------

Expr *Parser::ParseExpr(bool optional, int precedence)
{
  Expr *left = nullptr;

  switch (token) {
  case TokId:
    left = ParseExprId();
    break;
  case TokNumberLiteral:
    left = ParseExprNumberLiteral();
    break;
  case TokStringLiteral:
    left = ParseExprStringLiteral();
    break;
  case TokLeftParen:
    EatTerminal(TokLeftParen);
    left = ParseExpr();
    if(left)left->grouped = true;
    EatTerminal(TokRightParen);
    break;
  case TokLeftBrace:
    left = ParseExprList();
    break;
  case TokPipe:
    left = ParseExprRange();
    break;
  default:
    break;
  }

  if (!left && IsUnaryPrefix(token)) {
    left = ParseExprUnaryOp(nullptr, INT_MAX);
  }

  if (!left) {
    if (!optional) {
      diag.Error(terminal.token.range.beg, Range(), "expected expression");
    }
    return nullptr;
  }

  while (true) {
    if (IsBinaryOrTernaryOperator(token)) {
      auto p = GetOperatorPrecedence(token);
      if (p >= precedence) {
        left = ParseExprBinaryOp(left, p);
        continue;
      }
    }

    if (IsUnaryPostfix(token)) {
      left = ParseExprUnaryOp(left, 0);
      continue;
    }

    break;
  }

  return left;
}

Expr *Parser::ParseExprId()
{
  auto node = new ExprId;
  PushNode(node);
  {
    EatTerminal(TokId, &node->name);
  }
  PopNode();

  if (token == TokLeftBracket) {
    return ParseExprIndex(node);
  } else if (token == TokLeftParen) {
    return ParseExprCall(node);
  } else {
    return node;
  }
}

Expr *Parser::ParseExprNumberLiteral()
{
  auto node = new ExprNumberLiteral();
  PushNode(node);
  {
    EatTerminal(&node->literal);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprStringLiteral()
{
  auto node = new ExprStringLiteral;
  PushNode(node);
  {
    EatTerminal(&node->literal);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprList()
{
  auto node = new ExprList;
  PushNode(node);
  {
    EatTerminal(TokLeftBrace);

    while (token != TokRightBrace) {
      node->elements.push_back(ParseExpr());

      if (token == TokComma) {
        EatTerminal();
      } else {
        break;
      }
    }

    EatTerminal(TokRightBrace);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprRange()
{
  auto node = new ExprRange;
  PushNode(node);
  {
    EatTerminal(TokPipe);
    node->lower = ParseExpr(true);
    EatTerminal(TokComma);
    node->upper = ParseExpr(true);
    EatTerminal(TokPipe);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprUnaryOp(Expr *left, int precedence)
{
  auto node = new ExprUnaryOp;

  if (left) {
    AdoptNode(left, node);
    PushNode(node);
    {
      node->prefix = false;
      EatTerminal(&node->op);
      node->expr = left;
    }
    PopNode();
  } else {
    PushNode(node);
    {
      node->prefix = true;
      EatTerminal(&node->op);
      node->expr = ParseExpr(false, precedence);
    }
    PopNode();
  }

  return node;
}

Expr *Parser::ParseExprBinaryOp(Expr *left, int precedence)
{
  if (token == TokOpTernary) {
    return ParseExprTernaryOp(left, precedence);
  }

  auto node = new ExprBinaryOp;
  AdoptNode(left, node);
  PushNode(node);
  {
    node->left = left;
    EatTerminal(&node->op);
    node->right = ParseExpr(false, precedence);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprTernaryOp(Expr *left, int precedence)
{
  auto node = new ExprTernaryOp;
  AdoptNode(left, node);
  PushNode(node);
  {
    node->cond = left;
    EatTerminal(TokOpTernary);
    node->thenValue = ParseExpr(false, precedence);

    EatTerminal(TokColon);
    node->elseValue = ParseExpr(false, precedence);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprIndex(Expr *left)
{
  auto node = new ExprIndex;
  AdoptNode(left, node);
  PushNode(node);
  {
    node->left = left;
    EatTerminal(TokLeftBracket);
    node->index = ParseExpr();

    while (token == TokComma) {
        EatTerminal();

        auto more = ParseExpr();
        node->moreIndexes.push_back(more);
    }

    EatTerminal(TokRightBracket);
  }
  PopNode();

  if (token == TokDot)
    return ParseExprIndexDotLookup(node);
  else
    return node;
}

Expr *Parser::ParseExprIndexDotLookup(Expr *left)
{
  auto node = new ExprIndexDotLookup;
  AdoptNode(left, node);
  PushNode(node);
  {
    EatTerminal(TokDot);
    node->id = ParseExpr();
    //EatTerminal(TokId, &node->id);
  }
  PopNode();

  return node;
}

Expr *Parser::ParseExprCall(Expr *left)
{
  auto id = dynamic_cast<ExprId *>(left);
  if (id) {
    auto it = functions.find(id->name->token.text);
    if (it != functions.end()) {
      return ParseExprCallBuiltin(left, it->second);
    }
  }

  auto node = new ExprCall;
  AdoptNode(left, node);
  PushNode(node);
  {
    node->left = left;
    EatTerminal(TokLeftParen);

    bool hasComma = false;

    while (token != TokRightParen) {
      node->args.push_back(ParseExpr());

      if (token == TokComma) {
        EatTerminal();
        hasComma = true;
      } else {
        hasComma = false;
        break;
      }
    }

    if (hasComma) {
      auto comma = node->children.back().get();
      diag.Error(comma->GetRange().beg, Range(),
                 "trailing comma in argument list");
    }

    EatTerminal(TokRightParen);
  }
  PopNode();
  return node;
}

Expr *Parser::ParseExprCallBuiltin(Expr *left, const vector<bool> &prototype)
{
  auto node = new ExprCall;
  AdoptNode(left, node);
  PushNode(node);
  {
    node->left = left;

    if (!prototype.empty() && prototype[0]) {
      lexer.FlagNextAsString(true, prototype.size() == 1);
    }

    EatTerminal(TokLeftParen);

    for (size_t i = 0; i < prototype.size(); i++) {
      if (i != 0) {
        if (prototype[i]) {
          lexer.FlagNextAsString(true, i == prototype.size() - 1);
        }

        EatTerminal(TokComma);
      }

      node->args.push_back(ParseExpr());
    }

    EatTerminal(TokRightParen);
  }
  PopNode();
  return node;
}
