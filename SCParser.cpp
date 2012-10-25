#include <cstdio>
#include "SCParserAction.hpp"
#include "SCParser.hpp"
#include "defs.hpp"
#include "ref.hpp"
#include <cstring>
#include "parserUtils.h"
#include "Reporter.hpp"
#include "error.hpp"

struct Action
{
	SCParser::ActionFn fn;
	ref<TokenValue> val;
};
	
class ActionQueue
{
public:
	void moveHead()
	{
		it = acts.begin();
	}
	bool getNext(Action &a)
	{
		if (it != acts.end()) {
			a = *it;
			++it;
			return true;
		} else {
			return false;
		}
	}

	void push(const Action &a)
	{
		acts.push_back(a);
	}

	void push(const List<Action> &as)
	{
		acts.push_all(as);
	}

	//typedef List<Action>::iterator Tag;
	size_t tag()
	{
		return acts.size();
	}

	void rewind(size_t tag)
	{
		assert(tag <= acts.size());
		acts.resize(tag);
	}
		
	void copy(size_t index, List<Action> &copyact)
	{
		List<Action>::iterator i = acts.begin();
		std::advance(i, index);
		copyact.assign(i, acts.end());
	}
	List<Action> acts;
	List<Action>::iterator it;
};

struct AcceptResultCache
{
	SCParser::Rule rule;
	int length;
	List<Action> acts;
};

SCParser::SCParser()
	: actionQueue(NULL)
	, hasTypedefDecl(false)
	, templateArgNestLevel(0)
{
}

SCParser::~SCParser()
{
}

AST * 
SCParser::parse(const char *file)
{
	Reporter.reset();
	if (!scanner.scan(file, tokens)) {
		return NULL;
	}
	tokenit.setTokens(&tokens);

#if 0
	List<Token>::iterator it = tokens.begin();
	while (it != tokens.end()) {
		Token t = *it;
		printf("%s ", t.val->toString().c_str());
		if (t.id == _SEMICOLON) {
			printf("\n");
		}
		++it;
	}
	printf("\n");
	fflush(stdout);
#endif

	AST *ast = NULL;
	SCParserAction parserAction;
	actionQueue = new ActionQueue();
	if (translation_unit()) {
		Action a;
		actionQueue->moveHead();
		while (actionQueue->getNext(a)) {
			ActionFn fn = a.fn;
			(parserAction.*fn)(a.val);
		}
		ast = parserAction.getAST();
	}
	delete actionQueue;

	return ast;
}

//used for preprocessor #if directive
AST *
SCParser::parseConstantExpression(const List<Token> &exps)
{
	tokens = exps;
	tokenit.setTokens(&tokens);

	AST *ast = NULL;
	SCParserAction parserAction;
	actionQueue = new ActionQueue();
	if (constant_expression()) {
		Action a;
		actionQueue->moveHead();
		while (actionQueue->getNext(a)) {
			ActionFn fn = a.fn;
			(parserAction.*fn)(a.val);
		}
		ast = parserAction.getAST();
	}
	delete actionQueue;
	return ast;
}

int
SCParser::getLastError()
{
	return Reporter.getLastError();
}

struct ReadyToBack {
	SCParser &parser;
	ReadyToBack(SCParser &p) : parser(p) {
		SCParser::BackTrackInfo bti = {parser.tokenit, parser.actionQueue->tag()};
		parser.btstack.push_back(bti);
		parser.dbgTraceIndent += "  ";
	}
	~ReadyToBack() {
		parser.dbgTraceIndent.resize(parser.dbgTraceIndent.size()-2);
		parser.btstack.pop_back();
	}
};

void 
SCParser::back()
{
	const BackTrackInfo &bti = btstack.back();
	if (bti.qtag == actionQueue->tag()) {
		assert(tokenit == bti.ttag);
		return;
	}

	tokenit = bti.ttag;
	actionQueue->rewind(bti.qtag);
	PARSE_DBG("!!! back to '%s'", tokenit.val()->toString().c_str());
}

//#define ENABLE_ACCEPT_CACHE
bool 
SCParser::nterm(Rule r, const char *rulename)
{
	if (tokenit.eof()) {
		return false;
	}

#ifdef ENABLE_ACCEPT_CACHE
	const Token &t = tokenit.get();
	//List<AcceptResultCache*>::const_iterator it = t.resultCache.find_if();
	List<AcceptResultCache*>::const_iterator it = t.resultCache.begin();
	while (it != t.resultCache.end()) {
		AcceptResultCache *arc = *it;
		if (arc->rule == r) {
			TokenIterator start = tokenit;
			actionQueue->push(arc->acts);
			tokenit += arc->length;
			const Token &t1 = start.get(0);
			const Token &t2 = tokenit.get(-1);
			PARSE_DBG("'%s' -> '%s' is accepted as <%s> (by cache)", 
			t1.val->toString().c_str(),
			t2.val->toString().c_str(), 
			rulename);
			//lastAcceptNterm = r;
			return true;
		}
		++it;
	}
#endif

	ReadyToBack rtb(*this);
	
	PARSE_DBG("%stry <%s>", dbgTraceIndent.c_str(), rulename);
	bool ok = (this->*r)();

	if (ok) {
		BackTrackInfo bti = btstack.back();
#ifdef ENABLE_ACCEPT_CACHE
		AcceptResultCache *arc = new AcceptResultCache();
		arc->rule = r;
		arc->length = tokenit - bti.ttag;
		assert(0 <= arc->length);
		actionQueue->copy(bti.qtag, arc->acts);
		bti.ttag.pushAcceptResult(arc);
		//DBG("acts %d", arc->acts.size());
#endif
		const Token &t1 = bti.ttag.get();
		if (t1 == tokenit.get()) {
			//accept <empty>
			PARSE_DBG("%s<empty> is accepted as <%s>", 
					  dbgTraceIndent.c_str(),
					  rulename);
		} else {
			const Token &t2 = tokenit.get(-1);
			PARSE_DBG("%s'%s' -> '%s' is accepted as <%s>", 
					  dbgTraceIndent.c_str(),
					  t1.val->toString().c_str(),
					  t2.val->toString().c_str(), 
					  rulename);
		}
		//lastAcceptNterm = r;
		return true;
	} else {
		back();
		PARSE_DBG("%s<%s> is rejected", dbgTraceIndent.c_str(), rulename);
		return false;
	}
}

bool 
SCParser::ntermopt(Rule r, const char *rulename)
{
	if (tokenit.eof()) {
		DBG("%s is empty", rulename);
		push(&SCParserAction::onEmpty, new TokenString(rulename));
		return true;
	}

#ifdef ENABLE_ACCEPT_CACHE
	const Token &t = tokenit.get();
	//List<AcceptResultCache*>::const_iterator it = t.resultCache.find_if();
	List<AcceptResultCache*>::const_iterator it = t.resultCache.begin();
	while (it != t.resultCache.end()) {
		AcceptResultCache *arc = *it;
		if (arc->rule == r) {
			TokenIterator start = tokenit;
			actionQueue->push(arc->acts);
			tokenit += arc->length;
			const Token &t1 = start.get();
			const Token &t2 = tokenit.get(-1);
			PARSE_DBG("'%s' -> '%s' is accepted as <%s> (by cache)", 
			t1.val->toString().c_str(),
			t2.val->toString().c_str(), 
			rulename);
			//lastAcceptNterm = r;
			return true;
		}
		++it;
	}
#endif

	ReadyToBack rtb(*this);

	PARSE_DBG("%stry opt <%s>", dbgTraceIndent.c_str(), rulename);
	bool ok = (this->*r)();
	
	if (ok) {
		BackTrackInfo bti = btstack.back();
#ifdef ENABLE_ACCEPT_CACHE
		AcceptResultCache *arc = new AcceptResultCache();
		arc->rule = r;
		arc->length = tokenit - bti.ttag;
		assert(0 <= arc->length);
		actionQueue->copy(bti.qtag, arc->acts);
		bti.ttag.pushAcceptResult(arc);
		//DBG("acts %d", arc->acts.size());
#endif
		const Token &t1 = bti.ttag.get();
		const Token &t2 = tokenit.get(-1);
		PARSE_DBG("%s'%s' -> '%s' is accepted as <%s>", 
				  dbgTraceIndent.c_str(),
				  t1.val->toString().c_str(),
				  t2.val->toString().c_str(), 
				  rulename);
		//lastAcceptNterm = r;
	} else {
		back();
		PARSE_DBG("%s<%s> is empty", dbgTraceIndent.c_str(), rulename);
		push(&SCParserAction::onEmpty, new TokenString(rulename));
	}
	return true;
}

bool 
SCParser::term(int id)
{
	if (tokenit.eof()) {
		return false;
	}

	const String &idstr = tokenit.val()->toString();
	if (keyword(id)) {
		PARSE_DBG("%s'%s' is accepted", dbgTraceIndent.c_str(), idstr.c_str());
		//lastAcceptTerm = id;
		return true;
	} else {
		return false;
	}
}

bool 
SCParser::termopt(int id)
{
	if (tokenit.eof()) {
		push(&SCParserAction::onEmpty, new TokenString(""));
		return true;
	}

	const String &idstr = tokenit.val()->toString();
	if (keyword(id)) {
		PARSE_DBG("%s'%s' is accepted", dbgTraceIndent.c_str(), idstr.c_str());
		//lastAcceptTerm = id;
	} else {
		//DBG("%sterm<%x> is empty", dbgTraceIndent.c_str(), id);
		push(&SCParserAction::onEmpty, new TokenString(idstr+"***"));
	}
	return true;
}



void 
SCParser::push(ActionFn fn, const ref<TokenValue> &val)
{
	Action a = {fn, val};
	actionQueue->push(a);
}

bool
SCParser::empty()
{
	push(&SCParserAction::onEmpty, new TokenString(""));
	return true;
}


void 
SCParser::addTypeName(const String &name)
{
	DBG("addTypeName %s", name.c_str());
	typeNames.push_back(name);
}
void 
SCParser::addTypeNames(const List<String> &names)
{
	List<String>::const_iterator it = names.begin();
	while (it != names.end()) {
		addTypeName(*it);
		++it;
	}
}

bool 
SCParser::isTypeName(const String &name)
{
	return typeNames.contain(name);
}

void 
SCParser::enterScope(const String &scope)
{
	scopeStack.push_back(scope);
	scopeNames.push_back(scope);
}
void 
SCParser::leaveScope()
{
	scopeStack.pop_back();
}

bool
SCParser::isScopeName(const String &name)
{
	return scopeNames.contain(name);
}

String
SCParser::getCurrentScopeName()
{
	String s;
	List<String>::iterator it = scopeStack.begin();
	while (it != scopeStack.end()) {
		s += *it;
		s += "::";
		++it;
	}
	return s;
}

//tentative
#define PARSE_ERROR(id)												\
	do {															\
		if (Reporter.getLastError() == NO_ERROR) {					\
			printf("parse error [line %d] %s\n", __LINE__, #id);	\
			Reporter.error(id);										\
			/*assert(0);*/											\
		}															\
		return false;												\
	}while (false)


bool
SCParser::identifier()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_IDENT)) {
		push(&SCParserAction::onIdentifier, val);
		return true;
	}

	return false;
}

bool
SCParser::literal()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_DECIMAL_CONSTANT)) {
		push(&SCParserAction::onLiteral_Decimal, val);
		return true;
	} 
	if (tokenit.eat(_OCTAL_CONSTANT)) {
		push(&SCParserAction::onLiteral_Octal, val);
		return true;
	}
	if (tokenit.eat(_HEX_CONSTANT)) {
		push(&SCParserAction::onLiteral_Hex, val);
		return true;
	}
	if (tokenit.eat(_STRING_CONSTANT)) {
		push(&SCParserAction::onLiteral_String, val);
		return true;
	}
	if (tokenit.eat(_TRUE)) {
		push(&SCParserAction::onLiteral_True, val);
		return true;
	}
	if (tokenit.eat(_FALSE)) {
		push(&SCParserAction::onLiteral_False, val);
		return true;
	}

	return false;
}

bool
SCParser::keyword(int id)
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(id)) {
		push(&SCParserAction::onToken, val);
		return true;
	}
	return false;
}

/*
  translation-unit ::=
  [ declaration-seq ] [ sc-main-definition ]
*/
bool 
SCParser::translation_unit()
{
	if (ntermopt_(declaration_seq)) {
		push(&SCParserAction::onTranslationUnit);
		return true;
	}
	return false;
}

/*
  A.4 Expressions
*/

/*
  primary-expression ::=
  literal
  | this
  | ( expression_list )
  | id-expression
*/
bool
SCParser::primary_expression() 
{
	if (nterm_(literal)) {
		push(&SCParserAction::onPrimaryExp_Literal);
		return true;
	}

	if (tokenit.eat(_THIS)) {
		push(&SCParserAction::onPrimaryExp_This);
		return true;
	}

	if (term_(_LPAREN)) {
		++templateArgNestLevel;
		if (nterm_(expression_list) && 
			term_(_RPAREN) )  {
			--templateArgNestLevel;
			push(&SCParserAction::onPrimaryExp_Expression);
			return true;
		}
			
		PARSE_ERROR(PA_PRIMARY_EXP_PARENTHESIS_NOT_CLOSE);
	}
				
	if (nterm_(id_expression)) {
		push(&SCParserAction::onPrimaryExp_IdExp);
		return true;
	}

	return false;
}

