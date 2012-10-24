%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "def.h"
#include "AST.h"
	
using namespace std;
#define YYDEBUG 1
extern "C" void yyerror(const char *s) {
	fprintf(stderr, "%s\n",s);
}

int yylex(void);
//extern std::set<String> userTypeNames;
extern AST *result_ast;
extern int yyget_lineno (void );
%}
%union {
	Operator op;
	const char *s;
	Symbol *sym;
	int n;
	bool b;
	ASTExp *exp;
	ASTStm *stm;
	AST *ast;
	Scope *scope;
	Condition *cond;
	TranslationUnit *unit;
}
%token <s> _IDENT
%token <s> _TYPE_NAME
%token <s> _STRING_CONSTANT
%token <n> _DECIMAL_CONSTANT _OCTAL_CONSTANT _HEX_CONSTANT
%token <b> _TRUE _FALSE

%token _CHAR _BOOL _SHORT _INT _LONG _SIGNED _UNSIGNED
%token _VOID
%token _ENUM
%token _CONST _VOLATILE
%token _BREAK _CONTINUE _RETURN _GOTO
%token _DO _FOR _WHILE _IF _ELSE
%token _SWITCH _CASE _DEFAULT
%token _STRUCT _CLASS
%token _PRIVATE _PROTECTED _PUBLIC
%token _THIS _OPERATOR _TYPEDEF _FRIEND
%token _AUTO _REGISTER _STATIC _EXTERN
%token _MUTABLE _INLINE _VIRTUAL _EXPLICIT
%token _NAMESPACE _USING
%token _TEMPLATE _TYPENAME _EXPORT
%token _STATIC_CAST _DYNAMIC_CAST _REINTERPRET_CAST _CONST_CAST
%token _WAIT _WRITE

%token '=' _ADD_ASSIGN _SUB_ASSIGN _MUL_ASSIGN _DIV_ASSIGN _MOD_ASSIGN _LSHIFT_ASSIGN _RSHIFT_ASSIGN _AND_ASSIGN _XOR_ASSIGN _OR_ASSIGN _NOT_ASSIGN
%token _OR _AND
%token '|' '^' '&'
%token _EQ _NE 
%token '<' '>' _LE _GE
%token _LSHIFT _RSHIFT
%token '+' '-' '*' '/' '%'
%token _INC _DEC '!' '~' _ARROW _ARROW_STAR
%token _SCOPE

%token _ERROR

%type<n> integer_literal
%type<b> boolean_literal
%type<s> string_literal

%type<exp> expression_list
%type<exp> expression_list.opt
%type<exp> primary_expression
%type<exp> id_expression
%type<sym> unqualified_id
%type<exp> qualified_id
%type<scope> scoped_name_specifier
%type<scope> nested_name_specifier
%type<scope> nested_name_specifier.opt

%type<exp> postfix_expression
%type<exp> unary_expression
%type<op> unary_operator
%type<exp> cast_expression
%type<exp> multiplicative_expression
%type<exp> additive_expression
%type<exp> shift_expression
%type<exp> relational_expression
%type<exp> equality_expression
%type<exp> and_expression
%type<exp> exclusive_or_expression
%type<exp> inclusive_or_expression
%type<exp> logical_and_expression
%type<exp> logical_or_expression
%type<exp> conditional_expression
%type<exp> assignment_expression
%type<op> assignment_operator
%type<exp> expression
%type<exp> expression.opt

%type<stm> statement
%type<stm> expression_statement
%type<stm> compound_statement
%type<stm> statement_seq

%type<stm> labeled_statement
%type<stm> selection_statement
%type<cond> condition
%type<cond> condition.opt
%type<stm> iteration_statement
%type<stm> for_init_statement
%type<stm> jump_statement

%type<sym> identifier
%type<unit> translation_unit

%start program
%glr-parser
%%


program
: translation_unit/*statement_seq*/
{
	result_ast = $1;
}

