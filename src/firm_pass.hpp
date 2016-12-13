/*
 * MIT License
 *
 * Copyright (c) 2016 morrisfeist
 * Copyright (c) 2016 tpriesner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FIRM_PASS_H
#define FIRM_PASS_H

#include <libfirm/firm.h>
#include <libfirm/irgwalk.h>

#include <queue>

template <typename T, typename AttrT = void>
class FunctionPass {
protected:
  ir_graph *graph;
  std::queue<ir_node*> worklist;
  T *sub() { return static_cast<T*>(this); }
public:
  FunctionPass(ir_graph *firmgraph) : graph(firmgraph) {}

private:
  void initNodesTopological() {
    inc_irg_visited(graph);
    init_topological(get_irg_end(graph));
  }

  void init_topological(ir_node *irn)
  {
    if (irn_visited(irn)) {
      return;
    }

    /* only break loops at phi/block nodes */
    const bool is_loop_breaker = is_Phi(irn) || is_Block(irn);
    if (is_loop_breaker) {
      mark_irn_visited(irn);
    }

    if (!is_Block(irn)) {
      ir_node *const block = get_nodes_block(irn);
      init_topological(block);
    }

    for (int i = 0; i < get_irn_arity(irn); ++i) {
      ir_node *const pred = get_irn_n(irn, i);
      init_topological(pred);
    }

    if (is_loop_breaker || !irn_visited(irn)) {
      initNode(irn);
    }

    mark_irn_visited(irn);
  }

  [[noreturn]] void errorInvalid(ir_node *node) {
    ir_printf("ERROR: unexpected node %n\n", node);
    ir_finish();
    std::exit(1);
  }

public:
  void run() {
    ir_reserve_resources(graph, IR_RESOURCE_IRN_LINK);

    sub()->before();

    initNodesTopological(); // fills work queue
//     std::cout << "worklist size: " << worklist.size() << std::endl;

    // calls visit method on work queue items (add more items if necessary in visit* Methods)
    while(!worklist.empty()) {
      ir_node *node = worklist.front();
      worklist.pop();
      visitNode(node);
    }

    sub()->after();

    ir_free_resources(graph, IR_RESOURCE_IRN_LINK);
  }

