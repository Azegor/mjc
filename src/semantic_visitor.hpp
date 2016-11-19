#ifndef FIND_DEFS_H
#define FIND_DEFS_H

#include "ast.hpp"
#include "error.hpp"
#include "symboltable.hpp"

#include <algorithm>

class SemanticVisitor : public ast::Visitor {
  ast::Program *currentProgram = nullptr;
  ast::Class *currentClass = nullptr;
  ast::Method *currentMethod = nullptr;
  bool mainMethodFound = false;
  SymbolTable::SymbolTable symTbl;

  static ast::DummySystem dummySystem;
  static ast::DummySystemOut dummySystemOut;

  ast::Class *findClassByName(const std::string &className) {
    // implemented with binary search. TODO: maybe consider a set instead
    auto &classes = currentProgram->getClasses();
    auto pos =
        std::lower_bound(classes.begin(), classes.end(), className,
                         [](const ast::ClassPtr &cls, const std::string &str) {
                           return *cls < str;
                         });
    if ((pos == classes.end()) || (className < **pos)) {
      return nullptr;
    }
    return pos->get();
  }

  ast::RegularMethod *findMethodInClass(ast::Class *klass,
                                        const std::string &methodName) {
    ast::RegularMethod *method = nullptr;
    for (auto &m : klass->getMethods()->methods) {
      if (m->getName() == methodName) {
        method = m.get();
        break;
      }
    }
    return method;
  }

public:
  SemanticVisitor(std::string fileName) : Visitor(std::move(fileName)) {}
  void visitProgram(ast::Program &program) override;
  void visitClass(ast::Class &klass) override;
  void visitFieldList(ast::FieldList &fieldList) override;
  void visitMethodList(ast::MethodList &methodList) override;
  void visitMainMethodList(ast::MainMethodList &mainMethodList) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void visitBlock(ast::Block &block) override;
  void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  void visitVarRef(ast::VarRef &varRef) override;
  void visitParameter(ast::Parameter &param) override;
  void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
  void visitClassType(ast::ClassType &type) override;
  void visitNewArrayExpression(ast::NewArrayExpression &expr) override;
  void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  void visitIfStatement(ast::IfStatement &ifStatement) override;
  void visitWhileStatement(ast::WhileStatement &stmt) override;
  void visitField(ast::Field &field) override;
  void visitReturnStatement(ast::ReturnStatement &stmt) override;
  void visitBinaryExpression(ast::BinaryExpression &expr) override;
  void visitIntLiteral(ast::IntLiteral &lit) override;
  void visitBoolLiteral(ast::BoolLiteral &lit) override;
  void visitNullLiteral(ast::NullLiteral &lit) override;
  void visitThisLiteral(ast::ThisLiteral &lit) override;
  void visitMainMethod(ast::MainMethod &mainMethod) override;
  void visitFieldAccess(ast::FieldAccess &access) override;
  void visitUnaryExpression(ast::UnaryExpression &expr) override;
  void visitArrayAccess(ast::ArrayAccess &access) override;

  virtual ~SemanticVisitor() {}

private:
  template <typename T> void checkForDuplicates(T &list) {
    std::stable_sort(list.begin(), list.end(), ast::SortUniquePtrPred());
    auto firstDuplicate =
        std::adjacent_find(list.begin(), list.end(), ast::UniquePtrEqPred());
    if (firstDuplicate != list.end()) {
      error(**++firstDuplicate, "invalid duplicate definition of field");
      // first defition is at **firstDuplicate
    }
  }
};

#endif // FIND_DEFS_H
