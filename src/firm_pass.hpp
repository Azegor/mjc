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

template <typename T>
class FunctionPass {
  ir_graph *graph;
  T *sub() { return static_cast<T*>(this); }
public:
  FunctionPass(ir_graph *firmgraph) : graph(firmgraph) {}

private:
  /// typedef void irg_walk_func(ir_node *, void *)
  static void walk_wrapper(ir_node *node, void *env) {
    static_cast<T*>(env)->walk(node);
  }
  void todoImplement(ir_node *node) {
    ir_printf("TODO: implement node type %O\n", node);
  }
  [[noreturn]] void errorInvalid(ir_node *node) {
    ir_printf("ERROR: unexpected node %n\n", node);
    ir_finish();
    std::exit(1);
  }
  void walk(ir_node * node) {
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
        return todoImplement(node);
    }
  }

public:
  void run() {
    irg_walk_topological(graph, walk_wrapper, this);
  }

  // -----

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
  void visitPhi(ir_node *) {
    std::cout << "### visiting phi node" << std::endl;
  }
  void visitMod(ir_node *) {
    std::cout << "### visiting mod node" << std::endl;
  }

};

template <typename T>
class ProgramPass {
  // TODO;
};

#endif // FIRM_PASS_H
