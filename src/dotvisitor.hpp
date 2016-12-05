#ifndef DOT_VISITOR_H
#define DOT_VISITOR_H

#include "ast.hpp"
#include <cassert>
#include <ostream>
#include <unordered_map>
#include <vector>

class DotVisitor : public ast::Visitor {
private:
  enum {
    SHAPE_NONE,
    SHAPE_BOX,
  };
  std::ostream &s;
  size_t nodeCounter = 0;
  std::string parentNode;
  std::string nextEdgeLabel;

  std::unordered_map<const ast::Node *, std::string> nodeNames;

  std::vector<std::string> nodeStack;

  void edgeLabel(const std::string &label) { nextEdgeLabel = label; }

  std::string newNodeName() {
    std::stringstream ss;
    ss << "node" << (nodeCounter++);
    return ss.str();
  }

  std::string nodeDecl(const std::string &nodeLabel,
                       const ast::Node *node = nullptr,
                       int shape = SHAPE_NONE) {
    auto nodeName = newNodeName();
    if (node) {
      nodeNames[node] = nodeName;
    }

    s << nodeName << "[label=\"" << nodeLabel;
    switch (shape) {
    case SHAPE_BOX:
      s << "\", shape=box";
      break;
    case SHAPE_NONE:
      s << "\"";
    }
    s << "]" << '\n';

    s << parentNode << " -> " << nodeName;

    if (nextEdgeLabel != "") {
      s << " [label=\"" << nextEdgeLabel << "\"];\n";
      nextEdgeLabel = "";
    } else {
      s << " [label=\"\"];" << '\n';
    }

    s << '\n';

    return nodeName;
  }

  std::string nodeDeclForExistingName(const std::string &nodeName,
                                      const std::string &nodeLabel) {
    s << nodeName << "[label=\"" << nodeLabel;
    s << "\"]" << '\n';

    s << parentNode << " -> " << nodeName;

    if (nextEdgeLabel != "") {
      s << " [label=\"" << nextEdgeLabel << "\"];\n";
      nextEdgeLabel = "";
    } else {
      s << " [label=\"\"];" << '\n';
    }

    s << '\n';

    return nodeName;
  }

  void weakEdgeToNode(const std::string &nodeName,
                      const std::string &targetNodeName) {
    s << nodeName << " -> " << targetNodeName;
    if (nextEdgeLabel != "") {
      s << " [label=\"" << nextEdgeLabel << "\" weight=0 style=dashed];\n";
      nextEdgeLabel = "";
    } else {
      s << " [label=\"\" weight=0 style=dashed];" << '\n';
    }
    s << '\n';
  }

  std::string toplevelDecl(const ast::Node *node,
                           const std::string &nodeLabel) {
    auto nodeName = nodeNames[node];
    s << nodeName << "[label=\"" << nodeLabel << "\"]" << '\n';
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

  void visitProgram(ast::Program &program) override;
  void visitClass(ast::Class &klass) override;
  void visitField(ast::Field &field) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
//   void visitBlock(ast::Block &block) override;
  void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  void visitReturnStatement(ast::ReturnStatement &stmt) override;
  void visitVarRef(ast::VarRef &ident) override;
  void visitBinaryExpression(ast::BinaryExpression &expr) override;
  void visitIntLiteral(ast::IntLiteral &lit) override;
  void visitBoolLiteral(ast::BoolLiteral &lit) override;
  void visitNullLiteral(ast::NullLiteral &lit) override;
  void visitThisLiteral(ast::ThisLiteral &lit) override;
  void visitMainMethod(ast::MainMethod &mainMethod) override;
  void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  void visitFieldAccess(ast::FieldAccess &access) override;
  void visitIfStatement(ast::IfStatement &ifStatement) override;
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