/*
  id-expression ::=
  unqualified-id
  | qualified-id
*/
bool
SCParser::id_expression() 
{
	if (nterm_(qualified_id)) {
		push(&SCParserAction::onIdExp_QualifiedId);
		return true;
	}

	if (nterm_(unqualified_id)) {
		push(&SCParserAction::onIdExp_UnqualifiedId);
		return true;
	}

	return false;
}

/*
  unqualified-id ::=
  identifier
  | operator-function-id
  | conversion-function-id
  //| ~class-name
  | template-id
*/
bool
SCParser::unqualified_id() 
{
	const int t0 = tokenit.get(0).id;
	const int t1 = tokenit.get(1).id;
	if (t0 == _IDENT) {
		if (t1 == _LT) {
			if (nterm_(template_id)) {
				push(&SCParserAction::onUnqualifiedId_TemplateId);
				return true;
			}
			//This case is not the parse error because "ident '<'" is valid expression syntax
		}
		if (nterm_(identifier)) {
			push(&SCParserAction::onUnqualifiedId_Identifier);
			return true;
		}
		//should not reach here
		assert(0);
	}

	if (t0 == _OPERATOR) {
		if (nterm_(operator_function_id)) {
			push(&SCParserAction::onUnqualifiedId_OperatorId);
			return true;
		}

		if (nterm_(conversion_function_id)) {
			push(&SCParserAction::onUnqualifiedId_ConversionId);
			return true;
		}
	}

	return false;
}

/*
  qualified-id ::=
  [ :: ] nested-name-specifier [ template ] unqualified-id
  | :: identifier
  | :: operator-function-id
  | :: template-id
*/
bool
SCParser::qualified_id() 
{
	if (termopt_(_SCOPE) && nterm_(nested_name_specifier)) {
		if (termopt_(_TEMPLATE) &&
			nterm_(unqualified_id)) {
			push(&SCParserAction::onQualifiedId_Valid1);
			return true;
		}
		assert(0);
		PARSE_ERROR(0);
	} 
	back();

	if (term_(_SCOPE)) {
		if (nterm_(template_id)) {
			push(&SCParserAction::onQualifiedId_TemplateId);
			return true;
		}
			
		if (nterm_(identifier)) {
			//TODO: check '<' for template-id
			push(&SCParserAction::onQualifiedId_Identifier);
			return true;
		} 
			
		if (nterm_(operator_function_id)) {
			push(&SCParserAction::onQualifiedId_OperatorFuncId);
			return true;
		}
			
		PARSE_ERROR(PA_SCOPE_IDENTIFIER);
	}
		
	return false;
}

/*
  nested-name-specifier ::=
  class-or-namespace-name :: [ nested-name-specifier ]
  | class-or-namespace-name :: template nested-name-specifier
*/
bool
SCParser::nested_name_specifier() 
{
	if (nterm_(class_or_namespace_name) &&
		term_(_SCOPE)) {
		nterm_(nested_name_specifier_sub);
		return true;
	}
	return false;
}

bool
SCParser::nested_name_specifier_sub()
{
	//TODO: confilict 
	//A::template B<void>;
	//A::template B::C template D<int>;
	if (term_(_TEMPLATE) &&
		nterm_(nested_name_specifier)) {
		push(&SCParserAction::onNestedNameSpecifier_Valid2);
		return true;
	}
	back();

	if (ntermopt_(nested_name_specifier)) {
		push(&SCParserAction::onNestedNameSpecifier_Valid1);
		return true;
	}
	return false;
}

/*
  nested-opt-identifier ::=
  [ :: ] [ nested-name-specifier ] identifier
*/
bool
SCParser::nested_opt_identifier()
{
	if (termopt_(_SCOPE) && 
		ntermopt_(nested_name_specifier) &&
		nterm_(identifier)) {
		return true;
	}
	return false;
}
 
bool
SCParser::class_or_namespace_name()
{
	const Token &t0 = tokenit.get(0);
	const Token &t1 = tokenit.get(1);
	if (t0.id == _IDENT && t1.id == _SCOPE) {
		if (isScopeName(t0.val->toString())) {
			nterm_(identifier);
			push(&SCParserAction::onClassOrNamespaceName_Identifier);
			return true;
		} else {
		}
	}
	if (t0.id == _IDENT && t1.id == _LT) {
		if (isScopeName(t0.val->toString())) {
			if (nterm_(template_id)) {
				push(&SCParserAction::onClassOrNamespaceName_TemplateId);
				return true;
			}
		}
	}
	return false;
}

/*
  postfix-expression ::=
  primary-expression postfix-expression'
  | simple-type-specifier ( [ expression-list ] ) postfix-expression'
  | typename [ :: ] nested-name-specifier identifier ( [ expression-list ] ) postfix-expression'
  | typename [ :: ] nested-name-specifier [template] template-id ( [ expression-list ] ) postfix-expression'
  | cxxcast_id cxxcast_expression postfix-expression'
*/
bool
SCParser::postfix_expression() 
{
	if (nterm_(primary_expression) && 
		ntermopt_(postfix_expression_prime)) {
		push(&SCParserAction::onPostfixExp_PrimaryExp);
		return true;
	}
		
	if (nterm_(simple_type_specifier)) {
		if (term_(_LPAREN) && 
			ntermopt_(expression_list) &&
			term_(_RPAREN) && 
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp_SimpleTypeSpecifier);
			return true;
		}
		//PARSE_ERROR(0);
		//is declaration ?
		return false;
	}

	if (term_(_TYPENAME)) {
		//TODO:
		PARSE_ERROR(0);
	}

	if (nterm_(cxxcast_id)) {
		if (nterm_(cxxcast_expression) &&
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onCxxCastExp);
			return true;
		}
		PARSE_ERROR(PA_CXXCAST_NOT_VALID);
	}
	return false;
}

/*
  cxxcast_id ::=  dynamic_cast | static_cast | reinterpret_cast | const_cast
*/
bool
SCParser::cxxcast_id()
{
	const ref<TokenValue> &val = tokenit.val();
	//DBG("%s %d", val->toString().c_str(), val->toInt());
	if (tokenit.eat(_DYNAMIC_CAST) ||
		tokenit.eat(_STATIC_CAST) ||
		tokenit.eat(_REINTERPRET_CAST) ||
		tokenit.eat(_CONST_CAST)) {
		assert(val->isInt());
		push(&SCParserAction::onCxxCastId, val);
		return true;
	}
	return false;
} 
/*
  cxxcast_expression ::= < type-id > ( expression_list ) postfix-expression'
*/
bool
SCParser::cxxcast_expression() 
{
	if (term_(_LT) &&
		nterm_(type_id) &&
		term_(_GT) &&
		term_(_LPAREN) &&
		nterm_(expression_list) &&
		term_(_RPAREN)){
		return true;
	}
	return false;
}
/*
  postfix-expression' ::=
  empty
  | '[' expression_list ']' postfix-expression'
  | ( [ expression-list ] ) postfix-expression'
  | .  [ template ] id-expression postfix-expression'
  | -> [ template ] id-expression postfix-expression'
  | ++ postfix-expression'
  | -- postfix-expression'

*/
bool
SCParser::postfix_expression_prime()
{
	if (term_(_LBRACKET)) {
		if (nterm_(expression_list) &&
			term_(_RBRACKET) && 
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp_Subscript);
			return true;
		}
		PARSE_ERROR(PA_POSTFIX_EXP_BRACKET_NOT_CLOSE);
	}

	if (term_(_LPAREN)) {
		if (ntermopt_(expression_list) &&
			term_(_RPAREN) && 
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp_ExpressionList);
			return true;
		}
		PARSE_ERROR(PA_POSTFIX_EXP_PARENTHESIS_NOT_CLOSE);
	}

	if (term_(_DOT)) {
		if (termopt_(_TEMPLATE) &&
			nterm_(id_expression) &&
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp_MemberSelect);
			return true;
		}
			
		PARSE_ERROR(PA_DOT_OP_NOT_VALID);
	}
	if (term_(_ARROW)) {
		if (termopt_(_TEMPLATE) &&
			nterm_(id_expression) &&
			ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp_ArrowMemberSelect);
			return true;
		}

		PARSE_ERROR(PA_ARROW_OP_NOT_VALID);
	}

	if (nterm_(postfix_operator)) {
		if (ntermopt_(postfix_expression_prime)) {
			push(&SCParserAction::onPostfixExp);
			return true;
		}

		PARSE_ERROR(PA_POSTFIX_EXP_NOT_VALID);
	}

	return false;
}

bool
SCParser::postfix_operator()
{
	if (tokenit.eat(_INC)) {
		push(&SCParserAction::onPostfixOperator, new TokenOp("++", POST_INC));
		return true;
	} else if (tokenit.eat(_DEC)) {
		push(&SCParserAction::onPostfixOperator, new TokenOp("--", POST_DEC));
		return true;
	}
	return false;
}

/*
  expression-list ::=
  expression expression-list'

  expression-list' ::=
  empty
  | , expression expression-list'
*/
bool
SCParser::expression_list() 
{
	if (nterm_(expression) &&
		ntermopt_(expression_list_prime)) {
		push(&SCParserAction::onExpressionListHead);
		return true;
	}
			
	return false;
}

bool
SCParser::expression_list_prime() 
{
	if (term_(_COMMA)) {
		if (nterm_(expression) &&
			ntermopt_(expression_list_prime)) {
			push(&SCParserAction::onExpressionListRest);
			return true;
		}
		PARSE_ERROR(PA_EXP_EXPECTED);
	}
	return false;
}

/*
  unary-expression ::=
  postfix-expression
  | ++ cast-expression
  | -- cast-expression
  | unary-operator cast-expression
*/
bool
SCParser::unary_expression() 
{
	if (nterm_(postfix_expression)) {
		return true;
	}

	if (nterm_(unary_operator)) {
		if (nterm_(cast_expression)) {
			push(&SCParserAction::onUnaryExp);
			return true;
		}
		PARSE_ERROR(PA_UNARY_EXP_NOT_VALID);
	}

	return false;
}

/*
  unary-operator ::= one of
  * & + - ! ~
  */
bool
SCParser::unary_operator() 
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_INC) ||
		tokenit.eat(_DEC) ||
		tokenit.eat(_BANG) ||
		tokenit.eat(_TILDE)) {
		push(&SCParserAction::onUnaryOperator, val);
		return true;
	} else if (tokenit.eat(_STAR)) {
		push(&SCParserAction::onUnaryOperator, new TokenOp("*", UNARY_STAR));
		return true;
	} else if (tokenit.eat(_BIT_AND)) {
		push(&SCParserAction::onUnaryOperator, new TokenOp("&", UNARY_AND));
		return true;
	} else if (tokenit.eat(_PLUS)) {
		push(&SCParserAction::onUnaryOperator, new TokenOp("+", UNARY_PLUS));
		return true;
	} else if (tokenit.eat(_MINUS)) {
		push(&SCParserAction::onUnaryOperator, new TokenOp("-", UNARY_MINUS));
		return true;
	}
		
	return false;
}

/*
  cast-expression ::=
  unary-expression
  | ( type_id ) cast-expression
*/
bool
SCParser::cast_expression() 
{
	if (term_(_LPAREN) &&
		nterm_(type_id) &&
		term_(_RPAREN) &&
		nterm_(cast_expression)) {
		push(&SCParserAction::onCastExp);
		return true;
	}
	back();
	if (nterm_(unary_expression)) {
		return true;
	}
	return false;
}

bool 
SCParser::binary_expression()
{
	if (nterm_(cast_expression) &&
		ntermopt_(binary_expression_prime)) {
		push(&SCParserAction::onBinaryExpHead);
		return true;
	}

	return false;
}

bool 
SCParser::binary_expression_prime()
{
	if (nterm_(binary_operator)) {
		if (nterm_(cast_expression) &&
			ntermopt_(binary_expression_prime)) {
			push(&SCParserAction::onBinaryExp);
			return true;
		}
		PARSE_ERROR(PA_BINARY_EXP_NOT_VALID);
	}
	return false;
}

bool 
SCParser::binary_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (expectedTemplateArgEndDelimiter.back() == templateArgNestLevel) {
		if (tokenit.is(_GT)) {
			return false;
		}
	}
	if (tokenit.eat(_STAR) ||
		tokenit.eat(_SLASH) ||
		tokenit.eat(_PERCENT) ||
		tokenit.eat(_PLUS) ||
		tokenit.eat(_MINUS) ||
		tokenit.eat(_LSHIFT) ||
		tokenit.eat(_RSHIFT) ||
		tokenit.eat(_LT) ||
		tokenit.eat(_GT) ||
		tokenit.eat(_LE) ||
		tokenit.eat(_GE) ||
		tokenit.eat(_EQ) ||
		tokenit.eat(_NE) ||
		tokenit.eat(_BIT_AND) ||
		tokenit.eat(_BIT_XOR) ||
		tokenit.eat(_BIT_OR) ||
		tokenit.eat(_AND) ||
		tokenit.eat(_OR)) {
		push(&SCParserAction::onBinaryOperator, val);
		return true;
	}
	return false;
}

