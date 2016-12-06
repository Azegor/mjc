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

#ifndef CONST_PROP_PASS_H
#define CONST_PROP_PASS_H

#include "firm_pass.hpp"

typedef ir_tarval *(*tarval_combine)(ir_tarval const *, ir_tarval const *);

std::string tarvalToStr(ir_tarval *t) {
  if (t == tarval_unknown)
    return "Unknown";
  else if (t == tarval_bad)
    return "bad";
  else
    return std::to_string(get_tarval_long(t));

}

class ConstPropPass : public FunctionPass<ConstPropPass>
{
private:
  void setNodeLink(ir_node *node, ir_tarval *val) {
    ir_tarval *oldVal = (ir_tarval*) get_irn_link (node);

    std::cout << get_irn_opname(node) << ": " << tarvalToStr(val) << std::endl;
    set_irn_link (node, val);
    //std::cout << get_irn_opname(node) << std::endl;
    //std::cout << "Edges: " << std::endl;
    if (oldVal != val) {
      foreach_out_edge_safe(node, edge) {
        std::cout << "   " << get_irn_opname(get_edge_src_irn(edge)) << std::endl;
        worklist.push(get_edge_src_irn(edge));
      }
    }
  }

public:
  ConstPropPass(ir_graph *firmgraph) : FunctionPass(firmgraph) {}

  ir_tarval *supremum(ir_node *left, ir_node *right,
                      tarval_combine fn)
  {
    ir_tarval *leftVal = (ir_tarval *)get_irn_link(left);
    ir_tarval *rightVal = (ir_tarval *)get_irn_link(right);
    if (leftVal == tarval_bad || rightVal == tarval_bad)
      return tarval_bad;
    if (leftVal == tarval_unknown)
      return rightVal;
    if (rightVal == tarval_unknown)
      return leftVal;
    return (ir_tarval *)fn(leftVal, rightVal); // is constant value
  }

  void before() {
    ir_reserve_resources(graph, IR_RESOURCE_IRN_LINK);
    edges_activate(graph);
  }
  void after() {
    edges_deactivate(graph);
    ir_free_resources(graph, IR_RESOURCE_IRN_LINK);
  }

  void visitConst(ir_node *konst) {
    setNodeLink(konst, get_Const_tarval(konst));
  }

  void visitAdd(ir_node *add) {
    setNodeLink(add, supremum(get_Add_left(add), get_Add_right(add), tarval_add));
  }

  void visitSub(ir_node *sub) {
    setNodeLink(sub, supremum(get_Sub_left(sub), get_Sub_right(sub), tarval_sub));
  }

  void visitMod(ir_node *mod) {
    setNodeLink(mod, supremum(get_Mod_left(mod), get_Mod_right(mod), tarval_mod));
  }

  void visitMul(ir_node *mul) {
    setNodeLink(mul, supremum(get_Mul_left(mul), get_Mul_right(mul), tarval_mul));
  }

  void visitDiv(ir_node *div) {
    setNodeLink(div, supremum(get_Div_left(div), get_Div_right(div), tarval_div));
  }

  void visitMinus(ir_node *minus) {
    ir_tarval *opVal = (ir_tarval *)get_irn_link(get_Minus_op(minus));

    if (tarval_is_constant(opVal))
      setNodeLink(minus, tarval_neg(opVal));
    else
      setNodeLink(minus, opVal);
  }

  void visitNot(ir_node *_not) {
    ir_tarval *opVal = (ir_tarval *)get_irn_link(get_Not_op(_not));

    if (tarval_is_constant(opVal))
      setNodeLink(_not, tarval_not(opVal));
    else
      setNodeLink(_not, opVal);
  }

  void visitConv(ir_node *conv) {
    setNodeLink(conv, (ir_tarval *)get_irn_link(get_Conv_op(conv)));
  }

  void visitProj(ir_node *proj) {
    ir_node *pred = get_Proj_pred(proj);
    setNodeLink(proj, (ir_tarval *)get_irn_link(pred));
  }

  void visitPhi(ir_node *phi) {
    int nPreds = get_Phi_n_preds(phi);
    ir_tarval *val = tarval_unknown;

    for (int i = 0; i < nPreds; i ++) {
      ir_node *pred = get_Phi_pred(phi, i);
      ir_tarval *predVal = (ir_tarval *)get_irn_link(pred);

      if (predVal == tarval_bad) {
        val = tarval_bad;
        break;
      } else if (predVal == tarval_unknown) {
        // nop
      } else {
        // constant
        if (val == tarval_unknown) {
          val = predVal;
        } else {
          // both constant
          long curLong = get_tarval_long(val);
          long predLong = get_tarval_long(predVal);
          if (curLong != predLong) {
            val = tarval_bad;
            break;
          }
        }
      }

    }

    setNodeLink(phi, val);
  }

  void visitCmp(ir_node *cmp) {
    ir_tarval *leftVal = (ir_tarval *)get_irn_link(get_Cmp_left(cmp));
    ir_tarval *rightVal = (ir_tarval *)get_irn_link(get_Cmp_right(cmp));
    ir_tarval *val;

    if (leftVal == tarval_bad || rightVal == tarval_bad) {
      setNodeLink(cmp, tarval_bad);
      return;
    } else if (leftVal == tarval_unknown || rightVal == tarval_unknown) {
      setNodeLink(cmp, tarval_unknown);
      return;
    }

    ir_relation relation = tarval_cmp(leftVal, rightVal);

    switch (get_Cmp_relation(cmp)) {
    case ir_relation_equal:
      val = new_tarval_from_long (relation == ir_relation_equal ? 1 : 0, mode_Bu);
      break;
    case ir_relation_less_greater: // not equals
      if (relation == ir_relation_less || relation == ir_relation_greater)
        val = new_tarval_from_long(1, mode_Bu);
      else
        val = new_tarval_from_long(0, mode_Bu);
      break;
    case ir_relation_less:
      val = new_tarval_from_long (relation == ir_relation_less ? 1 : 0, mode_Bu);
      break;
    case ir_relation_less_equal:
      if (relation == ir_relation_less || relation == ir_relation_equal)
        val = new_tarval_from_long(1, mode_Bu);
      else
        val = new_tarval_from_long(0, mode_Bu);
      break;
    case ir_relation_greater:
      val = new_tarval_from_long (relation == ir_relation_greater ? 1 : 0, mode_Bu);
      break;
    case ir_relation_greater_equal:
      if (relation == ir_relation_greater || relation == ir_relation_equal)
        val = new_tarval_from_long(1, mode_Bu);
      else
        val = new_tarval_from_long(0, mode_Bu);
      break;

    default:
      assert(0);
    }

    setNodeLink(cmp, val);
  }
};

#endif // CONST_PROP_PASS_H
