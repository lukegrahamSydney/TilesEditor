#ifndef GS1PARSE_SYNTAXTREE_HPP
#define GS1PARSE_SYNTAXTREE_HPP

#include <gs1/parse/SyntaxTreeVisitor.hpp>
#include <gs1/parse/Token.hpp>

namespace gs1
{
struct SyntaxNode;
struct SyntaxTerminal;
struct Stmt;
struct Expr;

// --------------------------------------------------
// Core syntax tree structure
// --------------------------------------------------

struct SyntaxNodeOrTerminal {
  virtual ~SyntaxNodeOrTerminal() {}

  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual Range GetRange() const = 0;
  virtual Range GetFullRange() const = 0;
  virtual std::string GetType() const { return "SyntaxNodeOrTerminal"; };
  virtual bool hasEndOfLine() const { return true; }
  SyntaxNode *parent;
};

struct SyntaxNode : public SyntaxNodeOrTerminal {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual Range GetRange() const;
  virtual Range GetFullRange() const;

  virtual std::string GetBaseType() const { return "SyntaxNode"; };

  std::string GetType() const override { return "SyntaxNode"; };

  vector<shared_ptr<SyntaxNodeOrTerminal>> children;
};

struct SyntaxTerminal : public SyntaxNodeOrTerminal {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual Range GetRange() const;
  virtual Range GetFullRange() const;
  std::string GetType() const { return "SyntaxTerminal"; };
  bool hasEndOfLine() const override { return false; }
  Token token;
  vector<Token> leadingTrivia;
  vector<Token> trailingTrivia;
};

// --------------------------------------------------
// Statements
// --------------------------------------------------

struct Stmt : public SyntaxNode {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetBaseType() const { return "Stmt"; };

  virtual std::string GetType() const { return "Stmt"; };
};

struct StmtEmpty : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }
    bool hasEndOfLine() const override { return false; }
  virtual std::string GetType() const { return "StmtEmpty"; };
};

struct StmtBlock : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtBlock"; };
  bool hasEndOfLine() const override { return false; }
  vector<Stmt *> statements;

};

struct StmtIf : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtIf"; };
  bool hasEndOfLine() const override { return false; }

  Expr *cond;
  Stmt *thenBody;
  Stmt *elseBody;
};

struct StmtLoop : public Stmt {
  unsigned int breakPosition;
  unsigned int continuePosition;
  bool hasEndOfLine() const override { return false; }
};

struct StmtFor : public StmtLoop {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtFor"; };

  Expr *init;
  Expr *cond;
  Expr *step;
  Stmt *body;
};

struct StmtWhile : public StmtLoop {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtWhile"; };
  Expr *cond;
  Stmt *body;
};

struct StmtBreak : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtBreak"; };
};

struct StmtContinue : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtContinue"; };
};

struct StmtReturn : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtReturn"; };
};

struct StmtCommand : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtCommand"; };

  SyntaxTerminal *name;
  vector<Expr *> args;
};

struct StmtFunctionDecl : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "StmtFunctionDecl"; };
  bool hasEndOfLine() const override { return false; }
  SyntaxTerminal *name;
  Stmt *body;
};

// --------------------------------------------------
// Expressions
// --------------------------------------------------

struct Expr : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetBaseType() const { return "Expr"; };

  virtual std::string GetType() const { return "Expr"; };

  bool grouped = false;
  bool isAction = false;
  bool dontAddThis = false;
};

struct ExprId : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprId"; };

  SyntaxTerminal *name;
};

struct ExprNumberLiteral : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprNumberLiteral"; };

  SyntaxTerminal *literal;
};

struct ExprStringLiteral : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprStringLiteral"; };

  SyntaxTerminal *literal;
};

struct ExprList : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprList"; };

  vector<Expr *> elements;
};

struct ExprRange : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprRange"; };

  Expr*lower;
  Expr*upper;
};

struct ExprUnaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprUnaryOp"; };

  bool prefix;
  SyntaxTerminal *op;
  Expr *expr;
};

struct ExprBinaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprBinaryOp"; };

  SyntaxTerminal *op;
  Expr *left;
  Expr *right;
};

struct ExprTernaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprTernaryOp"; };

  Expr *cond;
  Expr *thenValue;
  Expr *elseValue;
};

struct ExprIndex : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprIndex"; };

  Expr *left;
  Expr *index;
  vector<Expr*> moreIndexes;
};

struct ExprIndexDotLookup : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprIndexDotLookup"; };

  Expr *id;
};

struct ExprCall : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetType() const { return "ExprCall"; };

  Expr *left;
  vector<Expr *> args;
};
}

#endif
