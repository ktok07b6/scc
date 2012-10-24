%{
#include <cstdio>
#include "token.hpp"
extern "C" int yywrap(void)
{
    return 1;
}

#define ECHO

%}

%x STRING
%x COMMENT
%x LINE_COMMENT
%x DIRECTIVE_HEAD
%x DIRECTIVE_BODY
%x HEADER_NAME
%x HEADER_NAME1
%x HEADER_NAME2
%x PP_STRING


WSPACE [[:blank:]]+
DIGIT [0-9]
NONZERO_DIGIT [1-9]
OCTAL_DIGIT [0-7]
HEX_DIGIT [0-9a-fA-F]
DECIMAL_CONSTANT ({NONZERO_DIGIT}{DIGIT}*)|0
OCTAL_CONSTANT 0{OCTAL_DIGIT}+
HEX_CONSTANT 0x{HEX_DIGIT}+

FRACTIONAL_CONSTANT ({DECIMAL_CONSTANT}[.]{DIGIT}*)|([.]{DIGIT}+)
EXPONENT_PART [eE][+\-]{DIGIT}*
FLOATING_CONSTANT ({FRACTIONAL_CONSTANT}{EXPONENT_PART}?)|({DIGIT}*{EXPONENT_PART})

/* SYMBOLS [<>+\-\*%/&^\|~?!=,.\:;()\{\}\[\]] */
DEFINE define
SYMBOLS [?,.\:;()\{\}\[\]]
IDENT [_a-zA-Z][_a-zA-Z0-9]*
PP_DIRECTIVE ^#
%%

{PP_DIRECTIVE}                 { BEGIN(DIRECTIVE_HEAD);}
<DIRECTIVE_HEAD>define   { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_DEFINE; }
<DIRECTIVE_HEAD>undef    { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_UNDEF; }
<DIRECTIVE_HEAD>include  { BEGIN(HEADER_NAME); lexval = new TokenString(yytext); return _PP_INCLUDE; }
<DIRECTIVE_HEAD>include_next  { BEGIN(HEADER_NAME); lexval = new TokenString(yytext); return _PP_INCLUDE_NEXT; }
<DIRECTIVE_HEAD>if       { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_IF; }
<DIRECTIVE_HEAD>elif     { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_ELIF; }
<DIRECTIVE_HEAD>ifdef    { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_IFDEF; }
<DIRECTIVE_HEAD>ifndef   { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_IFNDEF; }
<DIRECTIVE_HEAD>else     { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_ELSE; }
<DIRECTIVE_HEAD>endif    { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_ENDIF; }
<DIRECTIVE_HEAD>error    { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_ERROR; }
<DIRECTIVE_HEAD>warning  { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_WARNING; }
<DIRECTIVE_HEAD>line     { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_LINE; }
<DIRECTIVE_HEAD>pragma   { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_PRAGMA; }
<DIRECTIVE_HEAD>\n       { BEGIN(INITIAL); lexval = new TokenString(yytext); return _PP_END; }
<DIRECTIVE_HEAD>.   {}
<DIRECTIVE_BODY>"##"     { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_OP_CONCAT; }
<DIRECTIVE_BODY>"#"      { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_OP_TO_STR; }
<DIRECTIVE_BODY>defined  { BEGIN(DIRECTIVE_BODY); lexval = new TokenString(yytext); return _PP_DEFINED; }

<DIRECTIVE_BODY>\n       { BEGIN(INITIAL); lexval = new TokenString(yytext); return _PP_END; }

<HEADER_NAME>"<"    { BEGIN(HEADER_NAME1); }
<HEADER_NAME>\"     { BEGIN(HEADER_NAME2); }
<HEADER_NAME>{WSPACE} {}
<HEADER_NAME>.      { BEGIN(DIRECTIVE_BODY); return _ERROR; }

<HEADER_NAME1>">"   { 
  lexval = new TokenString(lexstr); lexstr.clear();
  BEGIN(DIRECTIVE_BODY);
  return _PP_HEADER_NAME;
}
<HEADER_NAME1>. { lexstr += yytext; }

<HEADER_NAME2>\"   {
  lexval = new TokenString(lexstr); lexstr.clear();
  BEGIN(DIRECTIVE_BODY);
  return _PP_HEADER_NAME;
}
<HEADER_NAME2>. { lexstr += yytext; }

<DIRECTIVE_BODY>\\\n {}

