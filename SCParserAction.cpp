#include "SCParserAction.hpp"

struct OpExp
{
	Operator op;
	ASTExp *exp;
};

void 
SCParserAction::ASTStack::dump() 
{
	std::vector<void*>::const_iterator it = stack.begin();
	while (it != stack.end()) {
		DBG("%p", *it);
		++it;
	}
}

void 
SCParserAction::ASTStack::push(void *p) 
{
	PARSE_DBG("ASTStack::push %p", p);
	stack.push_back(p);
	DUMP();
}

void 
SCParserAction::ASTStack::push(Operator op) 
{
	PARSE_DBG("ASTStack::push %d", (int)op);
	stack.push_back(reinterpret_cast<void*>(op));
	DUMP();
}

void 
SCParserAction::ASTStack::pushi(int i) 
{
	PARSE_DBG("ASTStack::push %d", (int)i);
	stack.push_back(reinterpret_cast<void*>(i));
	DUMP();
}

void *
SCParserAction::ASTStack::pop() 
{
	assert(!stack.empty());
	void *p = stack.back();
	stack.pop_back();
	PARSE_DBG("ASTStack::pop %p", p);
	DUMP();
	return p;
}

Operator 
SCParserAction::ASTStack::popop() 
{
	assert(!stack.empty());
	intptr_t p = reinterpret_cast<intptr_t>(stack.back());
	stack.pop_back();
	Operator op = static_cast<Operator>(p);
	PARSE_DBG("ASTStack::pop %d", p);
	DUMP();
	return op;
}

bool 
SCParserAction::ASTStack::empty() const 
{
	return stack.empty();
}


AST *
SCParserAction::list2ast(ASTList *list, AST *ast)
{
	while (list) {
		list->ast->setLeft(ast);
		ASTList *old = list;
		ast = list->ast;
		list = list->next;
		delete old;
	}
	return ast;
}

AST *
SCParserAction::getAST() 
{
	if (!astStack.empty()) {
		return astStack.popas<AST*>();
	} else {
		return NULL;
	}
}

void
SCParserAction::onEmpty(const ref<TokenValue> &v) 
{
	FUNCLOG;
	PARSE_DBG("EMPTY <%s>", v->toString().c_str());
	astStack.push(NULL);
}

void
SCParserAction::onToken(const ref<TokenValue> &v) 
{
	FUNCLOG;
	PARSE_DBG("%s", v->toString().c_str());
	astStack.push(Symbol::symbol(v->toString()));
}

void
SCParserAction::onIdentifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(Symbol::symbol(v->toString()));
}

void
SCParserAction::onLiteral_Decimal(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(v->toInt()));
}

void
SCParserAction::onLiteral_Octal(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(v->toInt()));
}

void
SCParserAction::onLiteral_Hex(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(v->toInt()));
}

void
SCParserAction::onLiteral_String(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(v->toString().c_str()));
}

void
SCParserAction::onLiteral_True(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(true));
}

void
SCParserAction::onLiteral_False(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new PrimaryExpression(false));
}

//[ declaration-seq ] [ sc-main-definition ]
void
SCParserAction::onTranslationUnit(const ref<TokenValue> &v) 
{
	FUNCLOG;
	DeclarationSeq *seq = astStack.popas<DeclarationSeq*>();
	astStack.push(new TranslationUnit(seq));
}

void
SCParserAction::onPrimaryExp_Literal(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

void
SCParserAction::onPrimaryExp_This(const ref<TokenValue> &v) 
{
	FUNCLOG;
	UnqualifiedId *id = new UnqualifiedId(new ASTIdent(Symbol::symbol("this")));
	ScopedId *sid = new ScopedId(NULL, id);
	astStack.push(new PrimaryExpression(sid));
}

// ( expression )
void
SCParserAction::onPrimaryExp_Expression(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();//)
	SequenceExpression *exps = astStack.popas<SequenceExpression*>();
	astStack.pop();//(

	astStack.push(new PrimaryExpression(exps));
}

void
SCParserAction::onPrimaryExp_IdExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ScopedId *id = astStack.popas<ScopedId*>();
	astStack.push(new PrimaryExpression(id));
}

void
SCParserAction::onIdExp_UnqualifiedId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	UnqualifiedId *id = astStack.popas<UnqualifiedId*>();
	astStack.push(new ScopedId(NULL, id));
}

void
SCParserAction::onIdExp_QualifiedId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

void
SCParserAction::onUnqualifiedId_Identifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Symbol *ident = astStack.popas<Symbol*>();
	astStack.push(new UnqualifiedId(new ASTIdent(ident)));
}

void 
SCParserAction::onUnqualifiedId_TemplateId(const ref<TokenValue> &v)
{
	FUNCLOG;
	TemplateId *templ = astStack.popas<TemplateId*>();
	astStack.push(new UnqualifiedId(templ));
}

void 
SCParserAction::onUnqualifiedId_OperatorId(const ref<TokenValue> &v)
{
	FUNCLOG;
	OperatorFunctionId *opid = astStack.popas<OperatorFunctionId*>();
	astStack.push(new UnqualifiedId(opid));
}

void 
SCParserAction::onUnqualifiedId_ConversionId(const ref<TokenValue> &v)
{
	FUNCLOG;
	ConversionTypeId *convid = astStack.popas<ConversionTypeId*>();
	astStack.push(new UnqualifiedId(convid));
}

//[ :: ] nested-name-specifier [ template ] unqualified-id
void
SCParserAction::onQualifiedId_Valid1(const ref<TokenValue> &v) 
{
	FUNCLOG;
	UnqualifiedId *id   = astStack.popas<UnqualifiedId*>();
	Symbol *kw_template = astStack.popas<Symbol*>();//template opt
	Scope *scope        = astStack.popas<Scope*>();
	astStack.pop();//:: opt

	//TODO:
	(void)kw_template;

	astStack.push(new ScopedId(scope, id));
}

// :: identifier
void
SCParserAction::onQualifiedId_Identifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Symbol *sym = astStack.popas<Symbol*>();
	astStack.pop();//::

	UnqualifiedId *id = new UnqualifiedId(new ASTIdent(sym));
	astStack.push(new ScopedId(NULL, id));
}

// :: operator-function-id
void
SCParserAction::onQualifiedId_OperatorFuncId(const ref<TokenValue> &v)
{
	FUNCLOG;
	OperatorFunctionId *opid = astStack.popas<OperatorFunctionId*>();
	astStack.pop();//::

	UnqualifiedId *id = new UnqualifiedId(opid);
	astStack.push(new ScopedId(NULL, id));
}

// :: template-id
void
SCParserAction::onQualifiedId_TemplateId(const ref<TokenValue> &v)
{
	FUNCLOG;
	TemplateId *tempid = astStack.popas<TemplateId*>();
	astStack.pop();//::

	UnqualifiedId *id = new UnqualifiedId(tempid);
	astStack.push(new ScopedId(NULL, id));
}
	
// class-or-namespace-name :: [ nested-name-specifier ]
void
SCParserAction::onNestedNameSpecifier_Valid1(const ref<TokenValue> &v)
{
	FUNCLOG;
	Scope *scope = astStack.popas<Scope*>(); //opt
	astStack.pop(); //::
	ScopeName *scopeName = astStack.popas<ScopeName*>();

	if (scope) {
		scope->addOuter(scopeName);
	} else {
		scope = new Scope(scopeName);
	}
	astStack.push(scope);
}
//class-or-namespace-name :: template nested-name-specifier
void
SCParserAction::onNestedNameSpecifier_Valid2(const ref<TokenValue> &v)
{
	FUNCLOG;
	Scope *scope = astStack.popas<Scope*>();
	Symbol *kw_template = astStack.popas<Symbol*>(); //template
	//TODO:
	(void)kw_template;

	astStack.pop(); //::
	ScopeName *scopeName = astStack.popas<ScopeName*>();
	scope->addOuter(scopeName);
	astStack.push(scope);
}