translation_unit
: declaration_seq
{}
//| sc-main-definition
//{}


identifier
: _IDENT
{
	$$ = Symbol::symbol($1);
	free((void*)$1);
}

identifier.opt
:
| identifier

integer_literal
: _DECIMAL_CONSTANT {$$=$1;}
| _OCTAL_CONSTANT   {$$=$1;}
| _HEX_CONSTANT     {$$=$1;}

boolean_literal
: _TRUE  {$$=true;}
| _FALSE {$$=false;}

string_literal
: _STRING_CONSTANT {$$ = $1;}

template.opt
: /* empty */
| _TEMPLATE
{}


//--------------------------------------
//Expression
primary_expression
: integer_literal
{
	$$ = new PrimaryExpression($1);
}
| boolean_literal
{
	$$ = new PrimaryExpression($1);
}
| string_literal
{
	$$ = new PrimaryExpression($1);
}
| _THIS
{
	$$ = new PrimaryExpression(Symbol::symbol("this"));
}
| '(' expression ')'
{
	$$ = $2
}
| id_expression
{
	$$ = $1;
}

id_expression
: unqualified_id
{
	$$ = new PrimaryExpression($1);
}
| qualified_id
{
	$$ = $1;
}

unqualified_id
: identifier
{
	$$ = $1
}
//| operator_function_id
//| conversion_function_id
//| template_id
//{}

qualified_id
: scoped_name_specifier template.opt unqualified_id
{
	PrimaryExpression::ScopedSymbol *scoped = new PrimaryExpression::ScopedSymbol();
	scoped->sym = $3;
	scoped->scope = $1;assert($1);
	$$ = new PrimaryExpression(scoped);
}
| _SCOPE identifier
{
	$$ = new PrimaryExpression($2);
}
//| _SCOPE operator_function_id
//| _SCOPE template_id

global_scope
:
| _SCOPE
{}

scoped_name_specifier
: global_scope nested_name_specifier
{
	$$ = $2;
}

nested_name_specifier
: identifier _SCOPE 
{
	$$ = new Scope($1);
}
| nested_name_specifier identifier _SCOPE 
{
	Scope *scope = $1;
	scope->add($2);
}

nested_name_specifier.opt
: { $$ = NULL; }
| nested_name_specifier { $$ = $1; }


postfix_expression
: primary_expression
{
	$$ = $1;
}
| postfix_expression '[' expression ']'
{
	$$ = new Subscript($1, $3);
}
| postfix_expression '(' expression_list.opt ')'
{
	$$ = NULL;
}
//| simple_type_specifier '(' expression_list.opt ')'
//{}
  /*
| typename scoped_name_specifier identifier '(' expression_list.opt ')'
| typename scoped_name_specifier template.opt template_id '(' expression_list.opt ')'
  */
| postfix_expression '.'    template.opt id_expression
{
	$$ = new MemberSelect($1, $4, false);
}
| postfix_expression _ARROW template.opt id_expression
{
	$$ = new MemberSelect($1, $4, true);
}
| postfix_expression _INC
{
	$$ = new UnaryExpression(POST_INC, $1);
}
| postfix_expression _DEC
{
	$$ = new UnaryExpression(POST_DEC, $1);
}
/*
| _DYNAMIC_CAST     '<' type_id '>' '(' expression ')'
{}
| _STATIC_CAST      '<' type_id '>' '(' expression ')'
{}
| _REINTERPRET_CAST '<' type_id '>' '(' expression ')'
{}
| _CONST_CAST       '<' type_id '>' '(' expression ')'
{}
*/

expression_list
: expression {$$ = $1;}
| expression_list ',' expression
{
	SequenceExpression *seq = NULL;
	if ($1->op == SEQ) {
		seq = (SequenceExpression*)$1;
	} else {
		seq = new SequenceExpression();
		seq->exps.push_back($1);
	}
	seq->exps.push_back($3);
	$$ = seq;
}

