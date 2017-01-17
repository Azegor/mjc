#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include <memory>
#include <stack>

#include "ast.hpp"
#include <libfirm/firm.h>


class Value {
protected:
public:
  virtual ir_node *load() { __builtin_trap(); }
  virtual void store(ir_node *) { __builtin_trap(); }
  virtual ir_mode *getMode() { __builtin_trap(); }
  operator ir_node *() { return load(); }
  virtual ~Value() {}

  static ir_mode *getModeForType(ir_type *t) {
    if (is_Array_type(t)) {
      return mode_P;
    }
    return get_type_mode(t);
  }
};
class VarValue : public Value {
  size_t varIndex;
  ir_mode *mode;
public:
  VarValue(size_t index, ir_mode *mode) : varIndex(index), mode(mode) {
    assert(mode);
  }
  VarValue(size_t index, ir_type *type)
  : VarValue(index, get_type_mode(type)) {}
  ir_node *load() override {
    return get_value(varIndex, mode);
  }
  void store(ir_node *val) override {
    set_value(varIndex, val);
  }
  ir_mode *getMode() override { return mode; }
};
class FieldValue : public Value {
  ir_node * member;
public:
  FieldValue(ir_node *member) : member(member) {
    assert(member);
  }
  ir_node *load() override {
    ir_type *type = get_entity_type(get_Member_entity(member));
    assert(type);
    ir_mode *mode = getMode();
    assert(mode);
//     ir_printf("load field; type: %t, mode: %m\n", type, mode);
    ir_node *loadNode = new_Load(get_store(), member, mode, type, cons_none);
    ir_node *projRes  = new_Proj(loadNode, mode, pn_Load_res);
    ir_node *projM    = new_Proj(loadNode, mode_M, pn_Load_M);
    set_store(projM);
    return projRes;
  }
  void store(ir_node *val) override {
    ir_type *type = get_entity_type(get_Member_entity(member));
    assert(type);
//     ir_printf("store field; type: %t\n", type);
    ir_node *storeNode = new_Store(get_store(), member, val, type, cons_none);
    ir_node *projM    = new_Proj(storeNode, mode_M, pn_Load_M);
    set_store(projM);
  }
  ir_mode *getMode() override {
    ir_type *type = get_entity_type(get_Member_entity(member));
    assert(type);
    return getModeForType(type);
  }
};
class ArrayValue : public Value {
  ir_node *selNode;
  ir_type *elemType;
public:
  ArrayValue(ir_node *sel, ir_type *elemType) : selNode(sel), elemType(elemType) {
    assert(selNode);
    assert(elemType);
  }
  ir_node *load() override {
    ir_mode *mode = getMode();
//     ir_printf("load array; element type: %t, mode: %m\n", elemType, mode);
    ir_node *loadNode = new_Load(get_store(), selNode, mode, elemType, cons_none);
    ir_node *projM = new_Proj(loadNode, mode_M, pn_Load_M);
    ir_node *projRes = new_Proj(loadNode, mode, pn_Load_res);
    set_store(projM);
    return projRes;
  }
  void store(ir_node *val) override {
//     ir_printf("store array; element type: %t\n", elemType);
    ir_node *storeNode = new_Store(get_store(), selNode, val, elemType, cons_none);
    ir_node *projM = new_Proj(storeNode, mode_M, pn_Load_M);
    set_store(projM);
  }
  ir_mode *getMode() override { return getModeForType(elemType); }
};
class RValue : public Value {
  ir_node *value;
public:
  RValue(ir_node *val) : value(val) {}
  ir_node * load() override { return value; }
  ir_mode *getMode() override { return get_irn_mode(value); }
};

using ValuePtr = std::unique_ptr<Value>;

struct FirmMethod {
  ir_graph *graph;
  ir_entity *entity;
  size_t nParams;
  ir_node **params;
  std::vector<ast::VariableDeclaration*> localVars;
  FirmMethod(ir_entity *entity, size_t nParams, ir_node **params,
             std::vector<ast::VariableDeclaration*> localVars, ir_graph *graph) :
    graph(graph), entity(entity), nParams(nParams), params(params), localVars(localVars) {}

  ir_type * type() { return get_entity_type(entity); }
};

struct FirmField {
  ast::Field *field;
  ir_entity *entity;
  FirmField(ast::Field *field, ir_entity *entity)
      : field(field), entity(entity) {}

