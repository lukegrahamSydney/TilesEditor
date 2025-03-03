#include <cstdarg>

#include <gs1/common/Log.hpp>
#include <gs1/compiler/CompileVisitor.hpp>

#include <gs1/parse/SyntaxTreeVisitor.hpp>

using namespace gs1;

CompileVisitor::CompileVisitor(ISource &source, bool printTerminals)
    : level(0), printTerminals(printTerminals), source(source)
{
}

void CompileVisitor::Visit(SyntaxTerminal *node)
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

void CompileVisitor::Visit(StmtEmpty *node)
{
  PrintEnterNode(node, "StmtEmpty");

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtBlock *node)
{
  PrintEnterNode(node, "StmtBlock");

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtIf *node)
{
  PrintEnterNode(node, "StmtIf");

  // Write the condition
  node->cond->Accept(this);

  // Jump past the body if the condition is false
  body.Emit(OP_JEZ);

  // Reserve our offset for how far to jump
  Reservation offsetReservation = body.Reserve(4);

  // Write "then" body
  node->thenBody->Accept(this);

  // If there's an else body, we need to jump past it now,
  // as we're still enclosed in the "then" body
  if (node->elseBody != nullptr) {
    // We don't know how far to jump yet
    body.Emit(OP_JMP);
    Reservation elseOffsetReservation = body.Reserve(4);

    // Write the offset to jump past the if-body (and into the else body)
    offsetReservation.Emit(body.GetCurrentPosition() -
                           offsetReservation.GetPosition());
    Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                     offsetReservation.GetPosition(),
                     body.GetCurrentPosition());

    // Write the else body
    node->elseBody->Accept(this);

    // Fill the reservation for our jump to skip the else
    elseOffsetReservation.Emit(body.GetCurrentPosition() -
                               elseOffsetReservation.GetPosition());
    Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING ELSE OFFSET JUMP %d TO: %d\n",
                     elseOffsetReservation.GetPosition(),
                     body.GetCurrentPosition());
  } else {
    // Write the offset to jump past the if-body
    offsetReservation.Emit(body.GetCurrentPosition() -
                           offsetReservation.GetPosition());
    Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                     offsetReservation.GetPosition(),
                     body.GetCurrentPosition());
  }

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtFor *node)
{
  PrintEnterNode(node, "StmtFor");

  // Emit initialization
  node->init->Accept(this);

  uint32_t stepConditionPosition = body.GetCurrentPosition();

  // Emit condition
  node->cond->Accept(this);

  // Jump out if step condition fails
  body.Emit(OP_JEZ);
  Reservation failReservation = body.Reserve(4);

  // Emit body
  node->body->Accept(this);

  // Emit step
  node->step->Accept(this);

  // Jump back to step condition
  body.Emit(OP_JMP);
  body.Emit(stepConditionPosition - body.GetCurrentPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   body.GetCurrentPosition(), stepConditionPosition);

  failReservation.Emit(body.GetCurrentPosition() -
                       failReservation.GetPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   failReservation.GetPosition(), body.GetCurrentPosition());

  // Set "break" location
  node->breakPosition = body.GetCurrentPosition();

  // Set "continue" location
  node->continuePosition = stepConditionPosition;

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtWhile *node)
{
  PrintEnterNode(node, "StmtWhile");

  uint32_t conditionPosition = body.GetCurrentPosition();

  // Emit condition
  node->cond->Accept(this);

  // Jump out if step condition fails
  body.Emit(OP_JEZ);
  Reservation failReservation = body.Reserve(4);

  // Emit body
  node->body->Accept(this);

  // Jump back to condition check
  body.Emit(OP_JMP);
  body.Emit(conditionPosition - body.GetCurrentPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   body.GetCurrentPosition(), conditionPosition);

  failReservation.Emit(body.GetCurrentPosition() -
                       failReservation.GetPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   failReservation.GetPosition(), body.GetCurrentPosition());

  // Set "break" location
  node->breakPosition = body.GetCurrentPosition();

  // Set "continue" location
  node->continuePosition = conditionPosition;

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtBreak *node)
{
  PrintEnterNode(node, "StmtBreak");

  // Iterate up tree until we find the parent loop
  SyntaxNode *parent = node->parent;
  while (parent != nullptr) {
    if (parent->GetType() == "StmtWhile" || parent->GetType() == "StmtFor") {
      // Emit jump out of bytecode to continue location

      unsigned int breakPosition = ((StmtLoop *)parent)->continuePosition;

      body.Emit(OP_JMP);
      body.Emit(body.GetCurrentPosition() - breakPosition);
      Log::Get().Print(LOGLEVEL_VERBOSE,
                       "BREAK: PRINTING OFFSET JUMP %d TO: %d\n",
                       body.GetCurrentPosition(), breakPosition);

      break;
    }

    parent = parent->parent;
  }

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtContinue *node)
{
  PrintEnterNode(node, "StmtContinue");

  // Iterate up tree until we find the parent loop
  SyntaxNode *parent = node->parent;
  while (parent != nullptr) {
    if (parent->GetType() == "StmtWhile" || parent->GetType() == "StmtFor") {
      // Emit jump out of bytecode to continue location

      unsigned int continuePosition = ((StmtLoop *)parent)->continuePosition;

      body.Emit(OP_JMP);
      body.Emit(body.GetCurrentPosition() - continuePosition);
      Log::Get().Print(LOGLEVEL_VERBOSE,
                       "CONTINUE: PRINTING OFFSET JUMP %d TO: %d\n",
                       body.GetCurrentPosition(), continuePosition);

      break;
    }

    parent = parent->parent;
  }

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtReturn *node)
{
  PrintEnterNode(node, "StmtReturn");

  // Emit return opcode
  body.Emit(OP_RET);

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtCommand *node)
{
  PrintEnterNode(node, "StmtCommand");

  // Table command name
  std::string &commandName = node->name->token.text;
  ConstantKey nameKey = header.constStringTable->GetKey(commandName);

  // Emit arguments
  SyntaxTreeVisitor::Visit(node);

  // Emit call and command name
  body.Emit(OP_CMD_CALL);
  body.Emit(PackedValue(PACKVALUE_CONST_STRING, nameKey.index));

  PrintLeaveNode();
}

