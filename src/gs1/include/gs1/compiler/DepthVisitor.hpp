#ifndef GS1PARSE_DEPTHVISITOR_HPP
#define GS1PARSE_DEPTHVISITOR_HPP

#include <gs1/parse/SyntaxTreeVisitor.hpp>

namespace gs1
{
class DepthVisitor : public SyntaxTreeVisitor
{
public:
  virtual ~DepthVisitor() {}

  virtual void Visit(struct SyntaxNodeOrTerminal *node);

  virtual void Visit(struct SyntaxNode *node);

  virtual void Visit(struct SyntaxTerminal *node);

  virtual void Visit(struct Stmt *node);

  virtual void Visit(struct StmtEmpty *node);

  virtual void Visit(struct StmtBlock *node);

  virtual void Visit(struct StmtIf *node);

  virtual void Visit(struct StmtFor *node);

  virtual void Visit(struct StmtWhile *node);

  virtual void Visit(struct StmtBreak *node);

  virtual void Visit(struct StmtContinue *node);

  virtual void Visit(struct StmtReturn *node);

  virtual void Visit(struct StmtCommand *node);

  virtual void Visit(struct StmtFunctionDecl *node);

  virtual void Visit(struct Expr *node);

  virtual void Visit(struct ExprId *node);

  virtual void Visit(struct ExprNumberLiteral *node);

  virtual void Visit(struct ExprStringLiteral *node);

  virtual void Visit(struct ExprList *node);

  virtual void Visit(struct ExprRange *node);

  virtual void Visit(struct ExprUnaryOp *node);

  virtual void Visit(struct ExprBinaryOp *node);

  virtual void Visit(struct ExprTernaryOp *node);

  virtual void Visit(struct ExprIndex *node);

  virtual void Visit(struct ExprIndexDotLookup *node);

  virtual void Visit(struct ExprCall *node);
};
}

#endif
