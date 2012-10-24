#include "ConstantExpEvaluator.hpp"
#include "defs.hpp"
#include "AST.hpp"
#include <cassert>
#include "PreProcessor.hpp"

int
ConstantExpEvaluator::visit(TranslationUnit *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ScopeName *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(Scope *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ASTIdent *ast)
{
	assert(0);
	return 0;
}


int
ConstantExpEvaluator::visit(PrimaryExpression *ast)
{
	switch (ast->op) {
	case PRIMARY_ID:  constant = false; return 0;
	case PRIMARY_INT: return ast->v.i;
	case PRIMARY_BOOL: return ast->v.b;
	case PRIMARY_STR: return 1;
	case PRIMARY_EXP: return ast->v.exp->accept(this);
	default:
		assert(0);
		break;
	}
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(UnqualifiedId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ScopedId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(SequenceExpression *ast)
{
	int x;
	List<ASTExp*>::iterator it = ast->exps.begin();
	while (it != ast->exps.end()) {
		ASTExp *exp = *it;
		x = exp->accept(this);
		++it;
	}
	return x;
}

int
ConstantExpEvaluator::visit(UnaryExpression *ast)
{
	int a = ast->e->accept(this);
	switch (ast->op) {
	case BANG:  return !a;
	case TILDE: return ~a;
	case UNARY_PLUS: return +a;
	case UNARY_MINUS: return -a;

	case PRE_INC:
	case PRE_DEC:
	case POST_INC:
	case POST_DEC:
	case UNARY_STAR:
	case UNARY_AND:
		/* TODO: not valid operator */
		break;
	default:
		assert(0);
	}
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(BinaryExpression *ast)
{
	int a = ast->l->accept(this);
	int b = ast->r->accept(this);
	switch (ast->op) {
	case STAR:    return a * b;
	case SLASH:   return a / b; 
	case PERCENT: return a % b;
	case PLUS:    return a + b;
	case MINUS:   return a - b;
	case LSHIFT:  return a << b;
	case RSHIFT:  return a >> b;
	case BIT_OR:  return a | b;
	case BIT_XOR: return a ^ b;
	case BIT_AND: return a & b;
	case EQ:      return a == b;
	case NE:      return a != b;
	case LT:      return a < b;
	case LE:      return a <= b;
	case GT:      return a > b;
	case GE:      return a >= b;
	case AND:     return a && b;
	case OR:      return a || b;
	default:
		assert(0);
	}
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(ConditionalExpression *ast)
{
	int cond = ast->cond->accept(this);
	if (constant) {
		if (cond) {
			return ast->l->accept(this);
		} else {
			return ast->r->accept(this);
		}
	}
	return 0;
}

int
ConstantExpEvaluator::visit(Subscript *ast)
{
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(MemberSelect *ast)
{
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(FunctionCall *ast)
{
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(Constructor *ast)
{
	constant = false;
	return 0;
}

int
ConstantExpEvaluator::visit(CastExpression *ast)
{
	//TODO:
	return ast->exp->accept(this);
}

int
ConstantExpEvaluator::visit(Condition *ast)
{
	assert(0);
	return 0;
}
	
int
ConstantExpEvaluator::visit(CompoundStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(DeclarationStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ExpressionStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(IfStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(SwitchStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(CaseStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(WhileStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(DoStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ForStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ForInitStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(JumpStatement *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(LabelStatement *ast)
{
	assert(0);
	return 0;
}
	
int
ConstantExpEvaluator::visit(DeclarationSeq *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(SimpleDeclaration *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(DeclSpecifier *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(DeclSpecifierSeq *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TypeSpecifier *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(UserTypeSpecifier *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TypeName *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(EnumSpecifier *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(EnumeratorDefinition *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(NamespaceDefinition *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(NamespaceAliasDefinition *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(UsingDeclaration *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(UsingDirective *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(TypeSpecifierSeq *ast)
{
	assert(0);
	return 0;
}
int
ConstantExpEvaluator::visit(TypeId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(InitDeclarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(InitDeclaratorList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(Declarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(FunctionDeclarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ArrayDeclarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(PtrOperator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ParameterDeclarationList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ParameterDeclaration *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(FunctionDefinition *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(FunctionBody *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(Initializer *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(InitializerClause *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(InitializerList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ClassSpecifier *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberSpecification *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberDeclaration *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberFunctionDefinition *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberTemplateDeclaration *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberBasicDeclarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberConstDeclarator *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemberBitField *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(BaseClause *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(BaseSpecifier *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemInitializerList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(MemInitializer *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ConversionTypeId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(OperatorFunctionId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateDeclaration *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateParameterList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateParameter *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TypeParameter *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateId *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateArgumentList *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(TemplateArgument *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ExplicitInstantiation *ast)
{
	assert(0);
	return 0;
}

int
ConstantExpEvaluator::visit(ExplicitSpecialization *ast)
{
	assert(0);
	return 0;
}