/*
  multiplicative-expression ::=
  cast-expression multiplicative-expression'

  multiplicative-expression' ::=
  empty
  | * cast-expression multiplicative-expression'
  | / cast-expression multiplicative-expression'
  | % cast-expression multiplicative-expression'
*/
#if 0
bool
SCParser::multiplicative_expression() 
{
	if (nterm_(cast_expression) &&
		ntermopt_(multiplicative_expression_prime)) {
		push(&SCParserAction::onMultiplicativeExpHead);
		return true;
	}

	return false;
}
bool
SCParser::multiplicative_expression_prime()
{
	if (nterm_(multiplicative_operator) &&
		nterm_(cast_expression) &&
		ntermopt_(multiplicative_expression_prime)) {
		push(&SCParserAction::onMultiplicativeExp);
		return true;
	}
	return false;
}

bool
SCParser::multiplicative_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_STAR) ||
		tokenit.eat(_SLASH) ||
		tokenit.eat(_PERCENT)) {
		push(&SCParserAction::onMultiplicativeOperator, val);
		return true;
	}
	return false;
}
	
/*
  additive-expression ::=
  multiplicative-expression additive-expression'

  additive-expression' ::=
  empty
  | + multiplicative-expression additive-expression'
  | - multiplicative-expression additive-expression'
	  
*/
bool
SCParser::additive_expression()
{
	if (nterm_(multiplicative_expression) &&
		ntermopt_(additive_expression_prime)) {
		push(&SCParserAction::onAdditiveExpHead);
		return true;
	}
	return false;
}

bool
SCParser::additive_expression_prime()
{
	if (nterm_(additive_operator) &&
		nterm_(multiplicative_expression) &&
		ntermopt_(additive_expression_prime)) {
		push(&SCParserAction::onAdditiveExp);
		return true;
	}
	return false;
}

bool
SCParser::additive_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_PLUS) ||
		tokenit.eat(_MINUS)) {
		push(&SCParserAction::onAdditiveOperator, val);
		return true;
	}
	return false;
}
	
/*
  shift-expression ::=
  additive-expression shift-expression'

  shift-expression' ::=
  empty
  | << additive-expression shift-expression'
  | >> additive-expression shift-expression'
*/
bool
SCParser::shift_expression()
{
	if (nterm_(additive_expression) &&
		ntermopt_(shift_expression_prime)) {
		push(&SCParserAction::onShiftExpHead);
		return true;
	}
	return false;
}

bool
SCParser::shift_expression_prime()
{
	if (nterm_(shift_operator) &&
		nterm_(additive_expression) &&
		ntermopt_(shift_expression_prime)) {
		push(&SCParserAction::onShiftExp);
		return true;
	}
	return false;
}

bool
SCParser::shift_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_LSHIFT) ||
		tokenit.eat(_RSHIFT)) {
		push(&SCParserAction::onShiftOperator, val);
		return true;
	}
	return false;
}

/*
  relational-expression ::=
  shift-expression relational-expression'

  relational-expression' ::=
  empty
  |  < shift-expression relational-expression'
  | > shift-expression  relational-expression'
  | <= shift-expression relational-expression'
  | >= shift-expression relational-expression'
*/
bool
SCParser::relational_expression()
{
	if (nterm_(shift_expression) &&
		ntermopt_(relational_expression_prime)) {
		push(&SCParserAction::onRelationalExpHead);
		return true;
	}
	return false;
}

bool
SCParser::relational_expression_prime()
{
	if (nterm_(relational_operator) &&
		nterm_(shift_expression) &&
		ntermopt_(relational_expression_prime)) {
		push(&SCParserAction::onRelationalExp);
		return true;
	}
	return false;
}

bool
SCParser::relational_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_LT) ||
		tokenit.eat(_GT) ||
		tokenit.eat(_LE) ||
		tokenit.eat(_GE)) {
		push(&SCParserAction::onRelationalOperator, val);
		return true;
	}
	return false;
}

/*
  equality-expression ::=
  relational-expression equality-expression'

  equality-expression' ::=
  empty
  | == relational-expression equality-expression'
  | != relational-expression equality-expression'
*/
bool
SCParser::equality_expression()
{
	if (nterm_(relational_expression) &&
		ntermopt_(equality_expression_prime)) {
		push(&SCParserAction::onEqualityExpHead);
		return true;
	}
	return false;
}

bool
SCParser::equality_expression_prime()
{
	if (nterm_(equality_operator) &&
		nterm_(relational_expression) &&
		ntermopt_(equality_expression_prime)) {
		push(&SCParserAction::onEqualityExp);
		return true;
	}
	return false;
}

bool
SCParser::equality_operator()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_EQ) ||
		tokenit.eat(_NE)) {
		push(&SCParserAction::onEqualityOperator, val);
		return true;
	}
	return false;
}
	
/*
  and-expression ::=
  equality-expression and-expression'

  and-expression' ::=
  empty
  | & equality-expression and-expression'
*/
bool
SCParser::and_expression()
{
	if (nterm_(equality_expression) &&
		ntermopt_(and_expression_prime)) {
		push(&SCParserAction::onAndExpHead);
		return true;
	}
	return false;
}

bool
SCParser::and_expression_prime()
{
	if (term_(_BIT_AND) &&
		nterm_(equality_expression) &&
		ntermopt_(and_expression_prime)) {
		push(&SCParserAction::onAndExp);
		return true;
	}
	return false;
}

/*
  exclusive-or-expression ::=
  and-expression exclusive-or-expression'

  exclusive-or-expression' ::=
  empty
  | ^ and-expression exclusive-or-expression'
*/
bool
SCParser::exclusive_or_expression()
{
	if (nterm_(and_expression) &&
		ntermopt_(exclusive_or_expression_prime)) {
		push(&SCParserAction::onExclusiveOrExpHead);
		return true;
	}
	return false;
}

bool
SCParser::exclusive_or_expression_prime()
{
	if (term_(_BIT_XOR) &&
		nterm_(and_expression) &&
		ntermopt_(exclusive_or_expression_prime)) {
		push(&SCParserAction::onExclusiveOrExp);
		return true;
	}
	return false;
}

/*
  inclusive-or-expression ::=
  exclusive-or-expression inclusive-or-expression'

  'inclusive-or-expression ::=
  empty
  | | exclusive-or-expression inclusive-or-expression'
*/
bool
SCParser::inclusive_or_expression()
{
	if (nterm_(exclusive_or_expression) &&
		ntermopt_(inclusive_or_expression_prime)) {
		push(&SCParserAction::onInclusiveOrExpHead);
		return true;
	}
	return false;
}

bool
SCParser::inclusive_or_expression_prime()
{
	if (term_(_BIT_OR) &&
		nterm_(exclusive_or_expression) &&
		ntermopt_(inclusive_or_expression_prime)) {
		push(&SCParserAction::onInclusiveOrExp);
		return true;
	}
	return false;
}

/*
  logical-and-expression ::=
  inclusive-or-expression logical-and-expression'

  logical-and-expression' ::=
  empty
  | && inclusive-or-expression logical-and-expression'
*/
bool
SCParser::logical_and_expression()
{
	if (nterm_(inclusive_or_expression) &&
		ntermopt_(logical_and_expression_prime)) {
		push(&SCParserAction::onLogicalAndExpHead);
		return true;
	}
	return false;
}

bool
SCParser::logical_and_expression_prime()
{
	if (term_(_AND) &&
		nterm_(inclusive_or_expression) &&
		ntermopt_(logical_and_expression_prime)) {
		push(&SCParserAction::onLogicalAndExp);
		return true;
	}
	return false;
}
	

/*
  logical-or-expression ::=
  logical-and-expression logical-or-expression'

  logical-or-expression' ::=
  empty
  | || logical-and-expression logical-or-expression'
*/
bool
SCParser::logical_or_expression()
{
	if (nterm_(logical_and_expression) &&
		ntermopt_(logical_or_expression_prime)) {
		push(&SCParserAction::onLogicalOrExpHead);
		return true;
	}
	return false;
}

bool
SCParser::logical_or_expression_prime()
{
	if (term_(_OR) &&
		nterm_(logical_and_expression) &&
		ntermopt_(logical_or_expression_prime)) {
		push(&SCParserAction::onLogicalOrExp);
		return true;
	}
	return false;
}

#endif

/*
  conditional-expression ::=
  logical-or-expression
  | logical-or-expression ? expression-list : assignment-expression
*/
bool
SCParser::conditional_expression() 
{
	//if (nterm_(logical_or_expression)) {
	if (nterm_(binary_expression)) {
		if (nterm_(conditional_expression_sub)) {
			push(&SCParserAction::onConditionalExp);
			return true;
		} else {
			return true;
		}
	}
	return false;
}

bool
SCParser::conditional_expression_sub()
{
	if (term_(_QUESTION)) {
		if (nterm_(expression_list) &&
			term_(_COLON) &&
			nterm_(assignment_expression)) {
			return true;
		}
		PARSE_ERROR(PA_CONDITIONAL_EXP_NOT_VALID);
	}
	return false;
}

/*
  assignment-expression ::=
  conditional-expression
  | logical-or-expression assignment-operator assignment-expression
*/
bool
SCParser::assignment_expression() 
{
	//DBG("%s", tokenit.val()->toString().c_str());
	if (!tokenit.isFollow(EXP_FOLLOW_MASK)) {
		return false;
	}

	//if (nterm_(logical_or_expression)) {
	if (nterm_(binary_expression)) {
		if (nterm_(assignment_operator)) {
			if (nterm_(assignment_expression)) {
				push(&SCParserAction::onAssignExp);
				return true;
			}
			PARSE_ERROR(PA_ASSIGNMENT_EXP_NOT_VALID);
		}

		if (nterm_(conditional_expression_sub)) {
			push(&SCParserAction::onConditionalExp);
			return true;
		}
		//logical_or_expression
		return true;
	}

	return false;
}
/*
  assignment-operator ::= one of
  = *= /= %= += -= >>= <<= &= ^= |=
*/
bool
SCParser::assignment_operator() 
{
	if (!tokenit.isFollow(ASSIGN_OP_FOLOW)) {
		return false;
	}
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_ASSIGN) ||
		tokenit.eat(_MUL_ASSIGN) ||
		tokenit.eat(_DIV_ASSIGN) ||
		tokenit.eat(_MOD_ASSIGN) ||
		tokenit.eat(_ADD_ASSIGN) ||
		tokenit.eat(_SUB_ASSIGN) ||
		tokenit.eat(_RSHIFT_ASSIGN) ||
		tokenit.eat(_LSHIFT_ASSIGN) ||
		tokenit.eat(_AND_ASSIGN) ||
		tokenit.eat(_OR_ASSIGN) ||
		tokenit.eat(_XOR_ASSIGN)) {
		push(&SCParserAction::onAssignOperator, val);
		return true;
	}

	return false;
}

/*
  expression ::=
  assignment-expression
*/
bool
SCParser::expression() 
{
	//DBG("%s", tokenit.val()->toString().c_str());
	if (!tokenit.isFollow(EXP_FOLLOW_MASK)) {
		return false;
	}
	if (nterm_(assignment_expression)) {
		return true;
	}
	return false;
}

/*
  constant-expression ::=
  conditional-expression
*/
bool
SCParser::constant_expression() 
{
	//DBG("%s", tokenit.val()->toString().c_str());
	if (!tokenit.isFollow(EXP_FOLLOW_MASK)) {
		return false;
	}
	if (nterm_(conditional_expression)) {
		return true;
	}
	return false;
}

/*
  A.5 Statements
*/

/*
  statement ::=
  labeled-statement
  | expression-statement
  | compound-statement
  | wait-statement
  | signal-assignment-statement
  | selection-statement
  | iteration-statement
  | jump-statement
  | declaration-statement
*/
bool
SCParser::statement()
{
	if (nterm_(labeled_statement) ||
		nterm_(compound_statement) ||
		nterm_(expression_statement) ||
		//nterm_(wait_statement) ||
		//nterm_(signal_assignment_statement) ||
		nterm_(selection_statement) ||
		nterm_(iteration_statement) ||
		nterm_(jump_statement) ||
		nterm_(declaration_statement)) {
		return true;
	}
	return false;
}

/*
  labeled-statement ::=
  identifier : statement
  | case constant-expression : statement
  | default : statement
*/
bool
SCParser::labeled_statement()
{
	if (nterm_(identifier) && term_(_COLON)) {
		if (nterm_(statement)) {
			push(&SCParserAction::onLabeledStm);
			return true;
		}
		PARSE_ERROR(PA_LABEL_STATEMENT_NOT_VALID);
	}
	back();

	if (term_(_CASE)) {
		if (nterm_(constant_expression) &&
			term_(_COLON) &&
			nterm_(statement)) {
			push(&SCParserAction::onCaseStm);
			return true;
		}
		PARSE_ERROR(PA_CASE_STATEMENT_NOT_VALID);
	}

	if (term_(_DEFAULT)) {
		if (term_(_COLON) &&
			nterm_(statement)) {
			push(&SCParserAction::onDefaultCaseStm);
			return true;
		}
		PARSE_ERROR(PA_DEFAULT_STATEMENT_NOT_VALID);
	}
	return false;
}

/*
  expression_statement ::=
  [ expression-list ] ;
*/
bool
SCParser::expression_statement()
{
	if (ntermopt_(expression_list)) {
		if (term_(_SEMICOLON)) {
			push(&SCParserAction::onExpressionStm);
			return true;
		}
		//PARSE_ERROR(0);
	}
	return false;
}

