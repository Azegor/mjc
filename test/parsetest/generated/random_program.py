#!/usr/bin/python3

from collections import defaultdict
import random

def weighted_choice(weights):
  if random.randint(0, 4) == 0:
    return 0
  rnd = random.random() * sum(weights)
  for i, w in enumerate(weights):
      rnd -= w
      if rnd < 0:
          return i


class RandGen:
  def __init__(self, prods):
    self.prod = prods
  def gen_random_convergent(self,
        symbol,
        cfactor=0.01,
        pcount=defaultdict(int)
    ):
    """ Generate a random sentence from the
        grammar, starting with the given symbol.

        Uses a convergent algorithm - productions
        that have already appeared in the
        derivation on each branch have a smaller
        chance to be selected.

        cfactor - controls how tight the
        convergence is. 0 < cfactor < 1.0

        pcount is used internally by the
        recursive calls to pass on the
        productions that have been used in the
        branch.
    """
    sentence = ''

    # The possible productions of this symbol are weighted
    # by their appearance in the branch that has led to this
    # symbol in the derivation
    #
    weights = []
    for prod in self.prod[symbol]:
        if prod in pcount:
            weights.append(cfactor ** (pcount[prod]))
        else:
            weights.append(1.0)

    #t1 = self.prod[symbol]
    #t2 = t1[weighted_choice(weights)]
    #print(t1, t2)
    rand_prod = self.prod[symbol][weighted_choice(weights)]

    # pcount is a single object (created in the first call to
    # this method) that's being passed around into recursive
    # calls to count how many times productions have been
    # used.
    # Before recursive calls the count is updated, and after
    # the sentence for this call is ready, it is rolled-back
    # to avoid modifying the parent's pcount.
    #
    pcount[rand_prod] += 1

    for sym in rand_prod:
        # for non-terminals, recurse
        if sym in self.prod:
            sentence += self.gen_random_convergent(
                                sym,
                                cfactor=cfactor,
                                pcount=pcount)
        else:
            sentence += sym + ' '

    # backtracking: clear the modification to pcount
    pcount[rand_prod] -= 1
    return sentence

# prods = {Sym, [Alts]}
prods = {
"Program": (("ClassDeclaration",),),

"ClassDeclaration": (("class", "IDENT", "{", "\n", "ClassMemberList", "}", "\n"),),

"ClassMemberList": ((), ("ClassMember", "\n", "ClassMemberList"),),

"ClassMember": (("Field",), ("Method",), ("MainMethod",),),

"Field": (("public", "Type", "IDENT", ";"),),
"MainMethod": (("public", "static", "void", "IDENT", "(", "String", "[", "]", "IDENT", ")", "Block"),),
"Method": (("public", "Type", "IDENT", "(", "ParametersOpt", ")", "Block"),),
"ParametersOpt": ((), ("Parameters",),),

"Parameters": (("Parameter", ",", "Parameters"), ("Parameter",),),
"Parameter": (("Type", "IDENT"),),

"Type": (("Type", "[", "]"), ("BasicType",),),

"BasicType": (("int",), ("boolean",), ("void",), ("IDENT",),),

"Statement": (("Block",), ("EmptyStatement",), ("IfStatement",), ("ExpressionStatement",), ("WhileStatement",), ("ReturnStatement",),),

"Block": (("{", "BlockStatementList", "}",),),
"BlockStatementList": ((), ("BlockStatement", "\n", "BlockStatementList"),),

"BlockStatement": (("Statement",), ("LocalVariableDeclarationStatement",),),

"LocalVariableDeclarationStatement": (("Type", "IDENT", "LocVarAsnList", ";",),),
"LocVarAsnList": ((), ("LocVarAsn", "LocVarAsnList"),),
"LocVarAsn": (("=", "Expression",),),
"EmptyStatement": ((";",),),
"WhileStatement": (("while", "(", "Expression", ")", "Statement",),),
"IfStatement": (("if", "(", "Expression", ")", "Statement", "OptElse",),),
"OptElse": ((), ("else", "Statement"),),

"ExpressionStatement": (("Expression", ";",),),
"ReturnStatement": (("return", "ExpressionOpt", ";",),),
"ExpressionOpt": ((), ("Expression",),),

"Expression": (("AssignmentExpression",),),

"AssignmentExpression": (("LogicalOrExpression", "AsnExprOpt"),),
"AsnExprOpt": ((), ("=", "AssignmentExpression"),),

"LogicalOrExpression": (("LogOrExprOpt", "LogicalAndExpression",),),
"LogOrExprOpt": ((), ("LogicalOrExpression", "||"),),

"LogicalAndExpression": (("LogAndExprOpt", "EqualityExpression",),),
"LogAndExprOpt": ((), ("LogicalAndExpression", "&&"),),

"EqualityExpression": (("EqExprOpt", "RelationalExpression",),),
"EqExprOpt": ((), ("EqualityExpression", "=="), ("EqualityExpression", "!="),),

"RelationalExpression": (("RelExprOpt", "AdditiveExpression",),),
"RelExprOpt": ((), ("RelationalExpression", "<"), ("RelationalExpression", "<="), ("RelationalExpression", ">"), ("RelationalExpression", ">="),),

"AdditiveExpression": (("AddExprOpt", "MultiplicativeExpression",),),
"AddExprOpt": ((), ("AdditiveExpression", "+"), ("AdditiveExpression", "-"),),

"MultiplicativeExpression": (("MultExprOpt", "UnaryExpression",),),
"MultExprOpt": ((), ("MultiplicativeExpression", "*"), ("MultiplicativeExpression", "/"), ("MultiplicativeExpression", "%"),),

"UnaryExpression": (("PostfixExpression",), ("!", "UnaryExpression"), ("-", "UnaryExpression"),),

"PostfixExpression": (("PrimaryExpression", "PostfixOpList",),),
"PostfixOpList": ((), ("PostfixOp", "PostfixOpList"),),

"PostfixOp": (("MethodInvocation",), ("FieldAccess",), ("ArrayAccess",),),

"MethodInvocation": ((".", "IDENT", "(", "ArgumentsOpt", ")",),),
"FieldAccess": ((".", "IDENT",),),
"ArrayAccess": (("[", "Expression", "]",),),

"ArgumentsOpt": ((), ("Arguments",),),

"Arguments": (("Expression", ",", "Arguments"), ("Expression",),),

"PrimaryExpression": (("null",), ("false",), ("true",), ("INTEGER_LITERAL",), ("IDENT",), ("IDENT", "(", "Arguments", ")"), ("this",), ("(", "Expression", ")"), ("NewObjectExpression",), ("NewArrayExpression",),),

"NewObjectExpression": (("new", "IDENT", "(", ")",),),
"NewArrayExpression": (("new", "BasicType", "[", "Expression", "]", "BracketListOpt",),),
"BracketListOpt": ((),("[", "]", "BracketListOpt"),),

"INTEGER_LITERAL": (("0",), ("190",), ("42",),),
"IDENT": (("a",), ("_1",), ("_x_",), ("a3_",),)
}

randGen = RandGen(prods)
for i in range(100):
  print(randGen.gen_random_convergent("Program"))