expression_list.opt
: { $$ = NULL; }
| expression_list { $$ = $1; }

unary_expression
: postfix_expression   
{
	$$=$1;
}
| _INC cast_expression
{
	$$ = new UnaryExpression(PRE_INC, $2);
}
| _DEC cast_expression
{
	$$ = new UnaryExpression(PRE_DEC, $2);
}
| unary_operator cast_expression
{
	$$ = new UnaryExpression($1, $2);
}

unary_operator
: '*' {	$$ = UNARY_STAR;}
| '&' {	$$ = UNARY_AND; }
| '+' {	$$ = UNARY_PLUS;}
| '-' {	$$ = UNARY_MINUS;}
| '!' {	$$ = BANG;}
| '~' {	$$ = TILDE;}
;

cast_expression
: unary_expression
{
	$$ = $1;
}
| '(' type_id ')' cast_expression
{}

multiplicative_expression
: cast_expression
{
	$$ = $1;
}
| multiplicative_expression '*' cast_expression
{
	$$ = new BinaryExpression(STAR, $1, $3);
}
| multiplicative_expression '/' cast_expression
{
	$$ = new BinaryExpression(SLASH, $1, $3);
}
| multiplicative_expression '%' cast_expression
{
	$$ = new BinaryExpression(SLASH, $1, $3);
}

additive_expression
: multiplicative_expression
{
	$$ = $1;
}
| additive_expression '+' multiplicative_expression
{
	$$ = new BinaryExpression(PLUS, $1, $3);
}
| additive_expression '-' multiplicative_expression
{
	$$ = new BinaryExpression(MINUS, $1, $3);
}

shift_expression
: additive_expression
{
	$$ = $1;
}
| shift_expression _LSHIFT additive_expression
{
	$$ = new BinaryExpression(LSHIFT, $1, $3);
}
| shift_expression _RSHIFT additive_expression
{
	$$ = new BinaryExpression(RSHIFT, $1, $3);
}

relational_expression
: shift_expression
{
	$$ = $1;
}
| relational_expression '<' shift_expression
{
	$$ = new BinaryExpression(LT, $1, $3);
}
| relational_expression '>' shift_expression
{
	$$ = new BinaryExpression(GT, $1, $3);
}
| relational_expression _LE shift_expression
{
	$$ = new BinaryExpression(LE, $1, $3);
}
| relational_expression _GE shift_expression
{
	$$ = new BinaryExpression(GE, $1, $3);
}

equality_expression
: relational_expression
{
	$$ = $1
}
| equality_expression _EQ relational_expression
{
	$$ = new BinaryExpression(EQ, $1, $3);
}
| equality_expression _NE relational_expression
{
	$$ = new BinaryExpression(NE, $1, $3);
}

and_expression
: equality_expression
{
	$$ = $1;
}
| and_expression '&' equality_expression
{
	$$ = new BinaryExpression(BIT_AND, $1, $3);
}

exclusive_or_expression
: and_expression
{
	$$ = $1;
}
| exclusive_or_expression '^' and_expression
{
	$$ = new BinaryExpression(BIT_XOR, $1, $3);
}

inclusive_or_expression
: exclusive_or_expression
{
	$$ = $1;
}
| inclusive_or_expression '|' exclusive_or_expression
{
	$$ = new BinaryExpression(BIT_OR, $1, $3);
}

logical_and_expression
: inclusive_or_expression
{
	$$ = $1;
}
| logical_and_expression _AND inclusive_or_expression
{
	$$ = new BinaryExpression(AND, $1, $3);
}

logical_or_expression
: logical_and_expression
{
	$$ = $1;
}
| logical_or_expression _OR logical_and_expression
{
	$$ = new BinaryExpression(OR, $1, $3);
}