/*
  compound-statement ::=
  { [ statement_seq ] }
*/
bool
SCParser::compound_statement()
{
	if (term_(_LBRACE)) {
		ntermopt_(statement_seq);
		if (term_(_RBRACE)) {
			push(&SCParserAction::onCompoundStm);
			return true;
		} else {
			//'}' is not found
			PARSE_ERROR(PA_COMPOUND_STATEMENT_NOT_CLOSE);
		}
	}
	return false;
}

/*
  statement-seq ::=
  statement [statement-seq]
*/
bool
SCParser::statement_seq()
{
	if (nterm_(statement) &&
		ntermopt_(statement_seq)) {
		push(&SCParserAction::onStatementSeq);
		return true;
	}
	return false;
}

/*
  wait-statement ::=
  wait ( [constant-expression] ) ;
*/
bool
SCParser::wait_statement()
{
	return false;
}

/*
  signal-assignment-statement ::=
  signal-or-port-identifier . write ( expression-list ) ;
  | signal-or-port-identifier = expression-list ;
*/
bool
SCParser::signal_assignment_statement()
{
	return false;
}

/*
  selection_statement ::=
  if ( condition ) statement
  | if ( condition ) statement else statement
  | switch ( condition ) statement
*/
bool
SCParser::selection_statement()
{
	if (term_(_IF)) {
		if (term_(_LPAREN) &&
			nterm_(condition) && term_(_RPAREN) &&
			nterm_(statement)) {
			if (nterm_(else_statement)) {
				push(&SCParserAction::onIfElseStm);			
			} else {
				push(&SCParserAction::onIfStm);
			}
			return true;
		}
		PARSE_ERROR(PA_IF_STATEMENT_NOT_VALID);
	}

	if (term_(_SWITCH)) {
		if (term_(_LPAREN) &&
			nterm_(condition) && term_(_RPAREN) &&
			nterm_(statement)) {
			push(&SCParserAction::onSwitchStm);
			return true;
		}
		PARSE_ERROR(PA_SWITCH_STATEMENT_NOT_VALID);
	}
	return false;
}
bool
SCParser::else_statement()
{
	if (term_(_ELSE)) {
		if (nterm_(statement)) {
			return true;
		}
		PARSE_ERROR(PA_ELSE_STATEMENT_NOT_VALID);
	}
	return false;

}
/*
  condition ::=
  expression-list
  | type_specifier_seq declarator = assignment_expression
*/
bool
SCParser::condition()
{
	if (nterm_(expression_list)) {
		push(&SCParserAction::onCondition_Exp);
		return true;
	}
	
	if (nterm_(type_specifier_seq)) { 
		if (nterm_(declarator) &&
			term_(_ASSIGN) &&
			nterm_(assignment_expression)) {
			push(&SCParserAction::onCondition_AssignExp);
			return true;
		}
		PARSE_ERROR(PA_CONDITION_NOT_VALID);
	}
	
	return false;
}

/*
  iteration-statement ::=
  while ( condition ) statement
  | do statement while ( expression-list ) ;
  | for ( for-init-statement [ condition ] ; [ expression-list ] ) statement
*/
bool
SCParser::iteration_statement()
{
	if (term_(_WHILE)) {
		if (term_(_LPAREN) &&
			nterm_(condition) &&
			term_(_RPAREN) &&
			nterm_(statement)) {
			push(&SCParserAction::onWhileStm);
			return true;
		}
		PARSE_ERROR(PA_WHILE_STATEMENT_NOT_VALID);
	}

	if (term_(_DO)) {
		if (nterm_(statement) &&
			term_(_WHILE) &&
			term_(_LPAREN) &&
			nterm_(expression_list) &&
			term_(_RPAREN) &&
			term_(_SEMICOLON)) {
			push(&SCParserAction::onDoWhileStm);
			return true;
		}
		PARSE_ERROR(PA_DO_STATEMENT_NOT_VALID);
	}

	if (term_(_FOR)) {
		if (term_(_LPAREN) &&
			nterm_(for_init_statement) &&
			ntermopt_(condition) &&
			term_(_SEMICOLON) &&
			ntermopt_(expression_list) &&
			term_(_RPAREN) &&
			nterm_(statement)) {
			push(&SCParserAction::onForStm);
			return true;
		}
		PARSE_ERROR(PA_FOR_STATEMENT_NOT_VALID);
	}
			
	return false;
}

/*
  for-init-statement ::=
  expression-statement
  | simple-declaration
*/
bool
SCParser::for_init_statement()
{
	if (nterm_(expression_statement)) {
		push(&SCParserAction::onForInitStm_ExpressionStm);
		return true;
	}

	if (nterm_(simple_declaration)) {
		push(&SCParserAction::onForInitStm_SimpleDecl);
		return true;
	}
	return false;
}

/*
  jump-statement ::=
  break ;
  | continue ;
  | return [ expression_list] ;
  | goto label-name ;
*/
bool
SCParser::jump_statement()
{
	if (term_(_BREAK)) {
		if (term_(_SEMICOLON)) {
			push(&SCParserAction::onBreakStm);
			return true;
		}
		PARSE_ERROR(PA_JUMP_STATEMENT_NOT_VALID);
	}
	if (term_(_CONTINUE)) {
		if (term_(_SEMICOLON)) {
			push(&SCParserAction::onContinueStm);
			return true;
		}
		PARSE_ERROR(PA_CONTINUE_STATEMENT_NOT_VALID);
	}
	if (term_(_RETURN)) {
		if (ntermopt_(expression_list) && term_(_SEMICOLON)) {
			push(&SCParserAction::onReturnStm);
			return true;
		}
		PARSE_ERROR(PA_RETURN_STATEMENT_NOT_VALID);
	}
	if (term_(_GOTO)) {
		if (nterm_(identifier) && term_(_SEMICOLON)) {
			push(&SCParserAction::onGotoStm);
			return true;
		}
		PARSE_ERROR(PA_GOTO_STATEMENT_NOT_VALID);
	}
	return false;
}

/*
  declaration-statement ::=
  block-declaration
*/
bool
SCParser::declaration_statement()
{
	if (nterm_(block_declaration)) {
		return true;
	}
	return false;
}

/*
  A.6 Declarations
*/

/*
  declaration-seq ::=
  declaration
  | declaration-seq declaration
*/
bool
SCParser::declaration_seq()
{
	if (nterm_(declaration) &&
		ntermopt_(declaration_seq)){
		push(&SCParserAction::onDeclSeq);
		return true;
	}
	return false;
}
/*
  declaration ::=
  block-declaration
  | function-definition
  | template-declaration
  | explicit-instantiation
  | explicit-specialization
  | namespace-definition
  | sc-process-definition
*/
bool
SCParser::declaration()
{
	if (nterm_(decl_specifier_seq)) {
		int found;
		found = tokenit.skipUntil({_LBRACE, _SEMICOLON}, false);
		if (found == _SEMICOLON) {
			if (nterm_(simple_declaration_without_decl_spec)) {
				push(&SCParserAction::onDecl_BlockDecl);
				return true;
			}
			//never reach here
			assert(0);
		} else if (found == _LBRACE) {
			if (nterm_(function_definition_without_decl_spec)) {
				push(&SCParserAction::onDecl_FuncDef);
				return true;
			}
			//or initializer?  i.e. a[] = {...};
			if (nterm_(simple_declaration_without_decl_spec)) {
				push(&SCParserAction::onDecl_BlockDecl);
				return true;
			}
			PARSE_ERROR(PA_DECLARATION_LBRACE_FOLLOW);
		} else {
			//not found '{' and ';'
			PARSE_ERROR(PA_DECLARATION_NOT_VALID);
		}
	}

	if (nterm_(template_declaration)) {
		return true;
	}
	if(nterm_(explicit_instantiation)) {
		return true;
	}
	if (nterm_(explicit_specialization)) {
		return true;
	}
	if (nterm_(namespace_alias_definition)) {
		return true;
	}
	if (nterm_(namespace_definition)) {
		return true;
	}
	if (nterm_(using_declaration)) {
		return true;
	}
	if (nterm_(using_directive)) {
		return true;
	}
	//TODO:
	//nterm_(_sc-process-definition)
	const Token &t = tokenit.get(0);
	if (t.id == _IDENT) {
		PARSE_ERROR(PA_DECLARATION_NOT_VALID);
	}
	return false;
}

/*
  block-declaration ::=
  simple-declaration
  | namespace-alias-definition
  | using-declaration
  | using-directive
*/
bool
SCParser::block_declaration()
{
	if (nterm_(simple_declaration)) {
		push(&SCParserAction::onBlockDecl_SimpleDecl);
		return true;
	}
	if (nterm_(namespace_alias_definition)) {
		return true;
	}
	if (nterm_(using_declaration)) {
		return true;
	}
	if (nterm_(using_directive)) {
		return true;
	}

	const Token &t = tokenit.get(0);
	if (t.id == _IDENT) {
		PARSE_ERROR(PA_DECLARATION_NOT_VALID);
	}
	return false;
}

/*
  simple-declaration ::=
  [ decl-specifier-seq ] [ init-declarator-list ] ;
*/
bool
SCParser::simple_declaration()
{
	if (ntermopt_(decl_specifier_seq) &&
		ntermopt_(init_declarator_list) &&
		term_(_SEMICOLON)) {
		if (hasTypedefDecl) {
			hasTypedefDecl = false;
			if (currentDeclaratorIds.empty()) {
				//valid typedef name is not found
				PARSE_ERROR(PA_TYPEDEF_NAME_NOT_VALID);
			}
			addTypeNames(currentDeclaratorIds);
		}
		currentDeclaratorIds.clear();
		push(&SCParserAction::onSimpleDecl);
		return true;
	}
	return false;
}

bool
SCParser::simple_declaration_without_decl_spec()
{
	if (ntermopt_(init_declarator_list)) {
		if (term_(_SEMICOLON)) {
			if (hasTypedefDecl) {
				hasTypedefDecl = false;
				if (currentDeclaratorIds.empty()) {
					//valid typedef name is not found
					PARSE_ERROR(PA_TYPEDEF_NAME_NOT_VALID);
				}
				addTypeNames(currentDeclaratorIds);
			}
			currentDeclaratorIds.clear();
			push(&SCParserAction::onSimpleDecl);
			return true;
		}
		PARSE_ERROR(PA_INIT_DECLARATOR_LIST_NOT_FOLLOW_SEMICOLON);
	}
	return false;
}

/*
  decl-specifier ::=
  storage-class-specifier
  | type-specifier
  | function-specifier
  | friend
  | typedef
*/
bool
SCParser::decl_specifier(int *baseTypeDecl, int *userTypeDecl)
{
	if (!tokenit.isFollow(DECL_SPEC_FOLLOW_MASK)) {
		return false;
	}
	const int t0 = tokenit.get(0).id;
	const int t1 = tokenit.get(1).id;
	if (t0 == _IDENT && t1 == _LPAREN) {
		//DBG("!!! %s is not decl_specifier !!!", tokenit.val()->toString().c_str());
		return false;
	}
	
	ReadyToBack rtb(*this);

	//type-specifier
	if (nterm_(simple_type_specifier_user_type) ||
		nterm_(class_specifier) ||
		nterm_(enum_specifier) ||
		nterm_(elaborated_type_specifier)) {
		if (*userTypeDecl || *baseTypeDecl) {
			ERROR("base type is declared more than once in decl-specfier-seq");
			back();
			return false;
		}
		(*userTypeDecl)++;
		(*baseTypeDecl)++;
		push(&SCParserAction::onDeclSpecifier_TypeSpecifier);
		return true;
	}

	if (nterm_(builtin_type_specifier)) {
		if (*userTypeDecl) {
			ERROR("base type is declared more than once in decl-specfier-seq");
			back();
			return false;
		}
		(*baseTypeDecl)++;
		push(&SCParserAction::onDeclSpecifier_TypeSpecifier);
		return true;
	}

	if (nterm_(cv_qualifier)) {
		push(&SCParserAction::onDeclSpecifier_TypeSpecifier);
		return true;
	}

	//storage-class-specifier | function-specifier | friend
	const Token &t = tokenit.get(0);
	if (t.id == _AUTO || t.id == _REGISTER || t.id == _STATIC || 
		t.id == _EXTERN || t.id == _MUTABLE ||
		t.id == _INLINE || t.id == _VIRTUAL || t.id == _EXPLICIT ||
		t.id == _FRIEND) {
		tokenit.next();
		push(&SCParserAction::onDeclSpecifier_OtherSpecifier, t.val);
		return true;
	}
	//typedef
	if (t.id == _TYPEDEF) {
		tokenit.next();
		push(&SCParserAction::onDeclSpecifier_OtherSpecifier, t.val);
		hasTypedefDecl = true;
		return true;
	}

	return false;
}

/*
  decl-specifier-seq ::=
  decl-specifier decl-specifier-seq'
*/
bool
SCParser::decl_specifier_seq()
{
	//TODO: To check that decl_specifier_seq has at least one type_specifier,
	//TODO: And it has only one user typename.
	//bool hasTypeSpec = false;
	int baseTypeDecl = 0, userTypeDecl = 0;
	int decl = 0;
	while (true) {
		const Token &t = tokenit.get(0);
		if (decl_specifier(&baseTypeDecl, &userTypeDecl)) {
			PARSE_DBG("%s'%s' is accepted as <decl_specifier>", dbgTraceIndent.c_str(), t.val->toString().c_str()); 
			++decl;
		} else {
			push(&SCParserAction::onEmpty, new TokenString("decl_specifier_seq"));
			break;
		}
	}
	if (decl && baseTypeDecl == 0) {
		//base type is not found in decl-specifier-seq
		PARSE_ERROR(PA_DECL_SPEC_BASE_TYPE_NOT_SPECIFIED);
	}
	for (int i = 0; i < decl; ++i) {
		push(&SCParserAction::onDeclSpecSeq);
	}
	return decl;
}