void 
SCParserAction::onClassOrNamespaceName_Identifier(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();
	astStack.push(new ScopeName(name));
}

void 
SCParserAction::onClassOrNamespaceName_TemplateId(const ref<TokenValue> &v)
{
	FUNCLOG;
	TemplateId *id = astStack.popas<TemplateId*>();
	astStack.push(new ScopeName(id));
}

// primary-expression postfix-expression'
void
SCParserAction::onPostfixExp_PrimaryExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next   = astStack.popas<ASTList*>(); //opt
	ASTExp *primary = astStack.popas<ASTExp*>();
	if (next) {
		AST *ast = list2ast(next, primary);
		PARSE_DBG("%s", ast->toString().c_str());
		astStack.push(ast);
	} else {
		astStack.push(primary);
	}
}

//simple-type-specifier ( [ expression-list ] ) postfix-expression'
void
SCParserAction::onPostfixExp_SimpleTypeSpecifier(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	astStack.pop(); //)
	SequenceExpression *args = astStack.popas<SequenceExpression*>(); //opt
	astStack.pop(); //(
	TypeSpecifier *typeSpec = astStack.popas<TypeSpecifier*>();

	Constructor *ctor = new Constructor(typeSpec, args);
	if (next) {
		AST *ast = list2ast(next, ctor);
		PARSE_DBG("%s", ast->toString().c_str());
		astStack.push(ast);
	} else {
		astStack.push(ctor);
	}
}

// [ expression ] postfix-expression'
void
SCParserAction::onPostfixExp_Subscript(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	astStack.pop(); //]
	SequenceExpression *exps = astStack.popas<SequenceExpression*>(); 
	astStack.pop(); //[
	Subscript *subscript = new Subscript(NULL, exps);

	ASTList *list = new ASTList();
	list->ast = subscript;
	list->next = next;
	astStack.push(list);
}

// ( [ expression-list ] ) postfix-expression'
void
SCParserAction::onPostfixExp_ExpressionList(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>();//opt
	astStack.pop();// )
	SequenceExpression *args  = astStack.popas<SequenceExpression*>();//opt
	astStack.pop();// (
		
	FunctionCall *func = new FunctionCall(NULL);
	func->args = args;
	ASTList *list = new ASTList();
	list->ast = func;
	list->next = next;
	astStack.push(list);
}

//.  [ template ] id-expression postfix-expression'
void
SCParserAction::onPostfixExp_MemberSelect(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	ASTExp *exp = astStack.popas<ASTExp*>();
	Symbol *kw_template = astStack.popas<Symbol*>(); //opt
	//TODO
	(void)kw_template;
	astStack.pop(); // .

	MemberSelect *memberSelect = new MemberSelect(NULL, exp, false);
	ASTList *list = new ASTList();
	list->ast = memberSelect;
	list->next = next;
	astStack.push(list);
}

//-> [ template ] id-expression postfix-expression'
void
SCParserAction::onPostfixExp_ArrowMemberSelect(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	ASTExp *exp = astStack.popas<ASTExp*>();
	Symbol *kw_template = astStack.popas<Symbol*>(); //opt
	//TODO
	(void)kw_template;
	astStack.pop(); // ->

	MemberSelect *memberSelect = new MemberSelect(NULL, exp, true);
	ASTList *list = new ASTList();
	list->ast = memberSelect;
	list->next = next;
	astStack.push(list);
}

// postfix-operator postfix-expression'
void
SCParserAction::onPostfixExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	Operator op = astStack.popop();
				
	UnaryExpression *unary = new UnaryExpression(op, NULL);

	ASTList *list = new ASTList();
	list->ast = unary;
	list->next = next;
	astStack.push(list);
}
void
SCParserAction::onPostfixOperator(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.push(v->toOp());
}

void
SCParserAction::onCxxCastId(const ref<TokenValue> &v)
{
	FUNCLOG;
	assert(v->isInt());
	astStack.pushi(v->toInt());
}
//cxxcast_id < type-id > ( expression ) postfix-expression'
void
SCParserAction::onCxxCastExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTList *next = astStack.popas<ASTList*>(); //opt
	astStack.pop();// )
	SequenceExpression *exps = astStack.popas<SequenceExpression*>();
	astStack.pop();// (
	astStack.pop();// >
	TypeId *typeId = astStack.popas<TypeId*>();
	astStack.pop();// <
	CastType castt = astStack.popi<CastType>();

	CastExpression *cast = new CastExpression(castt, typeId, exps);
	if (next) {
		AST *ast = list2ast(next, cast);
		PARSE_DBG("%s", ast->toString().c_str());
		astStack.push(ast);
	} else {
		astStack.push(cast);
	}
}

// expression expression-list'
void
SCParserAction::onExpressionListHead(const ref<TokenValue> &v)
{
	FUNCLOG;
	SequenceExpression *tail = astStack.popas<SequenceExpression*>(); //opt
	ASTExp *exp = astStack.popas<ASTExp*>();

	if (!tail) {
		tail = new SequenceExpression();
	}		
	tail->exps.push_front(exp);
	astStack.push(tail);
}

//  , expression expression-list'
void
SCParserAction::onExpressionListRest(const ref<TokenValue> &v)
{
	FUNCLOG;
	SequenceExpression *tail = astStack.popas<SequenceExpression*>(); //opt
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop();//','

	SequenceExpression *seq = NULL;
	if (tail) {
		seq = tail;
	} else {
		seq = new SequenceExpression();
	}
	seq->exps.push_front(exp);
	astStack.push(seq);
}

void
SCParserAction::onUnaryExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	Operator op = astStack.popop();

	UnaryExpression *unary = new UnaryExpression(op, exp);
	astStack.push(unary);
}
void
SCParserAction::onUnaryOperator(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.push(v->toOp());
}

//( type_id ) cast-expression
void
SCParserAction::onCastExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop(); // )
	TypeId *typeId = astStack.popas<TypeId*>();
	astStack.pop(); // (

	astStack.push(new CastExpression(C_CAST, typeId, exp));
}

void 
SCParserAction::onBinaryExpHead(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//dummy
	ASTExp *exp = astStack.popas<ASTExp*>();

	exp = makeBinExp(exp);
	astStack.push(exp);
}

void 
SCParserAction::onBinaryExp(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//dummy
	ASTExp *exp = astStack.popas<ASTExp*>();
	Operator op = astStack.popop();

	BinaryExpression *binexp = new BinaryExpression(op, NULL, exp);
	binExpStack.push_back(binexp);
	astStack.push(binexp);//dummy
}

void 
SCParserAction::onBinaryOperator(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.push(v->toOp());
}

