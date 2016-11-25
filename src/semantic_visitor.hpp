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
  int mainMethodCount = 0;
  SymbolTable::SymbolTable symTbl;

  static ast::DummyDefinition dummySystem;
  static ast::DummySystemOut dummySystemOut;
  static ast::DummyDefinition dummyMainArgDef;

  ast::Class *findClassByName(const std::string &className) {
    return currentProgram->findClassByName(className);
  }

  ast::RegularMethod *findMethodInClass(ast::Class *klass,
                                        const std::string &methodName) {
    auto &methods = klass->getMethods()->methods;
    auto pos = std::lower_bound(
        methods.begin(), methods.end(), methodName,
        [](const ast::RegularMethodPtr &m, const std::string &str) {
          return m->getName() < str;
        });
    if ((pos == methods.end()) || (methodName < (*pos)->getName())) {
      return nullptr;
    }
    return pos->get();
  }

  ast::Field *findFieldInClass(ast::Class *klass,
                               const std::string &fieldName) {
    auto &fields = klass->getFields()->fields;
    auto pos =
        std::lower_bound(fields.begin(), fields.end(), fieldName,
                         [](const ast::FieldPtr &f, const std::string &str) {
                           return f->getName() < str;
                         });
    if ((pos == fields.end()) || (fieldName < (*pos)->getName())) {
      return nullptr;
    }
    return pos->get();
  }

  Lexer &lexer;

public:
  SemanticVisitor(Lexer &lexer) : lexer(lexer) {}
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
  template <typename T>
  void checkForDuplicates(T &list, const std::string &name) {
    std::stable_sort(list.begin(), list.end(), ast::SortUniquePtrPred());
    auto firstDuplicate =
        std::adjacent_find(list.begin(), list.end(), ast::UniquePtrEqPred());
    if (firstDuplicate != list.end()) {
      error(**++firstDuplicate, "invalid duplicate definition of " + name);
      // first defition is at **firstDuplicate
    }
  }

  [[noreturn]] void error(const ast::Node &node, std::string msg,
                          bool reportAtScopeEnd = false) {
    auto &loc = node.getLoc();
    auto &errorToken = reportAtScopeEnd ? loc.endToken : loc.startToken;
    throw ast::SemanticError(loc, lexer.getFilename(), std::move(msg),
                             lexer.getCurrentLineFromInput(errorToken.line),
                             reportAtScopeEnd);
  }
  [[noreturn]] void error(SourceLocation loc, std::string msg,
                          bool reportAtScopeEnd = false) {
    auto &errorToken = reportAtScopeEnd ? loc.endToken : loc.startToken;
    throw ast::SemanticError(loc, lexer.getFilename(), std::move(msg),
                             lexer.getCurrentLineFromInput(errorToken.line),
                             reportAtScopeEnd);
  }
};

#endif // FIND_DEFS_H
