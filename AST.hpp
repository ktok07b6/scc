#ifndef __AST_HPP__
#define __AST_HPP__

#include <cassert>
#include "String.hpp"
#include "List.hpp"
#include "Symbol.hpp"
#include "defs.hpp"
#include "ASTVisitor.hpp"
#include "ref.hpp"
#include "Type.hpp"

#define DECL_ACCEPT(T) 	virtual T accept(ASTVisitor< T > *v) = 0

struct AST {
	AST() : type(NULL) {}
	virtual ~AST() {}
	virtual bool isNull() {return false;}
	virtual String toString() const {return "";}
	virtual String toSource() const {return "";}

	DECL_ACCEPT(void);
	DECL_ACCEPT(int);
	DECL_ACCEPT(Type *);
	virtual void setLeft(AST *l) {}
	Type *type;
};

#define ACCEPT_VOID virtual void accept(ASTVisitor<void> *v) { v->visit(this); }
#define ACCEPT(T) virtual T accept(ASTVisitor<T> *v) { return v->visit(this); }

#define ACCEPTABLE														\
	ACCEPT_VOID 														\
	ACCEPT(int)															\
	ACCEPT(Type*)

#define LEFT(left) 	virtual void setLeft(AST *__l) { left = static_cast<decltype(left)>(__l); }

struct ASTEmpty : AST {
	ASTEmpty() {}
	virtual bool isNull() {return true;}
	virtual String toString() const {return "ASTEmpty()";}
	virtual String toSource() const {return "";}
};

struct Scope;

struct ASTIdent;
struct ASTExp;
struct PrimaryExpression;
struct UnqualifiedId;
struct ScopedId;
struct UnaryExpression;
struct BinaryExpression;
struct ConditionalExpression;
struct Subscript;
struct MemberSelect;
struct FunctionCall;
struct Constructor;
struct CastExpression;

struct CompoundStatement;
struct ASTStm;
struct DeclarationStatement;
struct ExpressionStatement;
struct IfStatement;
struct WhileStatement;
struct DoStatement;
struct ForStatement;
struct Condition;
struct JumpStatement;

struct DeclarationSeq;
struct Declaration;
struct BlockDeclaration;
struct TypeSpecifier;
struct TypeName;
struct InitDeclaratorList;
struct Declarator;
struct FunctionDeclarator;
struct ArrayDeclarator;
struct PtrOperator;
struct ParameterDeclarationList;
struct ParameterDeclaration;
struct FunctionDefinition;
struct FunctionBody;
struct Initializer;
struct InitializerClause;
struct InitializerList;

struct ClassSpecifier;
struct MemDecl;
struct MemberDeclaration;
struct MemberFunctionDefinition;
struct MemberDeclarator;
struct MemberBasicDeclarator;
struct MemberConstDeclarator;
struct MemberBitField;
struct BaseClause;
struct BaseSpecifier;
struct MemInitializerList;
struct ConversionTypeId;
struct OperatorFunctionId;

struct TemplateDeclaration;
struct TemplateParameterList;
struct TemplateParameter;
struct TypeParameter;
struct TemplateId;
struct TemplateArgumentList;
struct TemplateArgument;


struct TranslationUnit : AST {
	TranslationUnit(DeclarationSeq *seq);
	virtual String toString() const;
	virtual String toSource() const;

	DeclarationSeq *seq;;
	ACCEPTABLE
};

struct ScopeName : AST {
	ScopeName(Symbol *);
	ScopeName(TemplateId *);
	virtual String toString() const;
	virtual String toSource() const;
	bool isTemplateId() {return templateId;}
	Symbol *name;
	TemplateId *templateId;
	ACCEPTABLE
};

struct Scope : AST {
	Scope(ScopeName *scopeName);
	void addOuter(ScopeName *scopeName);
	virtual String toString() const;
	virtual String toSource() const;
	String scopeName() const;
	
	List<ScopeName*> scopeNames;
	ACCEPTABLE
};

struct ASTIdent : AST {
	ASTIdent(Symbol *ident);
	virtual String toString() const;
	virtual String toSource() const;
	Symbol *ident;
	ACCEPTABLE
};

struct ASTExp : AST {
	ASTExp(Operator op);
	Operator op;
};
struct ASTStm : AST {};
struct ASTDecl : AST {};

