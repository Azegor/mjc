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
    if (get_tarval_mode(t) == mode_b) {
      return (tarval_b_true == t) ? "true" : "false";
    } else {
      return std::to_string(get_tarval_long(t));
    }
}

class ConstPropPass : public FunctionPass<ConstPropPass>
{
private:
  static void setTV(ir_node *n, ir_tarval *tv) { set_irn_link(n, tv); }
  static ir_tarval *getTV(ir_node *n) { return static_cast<ir_tarval* >(get_irn_link(n)); }

  void setNodeLink(ir_node *node, ir_tarval *val) {
    ir_tarval *oldVal = getTV(node);
    if (oldVal != val) {
      std::cout << get_irn_opname(node) << ": " << tarvalToStr(val) << std::endl;
      setTV(node, val);
      //std::cout << get_irn_opname(node) << std::endl;
      //std::cout << "Edges: " << std::endl;
      foreach_out_edge_safe(node, edge) {
        std::cout << "   " << get_irn_opname(get_edge_src_irn(edge)) << std::endl;
        enqueue(get_edge_src_irn(edge));
      }
    }
  }

public:
  ConstPropPass(ir_graph *firmgraph) : FunctionPass(firmgraph) {}

  ir_tarval *supremum(ir_node *left, ir_node *right,
                      tarval_combine fn)
  {
    ir_tarval *leftVal = getTV(left);
    ir_tarval *rightVal = getTV(right);
    if (leftVal == tarval_bad || rightVal == tarval_bad)
      return tarval_bad;
    if (leftVal == tarval_unknown)
      return rightVal;
    if (rightVal == tarval_unknown)
      return leftVal;
    return (ir_tarval *)fn(leftVal, rightVal); // is constant value
  }

  void before() {
    edges_activate(graph);
  }
  void after() {
    substituteNodes();

    edges_deactivate(graph);
  }

  void walk(ir_node * node)
  {
    set_irn_link(node, tarval_unknown);
    if (is_Const(node))
      enqueue(node);
  }

  void substituteNodes() {
    inc_irg_visited(graph); // "clear" visited flags
    ir_node *endNode = get_irg_end(graph);
    substNodesWalk(endNode);

    remove_unreachable_code(graph);
    remove_bads(graph);
  }
  // basically a copy of walk_topological without the pointers
  // TODO: make this walk backwards to save on node exchanges.
  void substNodesWalk(ir_node *irn) {
    if (irn_visited(irn))
      return;

    /* only break loops at phi/block nodes */
    const bool is_loop_breaker = is_Phi(irn) || is_Block(irn);
    if (is_loop_breaker)
      mark_irn_visited(irn);

    if (!is_Block(irn)) {
      ir_node *const block = get_nodes_block(irn);
      substNodesWalk(block);
    }

    for (int i = 0; i < get_irn_arity(irn); ++i) {
      ir_node *const pred = get_irn_n(irn, i);
      substNodesWalk(pred);
    }

    if (is_loop_breaker || !irn_visited(irn))
      substituteNode(irn);

    mark_irn_visited(irn);
  }

  bool substituteNode(ir_node *node) {
    ir_tarval *val = getTV(node);
    if (val && val != tarval_unknown && val != tarval_bad) {
      // for memops set memory pojection correctly
      if (is_memop(node)) {
        ir_node *mem_target = get_memop_mem(node); // memory "above"
        foreach_out_edge_safe(node, edge) { // find succ proj
          ir_node *succNode = get_edge_src_irn(edge);
          if (is_Proj(succNode) && get_irn_mode(succNode) == mode_M) {
            exchange(succNode, mem_target); // set to precceding proj
            break;
          }
        }
        // if succ Proj not existant, do nothing
      }
      exchange(node, new_r_Const(graph, val));
      return true;
    }
    if (is_Proj(node) && get_irn_mode(node) == mode_X) {
      ir_node *cond = get_Proj_pred(node);
      assert(is_Cond(cond));
      ir_node *cmp = get_Cond_selector(cond);
      ir_tarval *cmpVal = getTV(cmp);
      if(cmpVal != tarval_unknown && cmpVal != tarval_bad) {
        if ((cmpVal == tarval_b_true) == (get_Proj_num(node) == pn_Cond_true)) {
          // get_nodes_block might return wrong block, see docs
          exchange(node, new_r_Jmp(get_nodes_block(node)));
        } else {
          exchange(node, new_r_Bad(graph, mode_X));
        }
        return true;
      }
    }
    return false;
  }

  // ---- visit methods: ----

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
    ir_tarval *opVal = getTV(get_Minus_op(minus));

    if (tarval_is_constant(opVal))
      setNodeLink(minus, tarval_neg(opVal));
    else
      setNodeLink(minus, opVal);
  }

  void visitNot(ir_node *_not) {
    // TODO: are not nodes even generated in FirmVisitor?
    ir_tarval *opVal = getTV(get_Not_op(_not));

    if (tarval_is_constant(opVal))
      setNodeLink(_not, tarval_not(opVal));
    else
      setNodeLink(_not, opVal); // TODO: not 'tarval_unknown'?
  }

  // TODO
  void visitCond(ir_node *cond) {
    (void)cond;
    // TODO: if const -> replace with jump (will be fun(n|k)y)
    // watch out when replacing node regarding the work list (successor node)!
  }

  void visitConv(ir_node *conv) {
    setNodeLink(conv, tarval_convert_to(getTV(get_Conv_op(conv)), get_irn_mode(conv)));
  }

  void visitProj(ir_node *proj) {
    if (get_irn_mode(proj) != mode_M) {
      ir_node *pred = get_Proj_pred(proj);
      setNodeLink(proj, getTV(pred));
    }
  }

  void visitPhi(ir_node *phi) {
    int nPreds = get_Phi_n_preds(phi);
    ir_tarval *val = tarval_unknown;

    for (int i = 0; i < nPreds; i ++) {
      ir_node *pred = get_Phi_pred(phi, i);
      ir_tarval *predVal = getTV(pred);

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
    ir_tarval *leftVal = getTV(get_Cmp_left(cmp));
    ir_tarval *rightVal = getTV(get_Cmp_right(cmp));
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
      val = (relation == ir_relation_equal) ? tarval_b_true : tarval_b_false;
      break;
    case ir_relation_less_greater: // not equals
      if (relation == ir_relation_less || relation == ir_relation_greater)
        val = tarval_b_true;
      else
        val = tarval_b_false;
      break;
    case ir_relation_less:
      val = (relation == ir_relation_less) ? tarval_b_true : tarval_b_false;
      break;
    case ir_relation_less_equal:
      if (relation == ir_relation_less || relation == ir_relation_equal)
        val = tarval_b_true;
      else
        val = tarval_b_false;
      break;
    case ir_relation_greater:
      val = (relation == ir_relation_greater) ? tarval_b_true : tarval_b_false;
      break;
    case ir_relation_greater_equal:
      if (relation == ir_relation_greater || relation == ir_relation_equal)
        val = tarval_b_true;
      else
        val = tarval_b_false;
      break;

    default:
      assert(0);
    }

    setNodeLink(cmp, val);
  }
};

#endif // CONST_PROP_PASS_H