conditional_expression
: logical_or_expression
{
	$$ = $1;
}
| logical_or_expression '?' expression ':' assignment_expression
{
	$$ = new ConditionalExpression($1, $3, $5);
}

assignment_expression
: conditional_expression
{
	$$ = $1;
}
| logical_or_expression assignment_operator assignment_expression
{
	$$ = new BinaryExpression($2, $1, $3);
}

assignment_operator
: '='         { $$ = ASSIGN; }
| _MUL_ASSIGN { $$ = MUL_ASSIGN; }
| _DIV_ASSIGN { $$ = DIV_ASSIGN; }
| _MOD_ASSIGN { $$ = MOD_ASSIGN; }
| _ADD_ASSIGN { $$ = ADD_ASSIGN; }
| _SUB_ASSIGN { $$ = SUB_ASSIGN; }
| _LSHIFT_ASSIGN { $$ = LSHIFT_ASSIGN; }
| _RSHIFT_ASSIGN { $$ = RSHIFT_ASSIGN; }
| _AND_ASSIGN { $$ = AND_ASSIGN; }
| _XOR_ASSIGN { $$ = XOR_ASSIGN; }
| _OR_ASSIGN  { $$ = OR_ASSIGN; }
| _NOT_ASSIGN { $$ = NOT_ASSIGN; }
;

expression
: assignment_expression
{
	$$ = $1;
}

expression.opt
:            { $$ = NULL; }
| expression { $$ = $1;   }


constant_expression
: conditional_expression
{
}

constant_expression.opt
:
| constant_expression


//--------------------------------------
//Statement
statement
: expression_statement
{
	$$ = $1;
}
| compound_statement
{
	$$ = $1;
}
| labeled_statement
{}
| wait_statement
{}
//| signal_assignment_statement
//{}
| selection_statement
{}
| iteration_statement
{}
| jump_statement
{}
| declaration_statement
{}


expression_statement
: ';'
{
	$$ = new ExpressionStatement(NULL);
}
| expression ';'
{
	$$ = new ExpressionStatement($1);
}

compound_statement
: '{'               '}'
{
	$$ = new CompoundStatement();
}
| '{' statement_seq '}'
{
	$$ = $2;
}

statement_seq
: statement
{
	CompoundStatement *cs = new CompoundStatement();
	cs->stms.push_back($1);
	$$ = cs;
}
| statement_seq statement
{
	CompoundStatement *cs = static_cast<CompoundStatement *>($1);
	cs->stms.push_back($2);
	$$ = cs;
}

labeled_statement
: identifier ':' statement
{}
| _CASE constant_expression ':' statement
{}
| _DEFAULT ':' statement
{}

wait_statement
: _WAIT '(' constant_expression.opt ')' ';'
{}
 /*
signal_assignment_statement
: signal_or_port_identifier '.' _WRITE '(' expression ')' ';'
| signal_or_port_identifier '=' expression ';'
*/


selection_statement
: _IF '(' condition ')' statement
{
	$$ = new IfStatement($3, $5, NULL);
}
| _IF '(' condition ')' statement _ELSE statement
{
	$$ = new IfStatement($3, $5, $7);
}
| _SWITCH '(' condition ')' statement
{
	//TODO:
	$$ = NULL;
}

condition
: expression
{
	$$ = new Condition($1);
}
| type_specifier_seq declarator '=' assignment_expression
{}

condition.opt
:           { $$ = NULL; } 
| condition { $$ = $1; }


iteration_statement
: _WHILE '(' condition ')' statement
{
	$$ = new WhileStatement($3, $5);
}
| _DO statement _WHILE '(' expression ')' ';'
{
	$$ = new DoStatement($5, $2);
}
| _FOR '(' for_init_statement condition.opt ';' expression.opt ')' statement
{
	$$ = new ForStatement($3, $4, $6, $8);
}

for_init_statement
: expression_statement
{
	$$ = $1;
}
| simple_declaration
{}


