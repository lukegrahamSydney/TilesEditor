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

  std::string GetType() const override { return "SyntaxTerminal"; };
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

  std::string GetType() const { return "Stmt"; };
};

struct StmtEmpty : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }
    bool hasEndOfLine() const override { return false; }
    std::string GetType() const override { return "StmtEmpty"; };
};

struct StmtBlock : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtBlock"; };
  bool hasEndOfLine() const override { return false; }
  vector<Stmt *> statements;

};

struct StmtIf : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtIf"; };
  bool hasEndOfLine() const override { return false; }

  Expr *cond = nullptr;
  Stmt *thenBody = nullptr;
  Stmt *elseBody = nullptr;
};

struct StmtWith : public Stmt {
    virtual void Accept(SyntaxTreeVisitor* v) { v->Visit(this); }

    std::string GetType() const override { return "StmtWith"; };
    bool hasEndOfLine() const override { return false; }

    Stmt* init = nullptr;
    Stmt* body = nullptr;
};

struct StmtLoop : public Stmt {
  unsigned int breakPosition;
  unsigned int continuePosition;
  bool hasEndOfLine() const override { return false; }
};

struct StmtFor : public StmtLoop {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtFor"; };

  Expr *init = nullptr;
  Expr *cond = nullptr;
  Expr *step = nullptr;
  Stmt *body = nullptr;
};

struct StmtWhile : public StmtLoop {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtWhile"; };
  Expr *cond = nullptr;
  Stmt *body = nullptr;
};

struct StmtBreak : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtBreak"; };
};

struct StmtContinue : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtContinue"; };
};

struct StmtReturn : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtReturn"; };
};

struct StmtCommand : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtCommand"; };

  SyntaxTerminal *name = nullptr;
  vector<Expr *> args;
};

struct StmtFunctionDecl : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "StmtFunctionDecl"; };
  bool hasEndOfLine() const override { return false; }
  SyntaxTerminal *name = nullptr;
  Stmt *body;
};

// --------------------------------------------------
// Expressions
// --------------------------------------------------

struct Expr : public Stmt {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  virtual std::string GetBaseType() const { return "Expr"; };

  std::string GetType() const override { return "Expr"; };

  bool grouped = false;
  bool isAction = false;
  bool dontAddThis = false;
};

struct ExprId : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprId"; };

  SyntaxTerminal *name;
};

struct ExprNumberLiteral : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprNumberLiteral"; };

  SyntaxTerminal *literal = nullptr;
};

struct ExprStringLiteral : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprStringLiteral"; };

  SyntaxTerminal *literal = nullptr;
};

struct ExprList : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprList"; };

  vector<Expr *> elements;
};

struct ExprRange : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprRange"; };

  Expr*lower = nullptr;
  Expr*upper = nullptr;
};

struct ExprUnaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprUnaryOp"; };

  bool prefix;
  SyntaxTerminal *op = nullptr;
  Expr *expr = nullptr;
};

struct ExprBinaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprBinaryOp"; };

  SyntaxTerminal *op = nullptr;
  Expr *left = nullptr;
  Expr *right = nullptr;
};

struct ExprTernaryOp : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprTernaryOp"; };

  Expr *cond = nullptr;
  Expr *thenValue = nullptr;
  Expr *elseValue = nullptr;
};

struct ExprIndex : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprIndex"; };

  Expr *left = nullptr;
  Expr *index = nullptr;
  vector<Expr*> moreIndexes;
};

struct ExprIndexDotLookup : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprIndexDotLookup"; };

  Expr *id = nullptr;
};

struct ExprCall : public Expr {
  virtual void Accept(SyntaxTreeVisitor *v) { v->Visit(this); }

  std::string GetType() const override { return "ExprCall"; };

  Expr *left = nullptr;
  vector<Expr *> args;
};
}

#endif