  ir_type * type() { return get_entity_type(entity); }
};

struct FirmClass {
  ir_entity *entity;
  std::vector<FirmField> fieldEntities;
  FirmClass(ir_entity* entity) : entity(entity) {}

  ir_type * type() { return get_entity_type(entity); }
};


struct BoolReqInfo {
  BoolReqInfo() : requiresBool(false) {}
  BoolReqInfo(ir_node *t, ir_node *f)
      : trueTarget(t), falseTarget(f), requiresBool(true) {}

  ir_node *trueTarget;
  ir_node *falseTarget;
  bool requiresBool;
};

class FirmVisitor : public ast::Visitor {
private:
  std::string outFileName;

  bool printGraphs = false;
  int graphErrors = 0;

  ir_type *intType;
  ir_type *sizeType;
  ir_type *boolType;

  // System.out.println special case
  ir_type *sysoutType;
  ir_entity *sysoutEntity;

  // System.out.write
  ir_type *writeType;
  ir_entity *writeEntity;
  // System.out.flush
  ir_type *flushType;
  ir_entity *flushEntity;

  // System.in.read special case
  ir_type *sysinType;
  ir_entity *sysinEntity;

  ir_entity *callocEntity;


  //ir_type *currentClassType = nullptr;
  ast::Class *currentClass = nullptr;
  ast::Program *currentProgram = nullptr;
  ast::Method *currentMethod = nullptr;

  std::unordered_map<ast::Class *, FirmClass> classes;
  std::unordered_map<ast::Method *, FirmMethod> methods;

  std::stack<ValuePtr> nodeStack;
  std::stack<BoolReqInfo> reqBoolInfo;
  // targets for fumps from boolean returning expressions
//   ir_node *trueTarget = nullptr;
//   ir_node *falseTarget = nullptr;

  std::vector<ir_graph*> firmGraphs;

  ir_type *getIrType(ast::Type *type) {
    auto sType = type->getSemaType();
    return getIrType(sType);
  }
  ir_type *getIrType(const sem::Type &type) {
    switch (type.kind) {
    case sem::TypeKind::Int:
      return this->intType;
    case sem::TypeKind::Bool:
      return this->boolType;
    case sem::TypeKind::Array: {
      sem::Type innerType = type.getArrayInnerType();
      return new_type_pointer(new_type_array(getIrType(innerType), 0));
    }
    case sem::TypeKind::Class: {
      auto cls = this->currentProgram->findClassByName(type.name);
      assert(cls);
      return new_type_pointer(this->classes.at(cls).type());
    }
    default:
      assert(false);
      return nullptr; // slience compiler warning
    }
  }

  ir_mode *getIrMode(ast::Type *type) {
    auto sType = type->getSemaType();
    return getIrMode(sType);
  }
  ir_mode *getIrMode(const sem::Type &type) {
    switch(type.kind) {
    case sem::TypeKind::Int:
      return mode_Is;
    case sem::TypeKind::Bool:
      return mode_Bu;
    case sem::TypeKind::Array:
    case sem::TypeKind::Class:
      return mode_P;
    default:
      assert(false);
      return nullptr;
    }
  }

//   ir_node* booleanFromExpression(ast::Expression* expr) {
//     auto trueTarget = new_immBlock();
//     set_cur_block(trueTarget);
//     ir_node *trueNode = new_Const_long(mode_Bu, 1);
//     auto falseTarget = new_immBlock();
//     set_cur_block(falseTarget);
//     ir_node *falseNode = new_Const_long(mode_Bu, 0);
//
//     ir_node *exitBlock = new_immBlock();
//     set_cur_block(exitBlock);
//
//     pushRequiresBool(trueTarget, falseTarget);
//     expr->accept(this);
//     popRequiresBoolInfo();
//
//     mature_immBlock(trueTarget);
//     mature_immBlock(falseTarget);
//     mature_immBlock(exitBlock);
//
//     ir_node* const PhiIn[] = { trueNode, falseNode };
//     ir_node* phi = new_Phi(2, PhiIn, mode_Bu);
//     return phi;
//   }

  void pushNode(ValuePtr node) {
    nodeStack.push(std::move(node));
  }

  void pushNode(ir_node* node) {
    nodeStack.push(std::make_unique<RValue>(node));
  }

  ValuePtr popNode() {
    assert(nodeStack.size() > 0);
    ValuePtr n = std::move(nodeStack.top());
    nodeStack.pop();

    return n;
  }