jump_statement
: _BREAK ';'             { $$ = new JumpStatement(JumpStatement::BREAK); }
| _CONTINUE ';'          { $$ = new JumpStatement(JumpStatement::CONTINUE); }
| _RETURN            ';' { $$ = new JumpStatement(JumpStatement::RETURN); }
| _RETURN expression ';' { $$ = new JumpStatement(JumpStatement::RETURN, $2); }
| _GOTO identifier ';'   { $$ = new JumpStatement($2); }


declaration_statement
: block_declaration
{
}


//--------------------------------------
//Declarations

declaration_seq
: declaration
{}
| declaration_seq declaration
{}


declaration
: block_declaration
{}
| function_definition
{}
//| template_declaration
//{}
//| explicit_instantiation
//{}
//| explicit_specialization
//{}
| namespace_definition
{}
//| sc_process_definition
//{}


block_declaration
: simple_declaration
{}
| namespace_alias_definition
{}
| using_declaration
{}
| using_directive
{}


simple_declaration
: decl_specifier_seq.opt init_declarator_list.opt ';'
{}


decl_specifier
: storage_class_specifier
{}
| type_specifier
{}
//| function_specifier
//{}
| _FRIEND
{}
| _TYPEDEF
{}



decl_specifier_seq
: decl_specifier
{}
| decl_specifier_seq decl_specifier
{}

decl_specifier_seq.opt
: {}
| decl_specifier_seq {}


storage_class_specifier
: _AUTO     {}
| _REGISTER {}
| _STATIC   {}
| _EXTERN   {}
| _MUTABLE  {}


  /*
function_specifier
: _INLINE    {}
| _VIRTUAL   {}
| _EXPLICIT  {}
  */



type_specifier
: simple_type_specifier
{}
//| class_specifier
//{}
| enum_specifier
{}
| elaborated_type_specifier
{}
| cv_qualifier
{}
//| sc_type_specifier
//{}
//| sc_module_specifier
//{}


simple_type_specifier
: global_scope nested_name_specifier.opt identifier
{}
//| scoped_name_specifier _TEMPLATE template_id
//{}
| builtin_type_specifier
{}


builtin_type_specifier
: _CHAR   {}
| _BOOL   {}
| _SHORT  {}
| _INT    {}
| _LONG   {}
| _SIGNED  {}
| _UNSIGNED {}
| _VOID     {}


elaborated_type_specifier
:/* class_key global_scope nested_name_specifier.opt identifier
{}
|*/ _ENUM global_scope nested_name_specifier.opt identifier
{}
| _TYPENAME scoped_name_specifier identifier
{}
/*
| _TYPENAME scoped_name_specifier template.opt template_id
{}
*/


enum_specifier
: _ENUM identifier.opt '{' enumerator_list.opt '}'
{}

enumerator_list
: enumerator_difinition
{}
| enumerator_list ',' enumerator_difinition
{}

enumerator_list.opt
:
| enumerator_list


enumerator_difinition
: enumerator
{}
| enumerator '=' constant_expression
{}

enumerator
: identifier
{
}


namespace_name
: original_namespace_name
{}
| namespace_alias
{
}

original_namespace_name
: identifier
{
}

namespace_definition
: named_namespace_definition
{}
| unnamed_namespace_definition
{
}

named_namespace_definition
: original_namespace_definition
{}
| extension_namespace_definition
{
}

original_namespace_definition
: _NAMESPACE identifier namespace_body.opt
{
}

extension_namespace_definition
: _NAMESPACE original_namespace_name namespace_body.opt
{
}

unnamed_namespace_definition
: _NAMESPACE namespace_body.opt
{
}

namespace_body
: declaration_seq
{
}

namespace_body.opt
:
| namespace_body

namespace_alias
: identifier
{
}

namespace_alias_definition
: _NAMESPACE identifier '=' qualified_namespace_specifier ';'
{
}

