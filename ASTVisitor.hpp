#ifndef __AST_VISITOR_HPP__
#define __AST_VISITOR_HPP__

struct TranslationUnit;

struct ScopeName;
struct Scope;
struct ASTIdent;

struct PrimaryExpression;
struct UnqualifiedId;
struct ScopedId;
struct SequenceExpression;
struct UnaryExpression;
struct BinaryExpression;
struct ConditionalExpression;
struct Subscript;
struct MemberSelect;
struct FunctionCall;
struct Constructor;
struct CastExpression;

struct Condition;
struct CompoundStatement;
struct DeclarationStatement;
struct ExpressionStatement;
struct IfStatement;
struct SwitchStatement;
struct CaseStatement;
struct WhileStatement;
struct DoStatement;
struct ForStatement;
struct ForInitStatement;
struct JumpStatement;
struct LabelStatement;

struct DeclarationSeq;
struct SimpleDeclaration;
struct DeclSpecifier;
struct DeclSpecifierSeq;
struct TypeSpecifier;
struct UserTypeSpecifier;
struct TypeName;
struct EnumSpecifier;
struct EnumeratorDefinition;
struct NamespaceDefinition;
struct NamespaceAliasDefinition;
struct UsingDeclaration;
struct UsingDirective;
struct TypeSpecifierSeq;
struct TypeId;
struct InitDeclarator;
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
struct MemberSpecification;
struct MemberDeclaration;
struct MemberFunctionDefinition;
struct MemberTemplateDeclaration;
struct MemberBasicDeclarator;
struct MemberConstDeclarator;
struct MemberBitField;
struct BaseClause;
struct BaseSpecifier;
struct MemInitializerList;
struct MemInitializer;
struct ConversionTypeId;
struct OperatorFunctionId;

struct TemplateDeclaration;
struct TemplateParameterList;
struct TemplateParameter;
struct TypeParameter;
struct TemplateId;
struct TemplateArgumentList;
struct TemplateArgument;
struct ExplicitInstantiation;
struct ExplicitSpecialization;

template<typename T>
class ASTVisitor {
public:
	virtual ~ASTVisitor() {}

	virtual T visit(TranslationUnit*) = 0;

	virtual T visit(ScopeName*)=0;
	virtual T visit(Scope*)=0;
	virtual T visit(ASTIdent*)=0;

	virtual T visit(PrimaryExpression*)=0;
	virtual T visit(UnqualifiedId*)=0;
	virtual T visit(ScopedId*)=0;
	virtual T visit(SequenceExpression*)=0;
	virtual T visit(UnaryExpression*)=0;
	virtual T visit(BinaryExpression*)=0;
	virtual T visit(ConditionalExpression*)=0;
	virtual T visit(Subscript*)=0;
	virtual T visit(MemberSelect*)=0;
	virtual T visit(FunctionCall*)=0;
	virtual T visit(Constructor*)=0;
	virtual T visit(CastExpression*)=0;

	virtual T visit(Condition*)=0;	
	virtual T visit(CompoundStatement*)=0;
	virtual T visit(DeclarationStatement*)=0;
	virtual T visit(ExpressionStatement*)=0;
	virtual T visit(IfStatement*)=0;
	virtual T visit(SwitchStatement*)=0;
	virtual T visit(CaseStatement*)=0;
	virtual T visit(WhileStatement*)=0;
	virtual T visit(DoStatement*)=0;
	virtual T visit(ForStatement*)=0;
	virtual T visit(ForInitStatement*)=0;
	virtual T visit(JumpStatement*)=0;
	virtual T visit(LabelStatement*)=0;
	
	virtual T visit(DeclarationSeq*)=0;
	virtual T visit(SimpleDeclaration*)=0;
	virtual T visit(DeclSpecifier*)=0;
	virtual T visit(DeclSpecifierSeq*)=0;
	virtual T visit(TypeSpecifier*) = 0;
	virtual T visit(UserTypeSpecifier*) = 0;
	virtual T visit(TypeName*) = 0;
	virtual T visit(EnumSpecifier*) = 0;
	virtual T visit(EnumeratorDefinition*) = 0;
	virtual T visit(NamespaceDefinition*) = 0;
	virtual T visit(NamespaceAliasDefinition*) = 0;
	virtual T visit(UsingDeclaration*) = 0;
	virtual T visit(UsingDirective*) = 0;
	virtual T visit(TypeSpecifierSeq*) = 0;
	virtual T visit(TypeId*) = 0;
	virtual T visit(InitDeclarator*)=0;
	virtual T visit(InitDeclaratorList*)=0;
	virtual T visit(Declarator*)=0;
	virtual T visit(FunctionDeclarator*)=0;
	virtual T visit(ArrayDeclarator*)=0;
	virtual T visit(PtrOperator*)=0;
	virtual T visit(ParameterDeclarationList*)=0;
	virtual T visit(ParameterDeclaration*)=0;
	virtual T visit(FunctionDefinition*)=0;
	virtual T visit(FunctionBody*)=0;
	virtual T visit(Initializer*)=0;
	virtual T visit(InitializerClause*)=0;
	virtual T visit(InitializerList*)=0;

	virtual T visit(ClassSpecifier*)=0;
	virtual T visit(MemberSpecification*)=0;
	virtual T visit(MemberDeclaration*)=0;
	virtual T visit(MemberFunctionDefinition*)=0;
	virtual T visit(MemberTemplateDeclaration*)=0;
	virtual T visit(MemberBasicDeclarator*)=0;
	virtual T visit(MemberConstDeclarator*)=0;
	virtual T visit(MemberBitField*)=0;
	virtual T visit(BaseClause*)=0;
	virtual T visit(BaseSpecifier*)=0;
	virtual T visit(MemInitializerList*)=0;
	virtual T visit(MemInitializer*)=0;
	virtual T visit(ConversionTypeId*)=0;
	virtual T visit(OperatorFunctionId*)=0;

	virtual T visit(TemplateDeclaration*)=0;
	virtual T visit(TemplateParameterList*)=0;
	virtual T visit(TemplateParameter*)=0;
	virtual T visit(TypeParameter*)=0;
	virtual T visit(TemplateId*)=0;
	virtual T visit(TemplateArgumentList*)=0;
	virtual T visit(TemplateArgument*)=0;
	virtual T visit(ExplicitInstantiation*)=0;
	virtual T visit(ExplicitSpecialization*)=0;
};
	
#endif //__AST_VISITOR_H__