ASTExp *
SCParserAction::makeBinExp(ASTExp *e)
{
	if (binExpStack.empty()) return e;

	assert(binExpStack.back()->l == NULL);
	binExpStack.back()->l = e;

	while (true) {
		BinaryExpression *exp1, *exp2;
		exp1 = binExpStack.back();
		binExpStack.pop_back();
		if (!binExpStack.empty()) {
			exp2 = binExpStack.back();
			binExpStack.pop_back();
			if (getBinOpPriority(exp1->op) <= getBinOpPriority(exp2->op)) {
				exp2->l = exp1;
				binExpStack.push_back(exp2);
				continue;
			} else {
				exp2->l = exp1->r;
				exp1->r = exp2;
				binExpStack.push_back(exp1);
				continue;
			}
		} else {
			return exp1;
		}
	}
}

int
SCParserAction::getBinOpPriority(Operator op)
{
	switch (op) {
	case STAR:	case SLASH:	case PERCENT:
		return 0;
	case PLUS:	case MINUS:
		return 1;
	case LSHIFT: case RSHIFT:
		return 2;
	case LT: case LE: case GT: case GE:
		return 3;
	case EQ: case NE:
		return 4;
	case BIT_AND:
		return 5;
	case BIT_XOR:
		return 6;
	case BIT_OR:
		return 7;
	case AND:
		return 8;
	case OR:
		return 9;
	default:
		assert(0);
		return -1;
	}
}


//logical-or-expression ? expression : assignment-expression
void
SCParserAction::onConditionalExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *rexp = astStack.popas<ASTExp*>();
	astStack.pop(); //:
	SequenceExpression *lexp = astStack.popas<SequenceExpression*>();
	astStack.pop(); //?
	ASTExp *cond = astStack.popas<ASTExp*>();
	astStack.push(new ConditionalExpression(cond, lexp, rexp));
}

//logical-or-expression assignment-operator assignment-expression
void
SCParserAction::onAssignExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *assignor = astStack.popas<ASTExp*>();
	Operator op = astStack.popop();
	ASTExp *assignee = astStack.popas<ASTExp*>();
	astStack.push(new BinaryExpression(op, assignee, assignor));
}

void
SCParserAction::onAssignOperator(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(v->toOp());
}

//identifier : statement
void
SCParserAction::onLabeledStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop(); // :
	Symbol *label = astStack.popas<Symbol*>();
		
	astStack.push(new LabelStatement(label, stm));
}

//case constant-expression : statement
void
SCParserAction::onCaseStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop(); // :
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop(); // case

	astStack.push(new CaseStatement(exp, stm));
}

//default : statement
void
SCParserAction::onDefaultCaseStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop(); // :
	astStack.pop(); // default

	astStack.push(new CaseStatement(NULL, stm));
}

//[ expression-list ] ;
void
SCParserAction::onExpressionStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	SequenceExpression *seq = astStack.popas<SequenceExpression*>();//opt
	if (exp) {
		astStack.push(new ExpressionStatement(seq));
	} else {
		astStack.push(new ExpressionStatement(NULL));
	}
}

//{ [ statement_seq ] }
void
SCParserAction::onCompoundStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop(); // }
	CompoundStatement *cs = astStack.popas<CompoundStatement*>(); // opt
	astStack.pop(); // {

	if (cs) {
		astStack.push(cs);
	} else {
		astStack.push(new CompoundStatement());
	}
}

//statement [statement-seq]
void
SCParserAction::onStatementSeq(const ref<TokenValue> &v) 
{
	FUNCLOG;
	CompoundStatement *cs = astStack.popas<CompoundStatement*>();// opt
	ASTStm *stm = astStack.popas<ASTStm*>();
		
	if (cs == NULL) {
		cs = new CompoundStatement();
	}
	cs->stms.push_front(stm);
	astStack.push(cs);
}

//
void
SCParserAction::onWaitStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//NIY
}

//
void
SCParserAction::onSignalAssignmentStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//NIY
}

//if ( condition ) statement
void
SCParserAction::onIfStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *thenstm = astStack.popas<ASTStm*>();
	astStack.pop();// )
	Condition *cond = astStack.popas<Condition*>();
	astStack.pop();// (
	astStack.pop();// if

	astStack.push(new IfStatement(cond, thenstm, NULL));
}

//if ( condition ) statement else statement
void
SCParserAction::onIfElseStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *elsestm = astStack.popas<ASTStm*>();
	astStack.pop();// else
	ASTStm *thenstm = astStack.popas<ASTStm*>();
	astStack.pop();// )
	Condition *cond = astStack.popas<Condition*>();
	astStack.pop();// (
	astStack.pop();// if

	astStack.push(new IfStatement(cond, thenstm, elsestm));
}

//switch ( condition ) statement
void
SCParserAction::onSwitchStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop();// )
	Condition *cond = astStack.popas<Condition*>();
	astStack.pop();// (
	astStack.pop();// switch

	astStack.push(new SwitchStatement(cond, stm));
}

//expression
void
SCParserAction::onCondition_Exp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	SequenceExpression *exps = astStack.popas<SequenceExpression*>();
	astStack.push(new Condition(exps));
}

//type_specifier_seq declarator = assignment_expression
void
SCParserAction::onCondition_AssignExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop(); // = 
	Declarator *decl = astStack.popas<Declarator*>();
	TypeSpecifier *typeSpec = astStack.popas<TypeSpecifier*>();

	astStack.push(new Condition(typeSpec, decl, exp));
}

//while ( condition ) statement
void
SCParserAction::onWhileStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop();// )
	Condition *cond = astStack.popas<Condition*>();
	astStack.pop();// (
	astStack.pop();// while

	astStack.push(new WhileStatement(cond, stm));
}

//do statement while ( expression ) ;
void
SCParserAction::onDoWhileStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	astStack.pop();// )
	SequenceExpression *exps = astStack.popas<SequenceExpression*>();
	astStack.pop();// (
	astStack.pop();// while
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop();// do
		
	astStack.push(new DoStatement(exps, stm));
}

//for ( for-init-statement [ condition ] ; [ expression ] ) statement
void
SCParserAction::onForStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTStm *stm = astStack.popas<ASTStm*>();
	astStack.pop();// )
	SequenceExpression *exps = astStack.popas<SequenceExpression*>(); //opt
	astStack.pop();// ;
	Condition *cond = astStack.popas<Condition*>(); //opt
	ForInitStatement *init = astStack.popas<ForInitStatement*>();
	astStack.pop();// (
	astStack.pop();// for
		
	astStack.push(new ForStatement(init, cond, exps, stm));
}

//expression-statement
void
SCParserAction::onForInitStm_ExpressionStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ExpressionStatement *expstm = astStack.popas<ExpressionStatement*>();
	astStack.push(new ForInitStatement(expstm));
}

//simple-declaration
void
SCParserAction::onForInitStm_SimpleDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//break ;
void
SCParserAction::onBreakStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	astStack.pop();// break
	astStack.push(new JumpStatement(JumpStatement::BREAK));
}

//continue ;
void
SCParserAction::onContinueStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	astStack.pop();// continue
	astStack.push(new JumpStatement(JumpStatement::CONTINUE));
}

//return [ expression ] ;
void
SCParserAction::onReturnStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//NIY
	astStack.pop();// ;
	SequenceExpression *exps = astStack.popas<SequenceExpression*>(); //opt
	astStack.pop();// return

	astStack.push(new JumpStatement(JumpStatement::RETURN, exps));
}

//goto identifier ;
void
SCParserAction::onGotoStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	Symbol *label = astStack.popas<Symbol*>();
	astStack.pop();// goto

	astStack.push(new JumpStatement(label));
}

