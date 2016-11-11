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

#ifndef PPRINTER_H
#define PPRINTER_H

#include "ast.hpp"

class PrettyPrinterVisitor : public ast::Visitor {
  std::ostream& stream;
  std::string indentWith;
  int indentLevel;

  public:
    PrettyPrinterVisitor(std::ostream &stream, std::string indentWith) : stream(stream), indentWith(std::move(indentWith)), indentLevel(0) {} 

    void visitProgram(ast::Program &program) override {
      newline();
    }

    void visitClass(ast::Class &klass) override {
      newline();
      stream << "class " << klass.getName() << " {";
      indentLevel++;
      newline();
      klass.accept(this);
      indentLevel--;
      newline();
      stream << "}";
    }

    void visitField(ast::Field &field) override {
      newline();
      stream << "public ";
      field.getType().accept(this);
      stream << " " << field.getName() << ";";
    }

    void visitMethod(ast::Method &method) override {
      newline();
      stream << "public ";
      method.getReturnType().accept(this);
      stream << " " << method.getName() << "(";
      ast::ParameterList params = method.getParameters();
      if (params.size() >= 1) {
        params[0]->accept(this);
      }
      for(ast::ParameterList::size_type i = 1; i < params.size(); i++) {
        stream << ", ";
        params[i]->accept(this);
      }
      stream << ") {";
      indentLevel++;
      method.getBlock().accept(this);
      indentLevel--;
      newline();
      stream << "}";
    }

    void visitPrimitiveType(ast::PrimitiveType &primitiveType) override {
      switch (primitiveType.getType()) {
      case ast::PrimitiveType::TypeType::Boolean:
        stream << "boolean";
        break;
      case ast::PrimitiveType::TypeType::Int:
        stream << "int";
        break;
      case ast::PrimitiveType::TypeType::Void:
        stream << "void";
        break;
      default:
        stream << "NONE";
        break;
      }
    }

    void visitClassType(ast::ClassType &classType) override {
      stream << classType.getName();
    }

    void visitArrayType(ast::ArrayType &arrayType) override {
      arrayType.getElementType().accept(this);
      int dimension = arrayType.getDimension();
      for(int i=0; i<dimension; i++) {
        stream << "[]";
      }
    }

    void newline() {
      stream << std::endl;
      for (int i = 0; i < indentLevel; i++) {
        stream << indentWith;
      }
    }
};



#endif // PPRINTER_H