struct PrimaryExpression : ASTExp {
	PrimaryExpression(ScopedId *id);
	PrimaryExpression(int i);
	PrimaryExpression(bool b);
	PrimaryExpression(const char *s);
	PrimaryExpression(ASTExp *exp);
	virtual String toString() const;
	virtual String toSource() const;

	union {
		ScopedId *id;
		int i;
		bool b;
		String *str;
		ASTExp *exp; //?
	} v;
	ACCEPTABLE
};

struct UnqualifiedId : AST {
	UnqualifiedId(ASTIdent *ident);
	UnqualifiedId(OperatorFunctionId *op);
	UnqualifiedId(ConversionTypeId *conv);
	UnqualifiedId(TemplateId *templ);
	virtual String toString() const;
	virtual String toSource() const;

	enum {
		IDENT,
		OPERATOR_ID,
		CONVERSION_ID,
		TEMPLATE_ID
	};

	const int type;
	AST *id;
	ACCEPTABLE
};

//This class is used as qualified-id & id-expression
struct ScopedId : AST {
	ScopedId(Scope *scope, UnqualifiedId *id);
	virtual String toString() const;
	virtual String toSource() const;

	Scope *scope;//opt
	UnqualifiedId *id;
	ACCEPTABLE
};

struct SequenceExpression : ASTExp {
	SequenceExpression();
	virtual String toString() const;
	virtual String toSource() const;

	List<ASTExp*> exps;
	ACCEPTABLE
};

struct UnaryExpression : ASTExp {
	UnaryExpression(Operator op, ASTExp *e);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *e;
	ACCEPTABLE
	LEFT(e)
};

struct BinaryExpression : ASTExp {
	BinaryExpression(Operator op, ASTExp *l, ASTExp *r);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *l;
	ASTExp *r;
	ACCEPTABLE
	LEFT(l)
};

struct ConditionalExpression : ASTExp {
	ConditionalExpression(ASTExp *cond, ASTExp *l, ASTExp *r);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *cond;
	ASTExp *l;
	ASTExp *r;
	ACCEPTABLE
};

struct Subscript : ASTExp {//x[y]
	Subscript(ASTExp *exp, ASTExp *index);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *exp;
	ASTExp *index;
	ACCEPTABLE;
	LEFT(exp)
};

struct MemberSelect : ASTExp {//x.y
	MemberSelect(ASTExp *receiver, ASTExp *member, bool arrow);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *receiver;
	ASTExp *member;
	bool lvalue;
	bool arrow;
	ACCEPTABLE
	LEFT(receiver)
};

struct FunctionCall : ASTExp {
	FunctionCall(ASTExp *func);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *func;
	SequenceExpression *args;//opt
	ACCEPTABLE
	LEFT(func)
};

struct Constructor : ASTExp {
	Constructor(TypeSpecifier* typeSpec, SequenceExpression *args);
	virtual String toString() const;
	virtual String toSource() const;

	TypeSpecifier *typeSpec;
	SequenceExpression *args;//opt
	ACCEPTABLE
};

struct CastExpression : ASTExp {
	CastExpression(CastType cast, TypeId *typeId, ASTExp *exp);
	virtual String toString() const;
	virtual String toSource() const;

	CastType cast;
	TypeId *typeId;
	ASTExp *exp;
	ACCEPTABLE
};


struct Condition : AST {
	Condition(ASTExp *exp);
	Condition(TypeSpecifier *typeSpec, Declarator *decl, ASTExp *exp);
	virtual String toString() const;
	virtual String toSource() const;

	TypeSpecifier *typeSpec;
	Declarator *decl;
	ASTExp *exp;
	ACCEPTABLE
};


struct CompoundStatement : ASTStm {
	CompoundStatement();
	virtual String toString() const;
	virtual String toSource() const;

	List<ASTStm*> stms;
	ACCEPTABLE
};

struct DeclarationStatement : ASTStm {
	DeclarationStatement(BlockDeclaration *decl);
	virtual String toString() const;
	virtual String toSource() const;

	BlockDeclaration *decl;
	ACCEPTABLE
};

struct ExpressionStatement : ASTStm {
	ExpressionStatement(ASTExp *exp);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *exp;
	ACCEPTABLE
};

