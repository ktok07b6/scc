#ifndef __SCPARSER_ACTION_HPP__
#define __SCPARSER_ACTION_HPP__

#include "AST.hpp"
#include "SCParser.hpp"
#include <vector>

//#define DUMP() dump()
#define DUMP()

class SCParserAction
{
	struct ASTStack
	{
		void dump();
		void push(void *p);
		void push(Operator op);
		void pushi(int i);
		void *pop();

		template <typename T>
		T popas() {
			assert(!stack.empty());
			void *vp = stack.back();
			T p = NULL;
			if (vp) {
				p = static_cast<T>(vp);
				if (!p) {
					ERROR("AST stack cuppupted!!!");
					assert(0);
				}
			}
			stack.pop_back();
			PARSE_DBG("ASTStack::pop %p", p);
			DUMP();
			return p;
		}

		Operator popop();

		template <typename T>
		T popi() {
			assert(!stack.empty());
			intptr_t p = reinterpret_cast<intptr_t>(stack.back());
			stack.pop_back();
			T i = static_cast<T>(p);
			PARSE_DBG("ASTStack::pop %d", i);
			DUMP();
			return i;
		}

		bool empty() const;
		std::vector<void*> stack;
	};
	ASTStack astStack;
	std::vector<BinaryExpression*> binExpStack;

	struct ASTList
	{
		AST *ast;
		ASTList *next;
	};

	AST *list2ast(ASTList *list, AST *ast);

public:
	AST *getAST();
	ASTExp *makeBinExp(ASTExp *left);
	int getBinOpPriority(Operator op);

	void onEmpty(const ref<TokenValue> &v);
	void onToken(const ref<TokenValue> &v);

	void onIdentifier(const ref<TokenValue> &v);
	void onLiteral_Decimal(const ref<TokenValue> &v);
	void onLiteral_Octal(const ref<TokenValue> &v);
	void onLiteral_Hex(const ref<TokenValue> &v);
	void onLiteral_String(const ref<TokenValue> &v);
	void onLiteral_True(const ref<TokenValue> &v);
	void onLiteral_False(const ref<TokenValue> &v);

	void onTranslationUnit(const ref<TokenValue> &v);

	void onPrimaryExp_Literal(const ref<TokenValue> &v);
	void onPrimaryExp_This(const ref<TokenValue> &v);
	void onPrimaryExp_Expression(const ref<TokenValue> &v);
	void onPrimaryExp_IdExp(const ref<TokenValue> &v);

	void onIdExp_UnqualifiedId(const ref<TokenValue> &v);
	void onIdExp_QualifiedId(const ref<TokenValue> &v);
	void onUnqualifiedId_Identifier(const ref<TokenValue> &v);
	void onUnqualifiedId_TemplateId(const ref<TokenValue> &v);
	void onUnqualifiedId_OperatorId(const ref<TokenValue> &v);
	void onUnqualifiedId_ConversionId(const ref<TokenValue> &v);
	void onQualifiedId_Valid1(const ref<TokenValue> &v);
	void onQualifiedId_Identifier(const ref<TokenValue> &v);
	void onQualifiedId_OperatorFuncId(const ref<TokenValue> &v);
	void onQualifiedId_TemplateId(const ref<TokenValue> &v);
	
	void onNestedNameSpecifier_Valid1(const ref<TokenValue> &v);
	void onNestedNameSpecifier_Valid2(const ref<TokenValue> &v);
	void onClassOrNamespaceName_Identifier(const ref<TokenValue> &v);
	void onClassOrNamespaceName_TemplateId(const ref<TokenValue> &v);

	void onPostfixExp_PrimaryExp(const ref<TokenValue> &v);
	void onPostfixExp_SimpleTypeSpecifier(const ref<TokenValue> &v);
	void onPostfixExp_Subscript(const ref<TokenValue> &v);
	void onPostfixExp_ExpressionList(const ref<TokenValue> &v);
	void onPostfixExp_MemberSelect(const ref<TokenValue> &v);
	void onPostfixExp_ArrowMemberSelect(const ref<TokenValue> &v);
	void onPostfixExp(const ref<TokenValue> &v);
	void onPostfixOperator(const ref<TokenValue> &v);
		
	void onCxxCastId(const ref<TokenValue> &v);
	void onCxxCastExp(const ref<TokenValue> &v);
		
	void onExpressionListHead(const ref<TokenValue> &v);
	void onExpressionListRest(const ref<TokenValue> &v);

	void onUnaryExp(const ref<TokenValue> &v);
	void onUnaryOperator(const ref<TokenValue> &v);
	void onCastExp(const ref<TokenValue> &v);
	void onBinaryExpHead(const ref<TokenValue> &v);
	void onBinaryExp(const ref<TokenValue> &v);
	void onBinaryOperator(const ref<TokenValue> &v);

	void onConditionalExp(const ref<TokenValue> &v);
	void onAssignExp(const ref<TokenValue> &v);
	void onAssignOperator(const ref<TokenValue> &v);