protected:
  void before() {};
  void after() {};

  void enqueue(ir_node *node) {
    worklist.push(node);
  }

  static void setVal(ir_node *n, AttrT *tv) { set_irn_link(n, tv); }
  static ir_tarval *getVal(ir_node *n) { return static_cast<AttrT* >(get_irn_link(n)); }

  void initNode(ir_node * node) {
    enqueue(node); // enqueue every node!
    switch(get_irn_opcode(node)) {
      case iro_ASM: return sub()->initASM(node);
      case iro_Add: return sub()->initAdd(node);
      case iro_Address: return sub()->initAddress(node);
      case iro_Align: return sub()->initAlign(node);
      case iro_Alloc: return sub()->initAlloc(node);
      case iro_Anchor: return sub()->initAnchor(node);
      case iro_And: return sub()->initAnd(node);
      case iro_Bad: return sub()->initBad(node);
      case iro_Bitcast: return sub()->initBitcast(node);
      case iro_Block: return sub()->initBlock(node);
      case iro_Builtin: return sub()->initBuiltin(node);
      case iro_Call: return sub()->initCall(node);
      case iro_Cmp: return sub()->initCmp(node);
      case iro_Cond: return sub()->initCond(node);
      case iro_Confirm: return sub()->initConfirm(node);
      case iro_Const: return sub()->initConst(node);
      case iro_Conv: return sub()->initConv(node);
      case iro_CopyB: return sub()->initCopyB(node);
      case iro_Deleted: return sub()->initDeleted(node);
      case iro_Div: return sub()->initDiv(node);
      case iro_Dummy: return sub()->initDummy(node);
      case iro_End: return sub()->initEnd(node);
      case iro_Eor: return sub()->initEor(node);
      case iro_Free: return sub()->initFree(node);
      case iro_IJmp: return sub()->initIJmp(node);
      case iro_Id: return sub()->initId(node);
      case iro_Jmp: return sub()->initJmp(node);
      case iro_Load: return sub()->initLoad(node);
      case iro_Member: return sub()->initMember(node);
      case iro_Minus: return sub()->initMinus(node);
      case iro_Mod: return sub()->initMod(node);
      case iro_Mul: return sub()->initMul(node);
      case iro_Mulh: return sub()->initMulh(node);
      case iro_Mux: return sub()->initMux(node);
      case iro_NoMem: return sub()->initNoMem(node);
      case iro_Not: return sub()->initNot(node);
      case iro_Offset: return sub()->initOffset(node);
      case iro_Or: return sub()->initOr(node);
      case iro_Phi: return sub()->initPhi(node);
      case iro_Pin: return sub()->initPin(node);
      case iro_Proj: return sub()->initProj(node);
      case iro_Raise: return sub()->initRaise(node);
      case iro_Return: return sub()->initReturn(node);
      case iro_Sel: return sub()->initSel(node);
      case iro_Shl: return sub()->initShl(node);
      case iro_Shr: return sub()->initShr(node);
      case iro_Shrs: return sub()->initShrs(node);
      case iro_Size: return sub()->initSize(node);
      case iro_Start: return sub()->initStart(node);
      case iro_Store: return sub()->initStore(node);
      case iro_Sub: return sub()->initSub(node);
      case iro_Switch: return sub()->initSwitch(node);
      case iro_Sync: return sub()->initSync(node);
      case iro_Tuple: return sub()->initTuple(node);
      case iro_Unknown: return sub()->initUnknown(node);
      default:
        return errorInvalid(node);
    }
  }

  void visitNode(ir_node * node) {
    switch(get_irn_opcode(node)) {
      case iro_ASM: return sub()->visitASM(node);
      case iro_Add: return sub()->visitAdd(node);
      case iro_Address: return sub()->visitAddress(node);
      case iro_Align: return sub()->visitAlign(node);
      case iro_Alloc: return sub()->visitAlloc(node);
      case iro_Anchor: return sub()->visitAnchor(node);
      case iro_And: return sub()->visitAnd(node);
      case iro_Bad: return sub()->visitBad(node);
      case iro_Bitcast: return sub()->visitBitcast(node);
      case iro_Block: return sub()->visitBlock(node);
      case iro_Builtin: return sub()->visitBuiltin(node);
      case iro_Call: return sub()->visitCall(node);
      case iro_Cmp: return sub()->visitCmp(node);
      case iro_Cond: return sub()->visitCond(node);
      case iro_Confirm: return sub()->visitConfirm(node);
      case iro_Const: return sub()->visitConst(node);
      case iro_Conv: return sub()->visitConv(node);
      case iro_CopyB: return sub()->visitCopyB(node);
      case iro_Deleted: return sub()->visitDeleted(node);
      case iro_Div: return sub()->visitDiv(node);
      case iro_Dummy: return sub()->visitDummy(node);
      case iro_End: return sub()->visitEnd(node);
      case iro_Eor: return sub()->visitEor(node);
      case iro_Free: return sub()->visitFree(node);
      case iro_IJmp: return sub()->visitIJmp(node);
      case iro_Id: return sub()->visitId(node);
      case iro_Jmp: return sub()->visitJmp(node);
      case iro_Load: return sub()->visitLoad(node);
      case iro_Member: return sub()->visitMember(node);
      case iro_Minus: return sub()->visitMinus(node);
      case iro_Mod: return sub()->visitMod(node);
      case iro_Mul: return sub()->visitMul(node);
      case iro_Mulh: return sub()->visitMulh(node);
      case iro_Mux: return sub()->visitMux(node);
      case iro_NoMem: return sub()->visitNoMem(node);
      case iro_Not: return sub()->visitNot(node);
      case iro_Offset: return sub()->visitOffset(node);
      case iro_Or: return sub()->visitOr(node);
      case iro_Phi: return sub()->visitPhi(node);
      case iro_Pin: return sub()->visitPin(node);
      case iro_Proj: return sub()->visitProj(node);
      case iro_Raise: return sub()->visitRaise(node);
      case iro_Return: return sub()->visitReturn(node);
      case iro_Sel: return sub()->visitSel(node);
      case iro_Shl: return sub()->visitShl(node);
      case iro_Shr: return sub()->visitShr(node);
      case iro_Shrs: return sub()->visitShrs(node);
      case iro_Size: return sub()->visitSize(node);
      case iro_Start: return sub()->visitStart(node);
      case iro_Store: return sub()->visitStore(node);
      case iro_Sub: return sub()->visitSub(node);
      case iro_Switch: return sub()->visitSwitch(node);
      case iro_Sync: return sub()->visitSync(node);
      case iro_Tuple: return sub()->visitTuple(node);
      case iro_Unknown: return sub()->visitUnknown(node);
      default:
        return errorInvalid(node);
    }
  }

  void defaultInitOp(ir_node *) { /* default default is nothing ^^ */ }

  void initASM(ir_node * node) { sub()->defaultInitOp(node); }
  void initAdd(ir_node * node) { sub()->defaultInitOp(node); }
  void initAddress(ir_node * node) { sub()->defaultInitOp(node); }
  void initAlign(ir_node * node) { sub()->defaultInitOp(node); }
  void initAlloc(ir_node * node) { sub()->defaultInitOp(node); }
  void initAnchor(ir_node * node) { sub()->defaultInitOp(node); }
  void initAnd(ir_node * node) { sub()->defaultInitOp(node); }
  void initBad(ir_node * node) { sub()->defaultInitOp(node); }
  void initBitcast(ir_node * node) { sub()->defaultInitOp(node); }
  void initBlock(ir_node * node) { sub()->defaultInitOp(node); }
  void initBuiltin(ir_node * node) { sub()->defaultInitOp(node); }
  void initCall(ir_node * node) { sub()->defaultInitOp(node); }
  void initCmp(ir_node * node) { sub()->defaultInitOp(node); }
  void initCond(ir_node * node) { sub()->defaultInitOp(node); }
  void initConfirm(ir_node * node) { sub()->defaultInitOp(node); }
  void initConst(ir_node * node) { sub()->defaultInitOp(node); }
  void initConv(ir_node * node) { sub()->defaultInitOp(node); }
  void initCopyB(ir_node * node) { sub()->defaultInitOp(node); }
  void initDeleted(ir_node * node) { sub()->defaultInitOp(node); }
  void initDiv(ir_node * node) { sub()->defaultInitOp(node); }
  void initDummy(ir_node * node) { sub()->defaultInitOp(node); }
  void initEnd(ir_node * node) { sub()->defaultInitOp(node); }
  void initEor(ir_node * node) { sub()->defaultInitOp(node); }
  void initFree(ir_node * node) { sub()->defaultInitOp(node); }
  void initIJmp(ir_node * node) { sub()->defaultInitOp(node); }
  void initId(ir_node * node) { sub()->defaultInitOp(node); }
  void initJmp(ir_node * node) { sub()->defaultInitOp(node); }
  void initLoad(ir_node * node) { sub()->defaultInitOp(node); }
  void initMember(ir_node * node) { sub()->defaultInitOp(node); }
  void initMinus(ir_node * node) { sub()->defaultInitOp(node); }
  void initMod(ir_node * node) { sub()->defaultInitOp(node); }
  void initMul(ir_node * node) { sub()->defaultInitOp(node); }
  void initMulh(ir_node * node) { sub()->defaultInitOp(node); }
  void initMux(ir_node * node) { sub()->defaultInitOp(node); }
  void initNoMem(ir_node * node) { sub()->defaultInitOp(node); }
  void initNot(ir_node * node) { sub()->defaultInitOp(node); }
  void initOffset(ir_node * node) { sub()->defaultInitOp(node); }
  void initOr(ir_node * node) { sub()->defaultInitOp(node); }
  void initPhi(ir_node * node) { sub()->defaultInitOp(node); }
  void initPin(ir_node * node) { sub()->defaultInitOp(node); }
  void initProj(ir_node * node) { sub()->defaultInitOp(node); }
  void initRaise(ir_node * node) { sub()->defaultInitOp(node); }
  void initReturn(ir_node * node) { sub()->defaultInitOp(node); }
  void initSel(ir_node * node) { sub()->defaultInitOp(node); }
  void initShl(ir_node * node) { sub()->defaultInitOp(node); }
  void initShr(ir_node * node) { sub()->defaultInitOp(node); }
  void initShrs(ir_node * node) { sub()->defaultInitOp(node); }
  void initSize(ir_node * node) { sub()->defaultInitOp(node); }
  void initStart(ir_node * node) { sub()->defaultInitOp(node); }
  void initStore(ir_node * node) { sub()->defaultInitOp(node); }
  void initSub(ir_node * node) { sub()->defaultInitOp(node); }
  void initSwitch(ir_node * node) { sub()->defaultInitOp(node); }
  void initSync(ir_node * node) { sub()->defaultInitOp(node); }
  void initTuple(ir_node * node) { sub()->defaultInitOp(node); }
  void initUnknown(ir_node * node) { sub()->defaultInitOp(node); }

  void visitASM(ir_node *) {}
  void visitAdd(ir_node *) {}
  void visitAddress(ir_node *) {}
  void visitAlign(ir_node *) {}
  void visitAlloc(ir_node *) {}
  void visitAnchor(ir_node *) {}
  void visitAnd(ir_node *) {}
  void visitBad(ir_node *) {}
  void visitBitcast(ir_node *) {}
  void visitBlock(ir_node *) {}
  void visitBuiltin(ir_node *) {}
  void visitCall(ir_node *) {}
  void visitCmp(ir_node *) {}
  void visitCond(ir_node *) {}
  void visitConfirm(ir_node *) {}
  void visitConst(ir_node *) {}
  void visitConv(ir_node *) {}
  void visitCopyB(ir_node *) {}
  void visitDeleted(ir_node *) {}
  void visitDiv(ir_node *) {}
  void visitDummy(ir_node *) {}
  void visitEnd(ir_node *) {}
  void visitEor(ir_node *) {}
  void visitFree(ir_node *) {}
  void visitIJmp(ir_node *) {}
  void visitId(ir_node *) {}
  void visitJmp(ir_node *) {}
  void visitLoad(ir_node *) {}
  void visitMember(ir_node *) {}
  void visitMinus(ir_node *) {}
  void visitMod(ir_node *) {}
  void visitMul(ir_node *) {}
  void visitMulh(ir_node *) {}
  void visitMux(ir_node *) {}
  void visitNoMem(ir_node *) {}
  void visitNot(ir_node *) {}
  void visitOffset(ir_node *) {}
  void visitOr(ir_node *) {}
  void visitPhi(ir_node *) {}
  void visitPin(ir_node *) {}
  void visitProj(ir_node *) {}
  void visitRaise(ir_node *) {}
  void visitReturn(ir_node *) {}
  void visitSel(ir_node *) {}
  void visitShl(ir_node *) {}
  void visitShr(ir_node *) {}
  void visitShrs(ir_node *) {}
  void visitSize(ir_node *) {}
  void visitStart(ir_node *) {}
  void visitStore(ir_node *) {}
  void visitSub(ir_node *) {}
  void visitSwitch(ir_node *) {}
  void visitSync(ir_node *) {}
  void visitTuple(ir_node *) {}
  void visitUnknown(ir_node *) {}
};

