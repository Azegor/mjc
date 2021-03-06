/*
 * MIT License
 *
 * Copyright (c) 2016 Benedikt Morbach
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

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <deque>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace ast {
class Type;
class DummyDefinition;
class DummySystemOut;
class DummySystemIn;
}

namespace SymbolTable {

class Definition;

class Scope {
  friend class SymbolTable;
  std::unique_ptr<Scope> parent;
  size_t oldSize;

  Scope(std::unique_ptr<Scope> parent, int parentSize)
      : parent(std::move(parent)), oldSize(parentSize) {}
};

class Symbol {
  friend class StringTable;
  friend class SymbolTable;
  Scope *currentScope = nullptr;
  Definition *currentDef = nullptr;
  explicit Symbol(std::string name) : name(std::move(name)) {}
  friend class ast::DummyDefinition;
  friend class ast::DummySystemOut;
  friend class ast::DummySystemIn;

public:
  std::string name;
  Symbol(const Symbol &) = delete;
  Symbol(Symbol &&o)
      : currentScope(o.currentScope), currentDef(o.currentDef),
        name(std::move(o.name)) {}
};

class StringTable {
  std::unordered_map<std::string, Symbol> symbols;

public:
  Symbol &findOrInsert(const std::string &name) {
    auto pos = symbols.find(name);
    if (pos == symbols.end()) {
      pos = symbols.emplace(name, Symbol{name}).first;
    }
    return pos->second;
  }
};

class Definition {
public:
  virtual Symbol &getSymbol() const = 0;
  virtual ast::Type *getType() const = 0;
};

class SymbolTable {
private:
  class Change {
    Symbol &sym;
    Definition *prevDef;
    Scope *prevScope;

  public:
    Change(Symbol &sym, Definition *newDef, Scope *newScope)
        : sym(sym), prevDef(sym.currentDef), prevScope(sym.currentScope) {
      sym.currentDef = newDef;
      sym.currentScope = newScope;
    }
    ~Change() {
      sym.currentDef = prevDef;
      sym.currentScope = prevScope;
    }
  };

  std::deque<std::unique_ptr<Change>> changes;
  std::unique_ptr<Scope> currentScope = nullptr;

public:
  void enterScope();
  void leaveScope();
  void insert(Symbol &sym, Definition *def);
  Definition *lookup(const Symbol &sym) const;
  bool isDefinedInCurrentScope(const Symbol &sym) const;
};
}
#endif // SYMBOLTABLE_H
