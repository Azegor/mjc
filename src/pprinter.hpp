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
      field.getType()->accept(this);
      stream << " " << field.getName() << ";";
    }

    void visitMethod(ast::Method &method) override {
      newline();
      stream << "public ";
      method.getReturnType()->accept(this);
      stream << " " << method.getName() << "(";
      ast::ParameterList params = method.getParameters();
      if (params.size() >= 1) {
        params[0]->accept(this);
      }
      for(ast::ParameterList::size_type i = 1; i < params.size(); i++) {
        stream << ", ";
        params[i]->accept(this);
      }
      stream << ") ";
      method.getBlock()->accept(this);
    }

    void visitMainMethod(ast::MainMethod &mainMethod) { 
      newline();
      stream << "public static void " << mainMethod.getName() << "(String[] " << mainMethod.getArgName() << ") ";
      mainMethod.getBlock()->accept(this);
    }

    void visitParameter(ast::Parameter &parameter) override {
      parameter.getType()->accept(this);
      stream << " " << parameter.getName();
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
      arrayType.getElementType()->accept(this);
      int dimension = arrayType.getDimension();
      for(int i=0; i<dimension; i++) {
        stream << "[]";
      }
    }

    void visitBlock(ast::Block &block) override {
      stream << " {";
      if(block.getContainsNothingExceptOneSingleLonelyEmptyExpression()) {
        stream << " ";
      } else {
        indentLevel++;
        ast::BlockStmtList statements = block.getStatements();

        for(ast::BlockStmtList::size_type i = 0; i < statements.size(); i++) {
          if (statements[i] != nullptr) {
            //statements[i] is no EmptyStatement
            newline();
            statements[i]->accept(this);
          }
        }
        indentLevel--;
        newline();
      }
      stream << "}";
    }

    void visitVariableDeclaration(ast::VariableDeclaration &variableDeclartion) { 
      variableDeclartion.getType()->accept(this);
      stream << " " << variableDeclartion.getName();
      ast::ExprPtr initializer = variableDeclartion.getInitializer();
      if(initializer != nullptr) {
        stream << " = ";
        initializer->accept(this);
      }
      stream << ";";  
    }

    void visitExpressionStatement(ast::ExpressionStatement &exprStmt) {
      exprStmt.getExpression()->accept(this);
      stream << ";";
    }
    void visitIfStatement(ast::IfStatement &ifStatement) { 
      stream << "if (";
      ifStatement.getCondition()->accept(this);
      stream << ") ";
      ast::StmtPtr thenStatement = ifStatement.getThenStatement();
      if (thenStatement == nullptr) {
        stream << ";";
      } else {
        indentLevel++;
        thenStatement->accept(this);
        indentLevel--;
      }
      ast::StmtPtr elseStatement = ifStatement.getElseStatement();
      if (elseStatement != nullptr) {
        newline();
        stream << "else ";
        indentLevel++;
        elseStatement->accept(this);
        indentLevel--;
      }
    }
    void visitWhileStatement(ast::WhileStatement &whileStatement) { 
      stream << "while (";
      whileStatement.getCondition()->accept(this);
      stream << ") ";
      ast::StmtPtr statement = whileStatement.getStatement();
      if(statement == nullptr) {
        stream << ";";
      } else {
        indentLevel++;
        statement->accept(this);
        indentLevel--;
      }
    }
    void visitReturnStatement(ast::ReturnStatement &returnStatement) { 
      ast::ExprPtr expression = returnStatement.getExpression();
      if(expression == nullptr) {
        stream << "return;";
      } else {
        stream << "return ";
        expression->accept(this);
        stream << ";";
      }
    }

    void visitNewArrayExpression(ast::NewArrayExpression &newArrayExpression) {
      stream << "new ";
      newArrayExpression.getArrayType()->accept(this);
      //TODO: include size in ArrayType
      //currently new A[<expr>][][] gets displayed as new A[][][]
    }
    void visitNewObjectExpression(ast::NewObjectExpression &newObjectExpression) {
      stream << "new " << newObjectExpression.getName() << "()";
    }
    void visitIntLiteral(ast::IntLiteral &intLiteral) {
      stream << std::to_string(intLiteral.getValue());
    }
    void visitBoolLiteral(ast::BoolLiteral &boolLiteral) {
      stream << ( boolLiteral.getValue() ? "true" : "false" );
    }
    void visitNullLiteral(ast::NullLiteral &nullLiteral) {
      stream << "null";
    }
    void visitThisLiteral(ast::ThisLiteral &thisLiteral) {
      stream << "this";
    }
    void visitIdent(ast::Ident &ident) {
      stream << ident.getName();
    }

    void newline() {
      stream << std::endl;
      for (int i = 0; i < indentLevel; i++) {
        stream << indentWith;
      }
    }
};



#endif // PPRINTER_H