#ifndef __CONSTANT_EXP_EVALUATOR_HPP__
#define __CONSTANT_EXP_EVALUATOR_HPP__

#include "ASTVisitor.hpp"
class PreProcessor;

class ConstantExpEvaluator : public ASTVisitor<int>
{
	PreProcessor *preprocessor;
	bool constant;
public:
	ConstantExpEvaluator(PreProcessor *pp) 
		: preprocessor(pp)
		, constant(true)
	{
	}
	void init() {constant = true;}
	bool isConstant() {return constant;}

	virtual int visit(TranslationUnit*);

	virtual int visit(ScopeName*);
	virtual int visit(Scope*);
	virtual int visit(ASTIdent*);

	virtual int visit(PrimaryExpression*);
	virtual int visit(UnqualifiedId*);
	virtual int visit(ScopedId*);
	virtual int visit(SequenceExpression*);
	virtual int visit(UnaryExpression*);
	virtual int visit(BinaryExpression*);
	virtual int visit(ConditionalExpression*);
	virtual int visit(Subscript*);
	virtual int visit(MemberSelect*);
	virtual int visit(FunctionCall*);
	virtual int visit(Constructor*);
	virtual int visit(CastExpression*);

	virtual int visit(Condition*);	
	virtual int visit(CompoundStatement*);
	virtual int visit(DeclarationStatement*);
	virtual int visit(ExpressionStatement*);
	virtual int visit(IfStatement*);
	virtual int visit(SwitchStatement*);
	virtual int visit(CaseStatement*);
	virtual int visit(WhileStatement*);
	virtual int visit(DoStatement*);
	virtual int visit(ForStatement*);
	virtual int visit(ForInitStatement*);
	virtual int visit(JumpStatement*);
	virtual int visit(LabelStatement*);
	
	virtual int visit(DeclarationSeq*);
	virtual int visit(SimpleDeclaration*);
	virtual int visit(DeclSpecifier*);
	virtual int visit(DeclSpecifierSeq*);
	virtual int visit(TypeSpecifier*);
	virtual int visit(UserTypeSpecifier*);
	virtual int visit(TypeName*);
	virtual int visit(EnumSpecifier*);
	virtual int visit(EnumeratorDefinition*);
	virtual int visit(NamespaceDefinition*);
	virtual int visit(NamespaceAliasDefinition*);
	virtual int visit(UsingDeclaration*);
	virtual int visit(UsingDirective*);
	virtual int visit(TypeSpecifierSeq*);
	virtual int visit(TypeId*);
	virtual int visit(InitDeclarator*);
	virtual int visit(InitDeclaratorList*);
	virtual int visit(Declarator*);
	virtual int visit(FunctionDeclarator*);
	virtual int visit(ArrayDeclarator*);
	virtual int visit(PtrOperator*);
	virtual int visit(ParameterDeclarationList*);
	virtual int visit(ParameterDeclaration*);
	virtual int visit(FunctionDefinition*);
	virtual int visit(FunctionBody*);
	virtual int visit(Initializer*);
	virtual int visit(InitializerClause*);
	virtual int visit(InitializerList*);

	virtual int visit(ClassSpecifier*);
	virtual int visit(MemberSpecification*);
	virtual int visit(MemberDeclaration*);
	virtual int visit(MemberFunctionDefinition*);
	virtual int visit(MemberTemplateDeclaration*);
	virtual int visit(MemberBasicDeclarator*);
	virtual int visit(MemberConstDeclarator*);
	virtual int visit(MemberBitField*);
	virtual int visit(BaseClause*);
	virtual int visit(BaseSpecifier*);
	virtual int visit(MemInitializerList*);
	virtual int visit(MemInitializer*);
	virtual int visit(ConversionTypeId*);
	virtual int visit(OperatorFunctionId*);

	virtual int visit(TemplateDeclaration*);
	virtual int visit(TemplateParameterList*);
	virtual int visit(TemplateParameter*);
	virtual int visit(TypeParameter*);
	virtual int visit(TemplateId*);
	virtual int visit(TemplateArgumentList*);
	virtual int visit(TemplateArgument*);
	virtual int visit(ExplicitInstantiation*);
	virtual int visit(ExplicitSpecialization*);
};
#endif
