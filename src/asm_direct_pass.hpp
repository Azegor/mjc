#ifndef ASM_DIRECT_PASS_HPP
#define ASM_DIRECT_PASS_HPP

#include "firm_pass.hpp"
#include <sstream>
#include <cassert>
#include <stack>
#include <unordered_map>

class AsmDirectPass : public ProgramPass<AsmDirectPass> {
  const std::string &outputFileName;

public:
  AsmDirectPass(std::vector<ir_graph *> &graphs, const std::string &outputFileName)
      : ProgramPass(graphs), outputFileName(outputFileName) {
    println(".text");
  }

  //void before();
  void visitMethod(ir_graph *graph);
  void after();

  void println (const std::string &str) {
    ss << str << "\n";
  }

private:
  std::stringstream ss;
};

struct Register {
  const char *name;
  bool free;
};

class DirectFunctionPass : public FunctionPass<DirectFunctionPass> {
public:
  DirectFunctionPass(ir_graph *graph, std::stringstream &ss) : FunctionPass(graph), ss(ss) {
    this->funcName = get_entity_ld_name(get_irg_entity(graph));
    this->graph = graph;

    // Create block schedule
    edges_activate(this->graph);
    inc_irg_visited(this->graph);
    ir_node *block = get_irg_start_block(this->graph);

    std::queue<ir_node *> blockStack;
    blockStack.push(block);

    std::vector<ir_node *> blockList;

    while (!blockStack.empty()) {
      ir_node *curBlock = blockStack.front();
      blockStack.pop();

      if (irn_visited(curBlock))
        continue;

      blockSchedule.push_back(curBlock);
      blockMap.emplace(std::make_pair(curBlock, std::vector<std::string>()));
      blockList.push_back(curBlock);
      //std::cout << "EnList " << get_irn_node_nr(curBlock) << " " << get_irn_opname(curBlock) << std::endl;

      set_irn_visited(curBlock, get_irg_visited(this->graph));

      foreach_block_succ(curBlock, edge) {
        ir_node *pred = get_edge_src_irn(edge);
        blockStack.push(pred);
      }
    }
  }

  void after();

  void visitStart(ir_node *startNode);
  void visitReturn(ir_node *returnNode);
  void visitConst(ir_node *constNode);
  void visitEnd(ir_node *endNode);
  void visitBlock(ir_node *blockNode);
  void visitCall(ir_node *callNode);
  void visitAdd(ir_node *addNode);
  void visitMul(ir_node *mulNode);
  void visitJmp(ir_node *jmpNode);
  void visitPhi(ir_node *phiNode);
  void visitCond(ir_node *condNode);
  void visitCmp(ir_node *cmpNode);
  void visitProj(ir_node *projNode);

private:
  ir_graph *graph;
  std::vector<ir_node *> blockSchedule;
  std::unordered_map<ir_node *, std::vector<std::string>> blockMap;
  size_t blockNum = 0;
  std::stringstream &ss;
  const char *funcName;
  Register registers[2] = {
    {
      "rbx", true
    },
    {
      "rcx", true
    }
  };
  Register *getRegister() {
    if (registers[0].free) {
      registers[0].free = false;
      return &registers[0];
    } else {
      assert(registers[1].free);
      registers[1].free = false;
      return &registers[1];
    }
  }

  std::string getBlockLabel(ir_node *block) {
    return "L" + std::to_string(get_irn_node_nr(block));
  }

  std::vector<std::string>* getCommands(ir_node *block) {
    ir_node *b = block;
    //std::cout << "Node " << get_irn_node_nr(block) << " is in block " << get_irn_node_nr(b) << std::endl;
    if (!is_Block(b)) {
      b = get_nodes_block(block);
    std::cout << "Node " << get_irn_node_nr(block) << " is in block " << get_irn_node_nr(b) << std::endl;
    }
    //std::cout << "Node " << get_irn_node_nr(block) << " is in block " << get_irn_node_nr(b) << std::endl;
    assert(is_Block(b));
    //std::cout << "Accessing Commands of Block " << get_irn_node_nr(b) << std::endl;
    return &(this->blockMap[b]);
  }
};


#endif
