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

class ConstPropPass : public FunctionPass<ConstPropPass>
{
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
  }
  void after() {
    ir_free_resources(graph, IR_RESOURCE_IRN_LINK);
  }

  void visitConst(ir_node *konst) {
    set_irn_link(konst, get_Const_tarval(konst));
    dump_ir_graph(graph, "");
  }

  void visitAdd(ir_node *add)
  {
    set_irn_link(add, supremum(get_Add_left(add), get_Add_right(add), tarval_add));
  }
};

#endif // CONST_PROP_PASS_H