struct IfStatement : ASTStm {
	IfStatement(Condition *cond, ASTStm *thens, ASTStm *elses);
	virtual String toString() const;
	virtual String toSource() const;

	Condition *cond;
	ASTStm *thens;
	ASTStm *elses;// opt
	ACCEPTABLE
};

struct SwitchStatement : ASTStm {
	SwitchStatement(Condition *cond, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	Condition *cond;
	ASTStm *stm;
	ACCEPTABLE
};

struct CaseStatement : ASTStm {
	CaseStatement(ASTExp *exp, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *exp;//null as default
	ASTStm *stm;
	ACCEPTABLE
};

struct WhileStatement : ASTStm {
	WhileStatement(Condition *cond, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	Condition *cond;
	ASTStm *stm;
	ACCEPTABLE
};

struct DoStatement : ASTStm {
	DoStatement(ASTExp *exp, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *exp;
	ASTStm *stm;
	ACCEPTABLE
};

struct ForStatement : ASTStm {
	ForStatement(ForInitStatement *init, Condition *cond, ASTExp *exp, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	ForInitStatement *init;
	Condition *cond;// opt
	ASTExp *exp;// opt
	ASTStm *stm;
	ACCEPTABLE
};

struct ForInitStatement : ASTStm
{
	ForInitStatement(ExpressionStatement *expStm);
	ForInitStatement(SimpleDeclaration *simpleDecl);
	virtual String toString() const;
	virtual String toSource() const;

	ExpressionStatement *expStm;
	SimpleDeclaration *simpleDecl;
	ACCEPTABLE
};

struct JumpStatement : ASTStm {
	enum JumpType{
		CONTINUE,
		BREAK,
		RETURN,
		GOTO
	};

	JumpStatement(JumpType type, ASTExp *ret = NULL);
	JumpStatement(Symbol *label);
	virtual String toString() const;
	virtual String toSource() const;

	const JumpType jumpType;
	ASTExp *ret;
	Symbol *label;
	ACCEPTABLE
};

struct LabelStatement : ASTStm {
	LabelStatement(Symbol *label, ASTStm *stm);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *label;
	ASTStm *stm;
	ACCEPTABLE
};



struct DeclarationSeq : ASTDecl {
	DeclarationSeq();
	virtual String toString() const;
	virtual String toSource() const;

	List<Declaration*> decls;
	ACCEPTABLE
};

struct Declaration : ASTDecl {
	Declaration(int type);
	enum {
		BLOCK_DECL,
		FUNC_DEF,
		TEMPLATE_DECL,
		EXPLICIT_INST,
		EXPLICIT_SPECIAL,
		NAMESPACE_DEF,
		SC_PROCESS_DEF
	};
	const int type;
};

struct BlockDeclaration : Declaration
{
	BlockDeclaration();
};

struct SimpleDeclaration : BlockDeclaration {
	SimpleDeclaration(DeclSpecifierSeq *declSpecSeq, InitDeclaratorList *initDeclList);
	virtual String toString() const;
	virtual String toSource() const;

	DeclSpecifierSeq *declSpecSeq;
	InitDeclaratorList *initDeclList;
	ACCEPTABLE
};

struct DeclSpecifier : AST {
	DeclSpecifier(DeclSpecifierID declSpec);
	DeclSpecifier(TypeSpecifier *typeSpec);
	virtual String toString() const;
	virtual String toSource() const;

	DeclSpecifierID declId;
	TypeSpecifier *typeSpec;
	ACCEPTABLE
};

struct DeclSpecifierSeq : AST {
	DeclSpecifierSeq();
	virtual String toString() const;
	virtual String toSource() const;

	List<DeclSpecifier*> declSpecs;
	ACCEPTABLE
};

struct TypeSpecifier : AST {
	TypeSpecifier(TypeSpecifierID type);
	virtual String toString() const;
	virtual String toSource() const;

	TypeSpecifierID typeSpecId;
	ACCEPTABLE
};

struct UserTypeSpecifier : TypeSpecifier {
	UserTypeSpecifier(Scope *scope, TypeName *name);
	virtual String toString() const;
	virtual String toSource() const;