	void onLabeledStm(const ref<TokenValue> &v);
	void onCaseStm(const ref<TokenValue> &v);
	void onDefaultCaseStm(const ref<TokenValue> &v);
	void onExpressionStm(const ref<TokenValue> &v);
	void onCompoundStm(const ref<TokenValue> &v);
	void onStatementSeq(const ref<TokenValue> &v);
	void onWaitStm(const ref<TokenValue> &v);
	void onSignalAssignmentStm(const ref<TokenValue> &v);
	void onIfStm(const ref<TokenValue> &v);
	void onIfElseStm(const ref<TokenValue> &v);
	void onSwitchStm(const ref<TokenValue> &v);
	void onCondition_Exp(const ref<TokenValue> &v);
	void onCondition_AssignExp(const ref<TokenValue> &v);
	void onWhileStm(const ref<TokenValue> &v);
	void onDoWhileStm(const ref<TokenValue> &v);
	void onForStm(const ref<TokenValue> &v);
	void onForInitStm_ExpressionStm(const ref<TokenValue> &v);
	void onForInitStm_SimpleDecl(const ref<TokenValue> &v);
	void onBreakStm(const ref<TokenValue> &v);
	void onContinueStm(const ref<TokenValue> &v);
	void onReturnStm(const ref<TokenValue> &v);
	void onGotoStm(const ref<TokenValue> &v);
	void onDeclarationStm(const ref<TokenValue> &v);

	void onDeclSeq(const ref<TokenValue> &v);
	void onDecl_BlockDecl(const ref<TokenValue> &v);
	void onDecl_FuncDef(const ref<TokenValue> &v);
	void onBlockDecl_SimpleDecl(const ref<TokenValue> &v);
	void onBlockDecl_NamespaceAlias(const ref<TokenValue> &v);
	void onBlockDecl_UsingDecl(const ref<TokenValue> &v);
	void onBlockDecl_UsingDirective(const ref<TokenValue> &v);
	void onSimpleDecl(const ref<TokenValue> &v);
	void onDeclSpecifier_TypeSpecifier(const ref<TokenValue> &v);
	void onDeclSpecifier_OtherSpecifier(const ref<TokenValue> &v);
	void onDeclSpecSeq(const ref<TokenValue> &v);
	void onSimpleTypeSpecifier_TypeName(const ref<TokenValue> &v);
	void onSimpleTypeSpecifier_TemplateId(const ref<TokenValue> &v);
	void onBuiltinTypeSpecifier(const ref<TokenValue> &v);
	void onTypeName(const ref<TokenValue> &v);
	void onTypeName_TemplateId(const ref<TokenValue> &v);
	void onElaboratedTypeSpec_Class(const ref<TokenValue> &v);
	void onElaboratedTypeSpec_Enum(const ref<TokenValue> &v);
	void onElaboratedTypeSpec_Typename(const ref<TokenValue> &v);
	void onElaboratedTypeSpec_TypenameTemplateId(const ref<TokenValue> &v);

	void onEnumSpecifier(const ref<TokenValue> &v);
	void onEnumeratorListHead(const ref<TokenValue> &v);
	void onEnumeratorListRest(const ref<TokenValue> &v);
	void onEnumeratorDefinition(const ref<TokenValue> &v);
	void onEnumeratorDefinitionAssign(const ref<TokenValue> &v);

	void onNamespaceDefinition(const ref<TokenValue> &v);
	void onNamespaceAliasDefinition(const ref<TokenValue> &v);
	void onUsingDeclaration(const ref<TokenValue> &v);
	void onUsingDeclaration_Scoped(const ref<TokenValue> &v);
	void onUsingDirective(const ref<TokenValue> &v);

	void onInitDeclListHead(const ref<TokenValue> &v);
	void onInitDeclListRest(const ref<TokenValue> &v);
	void onInitDecl(const ref<TokenValue> &v);
	void onDecl_DirectDecl(const ref<TokenValue> &v);
	void onDecl_PtrDecl(const ref<TokenValue> &v);
	void onDirectDecl_DeclId(const ref<TokenValue> &v);
	void onDirectDecl_FuncPtrDecl(const ref<TokenValue> &v);
	void onDirectDecl_ParamDecl(const ref<TokenValue> &v);
	void onDirectDecl_ArrayDecl(const ref<TokenValue> &v);
	void onPtrOp_Ptr(const ref<TokenValue> &v);
	void onPtrOp_Reference(const ref<TokenValue> &v);
	void onPtrOp_ScopedPtr(const ref<TokenValue> &v);
	void onCVQualSeq(const ref<TokenValue> &v);
	void onCVQual(const ref<TokenValue> &v);
	void onDeclId_IdExp(const ref<TokenValue> &v);
	void onDeclId_Typename(const ref<TokenValue> &v);
	void onTypeId(const ref<TokenValue> &v);
	void onTypeSpecSeq(const ref<TokenValue> &v);
	void onParamDeclListHead(const ref<TokenValue> &v);
	void onParamDeclListHead_Elipsis(const ref<TokenValue> &v);
	void onParamDeclListRest(const ref<TokenValue> &v);
	void onParamDeclListRest_Elipsis(const ref<TokenValue> &v);
	void onParamDecl(const ref<TokenValue> &v);
	void onParamDeclAssign(const ref<TokenValue> &v);
	void onFuncDef(const ref<TokenValue> &v);
	void onFuncBody(const ref<TokenValue> &v);
	void onInitializer(const ref<TokenValue> &v);
	void onInitializer_Parenthesis(const ref<TokenValue> &v);
	void onInitializerClause_AssignExp(const ref<TokenValue> &v);
	void onInitializerClause_InitializerList(const ref<TokenValue> &v);
	void onInitializerClause_EmptyBraces(const ref<TokenValue> &v);
	void onInitializerListHead(const ref<TokenValue> &v);
	void onInitializerListRest(const ref<TokenValue> &v);