void CompileVisitor::Visit(StmtFunctionDecl *node)
{
  PrintEnterNode(node, "StmtFunctionDecl");

  // Table function decl name
  std::string &funcName = node->name->token.text;
  header.constStringTable->GetKey(funcName);

  Print("Function Decl found: %s", funcName.c_str());

  auto reservation = body.BeginFunction(funcName);

  SyntaxTreeVisitor::Visit(node);

  body.EndFunction(reservation);

  PrintLeaveNode();
}

// --------------------------------------------------
// Expressions
// --------------------------------------------------

void CompileVisitor::Visit(ExprId *node)
{
  PrintEnterNode(node, "ExprId");

  // Table id name
  std::string &idName = node->name->token.text;
  ConstantKey key = header.constStringTable->GetKey(idName);

  // Push string literal onto stack
  body.Emit(OP_PUSH);
  body.Emit(PackedValue(PACKVALUE_NAMED, key.index));

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprNumberLiteral *node)
{
  PrintEnterNode(node, "ExprNumberLiteral");

  // Table number literal
  float num = std::stof(node->literal->token.text);
  ConstantKey key = header.constNumberTable->GetKey(num);

  // Push number literal onto stack
  body.Emit(OP_PUSH);
  body.Emit(PackedValue(PACKVALUE_CONST_NUMBER, key.index));

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprStringLiteral *node)
{
  PrintEnterNode(node, "ExprStringLiteral");

  // Table string literal
  std::string str = node->literal->token.text;
  ConstantKey key = header.constStringTable->GetKey(str);

  // Push string literal onto stack
  body.Emit(OP_PUSH);
  body.Emit(PackedValue(PACKVALUE_CONST_STRING, key.index));

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprList *node)
{
  PrintEnterNode(node, "ExprList");

  SyntaxTreeVisitor::Visit(node);

  // Table size of array
  uint32_t size = node->elements.size();
  ConstantKey key = header.constNumberTable->GetKey(size);

  // Push size of array onto stack
  body.Emit(OP_PUSH);
  body.Emit(PackedValue(PACKVALUE_CONST_ARRAY, key.index));

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprRange *node)
{
  PrintEnterNode(node, "ExprRange");

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprUnaryOp *node)
{
  PrintEnterNode(node, "ExprUnaryOp");

  // Emit operands
  SyntaxTreeVisitor::Visit(node);

  // Check if parent operator uses the value or not
  // Only expressions use the value
  // Ternary expressions are an exception.
  // Check if the value is used at any point in a chain of ternary
  // expressions.
  SyntaxNode *parent = node->parent;
  bool needsPushedResult = false;

  while (!needsPushedResult) {
    if (parent->GetType() == "ExprTernaryOp") {
      parent = parent->parent;
      continue;
    }

    if (parent->GetBaseType() == "Expr")
      needsPushedResult = true;
    else
      break;
  }

  // Emit operation
  if (needsPushedResult) {
    switch (node->op->token.type) {
    case TokOpIncrement:
      body.Emit(OP_INCPUSH);
      break;

    case TokOpDecrement:
      body.Emit(OP_DECPUSH);
      break;

    default:
      break;
    }
  } else {
    switch (node->op->token.type) {
    case TokOpIncrement:
      body.Emit(OP_INC);
      break;

    case TokOpDecrement:
      body.Emit(OP_DEC);
      break;

    default:
      break;
    }
  }

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprBinaryOp *node)
{
  PrintEnterNode(node, "ExprBinaryOp");

  // Special logic for short-circuit logical ops
  if (node->op->token.type == TokOpAnd || node->op->token.type == TokOpOr) {
    // Emit left-side condition
    node->left->Accept(this);

    if (node->op->token.type == TokOpAnd) {
      // If "and", evaluate this condition and early-out if false
      // Write a jump at the end of the left-hand condition
      body.Emit(OP_JEZ);
      Reservation leftFailReservation = body.Reserve(4);

      // Write the other condition
      node->right->Accept(this);

      // Write a jump at the end of the right-hand condition
      body.Emit(OP_JEZ);
      Reservation rightFailReservation = body.Reserve(4);

      // Push a 1, this is the success block
      body.Emit(OP_PUSH);
      body.Emit(PackedValue(PACKVALUE_CONST_NUMBER,
                            header.constNumberTable->GetKey(1.0f).index));

      // Evaluated to true, jump to exit
      body.Emit(OP_JMP);
      Reservation successReservation = body.Reserve(4);

      // This is where we jump if it's false
      leftFailReservation.Emit(body.GetCurrentPosition() -
                               leftFailReservation.GetPosition());
      Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                       leftFailReservation.GetPosition(),
                       body.GetCurrentPosition());

      rightFailReservation.Emit(body.GetCurrentPosition() -
                                rightFailReservation.GetPosition());
      Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET FJUMP %d TO: %d\n",
                       rightFailReservation.GetPosition(),
                       body.GetCurrentPosition());

      // Push a zero, this is the failure block
      body.Emit(OP_PUSH);
      body.Emit(PackedValue(PACKVALUE_CONST_NUMBER,
                            header.constNumberTable->GetKey(0.0f).index));

      successReservation.Emit(body.GetCurrentPosition() -
                              successReservation.GetPosition());
      Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET SJUMP %d TO: %d\n",
                       successReservation.GetPosition(),
                       body.GetCurrentPosition());
    } else if (node->op->token.type == TokOpOr) {
      // If "or", evaluate this condition and early-IN (short-circuit) if true
      // Write a jump at the end of the left-hand condition
      // If it's false, we just pass through to the right-hand condition
      body.Emit(OP_JNZ);
      Reservation leftJumpReservation = body.Reserve(4);

      // Write the other condition
      node->right->Accept(this);

      // Write a jump at the end of the right-hand condition
      // This jump will pass through if true.
      body.Emit(OP_JEZ);
      Reservation rightJumpReservation = body.Reserve(4);

      // Fill the early-in reservation
      leftJumpReservation.Emit(body.GetCurrentPosition() -
                               leftJumpReservation.GetPosition());
      Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                       leftJumpReservation.GetPosition(),
                       body.GetCurrentPosition());

      // This is where we jump to if the right-hand pass-through failed
      rightJumpReservation.Emit(body.GetCurrentPosition() -
                                rightJumpReservation.GetPosition());
      Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                       rightJumpReservation.GetPosition(),
                       body.GetCurrentPosition());

      // Push a zero, this is the failure block
      body.Emit(0);
    }

    return;
  }

  // Assign needs special handling for array assignments
  if (node->op->token.type == TokOpAssign) {
    // Check if lhand has an array
    if (node->left->GetType() == "ExprIndex") {
      // Pushes ID
      ((ExprIndex *)node->left)->left->Accept(this);

      // Pushes index
      ((ExprIndex *)node->left)->index->Accept(this);

      // Pushes rValue
      node->right->Accept(this);

      body.Emit(OP_ARR_SET);

      return;
    }
  }

  // Emit extra left operand for OpAssign
  switch (node->op->token.type) {
  case TokOpAddAssign:
  case TokOpSubAssign:
  case TokOpMulAssign:
  case TokOpDivAssign:
  case TokOpModAssign:
  case TokOpPowAssign:
    // Pushes ID
    ((ExprId *)node->left)->Accept(this);

  default:
    break;
  }

  // Emit operands
  SyntaxTreeVisitor::Visit(node);

  // Emit operator
  switch (node->op->token.type) {
  case TokOpAssign:
    body.Emit(OP_ASSIGN);
    break;

  case TokOpEquals:
    body.Emit(OP_EQ);
    break;

  case TokOpLessThan:
    body.Emit(OP_LT);
    break;

  case TokOpGreaterThan:
    body.Emit(OP_GT);
    break;

  case TokOpAdd:
  case TokOpAddAssign:
    body.Emit(OP_ADD);
    break;

  case TokOpSub:
  case TokOpSubAssign:
    body.Emit(OP_SUB);
    break;

  case TokOpMul:
  case TokOpMulAssign:
    body.Emit(OP_MUL);
    break;

  case TokOpDiv:
  case TokOpDivAssign:
    body.Emit(OP_DIV);
    break;

  case TokOpMod:
  case TokOpModAssign:
    body.Emit(OP_MOD);
    break;

  case TokOpPow:
  case TokOpPowAssign:
    body.Emit(OP_POW);
    break;

  default:
    break;
  }

  // Emit OpAssign
  switch (node->op->token.type) {
  case TokOpAddAssign:
  case TokOpSubAssign:
  case TokOpMulAssign:
  case TokOpDivAssign:
  case TokOpModAssign:
  case TokOpPowAssign:
    body.Emit(OP_ASSIGN);

  default:
    break;
  }

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprTernaryOp *node)
{
  PrintEnterNode(node, "ExprTernaryOp");

  // Write the condition
  node->cond->Accept(this);

  // Jump past the left statement if the condition is false
  body.Emit(OP_JEZ);

  // Reserve our offset for how far to jump
  Reservation failReservation = body.Reserve(4);

  // Write "then" body
  node->thenValue->Accept(this);

  // Write jump to end
  body.Emit(OP_JMP);

  Reservation successReservation = body.Reserve(4);

  // Fill "fail" reservation
  failReservation.Emit(body.GetCurrentPosition() -
                       failReservation.GetPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   failReservation.GetPosition(), body.GetCurrentPosition());

  // Write "else" body
  node->elseValue->Accept(this);

  // Set success jump to here
  successReservation.Emit(body.GetCurrentPosition() -
                          successReservation.GetPosition());
  Log::Get().Print(LOGLEVEL_VERBOSE, "PRINTING OFFSET JUMP %d TO: %d\n",
                   successReservation.GetPosition(), body.GetCurrentPosition());

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprIndex *node)
{
  PrintEnterNode(node, "ExprIndex");

  // Array ID is already on the stack

  // Pushes the index onto the stack
  SyntaxTreeVisitor::Visit(node);

  // Emit the array lookup opcode
  body.Emit(OP_ARR_GET);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprIndexDotLookup *node)
{
  PrintEnterNode(node, "ExprIndexDotLookup");

  SyntaxTreeVisitor::Visit(node);

  PrintLeaveNode();
}