	Scope *scope;
	TypeName *name;
	ACCEPTABLE
};

struct TypeName : AST {
	TypeName(Symbol *name);
	TypeName(TemplateId *id);
	virtual String toString() const;
	virtual String toSource() const;

	bool isTemplateId() {return templateId;}
	Symbol *name;
	TemplateId *templateId;
	ACCEPTABLE
};

struct EnumSpecifier : TypeSpecifier {
	EnumSpecifier();
	EnumSpecifier(Symbol *name, Scope *scope);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *name;
	Scope *scope;//elaborated type specifier
	List<EnumeratorDefinition*> enumDefs;
	ACCEPTABLE
};

struct EnumeratorDefinition : AST {
	EnumeratorDefinition(Symbol *name, ASTExp *exp);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *name;
	ASTExp *exp;
	ACCEPTABLE
};

struct NamespaceDefinition : Declaration {
	NamespaceDefinition(Symbol *name, DeclarationSeq *body);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *name;//opt
	DeclarationSeq *body;//opt
	ACCEPTABLE
};

struct NamespaceAliasDefinition : BlockDeclaration {
	NamespaceAliasDefinition(Symbol *alias, ScopedId *original);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *alias;
	ScopedId *original;
	ACCEPTABLE
};

struct UsingDeclaration : BlockDeclaration {
	UsingDeclaration(ScopedId *id, bool typeName);
	virtual String toString() const;
	virtual String toSource() const;

	ScopedId *id;
	bool typeName;
	ACCEPTABLE
};

struct UsingDirective : BlockDeclaration {
	UsingDirective(ScopedId *namespaceId);
	virtual String toString() const;
	virtual String toSource() const;

	ScopedId *namespaceId;
	ACCEPTABLE
};

struct TypeSpecifierSeq : AST {
	TypeSpecifierSeq();
	virtual String toString() const;
	virtual String toSource() const;

	List<TypeSpecifier*> typeSpecs;
	ACCEPTABLE
};

struct TypeId : AST {
	TypeId(TypeSpecifierSeq *typeSpecSeq, Declarator *absDecl);
	virtual String toString() const;
	virtual String toSource() const;

	TypeSpecifierSeq *typeSpecSeq;
	Declarator *absDecl;//opt
	ACCEPTABLE
};

struct InitDeclarator : AST {
	InitDeclarator(Declarator *decl, Initializer *initializer);
	virtual String toString() const;
	virtual String toSource() const;

	Declarator *decl;
	Initializer *initializer;//opt
	ACCEPTABLE
};

struct InitDeclaratorList : AST {
	InitDeclaratorList();
	virtual String toString() const;
	virtual String toSource() const;

	List<InitDeclarator*> decls;
	ACCEPTABLE
};

struct Declarator : ASTDecl {
	Declarator();
	virtual String toString() const;
	virtual String toSource() const;
	
	virtual bool isFunction() const {return false;}
	virtual bool isArray() const {return false;}
	Symbol *symbolName();

	ScopedId* declId;
	Declarator *subDecl;
	List<PtrOperator*> ptrs;
	ACCEPTABLE
};

struct FunctionDeclarator : Declarator {
	FunctionDeclarator(ParameterDeclarationList *paramDecls, TypeSpecifierSeq *cvQuals);
	virtual String toString() const;
	virtual String toSource() const;

	virtual bool isFunction() const {return true;}

	ParameterDeclarationList *paramDecls;
	TypeSpecifierSeq *cvQuals;//opt
	ACCEPTABLE
};

struct ArrayDeclarator : Declarator {
	ArrayDeclarator(ASTExp *constExp, Declarator *next);
	virtual String toString() const;
	virtual String toSource() const;

	virtual bool isArray() const {return true;}

	ASTExp *constExp;//opt
	Declarator *next;//opt;
	ACCEPTABLE
};

struct PtrOperator : AST {
	enum PtrType{
		POINTER,
		REFERENCE
	};
	PtrOperator(PtrType type, TypeSpecifierSeq *cvQuals, Scope *scope);
	virtual String toString() const;
	virtual String toSource() const;

	PtrType type;
	TypeSpecifierSeq *cvQuals;//opt
	Scope *scope;//opt
	ACCEPTABLE
};

struct ParameterDeclarationList : AST {
	ParameterDeclarationList(bool elipsis);
	virtual String toString() const;
	virtual String toSource() const;