//block-declaration
void
SCParserAction::onDeclarationStm(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BlockDeclaration *block = astStack.popas<BlockDeclaration*>();
	astStack.push(new DeclarationStatement(block));
}

// declaration [declaration-seq]
void
SCParserAction::onDeclSeq(const ref<TokenValue> &v) 
{
	DeclarationSeq *seq = astStack.popas<DeclarationSeq*>();//opt
	Declaration *decl   = astStack.popas<Declaration*>();

	if (seq == NULL) {
		seq = new DeclarationSeq();
	}
	seq->decls.push_front(decl);
	astStack.push(seq);
}

//block-declaration
void
SCParserAction::onDecl_BlockDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//function-definition
void
SCParserAction::onDecl_FuncDef(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//simple-declaration
void
SCParserAction::onBlockDecl_SimpleDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//namespace-alias-definition
void
SCParserAction::onBlockDecl_NamespaceAlias(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//using-declaration
void
SCParserAction::onBlockDecl_UsingDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//using-directive
void
SCParserAction::onBlockDecl_UsingDirective(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

// [ decl-specifier-seq ] [ init-declarator-list ] ;
void
SCParserAction::onSimpleDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	InitDeclaratorList *initDeclList = astStack.popas<InitDeclaratorList*>();// opt
	DeclSpecifierSeq *declSpecSeq    = astStack.popas<DeclSpecifierSeq*>();// opt
	astStack.push(new SimpleDeclaration(declSpecSeq, initDeclList));
}

//typ-specifier
void
SCParserAction::onDeclSpecifier_TypeSpecifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeSpecifier *typeSpec = astStack.popas<TypeSpecifier*>();
	astStack.push(new DeclSpecifier(typeSpec));
}

void
SCParserAction::onDeclSpecifier_OtherSpecifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	DeclSpecifierID declSpecId = static_cast<DeclSpecifierID>(v->toInt());
	astStack.push(new DeclSpecifier(declSpecId));
}

void
SCParserAction::onDeclSpecSeq(const ref<TokenValue> &v) 
{
	FUNCLOG;
	DeclSpecifierSeq *seq = astStack.popas<DeclSpecifierSeq*>();// opt
	DeclSpecifier *declSpec = astStack.popas<DeclSpecifier*>();
	if (seq == NULL) {
		seq = new DeclSpecifierSeq();
	}
	seq->declSpecs.push_front(declSpec);
	astStack.push(seq);
}

//[ :: ] [ nested-name-specifier ] type-name
void
SCParserAction::onSimpleTypeSpecifier_TypeName(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeName *name = astStack.popas<TypeName*>();
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt

	astStack.push(new UserTypeSpecifier(scope, name));
}

//[ :: ] nested-name-specifier template template-id
void 
SCParserAction::onSimpleTypeSpecifier_TemplateId(const ref<TokenValue> &v)
{
	FUNCLOG;
	TemplateId *id = astStack.popas<TemplateId*>();
	astStack.pop();// template
	Scope *scope = astStack.popas<Scope*>();
	astStack.pop();// :: opt

	astStack.push(new UserTypeSpecifier(scope, new TypeName(id)));
}

void
SCParserAction::onBuiltinTypeSpecifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	PARSE_DBG("%s", v->toString().c_str());
	assert(v->isInt());

	TypeSpecifierID type = static_cast<TypeSpecifierID>(v->toInt());
	astStack.push(new TypeSpecifier(type));
}

void
SCParserAction::onTypeName(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new TypeName(Symbol::symbol(v->toString())));
}

void
SCParserAction::onTypeName_TemplateId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateId *id = astStack.popas<TemplateId*>();
	astStack.push(new TypeName(id));
}

//class-key nested-opt-identifier
void 
SCParserAction::onElaboratedTypeSpec_Class(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();//identifier
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	TypeSpecifierID type = astStack.popi<TypeSpecifierID>();

	//bool clazz = (type == CLASS) ? true : false;
	astStack.push(new UserTypeSpecifier(scope, new TypeName(name)));
}

//enum nested-opt-identifier
void 
SCParserAction::onElaboratedTypeSpec_Enum(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();//identifier
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	astStack.pop();// enum

	astStack.push(new UserTypeSpecifier(scope, new TypeName(name)));
}

//typename [ :: ] nested-name-specifier identifier
void 
SCParserAction::onElaboratedTypeSpec_Typename(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();//identifier
	Scope *scope = astStack.popas<Scope*>();
	astStack.pop();// :: opt
	astStack.pop();// typename

	astStack.push(new UserTypeSpecifier(scope, new TypeName(name)));
}

//typename [ :: ] nested-name-specifier [ template ] template-id
void 
SCParserAction::onElaboratedTypeSpec_TypenameTemplateId(const ref<TokenValue> &v)
{
	FUNCLOG;
	TemplateId *id = astStack.popas<TemplateId*>();//identifier
	astStack.pop();// template opt
	Scope *scope = astStack.popas<Scope*>();
	astStack.pop();// :: opt
	astStack.pop();// typename

	astStack.push(new UserTypeSpecifier(scope, new TypeName(id)));
}

//enum [ identifier ] { [ enumerator-list ] }
void
SCParserAction::onEnumSpecifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// }
	EnumSpecifier *enumSpec = astStack.popas<EnumSpecifier*>(); // opt
	astStack.pop();// {
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();// enum

	if (enumSpec == NULL) {
		enumSpec = new EnumSpecifier();
	}
	enumSpec->name = name;
	astStack.push(enumSpec);
}

void
SCParserAction::onEnumeratorListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	EnumSpecifier *enumSpec = astStack.popas<EnumSpecifier*>();// opt
	EnumeratorDefinition *enumDef = astStack.popas<EnumeratorDefinition*>();
		
	if (enumSpec == NULL) {
		enumSpec = new EnumSpecifier();
	}
	enumSpec->enumDefs.push_front(enumDef);
	astStack.push(enumSpec);
}

void
SCParserAction::onEnumeratorListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	EnumSpecifier *enumSpec = astStack.popas<EnumSpecifier*>();// opt
	EnumeratorDefinition *enumDef = astStack.popas<EnumeratorDefinition*>();
	astStack.pop();// ,

	if (enumSpec == NULL) {
		enumSpec = new EnumSpecifier();
	}
	enumSpec->enumDefs.push_front(enumDef);
	astStack.push(enumSpec);
}

//identifier [= constant-expression]
void
SCParserAction::onEnumeratorDefinition(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *exp  = astStack.popas<ASTExp*>();// opt
	Symbol *name = astStack.popas<Symbol*>();

	astStack.push(new EnumeratorDefinition(name, exp));
}

//= constant-expression
void
SCParserAction::onEnumeratorDefinitionAssign(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop();// =
	astStack.push(exp);
}

//namespace [ identifier ] { namespace-body }
void 
SCParserAction::onNamespaceDefinition(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();// }
	DeclarationSeq *body = astStack.popas<DeclarationSeq*>();// opt
	astStack.pop();// {
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();//namespace

	astStack.push(new NamespaceDefinition(name, body));
}

//namespace identifier = qualified-namespace-specifier ;
void 
SCParserAction::onNamespaceAliasDefinition(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//;
	Symbol *name = astStack.popas<Symbol*>();//identifier
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();//:: opt
	astStack.pop();//=
	Symbol *alias = astStack.popas<Symbol*>();
	astStack.pop();//namespace

	UnqualifiedId *id = new UnqualifiedId(new ASTIdent(name));
	ScopedId *original = new ScopedId(scope, id);	
	astStack.push(new NamespaceAliasDefinition(alias, original));
}