  void pushRequiresNonBool() {
    reqBoolInfo.emplace();
  }
  void pushRequiresBool(ir_node *t, ir_node *f) {
    assert(t);
    assert(f);
    reqBoolInfo.emplace(t, f);
  }
  void popRequiresBoolInfo() {
    assert(!reqBoolInfo.empty());
    reqBoolInfo.pop();
  }
  bool requiresBool() const {
    assert(!reqBoolInfo.empty());
    return reqBoolInfo.top().requiresBool;
  }
  ir_node *currentTrueTarget() const {
    assert(requiresBool());
    return reqBoolInfo.top().trueTarget;
  }
  ir_node * currentFalseTarget() const {
    assert(requiresBool());
    return reqBoolInfo.top().falseTarget;
  }

  ir_node *getLoad(ir_node* src) {
    if (is_Member(src)) {
      ir_type * type = get_entity_type(get_Member_entity(src));
      ir_mode * mode = get_type_mode(type);
      ir_printf("field type: %t, mode: %m\n", type, mode);
      ir_node *loadNode = new_Load(get_store(), src, mode, type, cons_none);
      ir_node *projRes  = new_Proj(loadNode, mode, pn_Load_res);
      ir_node *projM    = new_Proj(loadNode, mode_M, pn_Load_M);
      set_store(projM);
      return projRes;
    } else if (is_Sel(src)) { // Array
      // TODO: ist this correct?
      ir_type * type = get_pointer_points_to_type(get_Sel_type(src)); // TODO: getElementType?
      ir_mode * mode = get_type_mode(type);
      ir_printf("array type: %t, mode: %m\n", type, mode);
      ir_node *loadNode = new_Load(get_store(), src, mode, type, cons_none);
      ir_node *projRes  = new_Proj(loadNode, mode_Is, pn_Load_res);
      ir_node *projM    = new_Proj(loadNode, mode_M, pn_Load_M);
      set_store(projM);
      return projRes;
    } else {
      ir_printf("Node: %O, %N\n", src, src);
      return src;
    }
  }
  void makeStore(ir_node* dest, ir_node* value);

public:
  FirmVisitor(bool print);
  virtual ~FirmVisitor() {
    for (auto& e : classes) {
      free_type(e.second.type());
    }
    for (auto& e : methods) {
      delete[] e.second.params;
    }
    //free_type(boolType);
    //free_type(sizeType);
    //free_type(intType);
  };

  std::vector<ir_graph*> &getFirmGraphs() { return firmGraphs; }

  void visitProgram(ast::Program &program) override;
  void visitMainMethod(ast::MainMethod &method) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void createClassEntity(ast::Class &klass);
  void visitClass(ast::Class &klass) override;
  void visitReturnStatement(ast::ReturnStatement &stmt) override;
  void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  void visitBoolLiteral(ast::BoolLiteral &lit) override;
  void visitIntLiteral(ast::IntLiteral &lit) override;
  void visitBinaryExpression(ast::BinaryExpression &expr) override;
  void visitVarRef(ast::VarRef &ref) override;
  void visitUnaryExpression(ast::UnaryExpression &expr) override;
  void visitThisLiteral(ast::ThisLiteral &lit) override;
  void visitNullLiteral(ast::NullLiteral &lit) override;
  void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  void visitField(ast::Field &field) override;
  void visitFieldAccess(ast::FieldAccess &access) override;
  void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
  void visitArrayAccess(ast::ArrayAccess &arrayAccess) override;
  void visitNewArrayExpression(ast::NewArrayExpression &newArrayExpression) override;
  void visitExpressionStatement(ast::ExpressionStatement &) override;
  void visitIfStatement(ast::IfStatement &stmt) override;
  void visitWhileStatement(ast::WhileStatement &whileStatement) override;

  // unimplemented:
//   void visitParameter(ast::Parameter &parameter) override { (void)parameter; assert(false); }
//   void visitPrimitiveType(ast::PrimitiveType &primitiveType) override { (void)primitiveType; assert(false); }
//   void visitClassType(ast::ClassType &classType) override { (void)classType; assert(false); }
//   void visitArrayType(ast::ArrayType &arrayType) override { (void)arrayType; assert(false); }
//   void visitBlock(ast::Block &block) override { (void)block; assert(false); }
};

#endif