/*
  type-specifier ::=
  simple-type-specifier
  | class-specifier
  | enum-specifier
  | elaborated-type-specifier
  | cv-qualifier
  | sc-type-specifier
  | sc-module-specifier
*/
bool
SCParser::type_specifier()
{
	if (!tokenit.isFollow(TYPE_SPEC_FOLLOW)) {
		return false;
	}
	if (nterm_(simple_type_specifier) ||
		nterm_(class_specifier) ||
		nterm_(enum_specifier) ||
		nterm_(elaborated_type_specifier) ||
		nterm_(cv_qualifier)) {
		return true;
	}
	return false;
}

/*
  simple-type-spcifier ::=
  [ :: ] [ nested-name-specifier ] type-name
  | [ :: ] nested-name-specifier template template-id
  | char | bool | short | int | long | signed | unsigned | void
*/
bool
SCParser::simple_type_specifier()
{
	if (nterm_(simple_type_specifier_user_type)) {
		return true;
	}

	if (nterm_(builtin_type_specifier)) {
		return true;
	}

	return false;
}

bool
SCParser::simple_type_specifier_user_type()
{
	if (termopt_(_SCOPE) &&
		ntermopt_(nested_name_specifier) &&
		nterm_(type_name)) {
		push(&SCParserAction::onSimpleTypeSpecifier_TypeName);
		return true;
	}
	back();

	if (termopt_(_SCOPE) &&
		nterm_(nested_name_specifier) &&
		term_(_TEMPLATE) &&
		nterm_(template_id)) {
		push(&SCParserAction::onSimpleTypeSpecifier_TemplateId);
		return true;
	}
	return false;
}

bool
SCParser::builtin_type_specifier()
{
	/*TODO: 
	  "long int" | "int long"
	  "long long int" | "long int long" | "int long long"
	*/
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_CHAR) ||
		tokenit.eat(_BOOL) ||
		tokenit.eat(_INT) ||
		tokenit.eat(_VOID)) {
		assert(val->isInt());
		push(&SCParserAction::onBuiltinTypeSpecifier, val);
		return true;
	}
	if (tokenit.eat(_SHORT) ||
		tokenit.eat(_LONG) || 
		tokenit.eat(_SIGNED) ||
		tokenit.eat(_UNSIGNED)) {
		assert(val->isInt());
		push(&SCParserAction::onBuiltinTypeSpecifier, val);
		return true;
	}

	return false;
}

/*
  type-name ::=	class-name | enum-name | typedef-name
*/
bool
SCParser::type_name()
{
	const ref<TokenValue> &val = tokenit.val();
	//DBG("type_name ? %s", val->toString().c_str());
	const int t0 = tokenit.get(0).id;
	const int t1 = tokenit.get(1).id;
	if (t0 == _IDENT) {
		if (t1 == _LT) {
			//"identifier <"
			if (nterm_(template_id)) {
				push(&SCParserAction::onTypeName_TemplateId);
				return true;
			}
			PARSE_ERROR(PA_TYPENAME_TEMPLATE_ID_EXPECTED);
		}
		if (isTypeName(val->toString())) {
			tokenit.eat(_IDENT);
			push(&SCParserAction::onTypeName, val);
			return true;
		}
	}
	return false;
}


/*
  elaborated-type-specifier ::=
  class-key nested-opt-identifier
  | enum nested-opt-identifier
  | typename [ :: ] nested-name-specifier identifier
  | typename [ :: ] nested-name-specifier [ template ] template-id
*/
bool
SCParser::elaborated_type_specifier()
{
	if (nterm_(class_key)) {
		if (nterm_(nested_opt_identifier)) {
			push(&SCParserAction::onElaboratedTypeSpec_Class);
			return true;
		}
		//expected identifier
		PARSE_ERROR(PA_ELABORATED_TYPE_IDENT_EXPECTED);
	}
	if (term_(_ENUM)) {
		if (nterm_(nested_opt_identifier)) {
			push(&SCParserAction::onElaboratedTypeSpec_Enum);
			return true;
		}
		//expected identifier
		PARSE_ERROR(PA_ELABORATED_TYPE_IDENT_EXPECTED);
	}

	if (term_(_TYPENAME)) {
		if (termopt_(_SCOPE) && nterm_(nested_name_specifier)) {
			if (term_(_TEMPLATE)) {
				if (nterm_(template_id)) {
					push(&SCParserAction::onElaboratedTypeSpec_TypenameTemplateId);
					return true;
				}
				PARSE_ERROR(PA_ELABORATED_TYPE_TEMPLATEID_EXPECTED);
			}
			
			//conflicting ident & template-id
			const int t0 = tokenit.get(0).id;
			const int t1 = tokenit.get(1).id;
			if (t0 == _IDENT) {
				if (t1 == _LT) {
					empty();//empty "template" 
					if (nterm_(template_id)) {
						push(&SCParserAction::onElaboratedTypeSpec_TypenameTemplateId);
						return true;
					}
					PARSE_ERROR(PA_ELABORATED_TYPE_TEMPLATEID_EXPECTED);
				} else {
					if (nterm_(identifier)) {
						push(&SCParserAction::onElaboratedTypeSpec_Typename);
						return true;
					}
					assert(0);
				}
			}
			//identifier is expected
			PARSE_ERROR(PA_ELABORATED_TYPE_IDENT_EXPECTED);
		}
		//nested-name-specifier is expected
		PARSE_ERROR(PA_ELABORATED_TYPE_NESTED_NAME_EXPECTED);
	}
	return false;
}


/*
  enum-specifier ::=
  enum [ identifier ] { [ enumerator-list ] }
*/
bool
SCParser::enum_specifier()
{
	if (term_(_ENUM)) {
		const ref<TokenValue> &name = tokenit.val();
		if (ntermopt_(identifier) &&
			term_(_LBRACE) &&
			ntermopt_(enumerator_list) &&
			term_(_RBRACE)) {
			push(&SCParserAction::onEnumSpecifier);
			addTypeName(name->toString());
			return true;
		}
		//or elaborated enum type spec
		//PARSE_ERROR(0);
	}
	return false;
}

/*
  enumerator-list ::=
  enumerator-difinition
  | enumerator-list , enumerator-difinition
*/
bool
SCParser::enumerator_list()
{
	if (nterm_(enumerator_definition) &&
		ntermopt_(enumerator_list_prime)) {
		push(&SCParserAction::onEnumeratorListHead);
		return true;
	}
	return false;
}
bool
SCParser::enumerator_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(enumerator_definition) &&
			ntermopt_(enumerator_list_prime)) {
			push(&SCParserAction::onEnumeratorListRest);
			return true;
		}
		PARSE_ERROR(PA_ENUMERATOR_EXPECTED);
	}
	return false;
}

/*
  enumerator-difinition ::=
  identifier [= constant-expression]
*/
bool
SCParser::enumerator_definition()
{
	if (nterm_(identifier) &&
		ntermopt_(enumerator_definition_assign)) {
		push(&SCParserAction::onEnumeratorDefinition);
		return true;
	}
	return false;
}
bool
SCParser::enumerator_definition_assign()
{
	if (term_(_ASSIGN)) {
		if (nterm_(constant_expression)) {
			push(&SCParserAction::onEnumeratorDefinitionAssign);
			return true;
		}
		PARSE_ERROR(PA_ENUMERATOR_ASSIGN_EXP_EXPECTED);
	}
	return false;
}

/*
  namespace-definition ::=
  namespace [ identifier ] { namespace-body }
*/
bool
SCParser::namespace_definition()
{
	if (term_(_NAMESPACE)) {
		const Token &t = tokenit.get(0);
		if (ntermopt_(identifier) && term_(_LBRACE)) {
			if (t.id == _IDENT) {
				enterScope(t.val->toString());
			} else {
				enterScope("<unnamed>");
			}
			if (ntermopt_(declaration_seq) &&
				term_(_RBRACE)) {
				leaveScope();
				push(&SCParserAction::onNamespaceDefinition);
				return true;
			}
		}
		PARSE_ERROR(PA_NAMESPACE_NOT_VALID);
	}
	return false;
}

/*
  namespace-body ::=
  [ declaration-seq ]
*/
bool
SCParser::namespace_body()
{
	return false;
}

/*
  namespace-alias-definition ::=
  namespace identifier = qualified-namespace-specifier ;
*/
bool
SCParser::namespace_alias_definition()
{
	if (term_(_NAMESPACE)) {
		if (nterm_(identifier) &&
			term_(_ASSIGN) &&
			nterm_(nested_opt_identifier) &&
			term_(_SEMICOLON)) {
			push(&SCParserAction::onNamespaceAliasDefinition);
			return true;
		}
	}
	return false;
}

/*
  qualified-namespace-specifier ::=
  nested-opt-identifier
*/
bool
SCParser::qualified_namespace_specifier()
{
	return false;
}

/*
  using-declaration ::=
  using [ typename ] [ :: ] nested-name-specifier unqualified-id ;
  | using :: unqualified-id ;
*/
bool
SCParser::using_declaration()
{
	const int t0 = tokenit.get(0).id;
	const int t1 = tokenit.get(1).id;
	if (t0 == _USING) {
		if (t1 == _SCOPE) {
			if (term_(_USING) &&
				term_(_SCOPE) &&
				nterm_(unqualified_id) &&
				term_(_SEMICOLON)) {
				push(&SCParserAction::onUsingDeclaration);
				return true;
			}
		} else {
			if (term_(_USING) &&
				termopt_(_TYPENAME) &&
				termopt_(_SCOPE) &&
				nterm_(nested_name_specifier) &&
				nterm_(unqualified_id) &&
				term_(_SEMICOLON)) {
				push(&SCParserAction::onUsingDeclaration_Scoped);
				return true;
			}
		}
		//or using directive?
		//TODO:
		//in case of member_declaration, parsing must be error here.
	}
	return false;
}

/*
  using-directive ::=
  using namespace nested-opt-identifier ;
*/
bool
SCParser::using_directive()
{
	if (term_(_USING)) {
		if (term_(_NAMESPACE) &&
			nterm_(nested_opt_identifier) &&
			term_(_SEMICOLON)) {
			push(&SCParserAction::onUsingDirective);
			return true;
		}
		PARSE_ERROR(PA_USING_NOT_VALID);
	}
	return false;
}

/*
  A.6-1 SystemC Type Specifiers
*/
/*
  sc-type-specifier ::=
  sc_int < constant-expression >
  | sc_uint < constant-expression >
  | sc_bigint < constant_expression >
  | sc_biguint < constant_expression >
  | sc_logic
  | sc_lv < constant_expression >
  | sc_bit
  | sc_bv < constant_expression >
  | sc_fixed < constant_expression , constant_expression
  [ , sc-quantization-mode-specifier ] [ , sc-overflow-mode-specifier ]
  [ , constant-expression ] >
  | sc_ufixed < constant_expression , constant_expression
  [ , sc-quantization-mode-specifier ] [ , sc-overflow-mode-specifier ] [ , constant-expression ] >
*/
/*
  sc-quantization-mode-specifier ::=
  SC_RND
  | SC_RND_ZERO
  | SC_RND_MIN_INF
  | SC_RND_INF
  | SC_RND_CONV
  | SC_TRN
  | SC_TRN_ZERO
*/
/*
  sc-overflow-mode-specifier ::=
  SC_SAT
  | SC_SAT_ZERO
  | SC_SAT_SYN
  | SC_WRAP
  | SC_WRAP_SM
*/

/*
  A.7 Declarators
*/
/*
  init-declarator-list ::=
  init-declarator
  | init-declarator-list , init-declarator
*/
bool
SCParser::init_declarator_list()
{
	if (nterm_(init_declarator) &&
		ntermopt_(init_declarator_list_prime)) {
		push(&SCParserAction::onInitDeclListHead);
		return true;
	}
	return false;
}
bool
SCParser::init_declarator_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(init_declarator) &&
			ntermopt_(init_declarator_list_prime)) {
			push(&SCParserAction::onInitDeclListRest);
			return true;
		}
		PARSE_ERROR(PA_INIT_DECLARATOR_EXPECTED);
	}
	return false;
}
	
/*
  init-declarator ::=
  declarator [ initializer ]
*/
bool
SCParser::init_declarator()
{
	if (nterm_(declarator) &&
		ntermopt_(initializer)) {
		push(&SCParserAction::onInitDecl);
		return true;
	}
	return false;
}

/*
  declarator ::=
  direct-declarator
  | ptr-operator declarator
*/
bool
SCParser::declarator()
{
	if (nterm_(ptr_operator)) {
		if (nterm_(declarator)) {
			push(&SCParserAction::onDecl_PtrDecl);
			return true;
		}
		//abstract declarator?
		return false;
		//PARSE_ERROR(0);
	}

	if (nterm_(direct_declarator)) {
		push(&SCParserAction::onDecl_DirectDecl);
		return true;
	}
	return false;
}

