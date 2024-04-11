#ifndef GS1COMPILER_COMPILEVISITOR_HPP
#define GS1COMPILER_COMPILEVISITOR_HPP

#include <gs1/compiler/BytecodeBody.hpp>
#include <gs1/compiler/BytecodeHeader.hpp>
#include <gs1/compiler/DepthVisitor.hpp>
#include <gs1/parse/Source.hpp>
#include <gs1/parse/SyntaxTree.hpp>

namespace gs1
{
class CompileVisitor : public DepthVisitor
{
public:
  CompileVisitor(ISource &source, bool printTerminals = false);

  void Visit(SyntaxTerminal *node);

  void Visit(StmtEmpty *node);

  void Visit(StmtBlock *node);

  void Visit(StmtIf *node);

  void Visit(StmtFor *node);

  void Visit(StmtWhile *node);

  void Visit(StmtBreak *node);

  void Visit(StmtContinue *node);

  void Visit(StmtReturn *node);

  void Visit(StmtCommand *node);

  void Visit(StmtFunctionDecl *node);

  void Visit(ExprId *node);

  void Visit(ExprNumberLiteral *node);

  void Visit(ExprStringLiteral *node);

  void Visit(ExprList *node);

  void Visit(ExprRange *node);

  void Visit(ExprUnaryOp *node);

  void Visit(ExprBinaryOp *node);

  void Visit(ExprTernaryOp *node);

  void Visit(ExprIndex *node);

  void Visit(ExprIndexDotLookup *node);

  void Visit(ExprCall *node);

  ByteBuffer GetBytecode();

private:
  BytecodeHeader header;
  BytecodeBody body;

  void PrintEnterNode(SyntaxNode *node, const char *name);

  void Print(const char *fmt, ...);

  void PrintLeaveNode();

  int level;
  bool printTerminals;
  ISource &source;
};
}

#endif