void CompileVisitor::Visit(ExprCall *node)
{
  PrintEnterNode(node, "ExprCall");

  std::string &commandName = ((ExprId *)node->left)->name->token.text;
  ConstantKey nameKey = header.constStringTable->GetKey(commandName);

  // Visit argument nodes
  for (auto &arg : node->args)
    arg->Accept(this);

  // Emit call and function name
  body.Emit(OP_CALL);
  body.Emit(PackedValue(PACKVALUE_CONST_STRING, nameKey.index));

  PrintLeaveNode();
}

ByteBuffer CompileVisitor::GetBytecode()
{
  ByteBuffer headerBuffer = header.GetByteBuffer();
  ByteBuffer bodyBuffer = body.GetByteBuffer();

  headerBuffer.WriteBytes(bodyBuffer.GetBytes(), bodyBuffer.GetLength());

  return headerBuffer;
}

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

void CompileVisitor::PrintEnterNode(SyntaxNode *node, const char *name)
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
}

void CompileVisitor::Print(const char *fmt, ...)
{
  static const int bufferSize = 1024;
  char message[bufferSize];

  va_list args;
  va_start(args, fmt);
  vsnprintf(message, bufferSize, fmt, args);
  va_end(args);

  Log::Get().Print(LOGLEVEL_VERBOSE, "%*s", level, "");
  Log::Get().Print(LOGLEVEL_VERBOSE, "* %s\n", message);
}

void CompileVisitor::PrintLeaveNode() { level -= 2; }
