#ifndef __TYPE_CHECK_HPP__
#define __TYPE_CHECK_HPP__

#include "ASTVisitor.hpp"
#include "ref.hpp"
#include "Type.hpp"
#include "ConstantExpEvaluator.hpp"

class TypeCheck : public ASTVisitor<Type*>
{
public:
	TypeCheck();
	int getLastError();
	void test();
	Type *makePointerType(List<PtrOperator*> &ptrs, Type *reftype);

	ConstantExpEvaluator evaluator;
	FunctionBody *currentFunction;
	List<Symbol*> gotoLabels;

	//helper functions
	Type *checkFunctionDeclarator(FunctionDeclarator *ast, bool declOnly);
	bool checkGotoLabel();

	//visitor interfaces
	virtual Type * visit(TranslationUnit*);
	virtual Type * visit(ScopeName*);
	virtual Type * visit(Scope*);
	virtual Type * visit(ASTIdent*);
	virtual Type * visit(PrimaryExpression*);
	virtual Type * visit(UnqualifiedId*);
	virtual Type * visit(ScopedId*);
	virtual Type * visit(SequenceExpression*);
	virtual Type * visit(UnaryExpression*);
	virtual Type * visit(BinaryExpression*);
	virtual Type * visit(ConditionalExpression*);
	virtual Type * visit(Subscript*);
	virtual Type * visit(MemberSelect*);
	virtual Type * visit(FunctionCall*);
	virtual Type * visit(Constructor*);
	virtual Type * visit(CastExpression*);

	virtual Type * visit(Condition*);	
	virtual Type * visit(CompoundStatement*);
	virtual Type * visit(DeclarationStatement*);
	virtual Type * visit(ExpressionStatement*);
	virtual Type * visit(IfStatement*);
	virtual Type * visit(SwitchStatement*);
	virtual Type * visit(CaseStatement*);
	virtual Type * visit(WhileStatement*);
	virtual Type * visit(DoStatement*);
	virtual Type * visit(ForStatement*);
	virtual Type * visit(ForInitStatement*);
	virtual Type * visit(JumpStatement*);
	virtual Type * visit(LabelStatement*);
	
	virtual Type * visit(DeclarationSeq*);
	virtual Type * visit(SimpleDeclaration*);
	virtual Type * visit(DeclSpecifier*);
	virtual Type * visit(DeclSpecifierSeq*);
	virtual Type * visit(TypeSpecifier*);
	virtual Type * visit(UserTypeSpecifier*);
	virtual Type * visit(TypeName*);
	virtual Type * visit(EnumSpecifier*);
	virtual Type * visit(EnumeratorDefinition*);
	virtual Type * visit(NamespaceDefinition*);
	virtual Type * visit(NamespaceAliasDefinition*);
	virtual Type * visit(UsingDeclaration*);
	virtual Type * visit(UsingDirective*);
	virtual Type * visit(TypeSpecifierSeq*);
	virtual Type * visit(TypeId*);
	virtual Type * visit(InitDeclarator*);
	virtual Type * visit(InitDeclaratorList*);
	virtual Type * visit(Declarator*);
	virtual Type * visit(FunctionDeclarator*);
	virtual Type * visit(ArrayDeclarator*);
	virtual Type * visit(PtrOperator*);
	virtual Type * visit(ParameterDeclarationList*);
	virtual Type * visit(ParameterDeclaration*);
	virtual Type * visit(FunctionDefinition*);
	virtual Type * visit(FunctionBody*);
	virtual Type * visit(Initializer*);
	virtual Type * visit(InitializerClause*);
	virtual Type * visit(InitializerList*);

	virtual Type * visit(ClassSpecifier*);
	virtual Type * visit(MemberSpecification*);
	virtual Type * visit(MemberDeclaration*);
	virtual Type * visit(MemberFunctionDefinition*);
	virtual Type * visit(MemberTemplateDeclaration*);
	virtual Type * visit(MemberBasicDeclarator*);
	virtual Type * visit(MemberConstDeclarator*);
	virtual Type * visit(MemberBitField*);
	virtual Type * visit(BaseClause*);
	virtual Type * visit(BaseSpecifier*);
	virtual Type * visit(MemInitializerList*);
	virtual Type * visit(MemInitializer*);
	virtual Type * visit(ConversionTypeId*);
	virtual Type * visit(OperatorFunctionId*);

	virtual Type * visit(TemplateDeclaration*);
	virtual Type * visit(TemplateParameterList*);
	virtual Type * visit(TemplateParameter*);
	virtual Type * visit(TypeParameter*);
	virtual Type * visit(TemplateId*);
	virtual Type * visit(TemplateArgumentList*);
	virtual Type * visit(TemplateArgument*);
	virtual Type * visit(ExplicitInstantiation*);
	virtual Type * visit(ExplicitSpecialization*);
};

#endif