/*
  direct-declarator ::=
  declarator-id direct-declarator' 
  | ( declarator ) direct-declarator' 
*/
bool
SCParser::direct_declarator()
{
	if (term_(_LPAREN)) {
		if (nterm_(declarator) &&
			term_(_RPAREN) &&
			ntermopt_(direct_declarator_prime)) {
			push(&SCParserAction::onDirectDecl_FuncPtrDecl);
			return true;
		}
		//direct-abstract-declarator?
		return false;
		//PARSE_ERROR(0);
	}

	if (nterm_(declarator_id) &&
		ntermopt_(direct_declarator_prime)) {
		push(&SCParserAction::onDirectDecl_DeclId);
		return true;
	}

	return false;
}

/*
  abstract-declarator ::=
  ptr_operator [ abstract-declarator ]
  | direct-abstract-declarator
*/
bool
SCParser::abstract_declarator()
{
	if (nterm_(ptr_operator)) {
		if (ntermopt_(abstract_declarator)) {
			push(&SCParserAction::onDecl_PtrDecl);
			return true;
		}
		PARSE_ERROR(PA_ABSTRACT_DECLARATOR_EXPECTED);
	}
	//back();
	if (nterm_(direct_abstract_declarator)) {
		push(&SCParserAction::onDecl_DirectDecl);
		return true;
	}
	return false;
}

/*
  direct-abstract-declarator ::=
  direct-declarator'
  | ( abstract-declarator ) direct-declarator' 
*/
bool
SCParser::direct_abstract_declarator()
{
	if (term_(_LPAREN)) {
		if (nterm_(abstract_declarator) &&
			term_(_RPAREN) &&
			ntermopt_(direct_declarator_prime)) {
			push(&SCParserAction::onDirectDecl_FuncPtrDecl);
			return true;
		}
		//PARSE_ERROR(0);
	}
	back();

	if (nterm_(direct_declarator_prime)) {
		return true;
	}

	return false;
}

/*
  direct-declarator' ::=
  ( parameter-declaration-clause ) [ cv-qualifier-seq ] direct-declarator'
  | [ [ constant-expression ] ] direct-declarator'
*/
bool
SCParser::direct_declarator_prime()
 {
	if (term_(_LPAREN) &&
		nterm_(parameter_declaration_clause) &&
		term_(_RPAREN) &&
		ntermopt_(cv_qualifier_seq)) {
		if (nterm_(direct_declarator_prime)) {
			PARSE_ERROR(0);//???
		}
		push(&SCParserAction::onDirectDecl_ParamDecl);
		return true;
	}
	back();

	if (term_(_LBRACKET) &&
		ntermopt_(constant_expression) &&
		term_(_RBRACKET) &&
		ntermopt_(direct_declarator_prime)) {
		push(&SCParserAction::onDirectDecl_ArrayDecl);
		return true;
	}
		
	return false;
}
/*
  ptr-operator ::=
  * [ cv-qualifier-seq ]
  | &
  | [ :: ] [ nested-name-specifier ] * [ cv-qualifier-seq ]
*/
bool
SCParser::ptr_operator()
{
	if (term_(_STAR) &&
		ntermopt_(cv_qualifier_seq)) {
		push(&SCParserAction::onPtrOp_Ptr);
		return true;
	}
	if (term_(_BIT_AND)) {
		push(&SCParserAction::onPtrOp_Reference);
		return true;
	}
	if (termopt_(_SCOPE) &&
		ntermopt_(nested_name_specifier) &&
		term_(_STAR) &&
		ntermopt_(cv_qualifier_seq)) {
		push(&SCParserAction::onPtrOp_ScopedPtr);
		return true;
	}
	return false;
}

/*
  cv-qualifier-seq ::=
  cv-qualifier [ cv-qualifier-seq ]
*/
bool
SCParser::cv_qualifier_seq()
{
	if (nterm_(cv_qualifier) &&
		ntermopt_(cv_qualifier_seq)) {
		push(&SCParserAction::onCVQualSeq);
		return true;
	}
	return false;
}

/*
  cv-qualifier ::=
  const
  | volatile
*/
bool
SCParser::cv_qualifier()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_CONST) ||
		tokenit.eat(_VOLATILE)) {
		push(&SCParserAction::onCVQual, val);
		return true;
	}
	return false;
}

/*
  declarator-id ::=
  id-expression
  | [ :: ] [ nested-name-specifier ] type-name
*/
bool
SCParser::declarator_id()
{
	const Token &t = tokenit.get(0);
	if (nterm_(id_expression)) {
		//save identifier as a new typename for typedef declaration
		if (t.id == _IDENT) {
			currentDeclaratorIds.push_back(t.val->toString());
		}
		push(&SCParserAction::onDeclId_IdExp);
		return true;
	}

	//FIXME: Can NOT reach here. because substitute id-expression for this token sequence. 
	if (termopt_(_SCOPE) &&
		ntermopt_(nested_name_specifier) &&
		nterm_(type_name)) {
		assert(0);
		push(&SCParserAction::onDeclId_Typename);
		return true;
	}

	return false;
}

/*
  type-id ::=
  type-specifier-seq [ abstract-declarator ]
*/
bool
SCParser::type_id()
{
	if (nterm_(type_specifier_seq) &&
		ntermopt_(abstract_declarator)) {
		push(&SCParserAction::onTypeId);
		return true;
	}
	return false;
}

/*
  type-specifier-seq ::=
  type-specifier [type-specifier-seq ]
*/
bool
SCParser::type_specifier_seq()
{
	if (!tokenit.isFollow(TYPE_SPEC_FOLLOW)) {
		return false;
	}

	if (nterm_(type_specifier) &&
		ntermopt_(type_specifier_seq_prime)) {
		push(&SCParserAction::onTypeSpecSeq);
		return true;
	}
	return false;
}
bool
SCParser::type_specifier_seq_prime()
{
	if (!tokenit.isFollow(TYPE_SPEC_FOLLOW)) {
		return false;
	}

	if (nterm_(type_specifier) &&
		ntermopt_(type_specifier_seq_prime)) {
		push(&SCParserAction::onTypeSpecSeq);
		return true;
	}
	return false;
}

/*
  parameter-declaration-clause ::=
  [ parameter-declaration-list ] [ ... ]
  | parameter-declaration-list , ...
*/
bool
SCParser::parameter_declaration_clause()
{
	if (ntermopt_(parameter_declaration_list)) {
		return true;
	}
	return false;
}

/*
  parameter-declaration-list ::=
  parameter-declaration parameter-declaration-list'
  | ...
*/
bool
SCParser::parameter_declaration_list()
{
	if (nterm_(parameter_declaration) &&
		ntermopt_(parameter_declaration_list_prime)) {
		push(&SCParserAction::onParamDeclListHead);
		return true;
	}
	if (term_(_ELIPSIS)) {
		push(&SCParserAction::onParamDeclListHead_Elipsis);
		return true;
	}
	return false;
}
/*
  parameter-declaration-list' ::=
  empty
  | , parameter-declaration parameter-declaration-list'
  | , ...
*/
bool
SCParser::parameter_declaration_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(parameter_declaration) &&
			ntermopt_(parameter_declaration_list_prime)) {
			push(&SCParserAction::onParamDeclListRest);
			return true;
		}
		if (term_(_ELIPSIS)) {
			push(&SCParserAction::onParamDeclListRest_Elipsis);
			return true;
		}
		PARSE_ERROR(PA_PARAMETER_DECLARATION_EXPECTED);
	}
	return false;
}
/*
  parameter-declaration ::=
  decl-specifier-seq declarator [= assignment-expression]
  | decl-specifier-seq [ abstract-declarator ] [= assignment-expression]
  | [ const ] sc-signal-declaration & identifier
*/
bool
SCParser::parameter_declaration()
{
	if (nterm_(decl_specifier_seq)) {
		if (nterm_(declarator) &&
			ntermopt_(parameter_declaration_assign)) {
			push(&SCParserAction::onParamDecl);
			return true;
		}
	}
	back();
	if (nterm_(decl_specifier_seq)) {
		if (nterm_(abstract_declarator) &&
			ntermopt_(parameter_declaration_assign)) {
			push(&SCParserAction::onParamDecl);
			return true;
		}
	}
	back();
	if (nterm_(decl_specifier_seq)) {
		if (empty() &&
			ntermopt_(parameter_declaration_assign)) {
			push(&SCParserAction::onParamDecl);
			return true;
		}
	}
	//TODO:
	//[ const ] sc-signal-declaration & identifier

	return false;
}
bool
SCParser::parameter_declaration_assign()
{
	if (term_(_ASSIGN)) {
		if (nterm_(assignment_expression)) {
			push(&SCParserAction::onParamDeclAssign);
			return true;
		}
		PARSE_ERROR(PA_PARAMETER_DECLARATION_ASSIGN_EXP_EXPECTED);
	}
	return false;
}

/*
  function-definition ::=
  [ decl-specifier-seq ] declarator [ ctor-initializer ] function_body
*/
bool
SCParser::function_definition()
{
	if (ntermopt_(decl_specifier_seq) &&
		nterm_(declarator) &&
		ntermopt_(ctor_initializer) &&
		nterm_(function_body)) {
		push(&SCParserAction::onFuncDef);
		return true;
	}
	return false;
}
bool
SCParser::function_definition_without_decl_spec()
{
	if (nterm_(declarator) &&
		ntermopt_(ctor_initializer) &&
		nterm_(function_body)) {
		push(&SCParserAction::onFuncDef);
		return true;
	}
	return false;
}


/*
  function-body ::=
  compound-statement
*/
bool
SCParser::function_body()
{
	if (nterm_(compound_statement)) {
		push(&SCParserAction::onFuncBody);
		return true;
	}
	return false;
}


/*
  initializer ::=
  = initializer-clause
  | ( expression-list )
*/
bool
SCParser::initializer()
{
	if (term_(_ASSIGN)) {
		if (nterm_(initializer_clause)) {
			push(&SCParserAction::onInitializer);
			return true;
		}
		PARSE_ERROR(PA_INITIALIZER_ASSIGN_NOT_VALID);
	}
	if (term_(_LPAREN)) {
		if (nterm_(expression_list) &&
			term_(_RPAREN)) {
			push(&SCParserAction::onInitializer_Parenthesis);
			return true;
		}
		PARSE_ERROR(PA_INITIALIZER_PARENTHESIS_NOT_VALID);
	}
	return false;
}


/*
  initializer-clause ::=
  assignment-expression
  | { initializer-list [ , ] }
  | { }
*/
bool
SCParser::initializer_clause()
{
	if (nterm_(assignment_expression)) {
		push(&SCParserAction::onInitializerClause_AssignExp);
		return true;
	}
	if (term_(_LBRACE)) {
		if (term_(_RBRACE)) {
			push(&SCParserAction::onInitializerClause_EmptyBraces);
			return true;
		}
		if (nterm_(initializer_list) &&
			termopt_(_COMMA) &&
			term_(_RBRACE)) {
			push(&SCParserAction::onInitializerClause_InitializerList);
			return true;
		}
		PARSE_ERROR(PA_INITIALIZER_CLAUSE_BRACE_NOT_VALID);
	}
	return false;
}


/*
  initializer-list :=
  initializer-clause
  | initializer-list , iitializer-clause
*/
bool
SCParser::initializer_list()
{
	if (nterm_(initializer_clause) &&
		ntermopt_(initializer_list_prime)) {
		push(&SCParserAction::onInitializerListHead);
		return true;
	}
	return false;
}
bool
SCParser::initializer_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(initializer_clause) &&
			ntermopt_(initializer_list_prime)) {
			push(&SCParserAction::onInitializerListRest);
			return true;
		}
		PARSE_ERROR(PA_INITIALIZER_CLAUSE_EXPECTED);
	}
	return false;
}

/*
  A.8 Classes
  Classes are regarded as a module or a user defined type in SystemC synthesis. The syntax for module is described in A.8-1 section. There is many limitation for a user defined type.
*/

/*
  class-name ::=
  identifier
  | template-id
*/
/*
bool
SCParser::class_name()
{
	const int t0 = tokenit.get(0).id;
	const int t1 = tokenit.get(1).id;
	if (t0 == _IDENT) {
		if (t1 == _LT) {
			//"identifier <"
			if (nterm_(template_id)) {
				push(&SCParserAction::onClassName_TemplateId);
				return true;
			}
			PARSE_ERROR(0);
		}
		nterm_(identifier);
		push(&SCParserAction::onClassName);
		return true;
	}
	return false;
}
*/

/*
  class-specifier ::=
  class-head { [ member-specification ] }
*/
bool
SCParser::class_specifier()
{
	if (nterm_(class_head) &&
		term_(_LBRACE) &&
		ntermopt_(member_specification) &&
		term_(_RBRACE)) {
		leaveScope();
		push(&SCParserAction::onClassSpecifier);
		return true;
	}
	return false;
}