<DIRECTIVE_BODY>true    {lexval = new TokenString(yytext); return _TRUE;}
<DIRECTIVE_BODY>false   {lexval = new TokenString(yytext); return _FALSE;}

<DIRECTIVE_BODY>{IDENT} {
  lexval = new TokenString(yytext);
  return _IDENT;
}

<DIRECTIVE_BODY>\" {BEGIN(PP_STRING); yymore();}
<PP_STRING>\\\n {}
<PP_STRING>\\\"  {yymore();}
<PP_STRING>\n {
  lexval = new TokenString(yytext);
  BEGIN(DIRECTIVE_BODY);
  return _ERROR;
}
<PP_STRING>\"      { 
  lexval = new TokenString(yytext);
  BEGIN(DIRECTIVE_BODY);
  return _STRING_CONSTANT;
}
<PP_STRING>. {yymore();}

<DIRECTIVE_BODY>"..."   {lexval = new TokenOp(yytext,ELIPSIS); return _ELIPSIS;}
<DIRECTIVE_BODY>"+="    {lexval = new TokenOp(yytext,ADD_ASSIGN); return _ADD_ASSIGN;}
<DIRECTIVE_BODY>"-="    {lexval = new TokenOp(yytext,SUB_ASSIGN); return _SUB_ASSIGN;}
<DIRECTIVE_BODY>"*="    {lexval = new TokenOp(yytext,MUL_ASSIGN); return _MUL_ASSIGN;}
<DIRECTIVE_BODY>"/="    {lexval = new TokenOp(yytext,DIV_ASSIGN); return _DIV_ASSIGN;}
<DIRECTIVE_BODY>"%="    {lexval = new TokenOp(yytext,MOD_ASSIGN); return _MOD_ASSIGN;}
<DIRECTIVE_BODY>"<<="   {lexval = new TokenOp(yytext,LSHIFT_ASSIGN); return _LSHIFT_ASSIGN;}
<DIRECTIVE_BODY>">>="   {lexval = new TokenOp(yytext,RSHIFT_ASSIGN); return _RSHIFT_ASSIGN;}
<DIRECTIVE_BODY>"&="    {lexval = new TokenOp(yytext,AND_ASSIGN); return _AND_ASSIGN;}
<DIRECTIVE_BODY>"^="    {lexval = new TokenOp(yytext,XOR_ASSIGN); return _XOR_ASSIGN;}
<DIRECTIVE_BODY>"|="    {lexval = new TokenOp(yytext,OR_ASSIGN); return _OR_ASSIGN;}
<DIRECTIVE_BODY>"~="    {lexval = new TokenOp(yytext,NOT_ASSIGN); return _NOT_ASSIGN;}

<DIRECTIVE_BODY>"<<"    {lexval = new TokenOp(yytext,LSHIFT); return _LSHIFT; }
<DIRECTIVE_BODY>">>"    {lexval = new TokenOp(yytext,RSHIFT); return _RSHIFT; }
<DIRECTIVE_BODY>"++"    {lexval = new TokenOp(yytext,PRE_INC); return _INC; }
<DIRECTIVE_BODY>"--"    {lexval = new TokenOp(yytext,PRE_DEC); return _DEC; }
<DIRECTIVE_BODY>"&&"    {lexval = new TokenOp(yytext,AND); return _AND; }
<DIRECTIVE_BODY>"||"    {lexval = new TokenOp(yytext,OR); return _OR; }

<DIRECTIVE_BODY>"=="    {lexval = new TokenOp(yytext,EQ); return _EQ; }
<DIRECTIVE_BODY>"!="    {lexval = new TokenOp(yytext,NE); return _NE; }
<DIRECTIVE_BODY>">="    {lexval = new TokenOp(yytext,GE); return _GE; }
<DIRECTIVE_BODY>"<="    {lexval = new TokenOp(yytext,LE); return _LE; }
<DIRECTIVE_BODY>"<"     {lexval = new TokenOp(yytext,LT); return _LT; }
<DIRECTIVE_BODY>">"     {lexval = new TokenOp(yytext,GT); return _GT; }

