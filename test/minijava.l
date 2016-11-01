%{
#include<stdio.h>

enum {
#if 0
  TOK_ABSTRACT,
  TOK_ASSERT,
  TOK_BOOLEAN,
  TOK_BREAK,
  TOK_BYTE,
  TOK_CASE,
  TOK_CATCH,
  TOK_CHAR,
  TOK_CLASS,
  TOK_CONST,
  TOK_CONTINUE,
  TOK_DEFAULT,
  TOK_DOUBLE,
  TOK_DO,
  TOK_ELSE,
  TOK_ENUM,
  TOK_EXTENDS,
  TOK_FALSE,
  TOK_FINALLY,
  TOK_FINAL,
  TOK_FLOAT,
  TOK_FOR,
  TOK_GOTO,
  TOK_IF,
  TOK_IMPLEMENTS,
  TOK_IMPORT,
  TOK_INSTANCEOF,
  TOK_INTERFACE,
  TOK_INT,
  TOK_LONG,
  TOK_NATIVE,
  TOK_NEW,
  TOK_NULL,
  TOK_PACKAGE,
  TOK_PRIVATE,
  TOK_PROTECTED,
  TOK_PUBLIC,
  TOK_RETURN,
  TOK_SHORT,
  TOK_STATIC,
  TOK_STRICTFP,
  TOK_SUPER,
  TOK_SWITCH,
  TOK_SYNCHRONIZED,
  TOK_THIS,
  TOK_THROWS,
  TOK_THROW,
  TOK_TRANSIENT,
  TOK_TRUE,
  TOK_TRY,
  TOK_VOID,
  TOK_VOLATILE,
  TOK_WHILE,
#endif
  TOK_KEYWORD = 1,

  /* */
#if 0
  TOK_UNEQUAL,
  TOK_NOT,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_TIMESEQUAL,
  TOK_TIMES,
  TOK_PLUSPLUS,
  TOK_PLUSEQUAL,
  TOK_PLUS,
  TOK_COMMA,
  TOK_MINUSEQUAL,
  TOK_MINUSMINUS,
  TOK_MINUS,
  TOK_DOT,
  TOK_SLASHEQUAL,
  TOK_SLASH,
  TOK_COLON,
  TOK_SEMICOLON,
  TOK_LEFTLEFTEQUAL,
  TOK_LEFTLEFT,
  TOK_LEFT,
  TOK_EQUALEQUAL,
  TOK_EQUAL,
  TOK_RIGHTEQUAL,
  TOK_RIGHTRIGHTEQUAL,
  TOK_RIGHTRIGHTRIGHTEQUAL,
  TOK_RIGHTRIGHTRIGHT,
  TOK_RIGHTRIGHT,
  TOK_RIGHT,
  TOK_QUESTIONMARK,
  TOK_PERCENTEQUAL,
  TOK_PERCENT,
  TOK_ANDEQUAL,
  TOK_ANDAND,
  TOK_AND,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_CFLEXEQUAL,
  TOK_CFLEX,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_TILDE,
  TOK_BAREQUAL,
  TOK_BARBAR,
  TOK_BAR,
#endif
  TOK_OP,

  /* */
  TOK_STARTCOMMENT,
  TOK_ENDCOMMENT,

  /* */
  TOK_NUMBER,
  TOK_IDENT,
  TOK_EOF,
  TOK_OTHER,
  TOK_ERROR,
};

%}
%%
[ \t\r\n]   ;
"abstract"     { return TOK_KEYWORD;}
"assert"       { return TOK_KEYWORD;}
"boolean"      { return TOK_KEYWORD;}
"break"        { return TOK_KEYWORD;}
"byte"         { return TOK_KEYWORD;}
"case"         { return TOK_KEYWORD;}
"catch"        { return TOK_KEYWORD;}
"char"         { return TOK_KEYWORD;}
"class"        { return TOK_KEYWORD;}
"const"        { return TOK_KEYWORD;}
"continue"     { return TOK_KEYWORD;}
"default"      { return TOK_KEYWORD;}
"double"       { return TOK_KEYWORD;}
"do"           { return TOK_KEYWORD;}
"else"         { return TOK_KEYWORD;}
"enum"         { return TOK_KEYWORD;}
"extends"      { return TOK_KEYWORD;}
"false"        { return TOK_KEYWORD;}
"finally"      { return TOK_KEYWORD;}
"final"        { return TOK_KEYWORD;}
"float"        { return TOK_KEYWORD;}
"for"          { return TOK_KEYWORD;}
"goto"         { return TOK_KEYWORD;}
"if"           { return TOK_KEYWORD;}
"implements"   { return TOK_KEYWORD;}
"import"       { return TOK_KEYWORD;}
"instanceof"   { return TOK_KEYWORD;}
"interface"    { return TOK_KEYWORD;}
"int"          { return TOK_KEYWORD;}
"long"         { return TOK_KEYWORD;}
"native"       { return TOK_KEYWORD;}
"new"          { return TOK_KEYWORD;}
"null"         { return TOK_KEYWORD;}
"package"      { return TOK_KEYWORD;}
"private"      { return TOK_KEYWORD;}
"protected"    { return TOK_KEYWORD;}
"public"       { return TOK_KEYWORD;}
"return"       { return TOK_KEYWORD;}
"short"        { return TOK_KEYWORD;}
"static"       { return TOK_KEYWORD;}
"strictfp"     { return TOK_KEYWORD;}
"super"        { return TOK_KEYWORD;}
"switch"       { return TOK_KEYWORD;}
"synchronized" { return TOK_KEYWORD;}
"this"         { return TOK_KEYWORD;}
"throws"       { return TOK_KEYWORD;}
"throw"        { return TOK_KEYWORD;}
"transient"    { return TOK_KEYWORD;}
"true"         { return TOK_KEYWORD;}
"try"          { return TOK_KEYWORD;}
"void"         { return TOK_KEYWORD;}
"volatile"     { return TOK_KEYWORD;}
"while"        { return TOK_KEYWORD;}

