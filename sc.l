%{
#include <cstdio>

#include "AST.hpp"
#include "defs.hpp"
#include "token.hpp"
#include "parserUtils.h"
#include "Reporter.hpp"
#include "error.hpp"
#include "SourceReader.hpp"

//#include <set>
//extern std::set<String> userTypeNames;

extern "C" int yywrap(void)
{
    return 1;
}

#define YY_INPUT(buf,result,max_size)  {\
  buf[0] = SourceReader.getNextChar(); \
  result = buf[0] ? 1 : 0; \
  if (result <= 0) \
    result = YY_NULL; \
}

#define ECHO

inline void makeTokenInt(int i) {
  lexval = new TokenInt(yytext, i);
}

inline void makeTokenOp(Operator op) {
  lexval = new TokenOp(yytext, op);
}

inline void makeTokenString(const char *s) {
  lexval = new TokenString(s);
}

%}

%x STRING
%x COMMENT
%x LINE_COMMENT

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
SYMBOLS [?,.\:;()\{\}\[\]]
IDENT [_a-zA-Z][_a-zA-Z0-9]*

%%
{DECIMAL_CONSTANT} { 
  int number;
  sscanf(yytext, "%d", &number);
  makeTokenInt(number);
  return _DECIMAL_CONSTANT;
}
{OCTAL_CONSTANT} {
  int number;
  sscanf(yytext, "%o", &number);
  makeTokenInt(number);
  return _OCTAL_CONSTANT;
}
{HEX_CONSTANT} { 
  int number;
  sscanf(yytext, "%x", &number);
  makeTokenInt(number);
  return _HEX_CONSTANT;
}
{FLOATING_CONSTANT} {
  makeTokenString(yytext);
  printf("float\n");
  Reporter.error(SC_FLOAT_LITERAL);
  return _ERROR;
}

char      { makeTokenInt(CHAR); return _CHAR;}
bool      { makeTokenInt(BOOL); return _BOOL;}
short     { makeTokenInt(SHORT); return _SHORT;}
int       { makeTokenInt(INT); return _INT;}
long      { makeTokenInt(LONG); return _LONG;}
signed    { makeTokenInt(SIGNED); return _SIGNED;}
unsigned  { makeTokenInt(UNSIGNED); return _UNSIGNED;}
void      { makeTokenInt(VOID); return _VOID;}
true      { makeTokenInt(true); return _TRUE;}
false     { makeTokenInt(false); return _FALSE;}
enum      { makeTokenInt(_ENUM); return _ENUM;}
const     { makeTokenInt(CONST); return _CONST;}
volatile  { makeTokenInt(VOLATILE); return _VOLATILE;}
break     { makeTokenInt(BREAK); return _BREAK;}
continue  { makeTokenInt(CONTINUE); return _CONTINUE;}
return    { makeTokenInt(RETURN); return _RETURN;}
goto      { makeTokenInt(GOTO); return _GOTO;}
do        { makeTokenInt(_DO); return _DO;}
for       { makeTokenInt(_FOR); return _FOR;}
while     { makeTokenInt(_WHILE); return _WHILE;}
if        { makeTokenInt(_IF); return _IF;}
else      { makeTokenInt(_ELSE); return _ELSE;}
switch    { makeTokenInt(_SWITCH); return _SWITCH;}
case      { makeTokenInt(_CASE); return _CASE;}
default   { makeTokenInt(_DEFAULT); return _DEFAULT;}
struct    { makeTokenInt(STRUCT); return _STRUCT;}
class     { makeTokenInt(CLASS); return _CLASS;}
private   { makeTokenInt(PRIVATE); return _PRIVATE;}
protected { makeTokenInt(PROTECTED); return _PROTECTED;}
public    { makeTokenInt(PUBLIC); return _PUBLIC;}
this      { makeTokenString(yytext); return _THIS;}
operator  { makeTokenInt(_OPERATOR); return _OPERATOR;}

typedef   { makeTokenInt(TYPEDEF); return _TYPEDEF;}
friend    { makeTokenInt(FRIEND); return _FRIEND;}
auto      { makeTokenInt(AUTO); return _AUTO;}
register  { makeTokenInt(REGISTER); return _REGISTER;}
static    { makeTokenInt(STATIC); return _STATIC;}
extern    { makeTokenInt(EXTERN); return _EXTERN;}
mutable   { makeTokenInt(MUTABLE); return _MUTABLE;}
inline    { makeTokenInt(INLINE); return _INLINE;}
virtual   { makeTokenInt(VIRTUAL); return _VIRTUAL;}
explicit  { makeTokenInt(EXPLICIT); return _EXPLICIT;}
namespace { makeTokenInt(_NAMESPACE); return _NAMESPACE;}
using     { makeTokenInt(_USING); return _USING;}

template { makeTokenInt(_TEMPLATE); return _TEMPLATE;}
typename { makeTokenInt(_TYPENAME); return _TYPENAME;}
export   { makeTokenInt(_EXPORT); return _EXPORT;}

static_cast      { makeTokenInt(STATIC_CAST); return _STATIC_CAST;}
dynamic_cast     { makeTokenInt(DYNAMIC_CAST); return _DYNAMIC_CAST;}
reinterpret_cast { makeTokenInt(REINTERPRET_CAST); return _REINTERPRET_CAST;}
const_cast       { makeTokenInt(CONST_CAST); return _CONST_CAST;}

