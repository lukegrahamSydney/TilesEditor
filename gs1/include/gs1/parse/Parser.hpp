#ifndef GS1PARSE_PARSER_HPP
#define GS1PARSE_PARSER_HPP

#include <gs1/parse/Lexer.hpp>
#include <gs1/parse/SyntaxTree.hpp>

namespace gs1
{
typedef unordered_map<string, vector<bool>> PrototypeMap;

class Parser
{
public:
  Parser(DiagBuilder &diag, Lexer &lexer, const PrototypeMap &commands,
         const PrototypeMap &functions);
  shared_ptr<SyntaxNode> Parse();

private:
  void EatTerminal(SyntaxTerminal **ref = nullptr);
  void EatTerminal(TokenType type, SyntaxTerminal **ref = nullptr);

  void AdoptNode(SyntaxNode *child, SyntaxNode *newParent);
  void PushNode(SyntaxNode *node);
  void PopNode();

  // --------------------------------------------------
  // Statements
  // --------------------------------------------------

  Stmt *ParseStmt(bool optional = false);
  Stmt *ParseStmtEmpty();
  Stmt *ParseStmtBlock();
  Stmt *ParseStmtIf();
  Stmt *ParseStmtFor();
  Stmt *ParseStmtWhile();
  Stmt *ParseStmtBreak();
  Stmt *ParseStmtContinue();
  Stmt *ParseStmtReturn();
  Stmt *ParseStmtCommand(const vector<bool> &prototype);
  Stmt *ParseStmtFunctionDecl();

  // --------------------------------------------------
  // Expressions
  // --------------------------------------------------

  Expr *ParseExpr(bool optional = false, int precedence = 0);
  Expr *ParseExprId();
  Expr *ParseExprNumberLiteral();
  Expr *ParseExprStringLiteral();
  Expr *ParseExprList();
  Expr *ParseExprRange();
  Expr *ParseExprUnaryOp(Expr *left, int precedence);
  Expr *ParseExprBinaryOp(Expr *left, int precedence);
  Expr *ParseExprTernaryOp(Expr *left, int precedence);
  Expr *ParseExprIndex(Expr *left);
  Expr *ParseExprIndexDotLookup(Expr *left);
  Expr *ParseExprCall(Expr *left);
  Expr *ParseExprCallBuiltin(Expr *left, const vector<bool> &prototype);

  Lexer &lexer;
  DiagBuilder &diag;
  const TokenType &token;
  SyntaxTerminal terminal;
  vector<SyntaxNode *> stack;
  const PrototypeMap &commands;
  const PrototypeMap &functions;
};
}

#endif
