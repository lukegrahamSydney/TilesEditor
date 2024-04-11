#include <gs1/parse/SyntaxTree.hpp>

using namespace gs1;

// --------------------------------------------------
// Core syntax tree structure
// --------------------------------------------------

void SyntaxTreeVisitor::Visit(SyntaxNode *node)
{
  Visit((SyntaxNodeOrTerminal *)node);

  for (auto child : node->children) {
    child->Accept(this);
  }
}
