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

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "const_prop_pass.hpp"
#include "unused_fn_remove_pass.hpp"
#include "firm_pass.hpp"
#include <libfirm/firm.h>
#include <vector>

class Optimizer
{
  std::vector<ir_graph *> &firmGraphs;
  bool printGraphs, verifyGraphs;

public:
  Optimizer(std::vector<ir_graph *> &firmGraphs, bool printGraphs, bool verifyGraphs)
      : firmGraphs(firmGraphs), printGraphs(printGraphs), verifyGraphs(verifyGraphs) {}

  int run()
  {
    int graphErrors = 0;
    for (auto g : firmGraphs)
    {
      // -- run optimizer passes --
      //       ExampleFunctionPass efp(g);
      //       efp.run();
      ConstPropPass cpp(g);
      cpp.run();



      // -- print graphs and verify if necessary --

      if (printGraphs)
      {
        dump_ir_graph(g, "opt");
      }

      if (verifyGraphs)
      {
        if (irg_verify(g) == 0)
          graphErrors++;
      }
    }
    if (graphErrors)
      return false;

    UnusedFnRmPass ufrp(firmGraphs);
    ufrp.run();

    return true;
  }
};

#endif // OPTIMIZER_H
