#ifndef FIND_DEFS_H
#define FIND_DEFS_H

#include "ast.hpp"
#include "error.hpp"
#include "symboltable.hpp"

#include <algorithm>

class SemanticError : public CompilerError {
public:
  const SourceLocation srcLoc;
  const std::string filename, message;
  SemanticError(SourceLocation srcLoc, std::string filename,
                std::string message)
      : srcLoc(std::move(srcLoc)), filename(std::move(filename)),
        message(std::move(message)) {}
  const char *what() const noexcept override { return message.c_str(); }

  virtual void writeErrorMessage(std::ostream &out) const override {
    co::color_ostream<std::ostream> cl_out(out);
    cl_out << co::mode(co::bold) << filename << ':' << srcLoc.startToken.line
           << ':' << srcLoc.startToken.col << ": " << co::color(co::red)
           << "error: " << co::reset << message << std::endl;
    //     writeErrorLineHighlight(out);
  }
};

class FindDefsVisitor : public ast::Visitor {
  std::string fileName;
  ast::Program *currentProgram = nullptr;
  ast::Class *currentClass = nullptr;
  SymbolTable::SymbolTable symTbl;

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

public:
  FindDefsVisitor(std::string fileName) : fileName(std::move(fileName)) {}
  void visitProgram(ast::Program &program) override;
  void visitClass(ast::Class &klass) override;
  void visitFieldList(ast::FieldList &fieldList) override;
  void visitMethodList(ast::MethodList &methodList) override;
  void visitMainMethodList(ast::MainMethodList &mainMethodList) override;
  //   void visitField(ast::Field &field) override;
  //   void visitMethod(ast::Method &method) override;
  //   void visitBlock(ast::Block &block) override;
  //   void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  //   void visitReturnStatement(ast::ReturnStatement &stmt) override;
  //   void visitVarRef(ast::VarRef &ident) override;
  //   void visitBinaryExpression(ast::BinaryExpression &expr) override;
  //   void visitIntLiteral(ast::IntLiteral &lit) override;
  //   void visitBoolLiteral(ast::BoolLiteral &lit) override;
  //   void visitNullLiteral(ast::NullLiteral &lit) override;
  //   void visitThisLiteral(ast::ThisLiteral &lit) override;
  //   void visitMainMethod(ast::MainMethod &mainMethod) override;
  //   void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  //   void visitFieldAccess(ast::FieldAccess &access) override;
  //   void visitIfStatement(ast::IfStatement &ifStatement) override;
  //   void visitUnaryExpression(ast::UnaryExpression &expr) override;
  //   void visitArrayAccess(ast::ArrayAccess &access) override;
  //   void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
  //   void visitNewArrayExpression(ast::NewArrayExpression &expr) override;
  //   void visitWhileStatement(ast::WhileStatement &stmt) override;
  //   void visitArrayType(ast::ArrayType &type) override;
  //   void visitPrimitiveType(ast::PrimitiveType &type) override;
  //   void visitClassType(ast::ClassType &type) override;
  //   void visitParameter(ast::Parameter &param) override;

  virtual ~FindDefsVisitor() {}

private:
  [[noreturn]] void error(const ast::Node &node, std::string msg) {
    throw SemanticError(node.getLoc(), fileName, std::move(msg));
  }
  [[noreturn]] void error(SourceLocation loc, std::string msg) {
    throw SemanticError(loc, fileName, std::move(msg));
  }

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

#endif