wait  { makeTokenInt(_WAIT); return _WAIT;}
write { makeTokenInt(_WRITE); return _WRITE;}

"..."   {makeTokenOp(ELIPSIS); return _ELIPSIS;}
"+="    {makeTokenOp(ADD_ASSIGN); return _ADD_ASSIGN;}
"-="    {makeTokenOp(SUB_ASSIGN); return _SUB_ASSIGN;}
"*="    {makeTokenOp(MUL_ASSIGN); return _MUL_ASSIGN;}
"/="    {makeTokenOp(DIV_ASSIGN); return _DIV_ASSIGN;}
"%="    {makeTokenOp(MOD_ASSIGN); return _MOD_ASSIGN;}
"<<="   {makeTokenOp(LSHIFT_ASSIGN); return _LSHIFT_ASSIGN;}
">>="   {makeTokenOp(RSHIFT_ASSIGN); return _RSHIFT_ASSIGN;}
"&="    {makeTokenOp(AND_ASSIGN); return _AND_ASSIGN;}
"^="    {makeTokenOp(XOR_ASSIGN); return _XOR_ASSIGN;}
"|="    {makeTokenOp(OR_ASSIGN); return _OR_ASSIGN;}
"~="    {makeTokenOp(NOT_ASSIGN); return _NOT_ASSIGN;}

"::"    {makeTokenInt(SCOPE); return _SCOPE; }

"<<"    {makeTokenOp(LSHIFT); return _LSHIFT; }
">>"    {makeTokenOp(RSHIFT); return _RSHIFT; }
"++"    {makeTokenOp(PRE_INC); return _INC; }
"--"    {makeTokenOp(PRE_DEC); return _DEC; }
"&&"    {makeTokenOp(AND); return _AND; }
"||"    {makeTokenOp(OR); return _OR; }

"->*"   {makeTokenOp(ARROW_STAR); return _ARROW_STAR; }
"->"    {makeTokenOp(ARROW); return _ARROW; }
"=="    {makeTokenOp(EQ); return _EQ; }
"!="    {makeTokenOp(NE); return _NE; }
">="    {makeTokenOp(GE); return _GE; }
"<="    {makeTokenOp(LE); return _LE; }
"<"     {makeTokenOp(LT); return _LT; }
">"     {makeTokenOp(GT); return _GT; }

"+"     {makeTokenOp(PLUS); return _PLUS; }
"-"     {makeTokenOp(MINUS); return _MINUS; }
"*"     {makeTokenOp(STAR); return _STAR; }
"/"     {makeTokenOp(SLASH); return _SLASH; }
"%"     {makeTokenOp(PERCENT); return _PERCENT; }
"&"     {makeTokenOp(BIT_AND); return _BIT_AND; }
"|"     {makeTokenOp(BIT_OR); return _BIT_OR; }
"^"     {makeTokenOp(BIT_XOR); return _BIT_XOR; }

"="     {makeTokenOp(ASSIGN); return _ASSIGN; }
"!"     {makeTokenOp(BANG); return _BANG; }
"~"     {makeTokenOp(TILDE); return _TILDE; }
","     {makeTokenOp(COMMA); return _COMMA; }

"?"     {makeTokenString(yytext); return _QUESTION; }
"."     {makeTokenString(yytext); return _DOT; }
":"     {makeTokenString(yytext); return _COLON; }
";"     {makeTokenString(yytext); return _SEMICOLON; }
"("     {makeTokenString(yytext); return _LPAREN; }
")"     {makeTokenString(yytext); return _RPAREN; }
"{"     {makeTokenString(yytext); return _LBRACE; }
"}"     {makeTokenString(yytext); return _RBRACE; }
"["     {makeTokenString(yytext); return _LBRACKET; }
"]"     {makeTokenString(yytext); return _RBRACKET; }

{IDENT} {
  //lexstr = yytext;
  makeTokenString(yytext);
 //if (userTypeNames.find(yytext) != userTypeNames.end()) {
 //   lexstr = yytext; return _TYPE_NAME;
 // } else {
    return _IDENT;
 // }
}

{WSPACE} {}

\" {BEGIN(STRING); yymore();}
<STRING>\\\n {}
<STRING>\\\"  {yymore();}
<STRING>\n {
  makeTokenString(yytext);
  BEGIN(INITIAL);
  Reporter.error(SC_NEW_LINE_IN_STRING);
  return _ERROR;
}
<STRING>\"      { 
  makeTokenString(yytext);
  BEGIN(INITIAL);
  return _STRING_CONSTANT;
}
<STRING>. {yymore();}


"/*"           { BEGIN(COMMENT); yymore();}
<COMMENT>\n    { yymore();}
<COMMENT>"*/"  { BEGIN(INITIAL);}

"//"             { BEGIN(LINE_COMMENT); yymore();}
"#"             { BEGIN(LINE_COMMENT); yymore();}
<LINE_COMMENT>\n { BEGIN(INITIAL);}

<<EOF>> { return 0; }

. {
  //lexstr = yytext;
  makeTokenString(yytext);
  Reporter.error(SC_UNKNOWN_TOKEN, yytext);
  return _ERROR;
}
%%