	void onClassSpecifier(const ref<TokenValue> &v);
	void onClassHead(const ref<TokenValue> &v);
	void onClassHead_Template(const ref<TokenValue> &v);
	void onClassHead_NestedId(const ref<TokenValue> &v);
	void onClassHead_NestedTemplate(const ref<TokenValue> &v);
	void onClassKey(const ref<TokenValue> &v);
	void onMemberSpec_MemberDecl(const ref<TokenValue> &v);
	void onMemberSpec_AccessSpec(const ref<TokenValue> &v);
	void onMemberDecl_FuncDef(const ref<TokenValue> &v);
	void onMemberDecl_UnqualifiedId(const ref<TokenValue> &v);
	void onMemberDecl_UsingDecl(const ref<TokenValue> &v);
	void onMemberDecl_TemplateDecl(const ref<TokenValue> &v);
	void onMemberDecl_MemberDeclList(const ref<TokenValue> &v);
	void onMemberDeclListHead(const ref<TokenValue> &v);
	void onMemberDeclListRest(const ref<TokenValue> &v);
	void onMemberDeclarator(const ref<TokenValue> &v);
	void onMemberDeclarator_PureSpec(const ref<TokenValue> &v);
	void onMemberDeclarator_ConstantInit(const ref<TokenValue> &v);
	void onMemberDeclarator_BitField(const ref<TokenValue> &v);
	void onPureSpec(const ref<TokenValue> &v);
	void onConstantInit(const ref<TokenValue> &v);

	void onBaseClause(const ref<TokenValue> &v);
	void onBaseSpecListHead(const ref<TokenValue> &v);
	void onBaseSpecListRest(const ref<TokenValue> &v);
	void onBaseSpec_Virtual(const ref<TokenValue> &v);
	void onBaseSpec_AccessSpec(const ref<TokenValue> &v);
	void onBaseSpec(const ref<TokenValue> &v);
	void onAccessSpec(const ref<TokenValue> &v);

	void onConversionFuncId(const ref<TokenValue> &v);
	void onConversionTypeId(const ref<TokenValue> &v);
	void onConversionDecl(const ref<TokenValue> &v);
	void onCtorInit(const ref<TokenValue> &v);
	void onMemInitListHead(const ref<TokenValue> &v);
	void onMemInitListRest(const ref<TokenValue> &v);
	void onMemInit(const ref<TokenValue> &v);
	void onMemInitId(const ref<TokenValue> &v);
	void onMemInitId_NestedClassName(const ref<TokenValue> &v);

	void onOperatorFuncId(const ref<TokenValue> &v);
	void onOperatorId(const ref<TokenValue> &v);

	void onTemplateDecl(const ref<TokenValue> &v);
	void onTemplateParamListHead(const ref<TokenValue> &v);
	void onTemplateParamListRest(const ref<TokenValue> &v);
	void onTemplateParam_TypeParam(const ref<TokenValue> &v);
	void onTemplateParam_ParamDecl(const ref<TokenValue> &v);
	void onTypeParam_Ident(const ref<TokenValue> &v);
	void onTypeParam_IdentAssign(const ref<TokenValue> &v);
	void onTypeParam_TemplateClass(const ref<TokenValue> &v);
	void onTypeParam_TemplateClassAssign(const ref<TokenValue> &v);
	void onTemplateId(const ref<TokenValue> &v);
	void onTemplateArgListHead(const ref<TokenValue> &v);
	void onTemplateArgListRest(const ref<TokenValue> &v);
	void onTemplateArg_AssignExp(const ref<TokenValue> &v);
	void onTemplateArg_TypeId(const ref<TokenValue> &v);
	void onTemplateArg_ScopedId(const ref<TokenValue> &v);
	void onExplicitInst(const ref<TokenValue> &v);
	void onExplicitSpecial(const ref<TokenValue> &v);
};
#endif
