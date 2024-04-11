#include <gs1/compiler/DepthVisitor.hpp>
#include <gs1/parse/SyntaxTree.hpp>

using namespace gs1;

// --------------------------------------------------
// Core syntax tree structure
// --------------------------------------------------

void DepthVisitor::Visit(SyntaxNodeOrTerminal *node) {}

void DepthVisitor::Visit(SyntaxNode *node)
{
  for (auto child : node->children) {
    child->Accept(this);
  }

  Visit((SyntaxNodeOrTerminal *)node);
}

void DepthVisitor::Visit(SyntaxTerminal *node)
{
  Visit((SyntaxNodeOrTerminal *)node);
}

// --------------------------------------------------
// Statements
// --------------------------------------------------

void DepthVisitor::Visit(Stmt *node) { Visit((SyntaxNode *)node); }

void DepthVisitor::Visit(StmtEmpty *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtBlock *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtIf *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtFor *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtWhile *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtBreak *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtContinue *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtReturn *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtCommand *node) { Visit((Stmt *)node); }

void DepthVisitor::Visit(StmtFunctionDecl *node) { Visit((Stmt *)node); }

// --------------------------------------------------
// Expressions
// --------------------------------------------------

void DepthVisitor::Visit(Expr *node) { Visit((SyntaxNode *)node); }

void DepthVisitor::Visit(ExprId *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprNumberLiteral *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprStringLiteral *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprList *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprRange *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprUnaryOp *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprBinaryOp *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprTernaryOp *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprIndex *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprIndexDotLookup *node) { Visit((Expr *)node); }

void DepthVisitor::Visit(ExprCall *node) { Visit((Expr *)node); }
