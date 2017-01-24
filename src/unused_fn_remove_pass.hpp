#ifndef UNUSED_FN_REMOVE_PASS
#define UNUSED_FN_REMOVE_PASS

#include "firm_pass.hpp"

#include <algorithm>
#include <set>
#include <vector>

static const char* getGraphFnName(ir_graph *g) {
  auto graph_entity = get_irg_entity(g);
  return get_entity_name(graph_entity);
}

class FindReferencedFunctionsPass : public FunctionPass<FindReferencedFunctionsPass> {
  std::vector<const char*> referencedFunctions;

public:
  FindReferencedFunctionsPass(ir_graph *firmgraph) : FunctionPass(firmgraph) {}

  void visitCall(ir_node *n) {
    referencedFunctions.push_back(get_entity_name(get_Call_callee(n)));
  }

  const std::vector<const char*> getReferencedFunctions() { return referencedFunctions; }
};

class UnusedFnRmPass : public ProgramPass<UnusedFnRmPass> {
  std::set<ir_graph*> referencedFunctions;

  ir_graph *getGraphForFnName(const std::string &fnName) {
    auto pos = std::find_if(allGraphs.begin(), allGraphs.end(), [&fnName](ir_graph *g) {
      return getGraphFnName(g) == fnName;
    });
    if (pos == allGraphs.end())
      return nullptr;
    return *pos;
  }

public:
  UnusedFnRmPass(std::vector<ir_graph *> &graphs) : ProgramPass(graphs) {}

  void initWorkList() {
    for (auto& g : allGraphs) {
      if (getGraphFnName(g) == "main"s) {
        enqueue(g);
        return; // only one main function
      }
    }
  }

  void visitMethod(ir_graph* graph) {
    if (referencedFunctions.find(graph) != referencedFunctions.end()) {
      return;
    }

    FindReferencedFunctionsPass refFns(graph);
    refFns.run();
    for (auto &fName : refFns.getReferencedFunctions()) {
      auto fnGraph = getGraphForFnName(fName);
      if (fnGraph) { // will not find builtin functions
        enqueue(fnGraph);
      }
    }

    referencedFunctions.insert(graph);
  }

  void after() {
    auto newEnd =
        std::remove_if(allGraphs.begin(), allGraphs.end(), [this](ir_graph *g) {
          return referencedFunctions.find(g) == referencedFunctions.end();
        });
    allGraphs.erase(newEnd, allGraphs.end());
  }

};

#endif // UNUSED_FN_REMOVE_PASS