qualified_namespace_specifier
: global_scope nested_name_specifier.opt namespace_name
{
}

using_declaration
: _USING scoped_name_specifier unqualified_id ';'
| _USING _SCOPE unqualified_id ';'
{
}

using_directive
: _USING _NAMESPACE global_scope nested_name_specifier.opt namespace_name ';'
{
}


//--------------------------------------
//Declarators

init_declarator_list
: init_declarator
| init_declarator_list ',' init_declarator
{
}

init_declarator_list.opt
:
| init_declarator_list



init_declarator
: declarator
| declarator initializer
{
}


declarator
: direct_declarator
| ptr_operator declarator
{
}


direct_declarator
: declarator_id
| direct_declarator '(' parameter_declaration_clause ')' cv_qualifier_seq.opt
| direct_declarator '[' constant_expression.opt ']'
| '(' declarator ')'
{
}


ptr_operator
: '&'


cv_qualifier_seq
: cv_qualifier 
| cv_qualifier cv_qualifier_seq
{
}


cv_qualifier_seq.opt
:
| cv_qualifier_seq


cv_qualifier
: _CONST    {}
| _VOLATILE {}


declarator_id
: id_expression
| global_scope nested_name_specifier.opt identifier
{
}


type_id
: type_specifier_seq 
  //| type_specifier_seq abstract_declarator
{
}


type_specifier_seq
: type_specifier 
| type_specifier type_specifier_seq
{
}


abstract_declarator
: ptr_operator 
| ptr_operator abstract_declarator
| direct_abstract_declarator
{
}

abstract_declarator.opt
:
| abstract_declarator


direct_abstract_declarator
: direct_abstract_declarator.opt '(' parameter_declaration_clause ')' cv_qualifier_seq.opt
| direct_abstract_declarator.opt '[' constant_expression.opt ']'
| '(' abstract_declarator ')'
{
}

direct_abstract_declarator.opt
:
| direct_abstract_declarator



parameter_declaration_clause
:
| parameter_declaration_list
//| [ parameter_declaration_list ] ...
 //| parameter_declaration_list ',' ...
{
}



parameter_declaration_list
: parameter_declaration
| parameter_declaration_list ',' parameter_declaration
{
}



parameter_declaration
: decl_specifier_seq declarator
| decl_specifier_seq declarator '=' assignment_expression
| decl_specifier_seq abstract_declarator.opt
| decl_specifier_seq abstract_declarator.opt '=' assignment_expression
//|        sc_signal_declaration '&' identifier
//| _CONST sc_signal_declaration '&' identifier
{
}


function_definition
: decl_specifier_seq.opt declarator function_body
  //| decl_specifier_seq.opt declarator ctor_initializer function_body
{
}


function_body
: compound_statement
{
}


initializer
: '=' initializer_clause
| '(' expression_list ')'
{
}



initializer_clause
: assignment_expression
| '{' initializer_list     '}'
| '{' initializer_list ',' '}'
| '{' '}'
{
}


initializer_list
: initializer_clause
| initializer_list ',' initializer_clause
{
}