"!="   { return TOK_OP; }
"!"    { return TOK_OP; }
"("    { return TOK_OP; }
")"    { return TOK_OP; }
"*="   { return TOK_OP; }
"*"    { return TOK_OP; }
"++"   { return TOK_OP; }
"+="   { return TOK_OP; }
"+"    { return TOK_OP; }
"-="   { return TOK_OP; }
"--"   { return TOK_OP; }
"-"    { return TOK_OP; }
"."    { return TOK_OP; }
","    { return TOK_OP; }
"/="   { return TOK_OP; }
"/"    { return TOK_OP; }
":"    { return TOK_OP; }
";"    { return TOK_OP; }
"<<="  { return TOK_OP; }
"<<"   { return TOK_OP; }
"<="   { return TOK_OP; }
"<"    { return TOK_OP; }
"=="   { return TOK_OP; }
"="    { return TOK_OP; }
">="   { return TOK_OP; }
">>="  { return TOK_OP; }
">>>=" { return TOK_OP; }
">>>"  { return TOK_OP; }
">>"   { return TOK_OP; }
">"    { return TOK_OP; }
"?"    { return TOK_OP; }
"%="   { return TOK_OP; }
"%"    { return TOK_OP; }
"&="   { return TOK_OP; }
"&&"   { return TOK_OP; }
"&"    { return TOK_OP; }
"["    { return TOK_OP; }
"]"    { return TOK_OP; }
"^="   { return TOK_OP; }
"^"    { return TOK_OP; }
"{"    { return TOK_OP; }
"}"    { return TOK_OP; }
"~"    { return TOK_OP; }
"|="   { return TOK_OP; }
"||"   { return TOK_OP; }
"|"    { return TOK_OP; }
"/*"   { return TOK_STARTCOMMENT; }
"*/"   { return TOK_ENDCOMMENT; }
[a-zA-Z_][a-zA-Z_0-9]+   { return TOK_IDENT; }
0|[1-9][0-9]*       { return TOK_NUMBER;}
<<EOF>>      { return TOK_EOF; }
[^\x00-\x7F] { return TOK_ERROR; } /* non-ascii characters */
.|\n         { return TOK_OTHER; } /* Matches everything unmatched */
%%

int main(int argc, char **argv)
{
  int in_comment = 0;
  int t;
  int t_len;
  const char *t_str;


  if (argc == 2) {
    yyin = fopen(argv[1], "r");
  } else {
    printf("ERROR\n");
    return -1;
  }

  while (t = yylex()) {
    t_str = yytext;
    t_len = yyleng;

    if (t == TOK_STARTCOMMENT)
      in_comment = 1;

    if (t == TOK_ERROR) {
      printf("error\n");
      return -1;
    }

    if (!in_comment) {
      if (t == TOK_NUMBER) {
        printf ("integer literal %s\n", t_str);
      } else if (t == TOK_IDENT) {
        printf ("identifier %s\n", t_str);
      } else if (t == TOK_ENDCOMMENT) {
        printf("*\n/\n");
      } else if (t == TOK_EOF) {
        printf ("EOF\n");
        break;
      } else if (t == TOK_OTHER) {
        printf("error: %s\n", t_str);
        return -1;
      } else {
        printf ("%s\n", t_str);
      }
    } else {
      if (t == TOK_EOF) {
        printf ("error\n");
        return -1;
      } else if (t == TOK_ENDCOMMENT) {
        in_comment = 0;
      }
    }
  }

  return 0;
}
