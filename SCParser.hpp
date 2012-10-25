#ifndef __SC_PARSER_HPP__
#define __SC_PARSER_HPP__

#include "token.hpp"
#include "List.hpp"
#include "debug.h"
#include "TokenIterator.hpp"
#include "Scanner.hpp"

class SCParserAction;
class ActionQueue;
struct AST;

class SCParser
{
public:
	typedef bool (SCParser::*Rule)();
	typedef void (SCParserAction::*ActionFn)(const ref<TokenValue> &val);

	struct BackTrackInfo {
		TokenIterator ttag;
		size_t qtag;
	};

	SCParser();
	virtual ~SCParser();

	AST *parse(const char *file);
	AST *parseConstantExpression(const List<Token> &exps);
	int getLastError();

private:
	void back();
	bool nterm(Rule r, const char *rulename);
	bool ntermopt(Rule r, const char *rulename);
	bool term(int tokenid);
	bool termopt(int tokenid);
	void push(ActionFn fn, const ref<TokenValue> &val = ref<TokenValue>());
	bool empty();
	void addTypeName(const String &name);
	void addTypeNames(const List<String> &names);
	bool isTypeName(const String &name);
	void enterScope(const String &scope);
	void leaveScope();
	bool isScopeName(const String &name);
	String getCurrentScopeName();

#define nterm_(rule) nterm(&SCParser::rule, #rule)
#define ntermopt_(rule) ntermopt(&SCParser::rule, #rule)
#define term_(id) term(id)
#define termopt_(id) termopt(id)

	bool identifier();
	bool literal();
	bool keyword(int id);

	bool translation_unit();
	/*
	  A.4 Expressions
	*/
	bool primary_expression(); 
	bool id_expression(); 
	bool unqualified_id(); 
	bool qualified_id(); 
	bool nested_name_specifier(); 
	bool nested_name_specifier_sub();
	bool nested_opt_identifier(); 
	bool class_or_namespace_name();
	bool postfix_expression(); 
	bool cxxcast_id();
	bool cxxcast_expression(); 
	bool postfix_expression_prime();
	bool postfix_operator();
	bool expression_list(); 
	bool expression_list_prime(); 
	bool unary_expression(); 
	bool unary_operator(); 
	bool cast_expression(); 

	bool binary_expression();
	bool binary_expression_prime();
	bool binary_operator();

	bool multiplicative_expression(); 
	bool multiplicative_expression_prime();
	bool multiplicative_operator();
	bool additive_expression();
	bool additive_expression_prime();
	bool additive_operator();
	bool shift_expression();
	bool shift_expression_prime();
	bool shift_operator();
	bool relational_expression();
	bool relational_expression_prime();
	bool relational_operator();
	bool equality_expression();
	bool equality_expression_prime();
	bool equality_operator();
	bool and_expression();
	bool and_expression_prime();
	bool exclusive_or_expression();
	bool exclusive_or_expression_prime();
	bool inclusive_or_expression();
	bool inclusive_or_expression_prime();
	bool logical_and_expression();
	bool logical_and_expression_prime();
	bool logical_or_expression();
	bool logical_or_expression_prime();
	bool conditional_expression(); 
	bool conditional_expression_sub();
	bool assignment_expression(); 
	bool assignment_operator(); 
	bool expression(); 
	bool constant_expression(); 
	/*
	  A.5 Statements
	*/
	bool statement();
	bool labeled_statement();
	bool expression_statement();
	bool compound_statement();
	bool statement_seq();
	bool wait_statement();
	bool signal_assignment_statement();
	bool selection_statement();
	bool else_statement();
	bool condition();
	bool iteration_statement();
	bool for_init_statement();
	bool jump_statement();
	bool declaration_statement();
	/*
	  A.6 Declarations
	*/
	bool declaration_seq();
	bool declaration();
	bool block_declaration();
	bool simple_declaration();
	bool simple_declaration_without_decl_spec();
	bool decl_specifier(int *baseTypeDecl, int *userTypeDecl);
	bool decl_specifier_seq();
	bool type_specifier();
	bool simple_type_specifier();
	bool simple_type_specifier_user_type();
	bool builtin_type_specifier();
	bool type_name();
	bool elaborated_type_specifier();
	bool enum_specifier();
	bool enumerator_list();
	bool enumerator_list_prime();
	bool enumerator_definition();
	bool enumerator_definition_assign();
	bool namespace_definition();
	bool namespace_body();
	bool namespace_alias_definition();
	bool qualified_namespace_specifier();
	bool using_declaration();
	bool using_directive();
	/*
	  A.7 Declarators
	*/
	bool init_declarator_list();
	bool init_declarator_list_prime();
	bool init_declarator();
	bool declarator();
	bool direct_declarator();
	bool direct_declarator_prime();
	bool abstract_declarator();
	bool direct_abstract_declarator();
	bool ptr_operator();
	bool cv_qualifier_seq();
	bool cv_qualifier();
	bool declarator_id();
	bool type_id();
	bool type_specifier_seq();
	bool type_specifier_seq_prime();
	bool parameter_declaration_clause();
	bool parameter_declaration_list();
	bool parameter_declaration_list_prime();
	bool parameter_declaration();
	bool parameter_declaration_assign();
	bool function_definition();
	bool function_definition_without_decl_spec();
	bool function_body();
	bool initializer();
	bool initializer_clause();
	bool initializer_list();
	bool initializer_list_prime();
	/*
	  A.8 Classes
	*/
	bool class_specifier();
	bool class_head();
	bool class_key();
	bool member_specification();
	bool member_declaration();
	bool member_declarator_list();
	bool member_declarator_list_prime();
	bool member_declarator();
	bool pure_specifier();
	bool constant_initializer();
	/*
	  A.9 Derived classes
	*/
	bool base_clause();
	bool base_specifier_list();
	bool base_specifier_list_prime();
	bool base_specifier();
	bool access_specifier();
	/*
	  A.10 Special member functions
	*/
	bool conversion_function_id();
	bool conversion_type_id();
	bool conversion_declarator();
	bool ctor_initializer();
	bool mem_initializer_list();
	bool mem_initializer_list_prime();
	bool mem_initializer();
	bool mem_initializer_id();
	/*
	  A.11 Overloading
	*/
	bool operator_function_id();
	bool operator_id();
	/*
	  A.12 Templates
	*/
	bool template_declaration();
	bool template_parameter_list();
	bool template_parameter_list_prime();
	bool template_parameter();
	bool type_parameter();
	bool template_id();
	bool template_argument_list();
	bool template_argument_list_prime();
	bool template_argument();
	bool explicit_instantiation();
	bool explicit_specialization();

	Scanner scanner;
	String dbgTraceIndent;
	List<Token> tokens;
	TokenIterator tokenit;
	ActionQueue *actionQueue;
	List<BackTrackInfo> btstack;
	List<String> typeNames;
	List<String> scopeStack;
	List<String> scopeNames;
	Rule lastAcceptNterm;
	int lastAcceptTerm;
	bool hasTypedefDecl;
	List<String> currentDeclaratorIds;
	int templateArgNestLevel;
	List<int> expectedTemplateArgEndDelimiter;
	friend struct ReadyToBack;
};

#endif