//--------------------------------------
//Classes
  /*
class_name
: identifier
//| template_id
{
}
  */
  /*
class_specifier
: class_head '{'                      '}'
| class_head '{' member_specification '}'
{
}
  */
  /*
class_head
: class_key 
| class_key identifier
| class_key            base_clause
| class_key identifier base_clause
| class_key nested_name_specifier identifier 
| class_key nested_name_specifier identifier base_clause
| class_key                       template_id 
| class_key                       template_id base_clause
| class_key nested_name_specifier template_id 
| class_key nested_name_specifier template_id base_clause
{
}
  */

  /*
class_key
: _CLASS  {}
| _STRUCT {}
  */

  /*
member_specification
: member_declaration 
{
}
| member_declaration member_specification
{
}
| access_specifier ':'
{
}
| access_specifier ':' member_specification
{
}
  */

  /*
member_declaration
:                    ';'
{}
| decl_specifier_seq ';'
{}
|                    member_declarator_list ';'
{}
| decl_specifier_seq member_declarator_list ';'
{}
| function_definition 
{}
| function_definition ';'
{}
|                   nested_name_specifier           unqualified_id ';'
{}
//|                   nested_name_specifier _TEMPLATE unqualified_id ';'
| _SCOPE nested_name_specifier           unqualified_id ';'
{}
//| _SCOPE nested_name_specifier _TEMPLATE unqualified_id ';'
| using_declaration
//| template_declaration
{
}
  */

  /*
member_declarator_list
: member_declarator
{}
| member_declarator_list     member_declarator
{}
| member_declarator_list ',' member_declarator
{
}
  */

  /*
member_declarator
: declarator 
{}
| declarator pure_specifier
{}
| declarator 
{}
| declarator constant_initializer
{}
|            ':' constant_expression
{}
| identifier ':' constant_expression
{
}
  */

  /*
pure_specifier
: '=' '0'
{
}
  */

  /*
constant_initializer
: '=' constant_expression
{
}
  */

//--------------------------------------
// Derived classes
  /*
base_clause
: ':' base_specifier_list
{
}
  */
  /*
base_specifier_list
: base_specifier
{}
| base_specifier_list ',' base_specifier
{
}
  */

  /*
base_specifier
:                       class_name
{}
| nested_name_specifier class_name
{}
| _SCOPE                       class_name
{}
| _SCOPE nested_name_specifier class_name
{}
| _VIRTUAL                                                          class_name
{}
| _VIRTUAL access_specifier                                         class_name
{}
| _VIRTUAL                  _SCOPE                       class_name
{}
| _VIRTUAL access_specifier _SCOPE                       class_name
{}
| _VIRTUAL                                    nested_name_specifier class_name
{}
| _VIRTUAL access_specifier                   nested_name_specifier class_name
{}
| _VIRTUAL                  _SCOPE nested_name_specifier class_name
{}
| _VIRTUAL access_specifier _SCOPE nested_name_specifier class_name
{}
| access_specifier                                                  class_name
{}
| access_specifier _VIRTUAL                                         class_name
{}
| access_specifier          _SCOPE                       class_name
{}
| access_specifier _VIRTUAL _SCOPE                       class_name
{}
| access_specifier                            nested_name_specifier class_name
{}
| access_specifier _VIRTUAL                   nested_name_specifier class_name
{}
| access_specifier          _SCOPE nested_name_specifier class_name
{}
| access_specifier _VIRTUAL _SCOPE nested_name_specifier class_name
{
}
  */

  /*
access_specifier
: _PRIVATE
{}
| _PROTECTED
{}
| _PUBLIC
{}
  */

//--------------------------------------
// Special member functions
  /*
conversion_function_id
: _OPERATOR conversion_type_id
{
}
*/

  /*
conversion_type_id
: type_specifier_seq 
| type_specifier_seq conversion_declarator
{
}
*/

  /*
conversion_declarator
: ptr_operator 
{}
| ptr_operator conversion_declarator
{
}
*/

  /*
ctor_initializer
: ':' mem_initializer_list
{
}
*/

  /*
mem_initializer_list
: mem_initializer
{}
| mem_initializer ',' mem_initializer_list
{
}
*/

  /*
mem_initializer
: mem_initializer_id '('      ')'
{}
| mem_initializer_id '(' expression_list ')'
{
}
*/

  /*
mem_initializer_id
:                       class_name
{}
| nested_name_specifier class_name
{}
| _SCOPE                       class_name
{}
| _SCOPE nested_name_specifier class_name
{}
| identifier
{
}
  */