// using :: unqualified-id ;
void 
SCParserAction::onUsingDeclaration(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//;
	UnqualifiedId *id = astStack.popas<UnqualifiedId*>();
	astStack.pop();//::
	astStack.pop();//using

	ScopedId *scopedid = new ScopedId(NULL, id);
	astStack.push(new UsingDeclaration(scopedid, false));
}

// using [ typename ] [ :: ] nested-name-specifier unqualified-id ;
void 
SCParserAction::onUsingDeclaration_Scoped(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//;
	UnqualifiedId *id = astStack.popas<UnqualifiedId*>();
	Scope *scope = astStack.popas<Scope*>();
	astStack.pop();//:: opt
	bool typeName = astStack.pop() ? true:false;
	astStack.pop();//using

	ScopedId *scopedid = new ScopedId(scope, id);
	astStack.push(new UsingDeclaration(scopedid, typeName));
}

// using namespace nested-opt-identifier ;
void 
SCParserAction::onUsingDirective(const ref<TokenValue> &v)
{
	FUNCLOG;
	astStack.pop();//;
	Symbol *name = astStack.popas<Symbol*>();//identifier
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();//:: opt
	astStack.pop();//namespace
	astStack.pop();//using

	UnqualifiedId *id = new UnqualifiedId(new ASTIdent(name));
	ScopedId *namespaceId = new ScopedId(scope, id);
	astStack.push(new UsingDirective(namespaceId));
}

//init-declarator init-declarator-list'
void
SCParserAction::onInitDeclListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	InitDeclaratorList *list = astStack.popas<InitDeclaratorList*>();// opt
	InitDeclarator *decl     = astStack.popas<InitDeclarator*>();

	if (list == NULL) {
		list = new InitDeclaratorList();
	}
	list->decls.push_front(decl);
	astStack.push(list);
}

//, init-declarator init-declarator-list'
void
SCParserAction::onInitDeclListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	InitDeclaratorList *list = astStack.popas<InitDeclaratorList*>();// opt
	InitDeclarator *decl     = astStack.popas<InitDeclarator*>();
	astStack.pop();// ,

	if (list == NULL) {
		list = new InitDeclaratorList();
	}
	list->decls.push_front(decl);
	astStack.push(list);
}

//declarator [ initializer ]
void
SCParserAction::onInitDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Initializer *init = astStack.popas<Initializer*>();// opt
	Declarator *decl  = astStack.popas<Declarator*>();

	astStack.push(new InitDeclarator(decl, init));
}

//direct-declarator
void
SCParserAction::onDecl_DirectDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//ptr-operator declarator
//ptr-operator [abstract-declarator]
void
SCParserAction::onDecl_PtrDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *decl = astStack.popas<Declarator*>();//opt
	PtrOperator *ptrOp = astStack.popas<PtrOperator*>();
	if (decl == NULL) {
		decl = new Declarator();
	}
	decl->ptrs.push_back(ptrOp);
	astStack.push(decl);
}

//[declarator-id] direct-declarator' 
//Note: declarator is abstract the case where declarator-id is NULL.
void
SCParserAction::onDirectDecl_DeclId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *decl = astStack.popas<Declarator*>();// opt
	ScopedId *declId = astStack.popas<ScopedId*>();
	if (decl == NULL) {
		decl = new Declarator();
	}
	decl->declId = declId;

	astStack.push(decl);
}

//( declarator ) direct-declarator' 
void
SCParserAction::onDirectDecl_FuncPtrDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *decl    = astStack.popas<Declarator*>();// opt
	astStack.pop();// )
	Declarator *subDecl = astStack.popas<Declarator*>();
	astStack.pop();// (

	if (decl == NULL) {
		decl = subDecl;
	} else {
		decl->subDecl = subDecl;
	}
	astStack.push(decl);
}

//( parameter-declaration-clause ) [ cv-qualifier-seq ]
void
SCParserAction::onDirectDecl_ParamDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeSpecifierSeq *cvSeq = astStack.popas<TypeSpecifierSeq*>();// opt
	astStack.pop();// )
	ParameterDeclarationList *paramDecls = astStack.popas<ParameterDeclarationList*>();
	astStack.pop();// (

	astStack.push(new FunctionDeclarator(paramDecls, cvSeq));
}

//[ [ constant-expression ] ] direct-declarator'
void
SCParserAction::onDirectDecl_ArrayDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *nextDecl = astStack.popas<Declarator*>();// opt
	astStack.pop();// ]
	ASTExp *constExp = astStack.popas<ASTExp*>();
	astStack.pop();// [

	astStack.push(new ArrayDeclarator(constExp, nextDecl));
}

//* [ cv-qualifier-seq ]
void
SCParserAction::onPtrOp_Ptr(const ref<TokenValue> &v) 
{
	TypeSpecifierSeq *cvSpecSeq = astStack.popas<TypeSpecifierSeq*>();// opt
	astStack.pop();// *
		   
	astStack.push(new PtrOperator(PtrOperator::POINTER, cvSpecSeq, NULL));
}

// &
void
SCParserAction::onPtrOp_Reference(const ref<TokenValue> &v) 
{
	astStack.pop();// &
	astStack.push(new PtrOperator(PtrOperator::REFERENCE, NULL, NULL));		
}

// [ :: ] [ nested-name-specifier ] * [ cv-qualifier-seq ]
void
SCParserAction::onPtrOp_ScopedPtr(const ref<TokenValue> &v) 
{
	TypeSpecifierSeq *cvSpecSeq = astStack.popas<TypeSpecifierSeq*>();// opt
	astStack.pop();// *
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	astStack.push(new PtrOperator(PtrOperator::POINTER, cvSpecSeq, scope));
}

//cv-qualifier [ cv-qualifier-seq ]
void
SCParserAction::onCVQualSeq(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeSpecifierSeq *seq   = astStack.popas<TypeSpecifierSeq*>();//opt
	TypeSpecifier *typeSpec = astStack.popas<TypeSpecifier*>();
	if (seq == NULL) {
		seq = new TypeSpecifierSeq();
	}
	seq->typeSpecs.push_front(typeSpec);
	astStack.push(seq);
}

void
SCParserAction::onCVQual(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(new TypeSpecifier(static_cast<TypeSpecifierID>(v->toInt())));
}

//id-expression
void
SCParserAction::onDeclId_IdExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//[ :: ] [ nested-name-specifier ] type-name
void
SCParserAction::onDeclId_Typename(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//do nothing
}

//type-specifier-seq [ abstract-declarator ]
void
SCParserAction::onTypeId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *absDecl = astStack.popas<Declarator*>(); // opt
	TypeSpecifierSeq *typeSpecSeq = astStack.popas<TypeSpecifierSeq*>();
	astStack.push(new TypeId(typeSpecSeq, absDecl));
}

//type-specifier [type-specifier-seq ]
void
SCParserAction::onTypeSpecSeq(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeSpecifierSeq *seq = astStack.popas<TypeSpecifierSeq*>();// opt
	TypeSpecifier *typeSpec = astStack.popas<TypeSpecifier*>();
	if (!seq) {
		seq = new TypeSpecifierSeq();
	}
	seq->typeSpecs.push_front(typeSpec);
	astStack.push(seq);
}

