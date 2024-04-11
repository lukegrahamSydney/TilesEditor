#ifndef GS1PARSE_SYNTAXTREEVISITOR_HPP
#define GS1PARSE_SYNTAXTREEVISITOR_HPP

namespace gs1
{

class SyntaxNodeOrTerminal;
class SyntaxNode;
class Stmt;
class Expr;

class SyntaxTreeVisitor
{
public:
  SyntaxTreeVisitor() {  }
  virtual ~SyntaxTreeVisitor() {  }

  virtual void Visit(struct SyntaxNodeOrTerminal *node) {}

  virtual void Visit(struct SyntaxNode *node);

  virtual void Visit(struct SyntaxTerminal *node)
  {
    Visit((SyntaxNodeOrTerminal *)node);
  }

  // --------------------------------------------------
  // Statements
  // --------------------------------------------------

  virtual void Visit(struct Stmt *node) { Visit((SyntaxNode *)node); }

  virtual void Visit(struct StmtEmpty *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtBlock *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtIf *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtFor *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtWhile *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtBreak *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtContinue *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtReturn *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtCommand *node) { Visit((Stmt *)node); }

  virtual void Visit(struct StmtFunctionDecl *node) { Visit((Stmt *)node); }

  // --------------------------------------------------
  // Expressions
  // --------------------------------------------------

  virtual void Visit(struct Expr *node) { Visit((SyntaxNode *)node); };

  virtual void Visit(struct ExprId *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprNumberLiteral *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprStringLiteral *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprList *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprRange *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprUnaryOp *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprBinaryOp *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprTernaryOp *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprIndex *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprIndexDotLookup *node) { Visit((Expr *)node); }

  virtual void Visit(struct ExprCall *node) { Visit((Expr *)node); }
};
}

#endif