<DIRECTIVE_BODY>"+"     {lexval = new TokenOp(yytext,PLUS); return _PLUS; }
<DIRECTIVE_BODY>"-"     {lexval = new TokenOp(yytext,MINUS); return _MINUS; }
<DIRECTIVE_BODY>"*"     {lexval = new TokenOp(yytext,STAR); return _STAR; }
<DIRECTIVE_BODY>"/"     {lexval = new TokenOp(yytext,SLASH); return _SLASH; }
<DIRECTIVE_BODY>"%"     {lexval = new TokenOp(yytext,PERCENT); return _PERCENT; }
<DIRECTIVE_BODY>"&"     {lexval = new TokenOp(yytext,BIT_AND); return _BIT_AND; }
<DIRECTIVE_BODY>"|"     {lexval = new TokenOp(yytext,BIT_OR); return _BIT_OR; }
<DIRECTIVE_BODY>"^"     {lexval = new TokenOp(yytext,BIT_XOR); return _BIT_XOR; }

<DIRECTIVE_BODY>"="     {lexval = new TokenOp(yytext,ASSIGN); return _ASSIGN; }
<DIRECTIVE_BODY>"!"     {lexval = new TokenOp(yytext,BANG); return _BANG; }
<DIRECTIVE_BODY>"~"     {lexval = new TokenOp(yytext,TILDE); return _TILDE; }
<DIRECTIVE_BODY>","     {lexval = new TokenOp(yytext,COMMA); return _COMMA; }

<DIRECTIVE_BODY>"?"     {lexval = new TokenString(yytext); return _QUESTION; }
<DIRECTIVE_BODY>"("     {lexval = new TokenString(yytext); return _LPAREN; }
<DIRECTIVE_BODY>")"     {lexval = new TokenString(yytext); return _RPAREN; }

<DIRECTIVE_BODY>{DECIMAL_CONSTANT} { 
  //lexstr = yytext;
  int number;
  sscanf(yytext, "%d", &number);
  lexval = new TokenInt(yytext,number);
  return _DECIMAL_CONSTANT;
}
<DIRECTIVE_BODY>{OCTAL_CONSTANT} {
  //lexstr = yytext;
  int number;
  sscanf(yytext, "%o", &number);
  lexval = new TokenInt(yytext,number);
  return _OCTAL_CONSTANT;
}
<DIRECTIVE_BODY>{HEX_CONSTANT} { 
  //lexstr = yytext;
  int number;
  sscanf(yytext, "%x", &number);
  lexval = new TokenInt(yytext,number);
  return _HEX_CONSTANT;
}
<DIRECTIVE_BODY>{FLOATING_CONSTANT} {
  //lexstr = yytext;
  lexval = new TokenString("floating value is not supported");
  return _ERROR;
}

<DIRECTIVE_BODY>{WSPACE} {
  lexval = new TokenString(yytext);
  return _SPC;
}
<DIRECTIVE_BODY>. { lexval = new TokenString(yytext); return _SOURCE_STRING; }

"("     {lexval = new TokenString(yytext); return _LPAREN; }
")"     {lexval = new TokenString(yytext); return _RPAREN; }
","     {lexval = new TokenString(yytext); return _COMMA; }


{IDENT} {
  lexval = new TokenString(yytext);
  return _IDENT;
}

\" {BEGIN(STRING); yymore();}
<STRING>\\\n {}
<STRING>\\"  {yymore();}
<STRING>\n {
  lexval = new TokenString(yytext);
  BEGIN(INITIAL);
  return _ERROR;
}
<STRING>\"      { 
  lexval = new TokenString(yytext);
  BEGIN(INITIAL);
  return _STRING_CONSTANT;
}
<STRING>. {yymore();}


"/*"           { BEGIN(COMMENT); yymore();}
<COMMENT>\n    {yymore();}
<COMMENT>"*/"  { 
  lexval = new TokenString(yytext);
  BEGIN(INITIAL);
  return _SOURCE_STRING;
}
<COMMENT>.     {yymore();}

"//"             { BEGIN(LINE_COMMENT); yymore();}
<LINE_COMMENT>\n {
  lexval = new TokenString(yytext);
  BEGIN(INITIAL);
  return _SOURCE_STRING;
}
<LINE_COMMENT>.  { yymore(); }

\\\n {}
\n { 
  lexval = new TokenString(yytext);
  return _NEW_LINE;
}

<<EOF>> { return 0; }

{WSPACE} {
  lexval = new TokenString(yytext);
  return _SPC;
}

. {
  lexval = new TokenString(yytext);
  return _SOURCE_STRING;
}
%%