//parameter-declaration parameter-declaration-list'
void
SCParserAction::onParamDeclListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ParameterDeclarationList *list = astStack.popas<ParameterDeclarationList*>();// opt
	ParameterDeclaration *paramDecl = astStack.popas<ParameterDeclaration*>();
	if (list == NULL) {
		list = new ParameterDeclarationList(false);
	}
	list->paramDecls.push_front(paramDecl);
	astStack.push(list);
}

//...
void
SCParserAction::onParamDeclListHead_Elipsis(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ...

	astStack.push(new ParameterDeclarationList(true));
}

//, parameter-declaration parameter-declaration-list'
void
SCParserAction::onParamDeclListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ParameterDeclarationList *list = astStack.popas<ParameterDeclarationList*>();// opt
	ParameterDeclaration *paramDecl = astStack.popas<ParameterDeclaration*>();
	astStack.pop(); //, 
	if (list == NULL) {
		list = new ParameterDeclarationList(false);
	}
	list->paramDecls.push_front(paramDecl);
	astStack.push(list);
}

//, ...
void
SCParserAction::onParamDeclListRest_Elipsis(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ...
	astStack.pop(); //, 

	astStack.push(new ParameterDeclarationList(true));
}

//decl-specifier-seq declarator [= assignment-expression]
//decl-specifier-seq [ abstract-declarator ] [= assignment-expression]
void
SCParserAction::onParamDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *assignExp = astStack.popas<ASTExp*>();// opt
	Declarator *decl  = astStack.popas<Declarator*>();// opt
	DeclSpecifierSeq *seq = astStack.popas<DeclSpecifierSeq*>();

	astStack.push(new ParameterDeclaration(seq, decl, assignExp));
}

//= assignment-expression
void
SCParserAction::onParamDeclAssign(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	astStack.pop();// =
	astStack.push(exp);
}

//[ decl-specifier-seq ] declarator [ ctor-initializer ] function_body
void
SCParserAction::onFuncDef(const ref<TokenValue> &v) 
{
	FUNCLOG;
	FunctionBody *body = astStack.popas<FunctionBody*>();
	MemInitializerList *memInits = astStack.popas<MemInitializerList*>();// opt
	Declarator *decl = astStack.popas<Declarator*>();
	DeclSpecifierSeq *declSpecSeq = astStack.popas<DeclSpecifierSeq*>();// opt
		
	astStack.push(new FunctionDefinition(declSpecSeq, decl, memInits, body));
}

//compound-statement
void
SCParserAction::onFuncBody(const ref<TokenValue> &v) 
{
	FUNCLOG;
	CompoundStatement *stm = astStack.popas<CompoundStatement*>();
	astStack.push(new FunctionBody(stm));
}

//= initializer-clause
void
SCParserAction::onInitializer(const ref<TokenValue> &v) 
{
	FUNCLOG;
	InitializerClause *init = astStack.popas<InitializerClause*>();
	astStack.pop();// =

	astStack.push(new Initializer(init));
}

//( expression-list )
void
SCParserAction::onInitializer_Parenthesis(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// )
	SequenceExpression *exps = astStack.popas<SequenceExpression*>();
	astStack.pop();// (
		
	astStack.push(new Initializer(exps));
}

//assignment-expression
void
SCParserAction::onInitializerClause_AssignExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *assignExp = astStack.popas<ASTExp*>();
	astStack.push(new InitializerClause(assignExp));
}

//{ initializer-list [ , ] }
void
SCParserAction::onInitializerClause_InitializerList(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// }
	astStack.pop();// , opt
	InitializerList *list = astStack.popas<InitializerList*>();
	astStack.pop();// {
	
	astStack.push(new InitializerClause(list));
}

//{ }
void
SCParserAction::onInitializerClause_EmptyBraces(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// }
	astStack.pop();// {

	astStack.push(new InitializerClause(new InitializerList()));
}

//initializer-clause initializer-list'
void
SCParserAction::onInitializerListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	InitializerList *rest = astStack.popas<InitializerList*>();
	InitializerClause *clause = astStack.popas<InitializerClause*>();

	if (rest == NULL) {
		rest = new InitializerList();
	}
	rest->clauses.push_front(clause);
	astStack.push(rest);
}

//, initializer-clause initializer-list'
void
SCParserAction::onInitializerListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	InitializerList *rest = astStack.popas<InitializerList*>();
	InitializerClause *clause = astStack.popas<InitializerClause*>();
	astStack.pop();// , 

	if (rest == NULL) {
		rest = new InitializerList();
	}
	rest->clauses.push_front(clause);
	astStack.push(rest);
}

//class-head { [ member-specification ] }
void
SCParserAction::onClassSpecifier(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// }
	MemberSpecification *members = astStack.popas<MemberSpecification*>();// opt
	astStack.pop();// {
	ClassSpecifier *classSpec = astStack.popas<ClassSpecifier*>();
		
	classSpec->members = members;
	astStack.push(classSpec);
}

//class-key [ identifier ] [ base_clause ]
void
SCParserAction::onClassHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *base = astStack.popas<BaseClause*>();// opt
	Symbol *name = astStack.popas<Symbol*>();// opt
	TypeSpecifierID type = astStack.popi<TypeSpecifierID>();
		
	bool clazz = (type == CLASS) ? true : false;
	astStack.push(new ClassSpecifier(new TypeName(name), NULL, base, clazz));
}

//class-key template-id [ base-clause ]
void
SCParserAction::onClassHead_Template(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *base     = astStack.popas<BaseClause*>();// opt
	TemplateId *tempId   = astStack.popas<TemplateId*>();
	TypeSpecifierID type = astStack.popi<TypeSpecifierID>();
		
	bool clazz = (type == CLASS) ? true : false;
	astStack.push(new ClassSpecifier(new TypeName(tempId), NULL, base, clazz));
}

//class-key nested-name-specifier identifier [ base-clause ]
void
SCParserAction::onClassHead_NestedId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *base     = astStack.popas<BaseClause*>();// opt
	Symbol *name = astStack.popas<Symbol*>();
	Scope *scope = astStack.popas<Scope*>();
	TypeSpecifierID type = astStack.popi<TypeSpecifierID>();
		
	bool clazz = (type == CLASS) ? true : false;
	astStack.push(new ClassSpecifier(new TypeName(name), scope, base, clazz));
}

//class-key nested-name-specifier template-id [ base-clause ]
void
SCParserAction::onClassHead_NestedTemplate(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *base     = astStack.popas<BaseClause*>();// opt
	TemplateId *tempId   = astStack.popas<TemplateId*>();
	Scope *scope = astStack.popas<Scope*>();
	TypeSpecifierID type = astStack.popi<TypeSpecifierID>();
		
	bool clazz = (type == CLASS) ? true : false;
	astStack.push(new ClassSpecifier(new TypeName(tempId), scope, base, clazz));
}

//class | struct
void
SCParserAction::onClassKey(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pushi(v->toInt());
}

//member-declaration [ member-specification ]
void
SCParserAction::onMemberSpec_MemberDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemberSpecification *members = astStack.popas<MemberSpecification*>();//opt
	MemDecl *memDecl = astStack.popas<MemDecl*>();

	if (members == NULL) {
		members = new MemberSpecification();
	}
	members->memDecls.push_front(memDecl);
	astStack.push(members);
}