/*
  class-head ::=
  class-key [ identifier ] [ base_clause ]
  | class-key nested-name-specifier identifier [ base-clause ]
  | class-key [ nested-name-specifier ] template-id [ base-clause ]
*/
bool
SCParser::class_head()
{
	if (nterm_(class_key)) {
		if (nterm_(nested_name_specifier)) {
			//class-key nested-name-specifier template-id [ base-clause ]
			const ref<TokenValue> &name = tokenit.val();
			if (nterm_(template_id) &&
				ntermopt_(base_clause)) {
				push(&SCParserAction::onClassHead_NestedTemplate);
				addTypeName(name->toString());
				enterScope(name->toString());
				return true;
			}

			//class-key nested-name-specifier identifier [ base-clause ]
			if (nterm_(identifier) &&
				ntermopt_(base_clause)) {
				push(&SCParserAction::onClassHead_NestedId);
				addTypeName(name->toString());
				enterScope(name->toString());
				return true;
			}

			PARSE_ERROR(PA_CLASS_IDENT_EXPECTED);
		}

		const ref<TokenValue> &name = tokenit.val();
		//class-key template-id [ base-clause ]
		if (nterm_(template_id) &&
			ntermopt_(base_clause)) {
			push(&SCParserAction::onClassHead_Template);
			addTypeName(name->toString());
			enterScope(name->toString());
			return true;
		}

		//class-key [ identifier ] [ base_clause ]
		if (ntermopt_(identifier) &&
			ntermopt_(base_clause)) {
			push(&SCParserAction::onClassHead);
			addTypeName(name->toString());
			enterScope(name->toString());
			return true;
		}

		PARSE_ERROR(PA_CLASS_IDENT_EXPECTED);
	}
	return false;
}

/*
  class-key ::= class | struct
*/
bool
SCParser::class_key()
{
	const ref<TokenValue> &val = tokenit.val();	
	if (tokenit.eat(_CLASS) ||
		tokenit.eat(_STRUCT)) {
		push(&SCParserAction::onClassKey, val);
		return true;
	}
	return false;
}

/*
  member-specification ::=
  member-declaration [ member-specification ]
  | access-specifier : [ member-specification ]
*/
bool
SCParser::member_specification()
{
	//if (tokenit.eat(_RBRACE)) {
	//	return false;
	//}

	if (nterm_(access_specifier)) {
		if (term_(_COLON) &&
			ntermopt_(member_specification)) {
			push(&SCParserAction::onMemberSpec_AccessSpec);
			return true;
		}
		PARSE_ERROR(PA_ACCESS_SPECIFIER_COLON_EXPECTED);
	}	

	if (nterm_(member_declaration) &&
		ntermopt_(member_specification)) {
		push(&SCParserAction::onMemberSpec_MemberDecl);
		return true;
	}
	return false;
}

/*
  member-declaration ::=
  [ decl-specifier-seq ] [ member-declarator-list ] ;
  | function-definition [ ; ]
  | [ :: ] nested-name-specifier [ template ] unqualified-id ;
  | using-declaration
  | template-declaration
*/
bool
SCParser::member_declaration()
{
	//TODO: ctor check
	int found;
	found = tokenit.skipUntil({_LBRACE, _SEMICOLON}, false);
	//when _LBRACE is found, this token will be function definition
	//or inner class-specifier
	if (found == _LBRACE) {
		if (nterm_(function_definition) &&
			termopt_(_SEMICOLON)) {
			push(&SCParserAction::onMemberDecl_FuncDef);
			return true;
		}
		//PARSE_ERROR(0);
	}

	if (ntermopt_(decl_specifier_seq) &&
		ntermopt_(member_declarator_list) &&
		term_(_SEMICOLON)) {
		if (hasTypedefDecl) {
			hasTypedefDecl = false;
			if (currentDeclaratorIds.empty()) {
				//valid typedef name is not found
				PARSE_ERROR(PA_TYPEDEF_NAME_NOT_VALID);
			}
			addTypeNames(currentDeclaratorIds);
		}
		currentDeclaratorIds.clear();
		push(&SCParserAction::onMemberDecl_MemberDeclList);
		return true;
	}
	back();

#if 0
	if (nterm_(decl_specifier_seq)) {
		int found;
		found = tokenit.skipUntil({_LBRACE, _SEMICOLON}, false);
		if (found == _SEMICOLON) {
			if (ntermopt_(member_declarator_list) && term_(_SEMICOLON)) {
				push(&SCParserAction::onMemberDecl_MemberDeclList);
				return true;
			}
			PARSE_ERROR(0);
		} else if (found == _LBRACE) {//when _LBRACE is found, this token will be function definition
			if (nterm_(function_definition_without_decl_spec) && termopt_(_SEMICOLON)) {
				push(&SCParserAction::onMemberDecl_FuncDef);
				return true;
			}
			PARSE_ERROR(0);
		} else {
			//not found '{' and ';'
			PARSE_ERROR(0);
		}
	}

	//FIXME	
	if (empty()) {
		int found;
		found = tokenit.skipUntil({_LBRACE, _SEMICOLON}, false);
		if (found == _SEMICOLON) {
			if (ntermopt_(member_declarator_list) && term_(_SEMICOLON)) {
				push(&SCParserAction::onMemberDecl_MemberDeclList);
				return true;
			}
		} else if (found == _LBRACE) {//when _LBRACE is found, this token will be function definition
			if (nterm_(function_definition_without_decl_spec) && termopt_(_SEMICOLON)) {
				push(&SCParserAction::onMemberDecl_FuncDef);
				return true;
			}
		}
	}
	back();
	if (empty() && empty() && term_(_SEMICOLON)) {
		push(&SCParserAction::onMemberDecl_MemberDeclList);
		return true;
	}
	back();
#endif

	if (termopt_(_SCOPE) && nterm_(nested_name_specifier)) {
		if (termopt_(_TEMPLATE) &&
			nterm_(unqualified_id) &&
			term_(_SEMICOLON)) {
			push(&SCParserAction::onMemberDecl_UnqualifiedId);
			return true;
		}

		PARSE_ERROR(PA_MEMBER_DECL_UNQUALIFIED_ID_EXPECTED);
	}
	back();

	if (nterm_(using_declaration)) {
		push(&SCParserAction::onMemberDecl_UsingDecl);
		return true;
	}

	if (nterm_(template_declaration)) {
		push(&SCParserAction::onMemberDecl_TemplateDecl);
		return true;
	}
	return false;
}

/*
  member-declarator-list ::=
  member-declarator member-declarator-list'

  member-declarator-list' ::=
  empty
  | [,] member-declarator member-declarator-list'

*/
bool
SCParser::member_declarator_list()
{
	if (nterm_(member_declarator) &&
		ntermopt_(member_declarator_list_prime)) {
		push(&SCParserAction::onMemberDeclListHead);
		return true;
	}
	return false;
}
bool
SCParser::member_declarator_list_prime()
{
	if (termopt_(_COMMA) && /* TODO: is this option ??? */
		nterm_(member_declarator) &&
		ntermopt_(member_declarator_list_prime)) {
		push(&SCParserAction::onMemberDeclListRest);
		return true;
	}
	return false;
}

/*
  member-declarator ::=
  declarator [ pure-specifier ]
  | declarator [ constant-initializer ]
  | [ identifier ] : constant-expression
*/
bool
SCParser::member_declarator()
{
	if (nterm_(declarator)) {
		/* pure_apecifier '=0' is parsed as constant_initializer
		if (nterm_(pure_specifier)) {
			push(&SCParserAction::onMemberDeclarator_PureSpec);
			return true;
		}
		*/
		//must be static const 
		if (nterm_(constant_initializer)) {
			push(&SCParserAction::onMemberDeclarator_ConstantInit);
			return true;
		}
		//TODO: ?
		//if (tokenit.is(_COLON) && lastAccepted(function_declarator)) {
		//    //this is ctor initializer (function_definition)
		//    return false;
		//}
		push(&SCParserAction::onMemberDeclarator);
		return true;
	}

	if (ntermopt_(identifier) &&
		term_(_COLON) &&
		nterm_(constant_expression)) {
		push(&SCParserAction::onMemberDeclarator_BitField);
		return true;
	}
	return false;
}
/*
  pure-specifier ::=
  = 0
*/
bool
SCParser::pure_specifier()
{
	if (term_(_ASSIGN)) {
		const ref<TokenValue> &val = tokenit.val();
		if (term_(_DECIMAL_CONSTANT)) {
			if (val->isInt() && val->toInt() == 0) {
				push(&SCParserAction::onPureSpec);
				return true;
			}
		}
		//or constant-initializer
	}
	return false;
}

/*
  constant-initializer ::=
  = constant-expression
*/
bool
SCParser::constant_initializer()
{
	if (term_(_ASSIGN)) {
		if (nterm_(constant_expression)) {
			push(&SCParserAction::onConstantInit);
			return true;
		}
		PARSE_ERROR(PA_CONSTANT_INITIALIZER_EXP_EXPECTED);
	}
	return false;
}

/*
  A.8-1 Module Declaration
*/
/*
  sc-module-specifier ::=
  sc-module-head { [ module-member-specification ] }
*/
/*
  sc-module-head ::=
  SC_MODULE( identifier )
  | class-key [ nested-name-specifier ] identifier : [ public ] sc_module
*/
/*
  sc-module-member-specification ::=
  sc-module-member-declaration [ sc-module-member-specification ]
  | access-specifier : [ sc-module-member-specification ]
*/
/*
  sc-module-member-declaration ::=
  member-declaration
  | sc-signal-dclaration
  | sc-sub-module-declaration
  | sc-module-constructor-definition
  | sc-module-constructor-declaration
  | sc-has-process-declaration
*/
/*
  sc-signal-declaration ::=
  sc-signal-key < type-specifier > signal-declarator-list ;
  | sc-resolved-key signal -declarator-list ;
  | sc-resolved-vector-key < constant-expression > signal -declarator-list ;
*/
/*
  signal-declarator-list ::=
  identifier
  | signal-declarator-list , identifier
  | sc_in_clk
  | sc_out_clk
  | sc_inout_clk
*/
/*
  sc-resolved-key ::=
  sc_signal_resolved
  | sc_in_resolved
  | sc_out_resolved
  | sc_inout_resolved
*/
/*
  sc-resolved-vector-key ::=
  sc_signal_rv
  | sc_in_rv
  | sc_out_rv
  | sc_inout_rv
*/
/*
  sc-sub-module-declaration ::=
  id-expression [ * ] identifier ;
*/
/*
  sc-module-constractor-declaration ::=
  SC_CTOR( identifier ) ;
  | identifier ( sc_module_name [ identifier ] [ , parameter-declaration-list ] ) ;
*/
/*
  sc-module-constructor-definition ::=
  SC_CTOR( identifier ) [ ctor-initializer ] sc-module-constructor-body
  | identifier ( sc_module_name identifier [ , parameter-declaration-list ] ) : sc_module ( identifier ) [ , mem-initializer-list ] sc-module-constructor-body
*/
/*
  sc-module-constractor-body ::=
  { [ sc-module-constractor-element-seq ] }
*/
/*
  sc-module-constractor-element-seq ::=
  sc-module-constractor-element
  | sc-module-constractor-element-seq sc-module-constractor-element
*/
/*
  sc-module-constractor-element ::=
  sc-module-instantiation-statement
  | sc-port-binding-statement
  | sc-process-statement
*/
/*
  sc-module-instantiation-statement ::=
  identifier = new [ :: ] [ nested-name-specifier ] class-name ( string_literal ) ;
*/
/*
  sc-port-binding-statement ::=
  sc-named-port-binding-statement ;
  | sc-positional-port-binding-statement ;
*/
/*
  sc-named-port-binding-statement ::=
  identifier -> id-expression ( id-expression ) ;
  | identifier . id-expression ( id-expression ) ;
*/
/*
  sc-positional-port-binding ::=
  [ * ] identifier ( identifier-list )
*/
/*
  identifier-list ::=
  id-expression
  | identifier-list id-expression
*/
/*
  sc-process-statement ::=
  SC_METHOD ( identifier ) ; sensitivity-list| SC_CTHREAD ( identifier , sc-event ) ; [ sc-watching-statement ]
*/
/*
  sc-process-definition ::=
  void sc-process-id ( ) sc-process-body
*/
/*
  sc-process-id ::=
  identifier
  | template-id
  | [ :: ] nested-name-specifier [ template ] identifier
  | [ :: ] nested-name-specifier [ template ] template-id
*/
/*
  sc-process-body ::=
  sc-method-body
  | sc-cthread-body
*/
/*
  sc-method-body ::=
  compound-statement
*/
/*
  sc-sensitivity-list ::=
  sc-sensitivity-clause
  | sc-sensitivity-list sc-sensitivity-clause
*/
/*
  sc-sensitivity-clause ::=
  sensitive ( sc-event ) ;
  | sensitive_pos ( identifier ) ;
  | sensitive_neg ( identifier ) ;
  | sensitive sc-event-stream ;
  | sensitive_pos sc-event-stream ;
  | sensitive_neg sc-event-stream ;
*/
/*
  sc-event-stream ::=
  << sc-event
  | sc-event-stream << sc-event
*/
/*
  sc-identifier-stream ::=
  << identifier
  | sc-identifier-stream << identifier
*/
/*
  sc-event ::=
  identifier
  | identifier . pos ( )
  | identifier . neg ( )
*/
/*
  sc-watching-satement ::=
  watching ( identifier . delayed ( ) == sc-watching-condition ) ;
*/
/*
  sc-watching-condition ::=
  boolean-literal
  | logic-literal
*/
/*
  sc-cthread-body ::=
  compound-statement wait ( ) ; while ( true ) { compound-statement }
*/
/*
  sc-has-process-declaration ::=
  SC_HAS_PROCESS( identifier ) ;
*/

