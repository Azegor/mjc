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

#include "symboltable.hpp"
#include <deque>
#include <memory>

namespace SymbolTable {

void SymbolTable::insert(Symbol &sym, Definition *def) {
  changes.emplace_back(new Change(sym, def, currentScope.get()));
}

void SymbolTable::enterScope() {
  currentScope = std::unique_ptr<Scope>{
      new Scope(std::move(currentScope), changes.size())};
}

void SymbolTable::leaveScope() {
  while (changes.size() > currentScope->oldSize) {
    // can't use .resize(oldSize), as that doesn't guarantee the right order
    changes.pop_back();
  }
  currentScope = std::move(currentScope->parent);
}

Definition *SymbolTable::lookup(const Symbol &sym) const {
  return sym.currentDef;
}

bool SymbolTable::isDefinedInCurrentScope(const Symbol &sym) const {
  return sym.currentScope == currentScope.get();
}
}