	List<ParameterDeclaration*> paramDecls;
	bool elipsis;//opt
	ACCEPTABLE
};

struct ParameterDeclaration : AST {
	ParameterDeclaration(DeclSpecifierSeq *declSpecSeq, Declarator *decl, ASTExp *assignExp);
	virtual String toString() const;
	virtual String toSource() const;

	DeclSpecifierSeq *declSpecSeq;
	Declarator *decl;//opt
	ASTExp *assignExp;//opt
	ACCEPTABLE
};

struct FunctionDefinition : Declaration {
	FunctionDefinition(DeclSpecifierSeq *declSpecSeq, Declarator *decl, MemInitializerList *memInits, FunctionBody *body);
	virtual String toString() const;
	virtual String toSource() const;

	DeclSpecifierSeq *declSpecSeq;
	Declarator *decl;
	MemInitializerList *memInits;
	FunctionBody *body;
	ACCEPTABLE
};

struct FunctionBody : AST {
	FunctionBody(CompoundStatement *stm);
	virtual String toString() const;
	virtual String toSource() const;

	CompoundStatement *stm;
	ACCEPTABLE
};

struct Initializer : AST {
	Initializer(InitializerClause *initClause);
	Initializer(SequenceExpression *exps);
	virtual String toString() const;
	virtual String toSource() const;

	InitializerClause *initClause;
	SequenceExpression *exps;
	ACCEPTABLE
};

struct InitializerClause : AST {
	InitializerClause();
	InitializerClause(ASTExp *assignExp);
	InitializerClause(InitializerList *list);
	virtual String toString() const;
	virtual String toSource() const;

	ASTExp *assignExp;
	InitializerList *list;
	ACCEPTABLE
};

struct InitializerList : AST {
	InitializerList();
	virtual String toString() const;
	virtual String toSource() const;

	List<InitializerClause*> clauses;
	ACCEPTABLE
};


struct ClassSpecifier : TypeSpecifier {
	ClassSpecifier(TypeName *name, Scope *scope, BaseClause *base, bool clazz);
	virtual String toString() const;
	virtual String toSource() const;

	TypeName *name;
	Scope *scope;//opt
	BaseClause *base;//opt
	MemberSpecification *members;//opt
	ACCEPTABLE
};

struct MemberSpecification : AST {
	MemberSpecification();
	virtual String toString() const;
	virtual String toSource() const;

	void setAccessSpec(AccessSpecifierID accessId);
	List<MemDecl*> memDecls;
	ACCEPTABLE
};

struct MemDecl : AST {
	MemDecl();
	AccessSpecifierID accessId;
};

struct MemberDeclaration : MemDecl {
	MemberDeclaration();
	virtual String toString() const;
	virtual String toSource() const;

	DeclSpecifierSeq *declSpecSeq;// opt
	List<MemberDeclarator*> memDecls;// opt
	ACCEPTABLE
};
struct MemberFunctionDefinition : MemDecl {
	MemberFunctionDefinition(FunctionDefinition *funcDef);
	virtual String toString() const;
	virtual String toSource() const;

	FunctionDefinition *funcDef;
	ACCEPTABLE
};
struct MemberTemplateDeclaration : MemDecl {
	MemberTemplateDeclaration(TemplateDeclaration *tempDecl);
	virtual String toString() const;
	virtual String toSource() const;

	TemplateDeclaration *tempDecl;
	ACCEPTABLE
};

struct MemberDeclarator : AST {
	virtual Symbol *memberName() = 0;
};
struct MemberBasicDeclarator : MemberDeclarator {
	MemberBasicDeclarator(Declarator *decl, bool pure);
	virtual String toString() const;
	virtual String toSource() const;

	virtual Symbol *memberName();
	Declarator *decl;
	bool pureVirtual;
	ACCEPTABLE
};
struct MemberConstDeclarator : MemberDeclarator {
	MemberConstDeclarator(Declarator *decl, ASTExp *constExp);
	virtual String toString() const;
	virtual String toSource() const;