//--------------------------------------
// Overloading
  /*
operator_function_id
: _OPERATOR operator
{
}
  */

  /*
operator
: '+' {}
| '-' {}
| '*' {}
| '/' {}
| '%' {}
| '^' {}
| '&' {}
| '|' {}
| '~' {}
| '!' {}
| '=' {}
| '<' {}
| '>' {}
| _ADD_ASSIGN   {}
| _SUB_ASSIGN   {}
| _MUL_ASSIGN   {}
| _DIV_ASSIGN   {}
| _MOD_ASSIGN   {}
| _XOR_ASSIGN   {}
| _AND_ASSIGN   {}
| _OR_ASSIGN    {}
| _NOT_ASSIGN   {}
| _LSHIFT   {}
| _RSHIFT   {}
| _LSHIFT_ASSIGN   {}
| _RSHIFT_ASSIGN   {}
| _EQ   {}
| _NE   {}
| _LE   {}
| _GE   {}
| _AND   {}
| _OR   {}
| _INC   {}
| _DEC   {}
| ','   {}
| _ARROW_STAR   {}
| _ARROW   {}
| '(' ')'   {}
| '[' ']'   {}
  */


//--------------------------------------
// Template
/*
template_declaration
:         _TEMPLATE '<' template_parameter_list '>' declaration
{}
| _EXPORT _TEMPLATE '<' template_parameter_list '>' declaration
{
}

template_parameter_list
: template_parameter
{}
| template_parameter_list ',' template_parameter
{
}

template_parameter
: type_parameter
{}
| parameter_declaration
{
}

type_parameter
: _CLASS
{}
| _CLASS identifier
{}
| _CLASS            '=' type_id
{}
| _CLASS identifier '=' type_id
{}
| _TYPENAME 
{}
| _TYPENAME identifier
{}
| _TYPENAME            '=' type_id
{}
| _TYPENAME identifier '=' type_id
{}
| _TYPENAME '<' template_parameter_list '>' _CLASS 
{}
| _TYPENAME '<' template_parameter_list '>' _CLASS identifier
{}
| _TYPENAME '<' template_parameter_list '>' _CLASS            '=' id_expression
{}
| _TYPENAME '<' template_parameter_list '>' _CLASS identifier '=' id_expression
{
}
*/

  /*
template_id
: template_name '<' '>'
{}
| template_name '<' template_argument_list '>'
{
}

template_name
: identifier
{
}

template_argument_list
: template_argument
{}
| template_argument_list ',' template_argument
{
}

template_argument
: assignment_expression
{}
| type_id
{}
| id_expression
{
}
  */
/*
explicit_instantiation
: _TEMPLATE declaration
{
}

explicit_specialization
: _TEMPLATE '<' '>' declaration
{
}

*/

//--------------------------------------
// Preprocessing directivesb
/*
preprocessing-file
: [ group ]
{
}

group
: group-part
| group group-part
{
}

group-part
: [ pp-token ] new-line
| if-section
| control-line
{
}

if-section
: if-group [ elif-groups ] [ else-group ] endif-line
{
}

if-group
: '#' if constant-expression new-line [ group ]
| '#' ifdef identifier new-line [ group ]
| '#' ifndef identifier new-line [ group ]
{
}

elif-groups
: elif-group
| elif-groups elif-group

elif-group
: '#' elif constant-expression new-line [ group ]
{
}

else-group
: '#' else new-line [ group ]
{
}

endif-line
: '#' endif new-line
{
}

control-line
: '#' include pp-tokens new-line
| '#' define identifier replacement-list new-line
| '#' define identifier lparen [ identifier-list ] replacement-list new-line
| '#' undef identifier new-line
| '#' line pp-tokens new-line
| '#' error [ pp-tokens ] new-line
| '#' pragma [ pp-tokens ] new-line
| '#' new-line
{
}

lparen
: the left_parenthesis character without preceding white_space
{
}

replacement-list
: [ pp-tokens ]
{
}

pp-tokens
: preprocessing-token
| pp-tokens preprocessing-token
{
}

new-line
: the new_line character
{
}
*/
