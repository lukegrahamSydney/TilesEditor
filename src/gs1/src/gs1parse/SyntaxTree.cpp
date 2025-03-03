#include <gs1/parse/SyntaxTree.hpp>

using namespace gs1;

Range SyntaxNode::GetRange() const
{
  Range r;
  r.beg = children.front()->GetRange().beg;
  r.end = children.back()->GetRange().end;
  return r;
}

Range SyntaxNode::GetFullRange() const
{
  Range r;
  r.beg = children.front()->GetFullRange().beg;
  r.end = children.back()->GetFullRange().end;
  return r;
}

Range SyntaxTerminal::GetRange() const { return token.range; }

Range SyntaxTerminal::GetFullRange() const
{
  Range r;

  if (leadingTrivia.empty()) {
    r.beg = token.range.beg;
  } else {
    r.beg = leadingTrivia.front().range.beg;
  }

  if (trailingTrivia.empty()) {
    r.end = token.range.end;
  } else {
    r.end = trailingTrivia.back().range.end;
  }

  return r;
}
