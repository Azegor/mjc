#include "return_visitor.hpp"

void ReturnAnalysisVisitor::visitMethod(ast::Method &method) {
  currentReturnType = method.getReturnType()->getSemaType();
  method.getBlock()->accept(this); // other children not necessary
}

void ReturnAnalysisVisitor::visitMainMethod(ast::MainMethod &mainMethod) {
  currentReturnType.setVoid();
  mainMethod.getBlock()->accept(this); // other children not necessary
}

void ReturnAnalysisVisitor::visitBlock(ast::Block &block) {
  block.acceptChildren(this);
  for (auto stmt : block.getStatements()) {
    if (stmt->cfb == sem::ControlFlowBehavior::Return) {
      block.cfb = sem::ControlFlowBehavior::Return;
      return; // no more checking necessary, remaining code is dead
    }
  }
  block.cfb = sem::ControlFlowBehavior::MayContinue;
}

void ReturnAnalysisVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  stmt.cfb = sem::ControlFlowBehavior::Return;
}