class ExampleFunctionPass : public FunctionPass<ExampleFunctionPass> {
public:
  ExampleFunctionPass(ir_graph *firmgraph) : FunctionPass(firmgraph) {}

  void defaultInitOp(ir_node *) {
    std::cout << "init some node (default op)" << std::endl;
  }
  void initEnd(ir_node *) {
    std::cout << "### init end node" << std::endl;
  }
  void initStart(ir_node *) {
    std::cout << "### init start node" << std::endl;
  }
  void visitEnd(ir_node *) {
    std::cout << "### visiting end node" << std::endl;
  }
  void visitStart(ir_node *) {
    std::cout << "### visiting start node" << std::endl;
  }
};

template <typename T>
class ProgramPass {
protected:
  std::queue<ir_graph*> worklist;
  T *sub() { return static_cast<T*>(this); }
public:
  ProgramPass(std::vector<ir_graph *> &graphs) {
    for (auto g : graphs) {
      enqueue(g);
    }
  }

  void run() {
    sub()->before();

    while(!worklist.empty()) {
      ir_graph *graph = worklist.front();
      worklist.pop();
      sub()->visitMethod(graph);
    }

    sub()->after();
  }

  void enqueue(ir_graph *graph) {
    worklist.push(graph);
  }

  void before() {}
  void after() {}

  void visitMethod(ir_graph *g) {}
};

#endif // FIRM_PASS_H

