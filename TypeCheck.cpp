#include "TypeCheck.hpp"
#include "AST.hpp"
#include "Reporter.hpp"
#include <cassert>
#include <cstdio>
#include <setjmp.h>
#include "error.hpp"
#define ENABLE_FUNCLOG
#include "debug.h"
#include "TypeCreator.hpp"
#include "SymbolTable.hpp"

jmp_buf env;

#define SEMANTIC_ERROR(err)												\
	do {																\
		if (Reporter.getLastError() == NO_ERROR) {						\
			printf("type check error [line %d] %s\n", __LINE__, #err);	\
			Reporter.error(err);										\
			longjmp(env, 1);											\
			/*assert(0);*/												\
		}																\
		return NULL;													\
	}while (false)


bool isAssignOp(Operator op)
{
	return (ASSIGN <= op && op <= NOT_ASSIGN);
}


TypeCheck::TypeCheck()
	: evaluator(NULL)
	, currentFunction(NULL)
{
	FUNCLOG;
	SymbolTable.init();
}

int 
TypeCheck::getLastError()
{
	FUNCLOG;
	return Reporter.getLastError();
}

Type *
TypeCheck::makePointerType(List<PtrOperator*> &ptrs, Type *reftype)
{
	List<PtrOperator*>::iterator it = ptrs.begin();
	while (it != ptrs.end()) {
		PtrOperator *ptrOp = *it;
		Type *t = ptrOp->accept(this);
		assert(t->is(Type::POINTER_T));
		PointerType *ptrType = t->as<PointerType>();
		ptrType->reference = reftype;
		reftype = ptrType;
		++it;
	}
	return reftype;
}

Type *
TypeCheck::visit(TranslationUnit *ast)
{
	FUNCLOG;
	if (setjmp(env) == 0) {
		assert(ast->seq);
		return ast->seq->accept(this);
	} else {
		return NULL;
	}
}

Type *
TypeCheck::visit(ScopeName *ast)
{
	FUNCLOG;
	if (ast->isTemplateId()) {
		Type * t = ast->templateId->accept(this);
		return t;
	} else {
		//Type * t = new NamedType
	}
	return NULL;
}

Type *
TypeCheck::visit(Scope *ast)
{
	FUNCLOG;
	return NULL;
}

Type *
TypeCheck::visit(ASTIdent *ast)
{
	FUNCLOG;
	Type *t = SymbolTable.findVar(ast->ident);
	return t;
}

Type *
TypeCheck::visit(PrimaryExpression *ast)
{
	FUNCLOG;
	//Type * t;
	switch (ast->op) {
	case PRIMARY_ID: {
		ScopedId *scopedId = ast->v.id;
		if (scopedId->scope) {
			//TODO:
		}
		UnqualifiedId *unqualifiedId = scopedId->id;
		String s = unqualifiedId->toSource();
		Type *t = SymbolTable.findVar(Symbol::symbol(s));
		if (!t) {
			SEMANTIC_ERROR(SE_UNDEFINED_SYMBOL);
		}
		return t;
	}
	case PRIMARY_INT: {
		IntType *itype = new IntType(32, true);
		if (ast->v.i == 0) {
			itype->setNullable(true);
		}
		return itype;
	}
	case PRIMARY_BOOL: return new IntType(8, false);
	case PRIMARY_STR: {
		PointerType *ptrType = new PointerType();
		ptrType->reference = new IntType(8, true);
		DBG("$$ %s", ptrType->toString().c_str());
		return ptrType;
	}
	case PRIMARY_EXP:  return ast->v.exp->accept(this);
	default:
		assert(0);
		break;
	}
	return NULL;
}

Type *
TypeCheck::visit(UnqualifiedId *ast)
{
	FUNCLOG;
	return ast->id->accept(this);
}

Type *
TypeCheck::visit(ScopedId *ast)
{
	FUNCLOG;
	if (ast->scope) {
		//TODO:
	}
	return ast->id->accept(this);
}

Type *
TypeCheck::visit(SequenceExpression *ast)
{
	FUNCLOG;
	Type *t = NULL;
	List<ASTExp*>::iterator it =  ast->exps.begin();
	while (it != ast->exps.end()) {
		ASTExp* exp = *it;
		t = exp->accept(this);
		++it;
	}
	return t;
}

Type *
TypeCheck::visit(UnaryExpression *ast)
{
	FUNCLOG;
	Type *t = ast->e->accept(this);
	t = t->actual();
	DBG("Unary before %s", t->toString().c_str());

	//TODO: operator overload check
	switch (ast->op) {
	case PRE_INC:
	case PRE_DEC:
	case BANG:
	case TILDE:
	case POST_INC:
	case POST_DEC:
		if (t->is(Type::CLASS_T)) {
		} else if (t->is(Type::INT_T)) {
		} else if (t->is(Type::POINTER_T)) {
		} else {
			SEMANTIC_ERROR(SE_UNARY_OP_NOT_ALLOWED);
		}
		break;
	case UNARY_STAR:
		if (t->is(Type::CLASS_T)) {
			assert(0);//TODO
		} else if (t->is(Type::POINTER_T)) {
			PointerType *ptr = t->as<PointerType>();
			t = ptr->reference;
		} else {
			SEMANTIC_ERROR(SE_UNARY_STAR);
		}
		break;
	case UNARY_AND: 
		if (t->is(Type::CLASS_T)) {
			assert(0);//TODO
		} else {
			PointerType *ptrType = new PointerType();
			ptrType->reference = t;
			t = ptrType;
		}
		break;
	case UNARY_PLUS:
	case UNARY_MINUS:
		if (!t->is(Type::INT_T)) {
			SEMANTIC_ERROR(SE_UNARY_SIGN);
		}
		break;
	default:
		assert(0);
		break;
	}

	DBG("Unary after %s", t->toString().c_str());
	return t;
}


Type *
TypeCheck::visit(BinaryExpression *ast)
{
	FUNCLOG;
	Type *a = ast->l->accept(this);
	Type *b = ast->r->accept(this);
	a = a->actual();
	b = b->actual();
	assert(a);
	assert(b);

	ClassType *classType = a->as<ClassType>();
	if (classType) {
		//TODO: operator overload check
		return a;
	}

	if (!a->coerceTo(b)) {
		//can't convert a and b
		SEMANTIC_ERROR(SE_CAN_NOT_CONVERT);
	}

	if (isAssignOp(ast->op) && a->isReadonly()) {
		//assignment of read-only variable
		SEMANTIC_ERROR(SE_ASSIGN_READONLY);
	}

	IntType *intType = a->as<IntType>();
	if (intType) {
	}

	FuncType *funcType = a->as<FuncType>();
	if (funcType) {
	}

	PointerType *ptrType = a->as<PointerType>();
	if (ptrType) {
	}
	return a;
}

Type *
TypeCheck::visit(ConditionalExpression *ast)
{
	FUNCLOG;
	Type *cond = ast->cond->accept(this);
	
	Type *l = ast->l->accept(this);
	Type *r = ast->r->accept(this);
	if (l->coerceTo(r)) {
		SEMANTIC_ERROR(SE_COND_EXP_DIFF_TYPE);
	}
	return l;
}

Type *
TypeCheck::visit(Subscript *ast)
{
	FUNCLOG;
	Type *t = ast->exp->accept(this);

	Type *indext = ast->index->accept(this);
	indext = indext->actual();
	//TODO: operator[] overload check
	if (!indext->is(Type::INT_T)) {
		SEMANTIC_ERROR(SE_ARRAY_INDEX_NOT_VALID);
	}

	if (t->is(Type::ARRAY_T)) {
		ArrayType *arrayType = t->as<ArrayType>();
		evaluator.init();
		const int index = ast->index->accept(&evaluator);
		if (evaluator.isConstant()) {
			if (index < 0 || arrayType->size <= index) {
				//WARNING?
				SEMANTIC_ERROR(SE_ARRAY_INDEX_RANGE_NOT_VALID);
			}
		}
		return arrayType->element;
	} else if (t->is(Type::POINTER_T)) {
		PointerType *ptrType = t->as<PointerType>();
		return ptrType->reference;
	} else {
		SEMANTIC_ERROR(SE_ERROR);
	}
}

Type *
TypeCheck::visit(MemberSelect *ast)
{
	FUNCLOG;
	Type *member = NULL;
	Type *t = ast->receiver->accept(this);
	if (t->actual()->is(Type::CLASS_T)) {
		ClassType *receiver = t->actual()->as<ClassType>();

		member = receiver->findMember(Symbol::symbol(ast->member->toSource()));
		//member = ast->member->accept(this);
		if (member == NULL) {
			SEMANTIC_ERROR(SE_ERROR);
		}
		DBG("member %s", member->toString().c_str());

	} else {
		assert(0);
		//TODO
	}
	
	if (member->is(Type::FIELD_T)) {
		return member->actual();
	} else {
		return member;
	}
}

Type *
TypeCheck::visit(FunctionCall *ast)
{
	FUNCLOG;
	Type *ft = ast->func->accept(this);
	assert(ft);
	DBG("$$$ %s", ft->toString().c_str());
	assert(ft->is(Type::FUNC_T) || ft->is(Type::METHOD_T));
	FuncType *funcType = ft->as<FuncType>();
	if (ast->args) {
		SequenceExpression *seq = ast->args;
		//DBG("funcType->args.size() %d", funcType->params.size());

		//check the number of argument
		{
			bool found_match_size = false;
			FuncType *ft = funcType;
			//loop for all overloaded functions
			while (ft) {
				if (ft->args.list.size() == seq->exps.size()) {
					found_match_size = true;
					break;
				}
				ft = ft->nextOverload;
			}
			if (!found_match_size) {
				SEMANTIC_ERROR(SE_NUMBER_OF_FUNC_PARAMS_NOT_VALID);
			}
		}

		//check the type of argument
		{
			bool found_match_type = false;
			FuncType *ft = funcType;
			//loop for all overloaded functions
			while (ft) {
				bool all_arg_type_match = true;
				List<Type*>::iterator iarg = ft->args.list.begin();
				List<ASTExp*>::iterator iexp = seq->exps.begin();
				while (iexp != seq->exps.end()) {
					ASTExp* exp = *iexp;
					Type *argt = *iarg;
					Type *t = exp->accept(this);
					if (!argt->coerceTo(t)) {
						all_arg_type_match = false;
						break;
					}
					++iarg;
					++iexp;
				}
				if (all_arg_type_match) {
					found_match_type = true;
					break;
				}
				ft = ft->nextOverload;
			}
			if (!found_match_type) {
				SEMANTIC_ERROR(SE_FUNC_PARAM_TYPE_NOT_VALID);
			}
		}
	} else {
		if (!funcType->args.list.empty()) {
			SEMANTIC_ERROR(SE_NUMBER_OF_FUNC_PARAMS_NOT_VALID);
		}
	}

	return funcType->actual();
}

Type *
TypeCheck::visit(Constructor *ast)
{
	FUNCLOG;
	//TODO:

	//ast->typeSpec->typeSpecId;
	if (ast->args) {
		ast->args->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(CastExpression *ast)
{
	FUNCLOG;
	ast->typeId->accept(this);
	ast->exp->accept(this);
	return NULL;
}


Type *
TypeCheck::visit(Condition *ast)
{
	FUNCLOG;
	if (ast->typeSpec) {
		Type *t = ast->typeSpec->accept(this);
		ast->decl->type = t;
		ast->decl->accept(this);
	} 
	Type *t = ast->exp->accept(this);
	if (!t->is(Type::INT_T)) {
		//SEMANTIC_ERROR(0);
	}

	return t;
}
	
Type *
TypeCheck::visit(CompoundStatement *ast)
{
	FUNCLOG;
	List<ASTStm*>::iterator it = ast->stms.begin();
	while (it != ast->stms.end()) {
		ASTStm *stm = *it;
		stm->accept(this);
		++it;
	}

	return NULL;
}

Type *
TypeCheck::visit(DeclarationStatement *ast)
{
	FUNCLOG;
	ast->decl->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(ExpressionStatement *ast)
{
	FUNCLOG;
	if (ast->exp) {
		ast->exp->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(IfStatement *ast)
{
	FUNCLOG;
	ast->cond->accept(this);
	ast->thens->accept(this);
	if (ast->elses) {
		ast->elses->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(SwitchStatement *ast)
{
	FUNCLOG;
	Type *t = ast->cond->accept(this);
	if (!t->coerceTo(new IntType(32, true))) {
		SEMANTIC_ERROR(SE_SWITCH_COND_MUSTBE_INT);
	}
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(CaseStatement *ast)
{
	FUNCLOG;
	if (ast->exp) {
		evaluator.init();
		int x = ast->exp->accept(&evaluator);
		if (!evaluator.isConstant()) {
			SEMANTIC_ERROR(SE_EXPECTED_CONST_EXP);
		}
		Type *t = ast->exp->accept(this);
		if (!t->coerceTo(new IntType(32, true))) {
			SEMANTIC_ERROR(SE_CAN_NOT_CONVERT);
		}
	} else {
		//TODO: default case dup check
	}
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(WhileStatement *ast)
{
	FUNCLOG;
	ast->cond->accept(this);
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(DoStatement *ast)
{
	FUNCLOG;
	ast->exp->accept(this);
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(ForStatement *ast)
{
	FUNCLOG;
	ast->init->accept(this);
	if (ast->cond) {
		ast->cond->accept(this);
	}
	if (ast->exp) {
		ast->exp->accept(this);
	}
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(ForInitStatement *ast)
{
	FUNCLOG;
	if (ast->expStm) {
		ast->expStm->accept(this);
	} else if (ast->simpleDecl) {
		ast->simpleDecl->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(JumpStatement *ast)
{
	FUNCLOG;
	switch (ast->jumpType) {
	case CONTINUE:
	case BREAK:
		break;
	case RETURN:
		assert(currentFunction);
		if (ast->ret) {
			if (currentFunction->type->is(Type::VOID_T)) {
				SEMANTIC_ERROR(SE_RETURN_VALUE_IN_VOID_FUNC);	
			}
			ast->ret->accept(this);
		}
		break;
	case GOTO:
		//check when leaving function
		gotoLabels.push_back(ast->label);
		break;
	default:
		break;
	}
	return NULL;
}

Type *
TypeCheck::visit(LabelStatement *ast)
{
	FUNCLOG;
	if (!SymbolTable.getCurrentFunctionScope()->addLabel(ast->label)) {
		SEMANTIC_ERROR(SE_DUPLICATE_LABEL);
	}
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(DeclarationSeq *ast)
{
	FUNCLOG;
	List<Declaration*>::iterator it = ast->decls.begin();
	while (it != ast->decls.end()) {
		Declaration *decl = *it;
		decl->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(SimpleDeclaration *ast)
{
	FUNCLOG;
	Type *declType = NULL;
	if (ast->declSpecSeq) {
		declType = ast->declSpecSeq->accept(this);
		assert(declType != NULL);
	}
	if (ast->initDeclList) {
		ast->initDeclList->type = declType;
		declType = ast->initDeclList->accept(this);
		//TODO: regist (name, scope, type) to SymbolTable
	}
	return declType;
}

Type *
TypeCheck::visit(DeclSpecifier *ast)
{
	FUNCLOG;
	assert(0);
	return NULL;
}

Type *
TypeCheck::visit(DeclSpecifierSeq *ast)
{
	FUNCLOG;
	int err;
	TypeCreator typeCreator;
	List<DeclSpecifier*>::iterator it = ast->declSpecs.begin();
	while (it != ast->declSpecs.end()) {
		DeclSpecifier *declSpec = *it;
		TypeSpecifier *typeSpec = declSpec->typeSpec;
		//TODO:make complete type and assign to currentDeclType
		if (typeSpec) {
			if ((err = typeCreator.add(this, typeSpec)) != NO_ERROR) {
				SEMANTIC_ERROR(err);
			}
		} else {
			if ((err = typeCreator.add(this, declSpec->declId)) != NO_ERROR) {
				SEMANTIC_ERROR(err);
			}
		}
		++it;
	}

	//TODO: save complete type
	if ((err = typeCreator.getType(&ast->type)) != NO_ERROR) {
		SEMANTIC_ERROR(err);
	}

	DBG("TypeCreator getType %s", ast->type->toString().c_str());
	return ast->type;
}

Type *
TypeCheck::visit(TypeSpecifier *ast)
{
	FUNCLOG;
	assert(0);
	return NULL;
}

Type *
TypeCheck::visit(UserTypeSpecifier *ast)
{
	FUNCLOG;
	if (ast->scope) {
		//TODO:
		Type *t = SymbolTable.findType(ast->name->name, Symbol::symbol(ast->scope->scopeName()));
		return t;
	} else {
		Type *t = SymbolTable.findType(ast->name->name);
		return t;
	}
}

Type *
TypeCheck::visit(TypeName *ast)
{
	FUNCLOG;
	if (ast->name) {
		//TODO: find ast->name from SymbolTable and get binding type
		//return new NamedType(ast->name, NULL);
	}
	if (ast->templateId) {
		//TODO: instantiate template
		ast->templateId->accept(this);
	}
	return NULL;
}
Type *
TypeCheck::visit(EnumSpecifier *ast)
{
	FUNCLOG;
	if (ast->name == NULL) {
		ast->name = Symbol::gensym("enum");
	}
	EnumType *enumType = new EnumType(ast->name);
	//regist to SymbolTable
	if (!SymbolTable.addType(enumType, ast->name)) {
		SEMANTIC_ERROR(SE_DUPLICATE_TYPE);
	}

	List<EnumeratorDefinition*>::iterator it = ast->enumDefs.begin();
	while (it != ast->enumDefs.end()) {
		EnumeratorDefinition *enumDef = *it;
		Type *t = enumDef->accept(this);
		enumType->addMember(enumDef->name);
		if (!SymbolTable.addVar(enumType, enumDef->name)) {
			SEMANTIC_ERROR(SE_DUPLICATE_SYMBOL);
		}
		++it;
	}

	return enumType;
}
Type *
TypeCheck::visit(EnumeratorDefinition *ast)
{
	FUNCLOG;
	Type *t;
	if (ast->exp) {
		t = ast->exp->accept(this);
		if (!t->canImplicitCast(new IntType(32, true))) {
			SEMANTIC_ERROR(SE_ENUM_ASSIGN);
		}
	}
	return NULL;
}
Type *
TypeCheck::visit(NamespaceDefinition *ast)
{
	FUNCLOG;
	if (ast->body) {
		ast->body->accept(this);
	}
	return NULL;
}
Type *
TypeCheck::visit(NamespaceAliasDefinition *ast)
{
	FUNCLOG;
	//ast->alias;//TODO: regist to SymbolTable
	ast->original->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(UsingDeclaration *ast)
{
	FUNCLOG;
	ast->id->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(UsingDirective *ast)
{
	FUNCLOG;
	ast->namespaceId->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(TypeSpecifierSeq *ast)
{
	FUNCLOG;
	int err;
	TypeCreator typeCreator;
	List<TypeSpecifier*>::iterator it = ast->typeSpecs.begin();
	while (it != ast->typeSpecs.end()) {
		TypeSpecifier *typeSpec = *it;
		//TODO:make complete type and assign to currentDeclType
		if ((err = typeCreator.add(this, typeSpec)) != NO_ERROR) {
			SEMANTIC_ERROR(err);
		}
		++it;
	}

	if ((err = typeCreator.getType(&ast->type)) != NO_ERROR) {
		SEMANTIC_ERROR(err);
	}
	DBG("type %s", ast->type->toString().c_str());
	return ast->type;
}

Type *
TypeCheck::visit(TypeId *ast)
{
	FUNCLOG;
	Type *t = ast->typeSpecSeq->accept(this);
	if (ast->absDecl) {
		ast->absDecl->accept(this);
	}
	return t;
}

Type *
TypeCheck::visit(InitDeclarator *ast)
{
	FUNCLOG;
	ast->decl->type = ast->type;
	ast->decl->accept(this);
	ast->type = ast->decl->type;
	if (ast->initializer) {
		Type *initType = ast->initializer->accept(this);
		if (!ast->type->coerceTo(initType)) {
			//TODO: more detailed error (i.e. illegal size of array initializer)
			SEMANTIC_ERROR(SE_CAN_NOT_CONVERT);
		}
	}
	return NULL;
}

Type *
TypeCheck::visit(InitDeclaratorList *ast)
{
	FUNCLOG;
	List<InitDeclarator*>::iterator it = ast->decls.begin();
	while (it != ast->decls.end()) {
		InitDeclarator *initDecl = *it;
		initDecl->type = ast->type;
		initDecl->accept(this);
		++it;
	}
	return NULL;
}


Type *
TypeCheck::visit(Declarator *ast)
{
	FUNCLOG;
	assert(ast->type);
	if (ast->type->is(Type::VOID_T)) {
		SEMANTIC_ERROR(SE_VOID_DECL);
	}

	if (!ast->ptrs.empty()) {
		ast->type = makePointerType(ast->ptrs, ast->type);
	}

	if (ast->subDecl) {
		ast->subDecl->type = ast->type;
		ast->subDecl->accept(this);
	} else if (ast->declId) {
		if (ast->declId->scope) {
			//TODO:
		}
		Symbol *sym = Symbol::symbol(ast->declId->id->toSource());
		if (!SymbolTable.addVar(ast->type, sym)) {
			SEMANTIC_ERROR(SE_DUPLICATE_SYMBOL);
		}
	}
	return ast->type;
}


Type *
TypeCheck::checkFunctionDeclarator(FunctionDeclarator *ast, bool declOnly)
{
}

void f(int x)
{
}

Type *
TypeCheck::visit(FunctionDeclarator *ast)
{
	FUNCLOG;
	//In case of ctor, return type is null.
	Type *returnType = ast->type;

	if (!ast->ptrs.empty()) {
		returnType = makePointerType(ast->ptrs, returnType);
	}

	Symbol *funcName = ast->symbolName();
	FuncType *funcType = new FuncType(funcName, returnType);

	SymbolTable.enterScope(SymbolTable::FUNCTION, funcName);
	if (ast->paramDecls) {
		ParameterDeclarationList *list = ast->paramDecls;
		List<ParameterDeclaration*>::iterator it = list->paramDecls.begin();
		while (it != list->paramDecls.end()) {
			ParameterDeclaration *paramDecl = *it;
			Type *paramType = paramDecl->accept(this);
			assert(paramType);
			funcType->args.list.push_back(paramType);
			++it;
		}
	}
	SymbolTable.updateScope(SymbolTable.currentScope, funcType->genMangledSymbol());
	SymbolTable.leaveScope();

	if (ast->subDecl) {
		ast->subDecl->type = funcType;
		ast->subDecl->accept(this);
		ast->type = ast->subDecl->type;
	} else if (ast->declId) {
		if (ast->declId->scope) {
			//TODO:
		}
		//function name is added to outer scope
		Type *preDeclared = NULL;
		if (!SymbolTable.addVar(funcType, funcType->name, false, &preDeclared)) {
			//already declared
			if (preDeclared->is(Type::FUNC_T)) {
				FuncType *ft = preDeclared->as<FuncType>();
				//ft->mergeParams(funcType);
				ast->type = ft;
				return ast->type;
			}
			SEMANTIC_ERROR(SE_DUPLICATE_SYMBOL);
		} else {
			ast->type = funcType;
		}
	}

	if (ast->cvQuals) {
		Type *t = ast->cvQuals->accept(this);
		assert(t->isReadonly());
		funcType->readonly(true);
	}

	return ast->type;
}

Type *
TypeCheck::visit(ArrayDeclarator *ast)
{
	FUNCLOG;
	assert(ast->type);

	if (!ast->ptrs.empty()) {
		ast->type = makePointerType(ast->ptrs, ast->type);
	}

	ArrayType *arrayType;
	if (ast->constExp) {
		Type *indexType = ast->constExp->accept(this);
		if (!indexType->is(Type::INT_T)) {
			SEMANTIC_ERROR(SE_ARRAY_SIZE_NOT_CONST);
		}
		evaluator.init();
		const int size = ast->constExp->accept(&evaluator);
		if (evaluator.isConstant()) {
			arrayType = new ArrayType(ast->type, size);
		} else {
			SEMANTIC_ERROR(SE_ARRAY_SIZE_NOT_CONST);
		}
	} else {
		arrayType = new ArrayType(ast->type, 0);
	}

	if (ast->next) {
		ast->next->type = arrayType;
		ast->next->accept(this);
	}

	if (ast->subDecl) {
		assert(0);
		//ast->subDecl->accept(this);
	} else if (ast->declId) {
		if (ast->declId->scope) {
			//TODO:
		}
		Symbol *sym = Symbol::symbol(ast->declId->id->toSource());
		if (!SymbolTable.addVar(arrayType, sym)) {
			SEMANTIC_ERROR(SE_DUPLICATE_SYMBOL);
		}
	}
	ast->type = arrayType;
	return ast->type;
}

Type *
TypeCheck::visit(PtrOperator *ast)
{
	FUNCLOG;
	Type *ptrType = new PointerType();
	if (ast->cvQuals) {
		Type *t = ast->cvQuals->accept(this);
		assert(t->isReadonly());
		ptrType->readonly(true);
	}
	return ptrType;
}

Type *
TypeCheck::visit(ParameterDeclarationList *ast)
{
	FUNCLOG;
	assert(0);
	return NULL;
}

Type *
TypeCheck::visit(ParameterDeclaration *ast)
{
	FUNCLOG;
	Type *paramType = NULL;
	paramType = ast->declSpecSeq->accept(this);
	if (ast->decl) {
		ast->decl->type = paramType;
		paramType = ast->decl->accept(this);
	}
	if (ast->assignExp) {
		Type *assignedType = ast->assignExp->accept(this);
		if (!paramType->coerceTo(assignedType)) {
			SEMANTIC_ERROR(SE_ERROR);
		}
	}
	return paramType;
}

bool 
TypeCheck::checkGotoLabel()
{
	SymbolTable::Scope *scope = SymbolTable.getCurrentFunctionScope();
	List<Symbol*>::iterator it = gotoLabels.begin();
	while (it != gotoLabels.end()) {
		Symbol *label = *it;
		if (scope->labels.find(label) == scope->labels.end()) {
			return false;
		}
		++it;
	}
	return true;
}

Type *
TypeCheck::visit(FunctionDefinition *ast)
{
	FUNCLOG;
	unsigned int funcDecls = 0;
	Type *returnType = NULL;
	if (ast->declSpecSeq) {
		Type *t = ast->declSpecSeq->accept(this);
		funcDecls = t->getFlags();
		t->setFlags(0);
		returnType = t;
		ast->decl->type = returnType;
	}

	Type *t = ast->decl->accept(this);
	assert(t->is(Type::FUNC_T));
	FuncType *funcType = t->as<FuncType>();
	funcType->setFlags(funcDecls);
	if (funcType->hasBody) {
		SEMANTIC_ERROR(SE_DUPLICATE_FUNC_DEF);
	} else {
		funcType->hasBody = true;
	}
	SymbolTable.enterScope(SymbolTable::FUNCTION, funcType->genMangledSymbol());
	if (ast->memInits) {
		ast->memInits->accept(this);
	}

	ast->body->type = returnType;
	ast->body->accept(this);
	
	if (!checkGotoLabel()) {
		SEMANTIC_ERROR(SE_UNDEFINED_LABEL);
	}
	SymbolTable.leaveScope();

	return funcType;
}

Type *
TypeCheck::visit(FunctionBody *ast)
{
	FUNCLOG;
	currentFunction = ast;
	ast->stm->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(Initializer *ast)
{
	FUNCLOG;
	if (ast->initClause) {
		return ast->initClause->accept(this);
	} else if (ast->exps) {
		return ast->exps->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(InitializerClause *ast)
{
	FUNCLOG;
	if (ast->assignExp) {
		return ast->assignExp->accept(this);
	} else if (ast->list) {
		return ast->list->accept(this);
	}
	assert(0);
	return NULL;
}

Type *
TypeCheck::visit(InitializerList *ast)
{
	FUNCLOG;
	InitializerType *initType = new InitializerType();
	List<InitializerClause*>::iterator it = ast->clauses.begin();
	while (it != ast->clauses.end()) {
		InitializerClause *clause = *it;
		Type *t = clause->accept(this);
		initType->subTypes.list.push_back(t);
		++it;
	}
	return initType;
}


Type *
TypeCheck::visit(ClassSpecifier *ast)
{
	FUNCLOG;
	Symbol *name;
	if (ast->name->isTemplateId()) {
		//TODO:
		assert(0);
	} else {
		name =  ast->name->name;
	}

	ClassType *classType = new ClassType(name, ast->typeSpecId == CLASS);
	if (!SymbolTable.addType(classType, name)) {
		//duplicated class specifier
		SEMANTIC_ERROR(SE_DUPLICATE_TYPE);
	}

	ast->name->accept(this);
	if (ast->base) {
		//TODO:
		ast->base->accept(this);
	}

	if (ast->members) {
		SymbolTable.enterScope(SymbolTable::CLASS, name);

		Type *tl = ast->members->accept(this);
		assert(tl->is(Type::TYPELIST_T));
		TypeList *tlist = tl->as<TypeList>();
		List<Type*>::iterator it = tlist->list.begin();
		while (it != tlist->list.end()) {
			Type *t = *it;
			if (t->is(Type::METHOD_T)) {
				DBG("add method %s to %s", t->toString().c_str(), name->toString().c_str());
				classType->addMethod(t->as<MethodType>());
			} else if (t->is(Type::FIELD_T)) {
				DBG("add field %s to %s", t->toString().c_str(), name->toString().c_str());
				classType->addField(t->as<FieldType>());
			}
			++it;
		}

		SymbolTable.leaveScope();
	}

	return classType;
}

Type *
TypeCheck::visit(MemberSpecification *ast)
{
	FUNCLOG;
	TypeList *tlist = new TypeList();
	List<MemDecl*>::iterator it = ast->memDecls.begin();
	while (it != ast->memDecls.end()) {
		MemDecl *memDecl = *it;
		Type *t = memDecl->accept(this);
		//TypeList check
		if (t->is(Type::TYPELIST_T)) {
			TypeList *l = t->as<TypeList>();
			tlist->list.push_all(l->list);
		} else {
			tlist->list.push_back(t);
		}
		++it;
	}
	return tlist;
}

Type *
TypeCheck::visit(MemberDeclaration *ast)
{
	FUNCLOG;
	Type *declType = NULL;
	if (ast->declSpecSeq) {
		declType = ast->declSpecSeq->accept(this);
	}

	if (ast->memDecls.size()) {
		TypeList *tlist = new TypeList();
		List<MemberDeclarator*>::iterator it = ast->memDecls.begin();
		while (it != ast->memDecls.end()) {
			/*
			  MemberDeclarator
			  -- MemberBasicDeclarator
			  -- MemberConstDeclarator
			  -- MemberBitField
			 */
			MemberDeclarator *memDecl = *it;
			memDecl->type = declType;
			declType = memDecl->accept(this);
			if (!declType->is(Type::FUNC_T)) {
				FieldType *ft = new FieldType(memDecl->memberName(), declType);
				ft->access = ast->accessId;
				ft->parent = NULL;
				declType = ft;
			} else {
				FuncType *ft = declType->as<FuncType>();
				MethodType *mt = MethodType::fromFuncType(ft);
				mt->access = ast->accessId;
				declType = mt;
			}
			tlist->list.push_back(declType);
			++it;
		}
		declType = tlist;
	} else {
		//no declataror example:
		//struct A {};
	}

	return declType;
}

Type *
TypeCheck::visit(MemberFunctionDefinition *ast)
{
	FUNCLOG;
	Type *t = ast->funcDef->accept(this);
	assert(t->is(Type::FUNC_T));
	FuncType *ft = t->as<FuncType>();
	MethodType *mt = MethodType::fromFuncType(ft);
	mt->access = ast->accessId;
	return mt;
}

Type *
TypeCheck::visit(MemberTemplateDeclaration *ast)
{
	FUNCLOG;
	//TODO:Template Member Type
	return ast->tempDecl->accept(this);
}

Type *
TypeCheck::visit(MemberBasicDeclarator *ast)
{
	FUNCLOG;
	assert(ast->type);
	ast->decl->type = ast->type;
	Type *t = ast->decl->accept(this);
	if (t->is(Type::FUNC_T)) {
		if (ast->pureVirtual) {
			t->declSpec(PURE_VIRTUAL);
		}
		
	} else {
		if (ast->pureVirtual) {
			assert(0);
			SEMANTIC_ERROR(0);
		}
	}
	return t;
}

Type *
TypeCheck::visit(MemberConstDeclarator *ast)
{
	FUNCLOG;
	assert(ast->type);
	ast->decl->type = ast->type;
	Type *declType = ast->decl->accept(this);
	Type *initType = ast->constExp->accept(this);
	if (!declType->coerceTo(initType)) {
		SEMANTIC_ERROR(SE_CAN_NOT_CONVERT);
	}

	evaluator.init();
	const int index = ast->constExp->accept(&evaluator);
	if (!evaluator.isConstant()) {
		SEMANTIC_ERROR(SE_EXPECTED_CONST_EXP);
	}
	return declType;
}

Type *
TypeCheck::visit(MemberBitField *ast)
{
	FUNCLOG;
	ast->constExp->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(BaseClause *ast)
{
	FUNCLOG;
	List<BaseSpecifier*>::iterator it = ast->baseSpecs.begin();
	while (it != ast->baseSpecs.end()) {
		BaseSpecifier *baseSpec = *it;
		baseSpec->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(BaseSpecifier *ast)
{
	FUNCLOG;
	ast->name->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(MemInitializerList *ast)
{
	FUNCLOG;
	List<MemInitializer*>::iterator it = ast->memInits.begin();
	while (it != ast->memInits.end()) {
		MemInitializer *memInit = *it;
		memInit->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(MemInitializer *ast)
{
	FUNCLOG;
	ast->name->accept(this);
	if (ast->exp) {
		ast->exp->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(ConversionTypeId *ast)
{
	FUNCLOG;
	ast->typeSpecSeq->accept(this);
	List<PtrOperator*>::iterator it = ast->ptrs.begin();
	while (it != ast->ptrs.end()) {
		PtrOperator *ptr = *it;
		ptr->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(OperatorFunctionId *ast)
{
	FUNCLOG;
	return NULL;
}


Type *
TypeCheck::visit(TemplateDeclaration *ast)
{
	FUNCLOG;
	ast->params->accept(this);
	ast->decl->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(TemplateParameterList *ast)
{
	FUNCLOG;
	List<TemplateParameter*>::iterator it = ast->params.begin();
	while (it != ast->params.end()) {
		TemplateParameter *param = *it;
		param->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(TemplateParameter *ast)
{
	FUNCLOG;
	if (ast->typeParam) {
		ast->typeParam->accept(this);
	} else {
		ast->paramDecl->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(TypeParameter *ast)
{
	FUNCLOG;
	if (ast->typeId) {
		ast->typeId->accept(this);
	}
	if (ast->params) {
		ast->params->accept(this);
	}
	if (ast->idexp) {
		ast->idexp->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(TemplateId *ast)
{
	FUNCLOG;
	if (ast->args) {
		ast->args->accept(this);
	}
	return NULL;
}

Type *
TypeCheck::visit(TemplateArgumentList *ast)
{
	FUNCLOG;
	List<TemplateArgument*>::iterator it = ast->args.begin();
	while (it != ast->args.end()) {
		TemplateArgument *arg = *it;
		arg->accept(this);
		++it;
	}
	return NULL;
}

Type *
TypeCheck::visit(TemplateArgument *ast)
{
	FUNCLOG;
	ast->arg->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(ExplicitInstantiation *ast)
{
	FUNCLOG;
	ast->decl->accept(this);
	return NULL;
}

Type *
TypeCheck::visit(ExplicitSpecialization *ast)
{
	FUNCLOG;
	ast->decl->accept(this);
	return NULL;
}

void 
TypeCheck::test()
{
	DeclSpecifierSeq *declSpecSeq = new DeclSpecifierSeq();
	InitDeclaratorList *initDeclList = new InitDeclaratorList();
	SimpleDeclaration *simpleDecl
		= new SimpleDeclaration(declSpecSeq, initDeclList);
	//int a;
	{
		DeclSpecifier *decl_int = new DeclSpecifier(new TypeSpecifier(INT));
		DeclSpecifier *decl_int2 = new DeclSpecifier(new TypeSpecifier(SHORT));
		DeclSpecifier *decl_int3 = new DeclSpecifier(new TypeSpecifier(INT));
		DeclSpecifier *decl_const = new DeclSpecifier(new TypeSpecifier(CONST));
		DeclSpecifier *decl_signed = new DeclSpecifier(new TypeSpecifier(SIGNED));
		DeclSpecifier *decl_unsigned = new DeclSpecifier(new TypeSpecifier(UNSIGNED));
		declSpecSeq->declSpecs.push_back(decl_int);
		declSpecSeq->declSpecs.push_back(decl_int2);
		declSpecSeq->declSpecs.push_back(decl_int3);
		declSpecSeq->declSpecs.push_back(decl_const);
		declSpecSeq->declSpecs.push_back(decl_unsigned);

		ScopedId *id_a 
			= new ScopedId(NULL, new UnqualifiedId(new ASTIdent(Symbol::symbol("a"))));
		Declarator *decl_id_a = new Declarator();
		decl_id_a->declId = id_a;
		InitDeclarator *init_decl_id_a = new InitDeclarator(decl_id_a, NULL);
		initDeclList->decls.push_back(init_decl_id_a);
	}

	Type *t = simpleDecl->accept(this);
}
