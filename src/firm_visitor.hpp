#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include <memory>
#include <stack>

#include "ast.hpp"
#include <libfirm/firm.h>


class Value {
protected:
public:
  virtual ir_node *load() { assert(false); }
  virtual void store(ir_node *) { assert(false); }
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
    ir_mode *mode = getModeForType(type);
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
    ir_mode *mode = getModeForType(elemType);
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
};
class RValue : public Value {
  ir_node *value;
public:
  RValue(ir_node *val) : value(val) {}
  ir_node * load() override { return value; }
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

class FirmVisitor : public ast::Visitor {
private:
  bool printGraphs = false;
  bool verifyGraphs = false;
  bool generateCode = false;
  int graphErrors = 0;

  ir_type *intType;
  ir_type *boolType;
//   std::unordered_map<ir_type *> arrayTypes; // TODO cache?

  // System.out.println special case
  ir_type *sysoutType;
  ir_entity *sysoutEntity;


  //ir_type *currentClassType = nullptr;
  ast::Class *currentClass = nullptr;
  ast::Program *currentProgram = nullptr;
  ast::Method *currentMethod = nullptr;

  std::unordered_map<ast::Class *, FirmClass> classes;
  std::unordered_map<ast::Method *, FirmMethod> methods;

  std::stack<ValuePtr> nodeStack;
  bool control_flow_from_binary = false;

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
      return new_type_array(getIrType(innerType), 0);
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
    switch(sType.kind) {
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
  
  ir_entity *makeCalloc(ir_type *returnType) {
    ir_type *callocType = new_type_method(2, 1, false, cc_cdecl_set, mtp_no_property);
    set_method_param_type(callocType, 0, intType);
    set_method_param_type(callocType, 1, intType);
    set_method_res_type(callocType, 0, returnType);
    ir_entity *callocEntity = new_global_entity(get_glob_type(), "calloc", callocType,
                                                ir_visibility_external, IR_LINKAGE_DEFAULT);

    return callocEntity;
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
  FirmVisitor(bool print, bool verify, bool gen);
  virtual ~FirmVisitor() {
    for (auto& e : classes) {
      free_type(e.second.type());
    }
    for (auto& e : methods) {
      delete[] e.second.params;
    }
    free_type(boolType);
    free_type(intType);

    ir_finish();
  };

  bool errorFound() {
    assert(this->verifyGraphs);
    return this->graphErrors > 0;
  }

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

  // unimplemented:
//   void visitParameter(ast::Parameter &parameter) override { (void)parameter; assert(false); }
//   void visitPrimitiveType(ast::PrimitiveType &primitiveType) override { (void)primitiveType; assert(false); }
//   void visitClassType(ast::ClassType &classType) override { (void)classType; assert(false); }
//   void visitArrayType(ast::ArrayType &arrayType) override { (void)arrayType; assert(false); }
//   void visitBlock(ast::Block &block) override { (void)block; assert(false); }
//   void visitIfStatement(ast::IfStatement &ifStatement) override { (void)ifStatement; assert(false); }
//   void visitWhileStatement(ast::WhileStatement &whileStatement) override { (void)whileStatement; assert(false); }
};

#endif