/*
  A.9 Derived classes
*/

/*
  base-clause ::=
  : base-specifier-list
*/
bool
SCParser::base_clause()
{
	if (term_(_COLON)) {
		if (nterm_(base_specifier_list)) {
			push(&SCParserAction::onBaseClause);
			return true;
		}
		PARSE_ERROR(PA_BASE_CLAUSE_BASE_SPECIFIER_EXPECTED);
	}
	return false;
}

/*
  base-specifier-list ::=
  base-specifier
  | base-specifier-list , base-specifier
*/
bool
SCParser::base_specifier_list()
{
	if (nterm_(base_specifier) &&
		ntermopt_(base_specifier_list_prime)) {
		push(&SCParserAction::onBaseSpecListHead);
		return true;
	}
	return false;
}
bool
SCParser::base_specifier_list_prime()
{
	if (term_(_COMMA) &&
		nterm_(base_specifier) &&
		ntermopt_(base_specifier_list_prime)) {
		push(&SCParserAction::onBaseSpecListRest);
		return true;
	}
	return false;
}

/*
  base-specifier ::=
  [ :: ] [ nested-name-specifier ] class-name
  | virtual [ access-specifier ] [ :: ] [ nested-name-specifier ] class-name
  | access-specifier [ virtual ] [ :: ] [ nested-name-specifier ] class-name
*/
bool
SCParser::base_specifier()
{
	if (term_(_VIRTUAL)) {
		if (ntermopt_(access_specifier) &&
			termopt_(_SCOPE) &&
			ntermopt_(nested_name_specifier) &&
			nterm_(type_name)) {
			push(&SCParserAction::onBaseSpec_Virtual);
			return true;
		}
				
		PARSE_ERROR(PA_BASE_SPECIFIER_CLASSNAME_EXPECTED);
	}
	if (nterm_(access_specifier)) {
		if (termopt_(_VIRTUAL) &&
			termopt_(_SCOPE) &&
			ntermopt_(nested_name_specifier) &&
			nterm_(type_name)) {
			push(&SCParserAction::onBaseSpec_AccessSpec);
			return true;
		}

		PARSE_ERROR(PA_BASE_SPECIFIER_CLASSNAME_EXPECTED);
	}
	if (termopt_(_SCOPE) &&
		ntermopt_(nested_name_specifier) &&
		nterm_(type_name)) {
		push(&SCParserAction::onBaseSpec);
		return true;
	}
	return false;
}

/*
  access-specifier ::= private | protected | public
*/
bool
SCParser::access_specifier()
{
	const ref<TokenValue> &val = tokenit.val();
	if (tokenit.eat(_PRIVATE) ||
		tokenit.eat(_PROTECTED) ||
		tokenit.eat(_PUBLIC)) {
		push(&SCParserAction::onAccessSpec, val);
		return true;
	}
	return false;
}

/*
  A.10 Special member functions
*/
/*
  conversion-function-id ::=
  operator conversion-type-id
*/
bool
SCParser::conversion_function_id()
{
	if (term_(_OPERATOR) &&
		nterm_(conversion_type_id)) {
		push(&SCParserAction::onConversionFuncId);
		return true;
	}
	return false;
}

/*
  converion-type-id ::=
  type-specifier-seq [ conversion-declarator ]
*/
bool
SCParser::conversion_type_id()
{
	if (nterm_(type_specifier_seq) &&
		ntermopt_(conversion_declarator)) {
		push(&SCParserAction::onConversionTypeId);
		return true;
	}
	return false;
}

/*
  conversion-declarator ::=
  ptr-operator [ conversion-declarator ]
*/
bool
SCParser::conversion_declarator()
{
	if (nterm_(ptr_operator) &&
		ntermopt_(conversion_declarator)) {
		push(&SCParserAction::onConversionDecl);
		return true;
	}
	return false;
}

/*
  ctor-initializer ::=
  : mem-initializer-list
*/
bool
SCParser::ctor_initializer()
{
	if (term_(_COLON)) {
		if (nterm_(mem_initializer_list)) {
			push(&SCParserAction::onCtorInit);
			return true;
		}
		PARSE_ERROR(PA_CTOR_INITIALIZER_LIST_EXPECTED);
	}
	return false;
}

/*
  mem-initializer-list ::=
  mem-initializer
  | mem-initializer , mem-initializer-list
*/
bool
SCParser::mem_initializer_list()
{
	if (nterm_(mem_initializer) &&
		ntermopt_(mem_initializer_list_prime)) {
		push(&SCParserAction::onMemInitListHead);
		return true;
	}
	return false;
}
bool
SCParser::mem_initializer_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(mem_initializer) &&
			ntermopt_(mem_initializer_list_prime)) {
			push(&SCParserAction::onMemInitListRest);
			return true;
		}
		PARSE_ERROR(PA_MEM_INITIALIZER_EXPECTED);
	}
	return false;
}

/*
  mem-initializer ::=
  mem-initializer-id ( [ expression-list ] )
*/
bool
SCParser::mem_initializer()
{
	if (nterm_(mem_initializer_id)) {
		if (term_(_LPAREN) &&
			ntermopt_(expression_list) &&
			term_(_RPAREN)) {
			push(&SCParserAction::onMemInit);
			return true;
		}
		PARSE_ERROR(PA_MEM_INITIALIZER_PARENTHESIS_EXPECTED);
	}
	return false;
}

/*
  mem-initializer-id ::=
  [ :: ] [ nested-name-specifier ] class-name
  | identifier
*/
bool
SCParser::mem_initializer_id()
{
	if (termopt_(_SCOPE) &&
		ntermopt_(nested_name_specifier) &&
		nterm_(type_name)) {
		push(&SCParserAction::onMemInitId_NestedClassName);
		return true;
	}
	back();
	if (nterm_(identifier)) {
		push(&SCParserAction::onMemInitId);
		return true;
	}
	return false;
}

/*
  A.11 Overloading
*/

/*
  operator-function-id ::=
  operator operator-id
*/
bool
SCParser::operator_function_id()
{
	if (term_(_OPERATOR)) {
		if (nterm_(operator_id)) {
			push(&SCParserAction::onOperatorFuncId);
			return true;
		}
		//or type conversion function
	}
	return false;
}

/*
  operator-id ::= one of
  //new delete new[] delete[]
  + - * / % ^ & | ~
  ! = < > += -= *= /= %=
  ^= &= |= << >> >>= <<= == !=
  <= >= && || ++ -- , ->* ->
  () []
*/
bool
SCParser::operator_id()
{
	const ref<TokenValue> &val = tokenit.val();
	if (val->isOp()) {
		tokenit.next();
		push(&SCParserAction::onOperatorId, val);
		return true;
	}

	if (tokenit.eat(_LPAREN)) {
		if (tokenit.eat(_RPAREN)) {
			push(&SCParserAction::onOperatorId, new TokenOp("()", PARENTHESIS));
			return true;
		}
		PARSE_ERROR(PA_OPERATOR_ID_RPAREN_EXPECTED);
	}

	if (tokenit.eat(_LBRACKET)) {
		if (tokenit.eat(_RBRACKET)) {
			push(&SCParserAction::onOperatorId, new TokenOp("[]", BRACKETS));
			return true;
		}
		PARSE_ERROR(PA_OPERATOR_ID_RBRACKET_EXPECTED);
	}

	return false;
}

/*
  A.12 Templates
*/

/*
  template-declaration ::=
  [ export ] template < template-parameter-list > declaration
*/
bool
SCParser::template_declaration()
{
	if (termopt_(_EXPORT) &&
		term_(_TEMPLATE)) {
		if (term_(_LT) &&
			nterm_(template_parameter_list) &&
			term_(_GT) &&
			nterm_(declaration)) {
			push(&SCParserAction::onTemplateDecl);
			return true;
		}
		//or explicit-instantiation 
		//or explicit-specialization
	}
	return false;
}

/*
  template-parameter-list ::=
  template-parameter
  | template-parameter-list , template-parameter
*/
bool
SCParser::template_parameter_list()
{
	if (nterm_(template_parameter) &&
		ntermopt_(template_parameter_list_prime)) {
		push(&SCParserAction::onTemplateParamListHead);
		return true;
	}
	return false;
}
bool
SCParser::template_parameter_list_prime()
{
	if (term_(_COMMA)) {
		if (nterm_(template_parameter) &&
			ntermopt_(template_parameter_list_prime)) {
			push(&SCParserAction::onTemplateParamListRest);
			return true;
		}
		PARSE_ERROR(PA_TEMPLATE_PARAMETER_EXPECTED);
	}
	return false;
}

/*
  template-parameter ::=
  type-parameter
  | parameter-declaration
*/
bool
SCParser::template_parameter()
{
	if (nterm_(type_parameter)) {
		push(&SCParserAction::onTemplateParam_TypeParam);
		return true;
	}
	if (nterm_(parameter_declaration)) {
		push(&SCParserAction::onTemplateParam_ParamDecl);
		return true;
	}
	return false;
}

/*
  type-parameter ::=
  class [ identifier ]
  | class [ identifier ] = type-id
  | typename [ identifier ]
  | typename [ identifier ] = type-id
  | template < template-parameter-list > class [identifier ]
  | template < template-parameter-list > class [identifier ] = id-expression
*/
bool
SCParser::type_parameter()
{
	if (term_(_CLASS) || term_(_TYPENAME)) {
		const Token &t = tokenit.get();
		String name;
		if (t.id == _IDENT) {
			name = t.val->toString();
		}
		ntermopt_(identifier);

		if (term_(_ASSIGN)) {
			if (nterm_(type_id)) {
				//class|typename [identifier] = type-id
				push(&SCParserAction::onTypeParam_IdentAssign);

				//FIXME:the identifier should be added in the current class scope only
				if (!name.empty()) {
					addTypeName(name);
				}
				return true;
			}
			//expected type-id
			PARSE_ERROR(PA_TYPE_PARAMETER_TYPE_ID_EXPECTED);
		}
		//class|typename [identifier]
		push(&SCParserAction::onTypeParam_Ident);
		//FIXME:the identifier should be added in the current class scope only
		if (!name.empty()) {
			addTypeName(name);
		}
		return true;
	}
	//back();

	if (term_(_TEMPLATE)) {
		if (term_(_LT) &&
			nterm_(template_parameter_list) &&
			term_(_GT) &&
			term_(_CLASS) &&
			ntermopt_(identifier)) {
			if (term_(_ASSIGN)) {
				if (nterm_(id_expression)) {
					//template < template-parameter-list > class [identifier ] = id-expression
					push(&SCParserAction::onTypeParam_TemplateClassAssign);
					return true;
				}
				//exptected id_expression
				PARSE_ERROR(PA_TYPE_PARAMETER_ID_EXP_EXPECTED);
			}
			//template < template-parameter-list > class [identifier ]
			push(&SCParserAction::onTypeParam_TemplateClass);
			return true;
		}
		PARSE_ERROR(PA_TYPE_PARAMETER_TEMPLATE_NOT_VALID);
	}
	return false;
}

/*
  template-id ::=
  identifer < [ template-argument-list ] >
*/
bool
SCParser::template_id()
{
	if (nterm_(identifier) &&
		term_(_LT)) {
		++templateArgNestLevel;
		expectedTemplateArgEndDelimiter.push_back(templateArgNestLevel);
		if (ntermopt_(template_argument_list) &&
			term_(_GT)) {
			expectedTemplateArgEndDelimiter.pop_back();
			--templateArgNestLevel;
			push(&SCParserAction::onTemplateId);
			return true;
		}
	}
	return false;
}

/*
  template-argument-list ::=
  template-argument
  | template-argument-list , template-argument
*/
bool
SCParser::template_argument_list()
{
	if (nterm_(template_argument) &&
		ntermopt_(template_argument_list_prime)) {
		push(&SCParserAction::onTemplateArgListHead);
		return true;
	}
	return false;
}
bool
SCParser::template_argument_list_prime()
{
	if (term_(_COMMA) &&
		nterm_(template_argument) &&
		ntermopt_(template_argument_list_prime)) {
		push(&SCParserAction::onTemplateArgListRest);
		return true;
	}
	return false;
}

/*
  template-argument ::=
  assignment-expression
  | type-id
  | id-expression
*/
bool
SCParser::template_argument()
{
	if (nterm_(assignment_expression)) {
		push(&SCParserAction::onTemplateArg_AssignExp);
		return true;
	} else if (nterm_(type_id)) {
		push(&SCParserAction::onTemplateArg_TypeId);
		return true;
	} else if (nterm_(id_expression)) {
		push(&SCParserAction::onTemplateArg_ScopedId);
		return true;
	}
	return false;
}

/*
  explicit-instantiation ::=
  template declaration
*/
bool
SCParser::explicit_instantiation()
{
	if (term_(_TEMPLATE) &&
		nterm_(declaration)) {
		push(&SCParserAction::onExplicitInst);
		return true;
	}
	return false;
}

/*
  explicit-specialization ::=
  template < > declaration
*/
bool
SCParser::explicit_specialization()
{
	if (term_(_TEMPLATE) &&
		term_(_LT) &&
		term_(_GT) &&
		nterm_(declaration)) {
		push(&SCParserAction::onExplicitSpecial);
		return true;
	}
	return false;
}

