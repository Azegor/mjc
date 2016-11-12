#ifndef DOT_VISITOR_H
#define DOT_VISITOR_H

#include "ast.hpp"
#include <ostream>
#include <vector>
#include <cassert>


class DotVisitor : public ast::Visitor {
private:
  enum {
    SHAPE_NONE,
    SHAPE_BOX,
  };
  std::ostream& s;
  size_t nodeCounter = 0;
  std::string parentNode;
  std::string nextEdgeLabel;

  std::vector<std::string> nodeStack;

  void edgeLabel(const std::string &label) {
    nextEdgeLabel = label;
  }

  std::string newNodeName() {
    std::stringstream ss;
    ss << "node" << (nodeCounter ++);
    return ss.str();
  }

  std::string nodeDecl(const std::string &nodeLabel, int shape = SHAPE_NONE) {
    auto nodeName = newNodeName();

    s << nodeName << "[label=\"" << nodeLabel;
    switch(shape) {
      case SHAPE_BOX:
        s << "\", shape=box";
        break;
      case SHAPE_NONE:
        s << "\"";
    }
    s << "]" << std::endl;

    if (nextEdgeLabel != "") {
      s << "edge[label=\"" << nextEdgeLabel << "\"];" << std::endl;
      nextEdgeLabel = "";
    } else {
      s << "edge[label=\"\"];" << std::endl;
    }

    s << parentNode << " -> " << nodeName << std::endl;

    return nodeName;
  }

  std::string toplevelDecl(const std::string &nodeLabel) {
    auto nodeName = newNodeName();
    s << nodeName << "[label=\"" << nodeLabel << "\"]" << std::endl;
    return nodeName;
  }

  void pushNode(const std::string &nodeName) {
    nodeStack.push_back(nodeName);
    this->parentNode = nodeName;
  }

  void popNode() {
    assert(nodeStack.size() > 0);
    assert(nodeStack.at(nodeStack.size() - 1) == parentNode);

    nodeStack.erase(nodeStack.end() - 1);

    if (nodeStack.size() > 0)
      this->parentNode = nodeStack.at(nodeStack.size() - 1);
    else
      this->parentNode = "";
  }

public:
  DotVisitor(std::ostream &stream) : s(stream) {}
  void visitProgram(ast::Program& program) override;
  void visitClass(ast::Class& klass) override;
  void visitField(ast::Field& field) override;
  void visitMethod(ast::Method& method) override;
  void visitBlock(ast::Block &block) override;
  void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  void visitReturnStatement(ast::ReturnStatement& stmt) override;
  void visitIdent(ast::Ident& ident) override;
  void visitBinaryExpression(ast::BinaryExpression &expr) override;
  void visitIntLiteral(ast::IntLiteral &lit) override;
  void visitBoolLiteral(ast::BoolLiteral &lit) override;
  void visitNullLiteral(ast::NullLiteral &lit) override;
  void visitThisLiteral(ast::ThisLiteral &lit) override;
  void visitMainMethod(ast::MainMethod &mainMethod) override;
  void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  void visitFieldAccess(ast::FieldAccess &access) override;
  void visitIfStatement(ast::IfStatement &ifStatement) override;
  void visitExpressionStatement(ast::ExpressionStatement &stmt) override;
  void visitUnaryExpression(ast::UnaryExpression &expr) override;
  void visitArrayAccess(ast::ArrayAccess &access) override;
  void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
  void visitNewArrayExpression(ast::NewArrayExpression &expr) override;
  void visitWhileStatement(ast::WhileStatement &stmt) override;
  void visitArrayType(ast::ArrayType &type) override;
  void visitPrimitiveType(ast::PrimitiveType &type) override;
  void visitClassType(ast::ClassType &type) override;
  void visitParameter(ast::Parameter &param) override;

  virtual ~DotVisitor() {}
};

#endif