//access-specifier : [ member-specification ]
void
SCParserAction::onMemberSpec_AccessSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemberSpecification *members = astStack.popas<MemberSpecification*>();//opt
	astStack.pop();// :
	AccessSpecifierID access = astStack.popi<AccessSpecifierID>();

	if (members == NULL) {
		members = new MemberSpecification();
	}
	//change access specification until settled
	assert((int)access <= 3);
	PARSE_DBG("%s", AccessSpecifierString[access]);
	members->setAccessSpec(access);
	astStack.push(members);
}

//function-definition [ ; ]
void
SCParserAction::onMemberDecl_FuncDef(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ; opt
	FunctionDefinition *funcDef = astStack.popas<FunctionDefinition*>();

	astStack.push(new MemberFunctionDefinition(funcDef));
}

//[ :: ] nested-name-specifier [ template ] unqualified-id ;
void
SCParserAction::onMemberDecl_UnqualifiedId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//TODO:
}

//using-declaration
void
SCParserAction::onMemberDecl_UsingDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	//TODO:
}

void 
SCParserAction::onMemberDecl_TemplateDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateDeclaration *tempDecl = astStack.popas<TemplateDeclaration*>();
	
	astStack.push(new MemberTemplateDeclaration(tempDecl));
}

//[ decl-specifier-seq ] [ member-declarator-list ] ;
void
SCParserAction::onMemberDecl_MemberDeclList(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// ;
	MemberDeclaration *memDecl = astStack.popas<MemberDeclaration*>();// opt
	DeclSpecifierSeq *declSpecSeq = astStack.popas<DeclSpecifierSeq*>();// opt
		
	if (memDecl == NULL) {
		memDecl = new MemberDeclaration();
	}
	memDecl->declSpecSeq = declSpecSeq;
	astStack.push(memDecl);
}

//member-declarator member-declarator-list'
void
SCParserAction::onMemberDeclListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemberDeclaration *declaration = astStack.popas<MemberDeclaration*>();// opt
	MemberDeclarator  *declarator = astStack.popas<MemberDeclarator*>();
	if (declaration == NULL) {
		declaration = new MemberDeclaration();
	}
	declaration->memDecls.push_front(declarator);
	astStack.push(declaration);
}

//, member-declarator member-declarator-list'
void
SCParserAction::onMemberDeclListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemberDeclaration *declaration = astStack.popas<MemberDeclaration*>();// opt
	MemberDeclarator  *declarator = astStack.popas<MemberDeclarator*>();
	astStack.pop();// ,

	if (declaration == NULL) {
		declaration = new MemberDeclaration();
	}
	declaration->memDecls.push_front(declarator);
	astStack.push(declaration);
}

//declarator [ constant-initializer ]
void
SCParserAction::onMemberDeclarator(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declarator *decl = astStack.popas<Declarator*>();
	astStack.push(new MemberBasicDeclarator(decl, false));
}

//declarator [ pure-specifier ]
void
SCParserAction::onMemberDeclarator_PureSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	bool pure = astStack.popi<bool>();
	Declarator *decl = astStack.popas<Declarator*>();

	astStack.push(new MemberBasicDeclarator(decl, pure));
}

//declarator [ constant-initializer ]
void
SCParserAction::onMemberDeclarator_ConstantInit(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *constExp = astStack.popas<ASTExp*>();// opt
	Declarator *decl = astStack.popas<Declarator*>();

	//pure virtual specifier '= 0' check
	if (decl->isFunction()) {
		if (constExp->op == PRIMARY_INT) {
			PrimaryExpression *e = (PrimaryExpression*)constExp;
			if (e->v.i == 0) {
				astStack.push(new MemberBasicDeclarator(decl, true));
				return;
			}
		}
	}
	astStack.push(new MemberConstDeclarator(decl, constExp));
}

//[ identifier ] : constant-expression
void
SCParserAction::onMemberDeclarator_BitField(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *constExp = astStack.popas<ASTExp*>();
	astStack.pop();// :
	Symbol *name = astStack.popas<Symbol*>();// opt

	astStack.push(new MemberBitField(name, constExp));
}

//= 0
void
SCParserAction::onPureSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// 0
	astStack.pop();// =
	astStack.pushi(true);
}

//= constant-expression
void
SCParserAction::onConstantInit(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *constExp = astStack.popas<ASTExp*>();
	astStack.pop();// = 
	astStack.push(constExp);
}

//: base-specifier-list
void
SCParserAction::onBaseClause(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *baseClause = astStack.popas<BaseClause*>();
	astStack.pop();// :
		
	astStack.push(baseClause);
}

//base-specifier base-specifier-list'
void
SCParserAction::onBaseSpecListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *baseClause = astStack.popas<BaseClause*>();// opt
	BaseSpecifier *baseSpec = astStack.popas<BaseSpecifier*>();

	if (baseClause == NULL) {
		baseClause = new BaseClause();
	}
	baseClause->baseSpecs.push_front(baseSpec);
	astStack.push(baseClause);
}

//, base-specifier base-specifier-list'
void
SCParserAction::onBaseSpecListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	BaseClause *baseClause = astStack.popas<BaseClause*>();// opt
	BaseSpecifier *baseSpec = astStack.popas<BaseSpecifier*>();
	astStack.pop();// ,

	if (baseClause == NULL) {
		baseClause = new BaseClause();
	}
	baseClause->baseSpecs.push_front(baseSpec);
	astStack.push(baseClause);
}

//virtual [ access-specifier ] [ :: ] [ nested-name-specifier ] class-name
void
SCParserAction::onBaseSpec_Virtual(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeName *name = astStack.popas<TypeName*>();
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	AccessSpecifierID accessId = astStack.popi<AccessSpecifierID>();// opt
	astStack.pop();// virtual
		
	astStack.push(new BaseSpecifier(name, scope, accessId, true));
}

//access-specifier [ virtual ] [ :: ] [ nested-name-specifier ] class-name
void
SCParserAction::onBaseSpec_AccessSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeName *name = astStack.popas<TypeName*>();
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	Symbol *kw_virtual = astStack.popas<Symbol*>();
	AccessSpecifierID accessId = astStack.popi<AccessSpecifierID>();

	astStack.push(new BaseSpecifier(name, scope, accessId, kw_virtual!=NULL ?true:false));
}

//[ :: ] [ nested-name-specifier ] class-name
void
SCParserAction::onBaseSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeName *name = astStack.popas<TypeName*>();
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt
	
	astStack.push(new BaseSpecifier(name, scope, DEFAULT_ACCESS_SPEC, false));
}

//private | protected | public
void
SCParserAction::onAccessSpec(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pushi(v->toInt());
}

//operator conversion-type-id
void
SCParserAction::onConversionFuncId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ConversionTypeId *conv = astStack.popas<ConversionTypeId*>();
	astStack.pop();//operator
	astStack.push(conv);
}

//type-specifier-seq [ conversion-declarator ]
void
SCParserAction::onConversionTypeId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ConversionTypeId *conv = astStack.popas<ConversionTypeId*>();// opt
	TypeSpecifierSeq *typeSpecSeq = astStack.popas<TypeSpecifierSeq*>();

	if (conv == NULL) {
		conv = new ConversionTypeId();
	}
	conv->typeSpecSeq = typeSpecSeq;
	astStack.push(conv);
}

//ptr-operator [ conversion-declarator ]
void
SCParserAction::onConversionDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ConversionTypeId *conv = astStack.popas<ConversionTypeId*>();// opt
	PtrOperator *ptr = astStack.popas<PtrOperator*>();

	if (conv == NULL) {
		conv = new ConversionTypeId();
	}
	conv->ptrs.push_front(ptr);
	astStack.push(conv);
}