	virtual Symbol *memberName();
	Declarator *decl;
	ASTExp *constExp;
	ACCEPTABLE
};
struct MemberBitField : MemberDeclarator {
	MemberBitField(Symbol *name, ASTExp *constExp);
	virtual String toString() const;
	virtual String toSource() const;

	virtual Symbol *memberName();
	Symbol *name;//opt
	ASTExp *constExp;
	ACCEPTABLE
};

struct BaseClause : AST {
	BaseClause();
	virtual String toString() const;
	virtual String toSource() const;

	List<BaseSpecifier*> baseSpecs;
	ACCEPTABLE
};
struct BaseSpecifier : AST {
	BaseSpecifier(TypeName *name, Scope *scope, AccessSpecifierID accessId, bool virt);
	virtual String toString() const;
	virtual String toSource() const;

	TypeName *name;
	Scope *scope;//opt 
	AccessSpecifierID accessId;//opt
	bool virtualInherit;//opt
	ACCEPTABLE
};

struct MemInitializerList : AST {
	MemInitializerList();
	virtual String toString() const;
	virtual String toSource() const;

	List<MemInitializer*> memInits;
	ACCEPTABLE
};

struct MemInitializer : AST {
	MemInitializer(TypeName *name, Scope* scope);
	virtual String toString() const;
	virtual String toSource() const;

	TypeName *name;
	Scope *scope;
	ASTExp *exp;	
	ACCEPTABLE
};

struct ConversionTypeId : AST {
	ConversionTypeId();
	virtual String toString() const;
	virtual String toSource() const;
	
	TypeSpecifierSeq *typeSpecSeq;
	List<PtrOperator*> ptrs;
	ACCEPTABLE
};

struct OperatorFunctionId : AST {
	OperatorFunctionId(Operator op);
	virtual String toString() const;
	virtual String toSource() const;

	const Operator op;
	ACCEPTABLE
};

struct TemplateDeclaration : Declaration {
	TemplateDeclaration(TemplateParameterList *params, Declaration *decl, bool expo);
	virtual String toString() const;
	virtual String toSource() const;

	TemplateParameterList *params;
	Declaration *decl;
	bool expo;
	ACCEPTABLE
};

struct TemplateParameterList : AST {
	TemplateParameterList();
	virtual String toString() const;
	virtual String toSource() const;

	List<TemplateParameter*> params;
	ACCEPTABLE
};

struct TemplateParameter : AST {
	TemplateParameter(TypeParameter *typeParam);
	TemplateParameter(ParameterDeclaration *paramDecl);
	virtual String toString() const;
	virtual String toSource() const;

	TypeParameter *typeParam;
	ParameterDeclaration *paramDecl;
	ACCEPTABLE
};

struct TypeParameter : AST {
	TypeParameter(Symbol *name, TypeId *typeId);
	TypeParameter(Symbol *name, TemplateParameterList *params, ScopedId *idexp);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *name;
	TypeId *typeId;
	TemplateParameterList *params;
	ScopedId *idexp;
	ACCEPTABLE
};

struct TemplateId : AST {
	TemplateId(Symbol *name, TemplateArgumentList *args);
	virtual String toString() const;
	virtual String toSource() const;

	Symbol *name;
	TemplateArgumentList *args;//opt
	ACCEPTABLE
};

struct TemplateArgumentList : AST {
	TemplateArgumentList();
	virtual String toString() const;
	virtual String toSource() const;

	List<TemplateArgument*> args;
	ACCEPTABLE
};

struct TemplateArgument : AST {
	TemplateArgument(ASTExp *assign);
	TemplateArgument(TypeId *typeId);
	TemplateArgument(ScopedId *scopedId);
	virtual String toString() const;
	virtual String toSource() const;

	enum {
		ASSIGN_EXP,
		TYPE_ID,
		SCOPED_ID
	};
	const int type;
	AST *arg;
	ACCEPTABLE
};

struct ExplicitInstantiation : Declaration {
	ExplicitInstantiation(Declaration *decl);
	virtual String toString() const;
	virtual String toSource() const;

	Declaration *decl;
	ACCEPTABLE
};

struct ExplicitSpecialization : Declaration {
	ExplicitSpecialization(Declaration *decl);
	virtual String toString() const;
	virtual String toSource() const;

	Declaration *decl;
	ACCEPTABLE
};

#endif //__AST_H__
