#include "DebugVisitor.hpp"
#include <gs1/common/Log.hpp>

using namespace gs1;

DebugVisitor::DebugVisitor(ISource &source, bool printTerminals)
    : level(0), printTerminals(printTerminals), source(source)
{
}

void DebugVisitor::Visit(SyntaxTerminal *node)
{
  if (printTerminals) {
    Log::Get().Print(LOGLEVEL_VERBOSE, "%*s", level, "");
    Log::Get().Print(LOGLEVEL_VERBOSE, "* %s(%s)\n",
                     GetTokenTypeName(node->token.type),
                     node->token.text.c_str());
  }
}

// --------------------------------------------------
// Statements
// --------------------------------------------------

void DebugVisitor::Visit(StmtEmpty *node) { PrintNode(node, "StmtEmpty"); }

void DebugVisitor::Visit(StmtBlock *node) { PrintNode(node, "StmtBlock"); }

void DebugVisitor::Visit(StmtIf *node) { PrintNode(node, "StmtIf"); }

void DebugVisitor::Visit(StmtFor *node) { PrintNode(node, "StmtFor"); }

void DebugVisitor::Visit(StmtWhile *node) { PrintNode(node, "StmtWhile"); }

void DebugVisitor::Visit(StmtBreak *node) { PrintNode(node, "StmtBreak"); }

void DebugVisitor::Visit(StmtContinue *node)
{
  PrintNode(node, "StmtContinue");
}

void DebugVisitor::Visit(StmtCommand *node) { PrintNode(node, "StmtCommand"); }

void DebugVisitor::Visit(StmtFunctionDecl *node)
{
  PrintNode(node, "StmtFunctionDecl");
}

// --------------------------------------------------
// Expressions
// --------------------------------------------------

void DebugVisitor::Visit(ExprId *node) { PrintNode(node, "ExprId"); }

void DebugVisitor::Visit(ExprNumberLiteral *node)
{
  PrintNode(node, "ExprNumberLiteral");
}

void DebugVisitor::Visit(ExprStringLiteral *node)
{
  PrintNode(node, "ExprStringLiteral");
}

void DebugVisitor::Visit(ExprList *node) { PrintNode(node, "ExprList"); }

void DebugVisitor::Visit(ExprRange *node) { PrintNode(node, "ExprRange"); }

void DebugVisitor::Visit(ExprUnaryOp *node) { PrintNode(node, "ExprUnaryOp"); }

void DebugVisitor::Visit(ExprBinaryOp *node)
{
  PrintNode(node, "ExprBinaryOp");
}

void DebugVisitor::Visit(ExprTernaryOp *node)
{
  PrintNode(node, "ExprTernaryOp");
}

void DebugVisitor::Visit(ExprIndex *node) { PrintNode(node, "ExprIndex"); }

void DebugVisitor::Visit(ExprCall *node) { PrintNode(node, "ExprCall"); }

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

void DebugVisitor::PrintNode(SyntaxNode *node, const char *name)
{
  auto text = source.GetRangeContents(node->GetRange());

  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == '\r' || text[i] == '\n') {
      text[i] = ' ';
    }
  }

  if (text.size() > 64) {
    text.resize(64);
    text += "...";
  }

  Log::Get().Print(LOGLEVEL_VERBOSE, "%*s", level, "");
  Log::Get().Print(LOGLEVEL_VERBOSE, "* %s(%s)\n", name, text.c_str());

  level += 2;
  SyntaxTreeVisitor::Visit(node);
  level -= 2;
}