//: mem-initializer-list
void
SCParserAction::onCtorInit(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemInitializerList *memInitList = astStack.popas<MemInitializerList*>();// opt
	astStack.pop();// :

	astStack.push(memInitList);
}

//mem-initializer mem-initializer-list'
void
SCParserAction::onMemInitListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemInitializerList *memInitList = astStack.popas<MemInitializerList*>();// opt
	MemInitializer *memInit         = astStack.popas<MemInitializer*>();

	if (memInitList == NULL) {
		memInitList = new MemInitializerList();
	}
	memInitList->memInits.push_front(memInit);
	astStack.push(memInitList);
}

//, mem-initializer mem-initializer-list'
void
SCParserAction::onMemInitListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	MemInitializerList *memInitList = astStack.popas<MemInitializerList*>();// opt
	MemInitializer *memInit         = astStack.popas<MemInitializer*>();
	astStack.pop();//,

	if (memInitList == NULL) {
		memInitList = new MemInitializerList();
	}
	memInitList->memInits.push_front(memInit);
	astStack.push(memInitList);
}

//mem-initializer-id ( [ expression-list ] )
void
SCParserAction::onMemInit(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// )
	ASTExp *exp = astStack.popas<ASTExp*>();// opt
	astStack.pop();// (
	MemInitializer *memInit = astStack.popas<MemInitializer*>();

	memInit->exp = exp;
	astStack.push(memInit);
}

//identifier
void
SCParserAction::onMemInitId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();
	//FIXME: this is variable name (not type-name...)
	astStack.push(new MemInitializer(new TypeName(name), NULL));
}

//[ :: ] [ nested-name-specifier ] class-name
void
SCParserAction::onMemInitId_NestedClassName(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeName *name = astStack.popas<TypeName*>();
	Scope *scope = astStack.popas<Scope*>();// opt
	astStack.pop();// :: opt

	astStack.push(new MemInitializer(name, scope));
}

//operator operator_id
void
SCParserAction::onOperatorFuncId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Operator op = astStack.popop();
	astStack.pop();// operator

	astStack.push(new OperatorFunctionId(op));
}

void
SCParserAction::onOperatorId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.push(v->toOp());
}

//[ export ] template < template-parameter-list > declaration
void
SCParserAction::onTemplateDecl(const ref<TokenValue> &v) 
{
	FUNCLOG;
	Declaration *decl = astStack.popas<Declaration*>();
	astStack.pop();// >
	TemplateParameterList *params = astStack.popas<TemplateParameterList*>();
	astStack.pop();// <
	astStack.pop();// template
	bool expo = astStack.pop() ? true : false;

	astStack.push(new TemplateDeclaration(params, decl, expo));
}

//template-parameter template-parameter-list'
void
SCParserAction::onTemplateParamListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateParameterList *params = astStack.popas<TemplateParameterList*>();// opt
	TemplateParameter *param = astStack.popas<TemplateParameter*>();

	if (params == NULL) {
		params = new TemplateParameterList();
	}
	params->params.push_front(param);
	astStack.push(params);
}

//, template-parameter template-parameter-list'
void
SCParserAction::onTemplateParamListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateParameterList *params = astStack.popas<TemplateParameterList*>();// opt
	TemplateParameter *param = astStack.popas<TemplateParameter*>();
	astStack.pop();// ,

	if (params == NULL) {
		params = new TemplateParameterList();
	}
	params->params.push_front(param);
	astStack.push(params);
}

void 
SCParserAction::onTemplateParam_TypeParam(const ref<TokenValue> &v)
{
	FUNCLOG;
	TypeParameter *tparam = astStack.popas<TypeParameter*>();

	astStack.push(new TemplateParameter(tparam));
}

void
SCParserAction::onTemplateParam_ParamDecl(const ref<TokenValue> &v)
{
	FUNCLOG;
	ParameterDeclaration *pdecl = astStack.popas<ParameterDeclaration*>();
	
	astStack.push(new TemplateParameter(pdecl));
}

//class|typename [identifier]
void
SCParserAction::onTypeParam_Ident(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();//class|typename

	astStack.push(new TypeParameter(name, NULL));
}

//class|typename [identifier] = type-id
void
SCParserAction::onTypeParam_IdentAssign(const ref<TokenValue> &v)
{
	FUNCLOG;
	TypeId *tid = astStack.popas<TypeId*>();
	astStack.pop();// = 
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();//class|typename

	astStack.push(new TypeParameter(name, tid));
}

//template < template-parameter-list > class [identifier ]
void
SCParserAction::onTypeParam_TemplateClass(const ref<TokenValue> &v)
{
	FUNCLOG;
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();//class
	astStack.pop();// >
	TemplateParameterList *params = astStack.popas<TemplateParameterList*>();
	astStack.pop();// <
	astStack.pop();// template

	astStack.push(new TypeParameter(name, params, NULL));
}

//template < template-parameter-list > class [identifier ] = id-expression
void
SCParserAction::onTypeParam_TemplateClassAssign(const ref<TokenValue> &v)
{
	FUNCLOG;
	ScopedId *scoped = astStack.popas<ScopedId*>();
	astStack.pop();// =
	Symbol *name = astStack.popas<Symbol*>();// opt
	astStack.pop();//class
	astStack.pop();// >
	TemplateParameterList *params = astStack.popas<TemplateParameterList*>();
	astStack.pop();// <
	astStack.pop();// template

	astStack.push(new TypeParameter(name, params, scoped));
}

//identifer < [ template-argument-list ] >
void
SCParserAction::onTemplateId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	astStack.pop();// >
	TemplateArgumentList *args = astStack.popas<TemplateArgumentList*>();// opt
	astStack.pop();// <
	Symbol *name = astStack.popas<Symbol*>();

	astStack.push(new TemplateId(name, args));
}

//template-argument template-argument-list'
void
SCParserAction::onTemplateArgListHead(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateArgumentList *args = astStack.popas<TemplateArgumentList*>();// opt
	TemplateArgument *arg = astStack.popas<TemplateArgument*>();

	if (args == NULL) {
		args = new TemplateArgumentList();
	}
	args->args.push_front(arg);
	astStack.push(args);
}

//, template-argument template-argument-list'
void
SCParserAction::onTemplateArgListRest(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TemplateArgumentList *args = astStack.popas<TemplateArgumentList*>();// opt
	TemplateArgument *arg = astStack.popas<TemplateArgument*>();
	astStack.pop();// ,

	if (args == NULL) {
		args = new TemplateArgumentList();
	}
	args->args.push_front(arg);
	astStack.push(args);
}

void
SCParserAction::onTemplateArg_AssignExp(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ASTExp *exp = astStack.popas<ASTExp*>();
	
	astStack.push(new TemplateArgument(exp));
}

void
SCParserAction::onTemplateArg_TypeId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	TypeId *tid = astStack.popas<TypeId*>();

	astStack.push(new TemplateArgument(tid));
}

void
SCParserAction::onTemplateArg_ScopedId(const ref<TokenValue> &v) 
{
	FUNCLOG;
	ScopedId *scoped = astStack.popas<ScopedId*>();

	astStack.push(new TemplateArgument(scoped));
}
	
void 
SCParserAction::onExplicitInst(const ref<TokenValue> &v)
{
	FUNCLOG;
	//TODO:
}

void 
SCParserAction::onExplicitSpecial(const ref<TokenValue> &v)
{
	FUNCLOG;
	//TODO:
}
