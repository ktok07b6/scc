#include "AST.hpp"
#include "Symbol.hpp"
//#define ENABLE_FUNCLOG
#include "debug.h"

const char *DeclSpecifierString[] = {
	"",
	"auto",
	"register",
	"static",
	"extern",
	"mutable",
	"inline",
	"virtual",
	"explicit",
	"friend",
	"typedef",
	"pure virtual"
};

const char * CVQualifierString[] = {
	"",
	"const",
	"volatile"
};
const char * TypeSpecifierString[] = {
	"char",
	"bool",
	"short",
	"int",
	"long",
	"long long",
	"signed",
	"unsigned",
	"void", 
	"const",
	"volatile",
	"struct",
	"class",
	"enum",
	""
};
const char * AccessSpecifierString[] = {
	"/*unspecified member access*/",
	"private", 
	"protected",
	"public"
};
const char * JumpTypeString[] = {
	"break",
	"continue",
	"return",
	"goto"
};
const char * CastTypeString[] = {
	"static_cast",
	"dynamic_cast",
	"reinterpret_cast",
	"const_cast",
	"c-style cast"
};

const char * OperatorString[] = {
	"++", "--", 
	"++", "--",
	"+", "-",
	"*", "&",
	"!", "~",

	"*", "/", "%",
	"+", "-",
	"<<", ">>",
	"|", "^", "&",

	"==", "!=",	"<", "<=", ">", ">=",

	"&&", "^^", "||",

	"=",
	"*=",
	"/=",
	"%=",   // reserved
	"+=",
	"-=", 
	"<<=",   // reserved
	">>=",   // reserved
	"&=",  // reserved
	"^=",   // reserved
	"|=",   // reserved
	"~=",

	"?", 
	"func call",
	"ctor call",
	"[]",
	".",
	"->",
	"->*",

	"()",
	"[]",
	"(cast)",
	",",
	"::",
	"...",

	"(symbol)",
	"(int)",
	"(bool)",
	"(string)",
	"(exp)",
	"seq"
};

//AST::toString list helper
template<typename T>
String toString(const List<T*> &list, const String &delim)
{
	String s;
	typename List<T*>::const_iterator it = list.begin();
	while (it != list.end()) {
		T *elem = *it;
		s += elem->toString();
		++it;
		if (it != list.end()) {
			s += delim;
		}
	}
	return s;
}

//AST::toSource list helper
template<typename T>
String toSource(const List<T*> &list, const String &delim)
{
	String s;
	typename List<T*>::const_iterator it = list.begin();
	while (it != list.end()) {
		T *elem = *it;
		s += elem->toSource();
		++it;
		if (it != list.end()) {
			s += delim;
		}
	}
	return s;
}


TranslationUnit::TranslationUnit(DeclarationSeq *seq)
	: seq(seq)
{
	FUNCLOG;
}

String
TranslationUnit::toString() const
{
	String s = "TranslationUnit(";
	if (seq) {
		s += seq->toString();
	}
	return s;
}

String
TranslationUnit::toSource() const
{
	if (seq) {
		return seq->toSource();
	} else {
		return "";
	}
}

ScopeName::ScopeName(Symbol *name)
	: name(name)
	, templateId(NULL)
{
	FUNCLOG;
	assert(name);
}
ScopeName::ScopeName(TemplateId *templateId)
	: name(NULL)
	, templateId(templateId)
{
	FUNCLOG;
	assert(templateId);
}

String 
ScopeName::toString() const
{
	String s = "ScopeName(";
	if (name) {
		s += name->toString();
	} else {
		s += templateId->toString();
	}
	return s;
}

String 
ScopeName::toSource() const
{
	String s;
	if (name) {
		s += name->toString();
	} else {
		s += templateId->toSource();
	}
	return s;
}


Scope::Scope(ScopeName *scopeName)
{
	FUNCLOG;
	addOuter(scopeName);
}

void
Scope::addOuter(ScopeName *scopeName)
{
	scopeNames.push_front(scopeName);
}

String
Scope::toString() const
{
	String s = "Scope(";
	s += ::toString(scopeNames, "::");
	s += ")";
	return s;
}

String
Scope::toSource() const
{
	return ::toSource(scopeNames, "::") + "::";
}

String
Scope::scopeName() const
{
	return ::toSource(scopeNames, "::");
}

ASTIdent::ASTIdent(Symbol *ident)
	: ident(ident)
{
	FUNCLOG;
	assert(ident);
}

String 
ASTIdent::toString() const
{
	return "ASTIdent(" + ident->toString() + ")";
}

String 
ASTIdent::toSource() const
{
	return ident->toString();
}


ASTExp::ASTExp(Operator op)
	: op(op)
{
}

PrimaryExpression::PrimaryExpression(ScopedId *id)
	: ASTExp(PRIMARY_ID)
{
	FUNCLOG;
	assert(id);
	v.id = id;
}
PrimaryExpression::PrimaryExpression(int i)
	: ASTExp(PRIMARY_INT)
{
	FUNCLOG;
	v.i = i;
}
PrimaryExpression::PrimaryExpression(bool b)
	: ASTExp(PRIMARY_BOOL)
{
	FUNCLOG;
	v.b = b;
}
PrimaryExpression::PrimaryExpression(const char *str)
	: ASTExp(PRIMARY_STR)
{
	FUNCLOG;
	assert(str);
	v.str = new String(str);
}
PrimaryExpression::PrimaryExpression(ASTExp *exp)
	: ASTExp(PRIMARY_EXP)
{
	FUNCLOG;
	assert(exp);
	v.exp = exp;
}

String
PrimaryExpression::toString() const
{
	String s = "PrimaryExpression(";
	switch (op) {
	case PRIMARY_ID: 
		s += v.id->toString();
		break;

	case PRIMARY_INT: {
 		char buf[64];
		sprintf(buf, "%d", v.i);
		s += buf;
	}
		break;

	case PRIMARY_BOOL:   
		s += v.b ? "true":"false";
		break;

	case PRIMARY_STR:   
		s += *v.str;
		break;

	case PRIMARY_EXP:
		s += "(" + v.exp->toString() + ")";
		break;

	default:
		break;
	}

	s += ")";
	return s;
}

String
PrimaryExpression::toSource() const
{
	switch (op) {
	case PRIMARY_ID:
		return v.id->toSource();

	case PRIMARY_INT: {
 		char buf[64];
		sprintf(buf, "%d", v.i);
		return buf;
	}

	case PRIMARY_BOOL:   
		return v.b ? "true":"false";

	case PRIMARY_STR:   
		return v.str->c_str();

	case PRIMARY_EXP:
		return "(" + v.exp->toSource() + ")";

	default:
		return "";
	}
}
UnqualifiedId::UnqualifiedId(ASTIdent *ident)
	: type(IDENT)
	, id(ident)
{
	FUNCLOG;
	assert(id);
}

UnqualifiedId::UnqualifiedId(OperatorFunctionId *op)
	: type(OPERATOR_ID)
	, id(op)
{
	FUNCLOG;
	assert(id);
}

UnqualifiedId::UnqualifiedId(ConversionTypeId *conv)
	: type(CONVERSION_ID)
	, id(conv)
{
	FUNCLOG;
	assert(id);
}

UnqualifiedId::UnqualifiedId(TemplateId *templ)
	: type(TEMPLATE_ID)
	, id(templ)
{
	FUNCLOG;
	assert(id);
}

String 
UnqualifiedId::toString() const
{
	String s = "UnqualifiedId(";
	s += id->toString();
	s += ")";
	return s;
}

String 
UnqualifiedId::toSource() const
{
	return id->toSource();
}

ScopedId::ScopedId(Scope *scope, UnqualifiedId *id)
	: scope(scope)
	, id(id)
{
	FUNCLOG;
	assert(id);
}

String 
ScopedId::toString() const
{
	String s = "ScopedId(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	s += id->toString();
	s += ")";
	return s;
}

String 
ScopedId::toSource() const
{
	String s;
	if (scope) {
		s += scope->toSource();
	}
	s += id->toSource();
	return s;
}

SequenceExpression::SequenceExpression()
	: ASTExp(SEQ)
{
	FUNCLOG;
}

String
SequenceExpression::toString() const
{
	String s = "SequenceExpression(";
	s += ::toString(exps, ", ");
	s += ")";
	return s;
}

String
SequenceExpression::toSource() const
{
	return ::toSource(exps, ", ");
}

UnaryExpression::UnaryExpression(Operator op, ASTExp *e)
	: ASTExp(op)
	, e(e)
{
	FUNCLOG;
	//assert(e);
}

String
UnaryExpression::toString() const
{
	String s = "UnaryExpression(";
	s += OperatorString[op];
	s += ", ";
	s += e->toString();
	s += ")";
	return s;
}

String
UnaryExpression::toSource() const
{
	String s;
	if (op == POST_INC || op == POST_DEC) {
		s = e->toSource();
		s += OperatorString[op];
	} else {
		s = OperatorString[op];
		s += e->toSource();
	}
	return s;
}

BinaryExpression::BinaryExpression(Operator op, ASTExp *l, ASTExp *r)
	: ASTExp(op)
	, l(l)
	, r(r)
{
	FUNCLOG;
	//DBG("op = %d", op);
	//assert(l);
	assert(r);
}

String
BinaryExpression::toString() const
{
	String s = "BinaryExpression(";
	s += OperatorString[op];
	s += ", ";
	s += l->toString();
	s += ", ";
	s += r->toString();
	s += ")";
	return s;
}

String
BinaryExpression::toSource() const
{
	String s;
	s = l->toSource();
	s += OperatorString[op];
	s += r->toSource();
	return s;
}

ConditionalExpression::ConditionalExpression(ASTExp *cond, ASTExp *l, ASTExp *r)
	: ASTExp(CONDITIONAL)
	, cond(cond)
	, l(l)
	, r(r)
{
	FUNCLOG;
	assert(cond);
	assert(l);
	assert(r);
}

String
ConditionalExpression::toString() const
{
	String s = "ConditionalExpression(";
	s += cond->toString();
	s += ", ";
	s += l->toString();
	s += ", ";
	s += r->toString();
	s += ")";
	return s;
}

String
ConditionalExpression::toSource() const
{
	String s;
	s = cond->toSource();
	s += "?";
	s += l->toSource();
	s += ":";
	s += r->toSource();
	return s;
}


Subscript::Subscript(ASTExp *exp, ASTExp *index)
	: ASTExp(SUBSCRIPT)
	, exp(exp)
	, index(index)

{
	FUNCLOG;
	//assert(exp);
	assert(index);
}

String
Subscript::toString() const
{
	String s = "Subscript(";
	s += exp->toString();
	s += ", ";
	s += index->toString();
	s += ")";
	return s;
}

String
Subscript::toSource() const
{
	String s;
	s = exp->toSource();
	s += "[";
	s += index->toSource();
	s += "]";
	return s;
}

MemberSelect::MemberSelect(ASTExp *receiver, ASTExp *member, bool arrow)
	: ASTExp(MEMBER_SELECT)
	, receiver(receiver)
	, member(member)
	, lvalue(false)
	, arrow(arrow)
{
	FUNCLOG;
	//assert(receiver);
	assert(member);
}

String
MemberSelect::toString() const
{
	String s = "MemberSelect(";
	s += receiver->toString();
	s += ", ";
	s += member->toString();
	s += ")";
	return s;
}

String
MemberSelect::toSource() const
{
	String s;
	s = receiver->toSource();
	if (arrow) {
		s += "->";
	} else {
		s += ".";
	}
	s += member->toSource();
	return s;
}

FunctionCall::FunctionCall(ASTExp *func)
	: ASTExp(FUNC_CALL)
	, func(func)
	, args(NULL)
{
	FUNCLOG;
	//assert(func);
}

String
FunctionCall::toString() const
{
	String s = "FunctionCall(";
	s += func->toString();
	s += ", ";
	if (args) {
		s += args->toString();
	}
	s += ")";
	return s;
}

String
FunctionCall::toSource() const 
{
	String s;
	s = func->toSource();
	s += "(";
	if (args) {
		s += args->toSource();
	}
	s += ")";
	return s;
}

Constructor::Constructor(TypeSpecifier *typeSpec, SequenceExpression *args)
	:ASTExp(CTOR_CALL)
	, typeSpec(typeSpec)
	, args(args)
{
	FUNCLOG;
	assert(typeSpec);
}

String
Constructor::toString() const
{
	String s = "Constructor(";
	s += typeSpec->toString();
	s += ", ";
	if (args) {
		s += args->toString();
	}
	s += ")";
	return s;
}

String
Constructor::toSource() const
{
	String s;
	s = typeSpec->toSource();
	s += "(";
	if (args) {
		s += args->toSource();
	}
	s += ")";
	return s;
}

CastExpression::CastExpression(CastType cast, TypeId *typeId, ASTExp *exp)
	: ASTExp(CAST)
	, cast(cast)
	, typeId(typeId)
	, exp(exp)
{
	FUNCLOG;
	assert(typeId);
	assert(exp);
}

String 
CastExpression::toString() const
{
	String s = "CastExpression(";
	s += CastTypeString[cast];
	s += ", ";
	s += typeId->toString();
	s += ", ";
	s += exp->toString();
	s += ")";
	return s;
}

String 
CastExpression::toSource() const
{
	String s;
	if (cast == C_CAST) {
		s = "(";
		s += typeId->toSource();
		s += ")";
		s += exp->toSource();
	} else {
		s = CastTypeString[cast];
		s += "<";
		s += typeId->toSource();
		s += ">(";
		s += exp->toSource();
		s += ")";
	}
	return s;
}


Condition::Condition(ASTExp *exp)
	: typeSpec(NULL)
	, decl(NULL)
	, exp(exp)
{
	FUNCLOG;
	assert(exp);
}

Condition::Condition(TypeSpecifier *typeSpec, Declarator *decl, ASTExp *exp)
	: typeSpec(typeSpec)
	, decl(decl)
	, exp(exp)
{
	FUNCLOG;
	//assert(typeSpec);
	//assert(decl);
	assert(exp);
}

String
Condition::toString() const
{
	String s = "Condition(";
	if (typeSpec) {
		s += typeSpec->toString();
		s += ", ";
		s += decl->toString();
		s += ", ";
		s += exp->toString();
	} else {
		s += exp->toString();
	}
	s += ")";
	return s;
}

String
Condition::toSource() const
{
	String s = "";
	if (typeSpec) {
		s += typeSpec->toSource();
		s += " ";
		s += decl->toSource();
		s += " = ";
		s += exp->toSource();
	} else {
		s += exp->toSource();
	}
	return s;
}



CompoundStatement::CompoundStatement()
{
	FUNCLOG;
}

String
CompoundStatement::toString() const
{
	String s = "\nCompoundStatement(";
	s += ::toString(stms, ", ");
	s += ")";
	return s;
}

String
CompoundStatement::toSource() const
{
	String s;
	s = "{\n";
	s += ::toSource(stms, "");
	s += "}\n";
	return s;
}

DeclarationStatement::DeclarationStatement(BlockDeclaration *decl)
	: decl(decl)
{
	FUNCLOG;
	assert(decl);
}

String
DeclarationStatement::toString() const
{
	String s = "\nDeclarationStatement(";
	s += decl->toString();
	s += ")";
	return s;
}

String
DeclarationStatement::toSource() const
{
	return decl->toSource();
}

ExpressionStatement::ExpressionStatement(ASTExp *exp)
	: exp(exp)
{
	FUNCLOG;
	//exp == NULL is valid;
}

String
ExpressionStatement::toString() const
{
	String s = "\nExpressionStatement(";
	if (exp) {
		s += exp->toString();
	}
	s += ")";
	return s;
}

String
ExpressionStatement::toSource() const
{
	if (exp) {
		return exp->toSource() + ";";
	} else {
		return ";";
	}
}

IfStatement::IfStatement(Condition *cond, ASTStm *thens, ASTStm *elses)
	: cond(cond)
	, thens(thens)
	, elses(elses)
{
	FUNCLOG;
	assert(cond);
	assert(thens);
}

String
IfStatement::toString() const
{
	String s = "\nIfStatement(";
	s += cond->toString();
	s += ", ";
	s += thens->toString();
	s += ", ";
	if (elses) {
		s += elses->toString();
	}
	s += ")";
	return s;
}

String
IfStatement::toSource() const
{
	String s = "if (";
	s += cond->toSource();
	s += ") ";
	s += thens->toSource();
	if (elses) {
		s += "else ";
		s += elses->toSource();
	}
	return s;
}

SwitchStatement::SwitchStatement(Condition *cond, ASTStm *stm)
	: cond(cond)
	, stm(stm)
{
	FUNCLOG;
	assert(cond);
	assert(stm);
}

String
SwitchStatement::toString() const
{
	String s = "\nSwitchStatement(";
	s += cond->toString();
	s += ", ";
	s += stm->toString();
	s += ")";
	return s;
}

String
SwitchStatement::toSource() const
{
	String s = "switch (";
	s += cond->toSource();
	s += ")";
	s += stm->toSource();
	return s;
}

CaseStatement::CaseStatement(ASTExp *exp, ASTStm *stm)
	: exp(exp)
	, stm(stm)
{
	FUNCLOG;
	//exp == NULL as "default:"
	assert(stm);
}

String
CaseStatement::toString() const
{
	String s = "CaseStatement(";
	if (exp) {
		s += exp->toString();
	} else {
		s += "default ";
	}
	s += ", ";
	s += stm->toString();
	s += ")";
	return s;
}

String
CaseStatement::toSource() const
{
	String s;
	if (exp) {
		s = "case ";
		s += exp->toSource();
	} else {
		s = "default ";
	}
	s += ":\n";
	s += stm->toSource();
	return s;
}

WhileStatement::WhileStatement(Condition *cond, ASTStm *stm)
	: cond(cond)
	, stm(stm)
{
	FUNCLOG;
	assert(cond);
	assert(stm);
}

String
WhileStatement::toString() const
{
	String s = "\nWhileStatement(";
	s += cond->toString();
	s += ", ";
	s += stm->toString();
	s += ")";
	return s;
}

String
WhileStatement::toSource() const
{
	String s;
	s = "while (";
	s += cond->toSource();
	s += ")";
	s += stm->toSource();
	return s;
}

DoStatement::DoStatement(ASTExp *exp, ASTStm *stm)
	: exp(exp)
	, stm(stm)
{
	FUNCLOG;
	assert(exp);
	assert(stm);
}

String
DoStatement::toString() const
{
	String s = "\nDoStatement(";
	s += stm->toString();
	s += ", ";
	s += exp->toString();
	s += ")";
	return s;
}

String
DoStatement::toSource() const
{
	String s;
	s = "do";
	s += stm->toSource();
	s += "while(";
	s += exp->toSource();
	s += ");";
	return s;
}

ForStatement::ForStatement(ForInitStatement *init, Condition *cond, ASTExp *exp, ASTStm *stm)
	: init(init)
	, cond(cond)
	, exp(exp)
	, stm(stm)
{
	FUNCLOG;
	assert(init);
	assert(stm);
}

String
ForStatement::toString() const
{
	String s = "\nForStatement(";
	s += init->toString();
	s += ", ";
	if (cond) {
		s += cond->toString();
	}
	s += ", ";
	if (exp) {
		s += exp->toString();
	}
	s += ", ";
	s += stm->toString();
	s += ")";
	return s;
}

String
ForStatement::toSource() const 
{
	String s;
	s = "for (";
	s += init->toSource();
	if (cond) {
		s += cond->toSource();
	}
	s += ";";
	if (exp) {
		s += exp->toSource();
	}
	s += ")";
	s += stm->toSource();
	return s;
}

ForInitStatement::ForInitStatement(ExpressionStatement *expStm)
	: expStm(expStm)
	, simpleDecl(NULL)
{
	FUNCLOG;
	assert(expStm);
}

ForInitStatement::ForInitStatement(SimpleDeclaration *simpleDecl)
	: expStm(NULL)
	, simpleDecl(simpleDecl)
{
	FUNCLOG;
	assert(simpleDecl);
}

String 
ForInitStatement::toString() const
{
	String s = "ForInitStatement(";
	if (expStm) {
		s += expStm->toString();
	} else if (simpleDecl) {
		s += simpleDecl->toString();
	}
	s += ")";
	return s;
}

String 
ForInitStatement::toSource() const
{
	String s;
	if (expStm) {
		s += expStm->toSource();
	} else if (simpleDecl) {
		s += simpleDecl->toSource();
	}
	return s;
}

JumpStatement::JumpStatement(JumpType type, ASTExp *ret)
	: jumpType(type)
	, ret(ret)
	, label(NULL)
{
	FUNCLOG;
}

JumpStatement::JumpStatement(Symbol *label)
	: jumpType(JumpStatement::GOTO)
	, ret(NULL)
	, label(label)
{
	FUNCLOG;
	assert(label);
}

String
JumpStatement::toString() const
{
	String s = "\nJumpStatement(";
	switch (jumpType) {
	case CONTINUE:
		s += "continue";
		break;
	case BREAK:
		s += "break";
		break;
	case RETURN:
		s += "return ";
		if (ret) {
			s += ret->toString();
		}
		break;
	case GOTO:
		s += "goto ";
		s += label->toString();
		break;
	default:
		break;
	}
	s += ")";
	return s;
}

String
JumpStatement::toSource() const
{
	String s;
	switch (jumpType) {
	case CONTINUE:
		s = "continue;\n";
		break;
	case BREAK:
		s = "break;\n";
		break;
	case RETURN:
		s = "return";
		if (ret) {
			s += " ";
			s += ret->toSource();
		}
		s += ";\n";
		break;
	case GOTO:
		s = "goto";
		s += label->toString();
		s+= ";\n";
		break;
	default:
		break;
	}
	return s;
}

LabelStatement::LabelStatement(Symbol *label, ASTStm *stm)
	: label(label)
	, stm(stm)
{
	FUNCLOG;
	assert(label);
}

String 
LabelStatement::toString() const
{
	String s = "\nLabelStatement(";
	s += label->toString();
	s += ", ";
	s += stm->toString();
	s += ")";
	return s;
}

String 
LabelStatement::toSource() const
{
	String s = label->toString();
	s += ":\n";
	s += stm->toSource();
	return s;
}


DeclarationSeq::DeclarationSeq()
{
	FUNCLOG;
}

String 
DeclarationSeq::toString() const
{
	String s = "DeclarationSeq(";
	s += ::toString(decls, ", ");
	s += ")";
	return s;
}

String 
DeclarationSeq::toSource() const
{
	return ::toSource(decls, " ");
}


Declaration::Declaration(int type)
	: type(type)
{
}

BlockDeclaration::BlockDeclaration()
	: Declaration(BLOCK_DECL)
{
	FUNCLOG;
}

SimpleDeclaration::SimpleDeclaration(DeclSpecifierSeq *declSpecSeq, InitDeclaratorList *initDeclList)
	: BlockDeclaration()
	, declSpecSeq(declSpecSeq)
	, initDeclList(initDeclList)
{
	FUNCLOG;
}

String 
SimpleDeclaration::toString() const
{
	String s = "SimpleDeclaration(";
	if (declSpecSeq) {
		s += declSpecSeq->toString();
	}
	if (initDeclList) {
		if (declSpecSeq) s += ", ";
		s += initDeclList->toString();
	}
	s += ")";
	return s;
}

String 
SimpleDeclaration::toSource() const
{
	String s;
	if (declSpecSeq) {
		s += declSpecSeq->toSource();
		s += " ";
	}
	if (initDeclList) {
		s += initDeclList->toSource();
	}
	s += ";\n";
	return s;
}

DeclSpecifier::DeclSpecifier(DeclSpecifierID declId)
	: declId(declId)
	, typeSpec(NULL)
{
	FUNCLOG;
}

DeclSpecifier::DeclSpecifier(TypeSpecifier *typeSpec)
	: declId(NO_DECL_SPEC)
	, typeSpec(typeSpec)
{
	FUNCLOG;
}

String
DeclSpecifier::toString() const
{
	String s ="DeclSpecifier(";
	if (typeSpec) {
		s += typeSpec->toString();
	} else {
		s += DeclSpecifierString[declId];
	}
	s += ")";
	return s;
}

String
DeclSpecifier::toSource() const
{
	if (typeSpec) {
		return typeSpec->toSource();
	} else {
		return DeclSpecifierString[declId];
	}
}

DeclSpecifierSeq::DeclSpecifierSeq()
	: declSpecs()
{}

String
DeclSpecifierSeq::toString() const
{
	String s ="DeclSpecifierSeq(";
	s += ::toString(declSpecs, ", ");
	s += ")";
	return s;
}

String
DeclSpecifierSeq::toSource() const
{
	return ::toSource(declSpecs, " ");
}

TypeSpecifier::TypeSpecifier(TypeSpecifierID type)
	: typeSpecId(type)
{
	FUNCLOG;
}

String
TypeSpecifier::toString() const
{
	String s = "TypeSpecifier(";
	s += TypeSpecifierString[typeSpecId];
	s += ")";
	return s;
}

String
TypeSpecifier::toSource() const
{
	return TypeSpecifierString[typeSpecId];
}

UserTypeSpecifier::UserTypeSpecifier(Scope *scope, TypeName *name)
	: TypeSpecifier(USER_TYPE)
	, scope(scope)
	, name(name)
{
	FUNCLOG;
	assert(name);
}

String 
UserTypeSpecifier::toString() const
{
	String s = "UserTypeSpecifier(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	s += name->toString();
	s += ")";
	return s;
}

String 
UserTypeSpecifier::toSource() const
{
	String s;
	if (scope) {
		s += scope->toSource();
	}
	s += name->toSource();
	return s;
}

TypeName::TypeName(Symbol *name)
	: name(name)
	, templateId(NULL)
{
	FUNCLOG;
	assert(name);
}

TypeName::TypeName(TemplateId *id)
	: name(NULL)
	, templateId(id)
{
	FUNCLOG;
	assert(id);
}

String 
TypeName::toString() const
{
	String s = "TypeName(";
	if (name) s += name->toString();
	else s += templateId->toString();
	s += ")";
	return s;
}

String 
TypeName::toSource() const
{
	if (name) return name->toString();
	else return templateId->toSource();
}

EnumSpecifier::EnumSpecifier()
	: TypeSpecifier(ENUM)
	, name(NULL)
	, scope(NULL)
	, enumDefs()
{
	FUNCLOG;
}

EnumSpecifier::EnumSpecifier(Symbol *name, Scope *scope)
	: TypeSpecifier(ENUM)
	, name(name)
	, scope(scope)
	, enumDefs()
{
	FUNCLOG;
	assert(name);
}

String 
EnumSpecifier::toString() const
{
	String s = "EnumSpecifier(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	if (name) {
		s += name->toString();
		s += ", ";
	}
	s += ::toString(enumDefs, ", ");
	s += ")";
	return s;
}

String 
EnumSpecifier::toSource() const
{
	String s = "enum ";
	if (scope) {
		s += scope->toSource();
	}
	if (name) {
		s += name->toString();
	}
	s += " {\n";
	s += ::toSource(enumDefs, ",\n");
	s += "\n}";
	return s;
}


EnumeratorDefinition::EnumeratorDefinition(Symbol *name, ASTExp *exp)
	: name(name)
	, exp(exp)
{
	FUNCLOG;
	assert(name);
}

String 
EnumeratorDefinition::toString() const
{
	String s = "EnumeratorDefinition(";
	s += name->toString();
	if (exp) {
		s += ", ";
		s += exp->toString();
	}
	s += ")";
	return s;
}

String 
EnumeratorDefinition::toSource() const
{
	String s = name->toString();
	if (exp) {
		s += " = ";
		s += exp->toSource();
	}
	return s;
}

NamespaceDefinition::NamespaceDefinition(Symbol *name, DeclarationSeq *body)
	: Declaration(NAMESPACE_DEF)
	, name(name)
	, body(body)
{
	FUNCLOG;
}


String 
NamespaceDefinition::toString() const
{
	String s = "NamespaceDefinition(";
	if (name) {
		s += name->toString();
	}
	if (body) {
		s += body->toString();
	}
	s += ")";
	return s;
}

String 
NamespaceDefinition::toSource() const
{
	String s = "namespace ";
	if (name) {
		s += name->toString();
		s += " ";
	}
	s += "{";
	if (body) {
		s += body->toSource();
	}
	s += "}";
	return s;
}


NamespaceAliasDefinition::NamespaceAliasDefinition(Symbol *alias, ScopedId *original)
	: BlockDeclaration()
	, alias(alias)
	, original(original)
{
	FUNCLOG;
	assert(alias);
	assert(original);
}

String 
NamespaceAliasDefinition::toString() const
{
	String s = "NamespaceAliasDefinition(";
	s += alias->toString();
	s += original->toString();
	s += ")";
	return s;
}

String 
NamespaceAliasDefinition::toSource() const
{
	String s = "namespace ";
	s += alias->toString();
	s += " = ";
	s += original->toSource();
	s += ";";
	return s;
}

UsingDeclaration::UsingDeclaration(ScopedId *id, bool typeName)
	: BlockDeclaration()
	, id(id)
	, typeName(typeName)
{
	FUNCLOG;
	assert(id);
}

String 
UsingDeclaration::toString() const
{
	String s = "UsingDeclaration(";
	s += typeName ? "typename, " : "";
	s += id->toString();
	s += ")";
	return s;
}

String 
UsingDeclaration::toSource() const
{
	String s = "using ";
	s += typeName ? "typename, " : "";
	s += "::";
	s += id->toSource();
	s += ";";
	return s;
}

UsingDirective::UsingDirective(ScopedId *namespaceId)
	: BlockDeclaration()
	, namespaceId(namespaceId)
{
	FUNCLOG;
	assert(namespaceId);
}

String 
UsingDirective::toString() const
{
	String s = "UsingDirective(";
	s += namespaceId->toString();
	s += ")";
	return s;
}

String 
UsingDirective::toSource() const
{
	String s = "using namespace ";
	s += namespaceId->toSource() + ";";
	return s;
}


TypeSpecifierSeq::TypeSpecifierSeq()
{
	FUNCLOG;
}

String 
TypeSpecifierSeq::toString() const
{
	String s = "TypeSpecifierSeq(";
	s += ::toString(typeSpecs, ", ");
	s += ")";
	return s;
}

String 
TypeSpecifierSeq::toSource() const
{
	return ::toSource(typeSpecs, " ");
}

TypeId::TypeId(TypeSpecifierSeq *typeSpecSeq, Declarator *absDecl)
	: typeSpecSeq(typeSpecSeq)
	, absDecl(absDecl)
{
	FUNCLOG;
}

String 
TypeId::toString() const
{
	String s = "TypeId(";
	s += typeSpecSeq->toString();
	s += ", ";
	if (absDecl) {
		s += absDecl->toString();
	}
	s += ")";
	return s;
}

String 
TypeId::toSource() const
{
	String s = typeSpecSeq->toSource();
	if (absDecl) {
		s += absDecl->toSource();
	}
	return s;
}

InitDeclarator::InitDeclarator(Declarator *decl, Initializer *init) 
	: decl(decl)
	, initializer(init)
{
	FUNCLOG;
	assert(decl);
}

String
InitDeclarator::toString() const
{
	String s = "InitDeclarator(";
	s += decl->toString();
	if (initializer) {
		s += ", ";
		s += initializer->toString();
	}
	s += ")";
	return s;
}

String
InitDeclarator::toSource() const
{
	String s = decl->toSource();
	if (initializer) {
		s += initializer->toSource();
	}
	return s;
}

InitDeclaratorList::InitDeclaratorList()
{
	FUNCLOG;
}

String
InitDeclaratorList::toString() const
{
	String s = "InitDeclaratorList(";
	s += ::toString(decls, ", ");
	s += ")";
	return s;
}

String
InitDeclaratorList::toSource() const
{
	return ::toSource(decls, ", ");
}

Declarator::Declarator()
	: declId(NULL)
	, subDecl(NULL)
	, ptrs()
{
	FUNCLOG;
}

String 
Declarator::toString() const
{
	String s = "Declarator(";
	s += ::toString(ptrs, ", ");

	if (subDecl) {
		s += ", ";
		s += "(";
		s += subDecl->toString();
		s += ")";
	} else if (declId) {
		s += ", ";
		s += declId->toString();
	}
	
	s += ")";
	return s;
}

String 
Declarator::toSource() const
{
	String s;
	s += ::toSource(ptrs, "");

	if (subDecl) {
		s += "(";
		s += subDecl->toSource();
		s += ")";
	} else if (declId) {
		s += declId->toSource();
	}
	
	return s;
}

Symbol *
Declarator::symbolName()
{
	if (subDecl) {
		return subDecl->symbolName();
	} else if (declId) {
		return Symbol::symbol(declId->id->toSource());
	}
	return NULL;
}

FunctionDeclarator::FunctionDeclarator(ParameterDeclarationList *paramDecls, TypeSpecifierSeq *cvQuals)
	: Declarator()
	, paramDecls(paramDecls)
	, cvQuals(cvQuals)
{
	FUNCLOG;
	//assert(paramDecls);
}

String 
FunctionDeclarator::toString() const
{
	String s = "FunctionDeclarator(";
	s += ::toString(ptrs, ", ");

	if (subDecl) {
		s += "(";
		s += subDecl->toString();
		s += ")";
	} else if (declId) {
		s += declId->toString();
	}

	if (paramDecls) {
		s += paramDecls->toString();
	}
	if (cvQuals) {
		s += cvQuals->toString();
	}
	s += ")";
	return s;
}

String 
FunctionDeclarator::toSource() const
{
	String s;
	s += Declarator::toSource();
	
	s += "(";
	if (paramDecls) {
		s += paramDecls->toSource();
	}
	s += ")";
	if (cvQuals) {
		s += cvQuals->toSource();
	}
	return s;
}

ArrayDeclarator::ArrayDeclarator(ASTExp *constExp, Declarator *next)
	: Declarator()
	, constExp(constExp)
	, next(next)
{
	FUNCLOG;
}

String 
ArrayDeclarator::toString() const
{
	String s = "ArrayDeclarator(";
	s += ::toString(ptrs, ", ");

	if (subDecl) {
		s += "(";
		s += subDecl->toString();
		s += ")";
	} else if (declId) {
		s += declId->toString();
	}

	if (constExp) {
		s += constExp->toString();
	}
	if (next) {
		s += " ";
		s += next->toString();
	}
	s += ")";
	return s;
}

String 
ArrayDeclarator::toSource() const
{
	String s;

	s += Declarator::toSource();

	s += "[";
	if (constExp) {
		s += constExp->toSource();
	}
	s += "]";
	if (next) {
		s += next->toSource();
	}
	return s;
}

PtrOperator::PtrOperator(PtrOperator::PtrType type, TypeSpecifierSeq *cvQuals, Scope *scope)
	: type(type)
	, cvQuals(cvQuals)
	, scope(scope)
{
	FUNCLOG;
}

String 
PtrOperator::toString() const
{
	String s = "PtrOperator(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	if (type == POINTER) s += "pointer";
	else s += "reference";
	
	if (cvQuals) {
		s += ", ";
		s += cvQuals->toString();
	}
	s += ")";
	return s;
}

String 
PtrOperator::toSource() const
{
	String s;
	if (scope) {
		s += scope->toSource();
	}
	if (type == POINTER) s += "*";
	else s += "& ";
	
	if (cvQuals) {
		s += cvQuals->toSource();
		s += " ";
	}
	return s;
}


ParameterDeclarationList::ParameterDeclarationList(bool elipsis)
  : elipsis(elipsis)
{
	FUNCLOG;
}

String 
ParameterDeclarationList::toString() const
{
	String s = "ParameterDeclarationList(";
	s += ::toString(paramDecls, ", ");
	if (elipsis) {
		s += ", ...";
	}
	s += ")";
	return s;
}


String 
ParameterDeclarationList::toSource() const
{
	String s = ::toSource(paramDecls, ", ");
	if (elipsis) {
		s += ", ...";
	}
	return s;
}


ParameterDeclaration::ParameterDeclaration(DeclSpecifierSeq *declSpecSeq, Declarator *decl, ASTExp *assignExp)
	: declSpecSeq(declSpecSeq)
	, decl(decl)
	, assignExp(assignExp)
{
	FUNCLOG;
	assert(declSpecSeq);
}

String
ParameterDeclaration::toString() const
{
	String s = "ParameterDeclaration(";
	s += declSpecSeq->toString();
	if (decl) {
		s += ", ";
		s += decl->toString();
	}
	if (assignExp) {
		s += ", ";
		s += assignExp->toString();
	}
	s += ")";
	return s;
}

String
ParameterDeclaration::toSource() const
{
	String s = declSpecSeq->toSource();
	s += " ";
	if (decl) {
		s += decl->toSource();
	}
	if (assignExp) {
		s += " = ";
		s += assignExp->toSource();
	}
	return s;
}

FunctionDefinition::FunctionDefinition(DeclSpecifierSeq *declSpecSeq, 
									   Declarator *decl, 
									   MemInitializerList *memInits, 
									   FunctionBody *body)
	: Declaration(FUNC_DEF)
	, declSpecSeq(declSpecSeq)
	, decl(decl)
	, memInits(memInits)
	, body(body)
{
	FUNCLOG;
	//In case of ctor, declSpecSeq is NULL.
	//assert(declSpecSeq);
	assert(decl);
	assert(body);
}

String
FunctionDefinition::toString() const
{
	String s = "FunctionDefinition(";
	if (declSpecSeq) {
		s += declSpecSeq->toString();
		s += ", ";
	}
	s += decl->toString();
	if (memInits) {
		s += ", ";
		s += memInits->toString();
	}
	s += ", ";
	s += body->toString();
	s += ")";
	return s;
}

String
FunctionDefinition::toSource() const
{
	String s;
	if (declSpecSeq) {
		s += declSpecSeq->toSource();
		s += " ";
	}
	s += decl->toSource();
	if (memInits) {
		s += memInits->toSource();
	}
	s += body->toSource();
	return s;
}

FunctionBody::FunctionBody(CompoundStatement *stm)
	: stm(stm)
{
	FUNCLOG;
	assert(stm);
}

String
FunctionBody::toString() const
{
	String s = "FunctionBody(";
	s += stm->toString();
	s += ")";
	return s;
}

String
FunctionBody::toSource() const
{
	return stm->toSource();
}

Initializer::Initializer(InitializerClause *initClause)
	: initClause(initClause)
	, exps(NULL)
{
	FUNCLOG;
	assert(initClause);
}

Initializer::Initializer(SequenceExpression *exps)
	: initClause(NULL)
	, exps(exps)
{
	FUNCLOG;
	assert(exps);
}
String 
Initializer::toString() const
{
	String s = "Initializer(";
	if (initClause) {
		s += initClause->toString();
	} else if (exps) {
		s += exps->toString();
	}
	s += ")";
	return s;
}

String 
Initializer::toSource() const
{
	String s;
	if (initClause) {
		s += "=";
		s += initClause->toSource();
	} else if (exps) {
		s += exps->toSource();
	}
	return s;
}

InitializerClause::InitializerClause()
	: assignExp(NULL)
	, list(NULL)
{
	FUNCLOG;
}

InitializerClause::InitializerClause(ASTExp *assignExp)
	: assignExp(assignExp)
	, list(NULL)
{
	FUNCLOG;
	assert(assignExp);
}

InitializerClause::InitializerClause(InitializerList *list)
	: assignExp(NULL)
	, list(list)
{
	FUNCLOG;
	assert(list);
}

String 
InitializerClause::toString() const
{
	String s = "InitializerClause(";
	if (assignExp) {
		s += assignExp->toString();
	} else if (list) {
		s += list->toString();
	}
	s += ")";
	return s;
}

String 
InitializerClause::toSource() const
{
	String s;
	if (assignExp) {
		s += assignExp->toSource();
	} else if (list) {
		s += "{";
		s += list->toSource();
		s += "}";
	}
	return s;
}


InitializerList::InitializerList()
	: clauses()
{
	FUNCLOG;
}

String 
InitializerList::toString() const
{
	String s = "InitializerList(";
	s += ::toString(clauses, ", ");
	s += ")";
	return s;
}

String 
InitializerList::toSource() const
{
	String s;
	s += ::toSource(clauses, ", ");
	return s;
}


ClassSpecifier::ClassSpecifier(TypeName *name, Scope *scope, BaseClause *base, bool clazz)
	: TypeSpecifier(clazz ? CLASS : STRUCT)
	, name(name)
	, scope(scope)
	, base(base)
	, members(NULL)
{
	FUNCLOG;
}

String 
ClassSpecifier::toString() const
{
	String s = "ClassSpecifier(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	if (name) {
		s += name->toString();
		s += ", ";
	}
	if (base) {
		s += base->toString();
		s += ", ";
	}
	if (members) {
		s += members->toString();
	}
	s += ")";
	return s;
}

String 
ClassSpecifier::toSource() const
{
	String s = "class ";
	if (scope) {
		s += scope->toSource();
	}
	if (name) {
		s += name->toSource();
		s += " ";
	}
	if (base) {
		s += base->toSource();
	}
	s += "{\n";
	if (members) {
		s += members->toSource();
	}
	s += "}";
	return s;
}

MemberSpecification::MemberSpecification()
{
	FUNCLOG;
}

String 
MemberSpecification::toString() const
{
	String s = "MemberSpecification(";
	s += ::toString(memDecls, ", ");
	s += ")";
	return s;
}

String 
MemberSpecification::toSource() const
{
	return ::toSource(memDecls, "");
}

void 
MemberSpecification::setAccessSpec(AccessSpecifierID accessId)
{
	List<MemDecl*>::const_iterator it = memDecls.begin();
	while (it != memDecls.end()) {
		MemDecl *decl = *it;
		if (decl->accessId == DEFAULT_ACCESS_SPEC) {
			decl->accessId = accessId;
		} else {
			break;
		}
		++it;
	}
}

MemDecl::MemDecl()
	: accessId(DEFAULT_ACCESS_SPEC)
{
}

MemberDeclaration::MemberDeclaration()
	: MemDecl()
	, declSpecSeq(NULL)
	, memDecls()
{
	FUNCLOG;
}

String 
MemberDeclaration::toString() const
{
	String s = "MemberDeclaration(";
	s += AccessSpecifierString[accessId];
	s += ", ";
	if (declSpecSeq) {
		s += declSpecSeq->toString();
		s += ", ";
	}
    s += ::toString(memDecls, ", ");
	s += ")";
	return s;
}

String 
MemberDeclaration::toSource() const
{
	String s;
	s += AccessSpecifierString[accessId];
	if (accessId != DEFAULT_ACCESS_SPEC) {
		s += ": ";
	}
	if (declSpecSeq) {
		s += declSpecSeq->toSource();
	}
	s += " ";
    s += ::toSource(memDecls, ", ");
	s += ";\n";
	return s;
}

MemberFunctionDefinition::MemberFunctionDefinition(FunctionDefinition *funcDef)
	: MemDecl()
	, funcDef(funcDef)
{
	FUNCLOG;
	assert(funcDef);
}

String 
MemberFunctionDefinition::toString() const
{
	String s = "MemberFunctionDefinition(";
	s += AccessSpecifierString[accessId];
	s += ", ";
	s += funcDef->toString();
	s += ")";
	return s;
}

String 
MemberFunctionDefinition::toSource() const
{
	String s;
	s += AccessSpecifierString[accessId];
	if (accessId != DEFAULT_ACCESS_SPEC) {
		s += ": ";
	}
	s += funcDef->toSource();
	return s;
}

MemberTemplateDeclaration::MemberTemplateDeclaration(TemplateDeclaration *tempDecl)
	: MemDecl()
	, tempDecl(tempDecl)
{
	FUNCLOG;
	assert(tempDecl);
}

String 
MemberTemplateDeclaration::toString() const
{
	String s = "MemberTemplateDeclaration(";
	s += tempDecl->toString();
	s += ")";
	return s;
}

String 
MemberTemplateDeclaration::toSource() const
{
	return tempDecl->toSource();
}


MemberBasicDeclarator::MemberBasicDeclarator(Declarator *decl, bool pure)
	: MemberDeclarator()
	, decl(decl)
	, pureVirtual(pure)
{
	FUNCLOG;
	assert(decl);
}

String 
MemberBasicDeclarator::toString() const
{
	String s = "MemberBasicDeclarator(";
	s += decl->toString();
	if (pureVirtual) {
		s += "(pure virtual)";
	}
	s += ")";
	return s;
}

String 
MemberBasicDeclarator::toSource() const
{
	String s = decl->toSource();
	if (pureVirtual) {
		s += " = 0";
	}
	return s;
}

Symbol *
MemberBasicDeclarator::memberName()
{
	return Symbol::symbol(decl->declId->toSource());
}

MemberConstDeclarator::MemberConstDeclarator(Declarator *decl, ASTExp *constExp)
	: MemberDeclarator()
	, decl(decl)
	, constExp(constExp)
{
	FUNCLOG;
	assert(decl);
	assert(constExp);
}

String 
MemberConstDeclarator::toString() const
{
	String s = "MemberConstDelarator(";
	s += decl->toString();
	s += ", ";
	s += constExp->toString();
	s += ")";
	return s;
}

String 
MemberConstDeclarator::toSource() const
{
	String s = decl->toSource();
	s += " = ";
	s += constExp->toSource();
	return s;
}

Symbol *
MemberConstDeclarator::memberName()
{
	return Symbol::symbol(decl->declId->toSource());
}

MemberBitField::MemberBitField(Symbol *name, ASTExp *constExp)
	: MemberDeclarator()
	, name(name)
	, constExp(constExp)
{
	FUNCLOG;
	assert(constExp);
}

String 
MemberBitField::toString() const
{
	String s = "MemberBitField(";
	if (name) {
		s += name->toString();
		s += ", ";
	}
	s += constExp->toString();
	s += ")";
	return s;
}

String 
MemberBitField::toSource() const
{
	String s;
	if (name) {
		s += name->toString();
	}
	s + ":";
	s += constExp->toSource();
	return s;
}

Symbol *
MemberBitField::memberName()
{
	return name;
}

BaseClause::BaseClause()
	: baseSpecs()
{
	FUNCLOG;
}

String 
BaseClause::toString() const
{
	String s = "BaseClause(";
	s += ::toString(baseSpecs, ", ");
	s += ")";
	return s;
}

String 
BaseClause::toSource() const
{
	return " : " + ::toSource(baseSpecs, ", ");
}


BaseSpecifier::BaseSpecifier(TypeName *name, Scope *scope, AccessSpecifierID accessId, bool virt)
	: name(name)
	, scope(scope)
	, accessId(accessId)
	, virtualInherit(virt)
{
	FUNCLOG;
	assert(name);
}

String 
BaseSpecifier::toString() const
{
	String s = "BaseSpecifier(";
	s += AccessSpecifierString[accessId];
	s += ", ";
	s += virtualInherit ? "virtual, ":"";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	s += name->toString();
	s += ")";
	return s;
}

String 
BaseSpecifier::toSource() const
{
	String s = AccessSpecifierString[accessId];
	s += " ";
	s += virtualInherit ? "virtual ":"";
	if (scope) {
		s += scope->toSource();
	}
	s += name->toSource();
	return s;
}


MemInitializerList::MemInitializerList()
{
	FUNCLOG;
}
String 
MemInitializerList::toString() const
{
	String s = "MemInitializerList(";
	s += ::toString(memInits, ", ");
	s += ")";
	return s;
}

String 
MemInitializerList::toSource() const
{
	return ": " + ::toSource(memInits, ", ");
}

MemInitializer::MemInitializer(TypeName *name, Scope* scope)
	: name(name)
	, scope(scope)
{
	FUNCLOG;
	assert(name);
}

String 
MemInitializer::toString() const
{
	String s = "MemInitializer(";
	if (scope) {
		s += scope->toString();
		s += ", ";
	}
	s += name->toString();
	s += ", ";
	if (exp) {
		s += exp->toString();
	}
	s += ")";
	return s;
}

String 
MemInitializer::toSource() const
{
	String s;
	if (scope) {
		s += scope->toSource();
	}
	s += name->toSource();
	s += "(";
	if (exp) {
		s += exp->toSource();
	}
	s += ")";
	return s;
}


ConversionTypeId::ConversionTypeId()
	: typeSpecSeq(NULL)
	, ptrs()
{
	FUNCLOG;
}

String 
ConversionTypeId::toString() const
{
	String s = "ConversionTypeId(";
	s += typeSpecSeq->toSource();
	s += ", ";
	s += ::toSource(ptrs, ", ");
	s += ")";
	return s;
}

String 
ConversionTypeId::toSource() const
{
	String s = "operator ";
	s += typeSpecSeq->toSource();
	s += ::toSource(ptrs, " ");
	return s;
}

OperatorFunctionId::OperatorFunctionId(Operator op)
	: op(op)
{
	FUNCLOG;
}

String 
OperatorFunctionId::toString() const
{
	String s= "OperatorFunctionId(";
	s += OperatorString[op];
	s += ")";
	return s;
}

String 
OperatorFunctionId::toSource() const
{
	return String("operator") + OperatorString[op];
}


TemplateDeclaration::TemplateDeclaration(TemplateParameterList *params, Declaration *decl, bool expo)
	: Declaration(TEMPLATE_DECL)
	, params(params)
	, decl(decl)
	, expo(expo)
{
	FUNCLOG;
	assert(params);
	assert(decl);
}

String 
TemplateDeclaration::toString() const
{
	String s = "TemplateDeclaration(";
	s += params->toString();
	s += ", ";
	s += decl->toString();
	s += expo ? ", export" : "";
	s += ")";
	return s;
}

String 
TemplateDeclaration::toSource() const
{
	String s = expo ? "export " : "";
	s += "template <";
	s += params->toSource();
	s += "> ";
	s += decl->toSource();
	return s;
}

TemplateParameterList::TemplateParameterList()
	: params()
{
	FUNCLOG;
}

String 
TemplateParameterList::toString() const
{
	String s = "TemplateParameterList(";
	s += ::toString(params, ", ");
	s += ")";
	return s;
}

String 
TemplateParameterList::toSource() const
{
	return ::toSource(params, ", ");
}

TemplateParameter::TemplateParameter(TypeParameter *typeParam)
	: typeParam(typeParam)
	, paramDecl(NULL)
{
	FUNCLOG;
	assert(typeParam);
}

TemplateParameter::TemplateParameter(ParameterDeclaration *paramDecl)
	: typeParam(NULL)
	, paramDecl(paramDecl)
{
	FUNCLOG;
	assert(paramDecl);
}

String 
TemplateParameter::toString() const
{
	String s = "TemplateParameter(";
	if (typeParam) {
		s += typeParam->toString();
	} else {
		s += paramDecl->toString();
	}
	s += ")";
	return s;
}

String 
TemplateParameter::toSource() const
{
	if (typeParam) {
		return typeParam->toSource();
	} else {
		return paramDecl->toSource();
	}
}

TypeParameter::TypeParameter(Symbol *name, TypeId *typeId)
	: name(name)
	, typeId(typeId)
	, params(NULL)
	, idexp(NULL)
{
	FUNCLOG;
}

TypeParameter::TypeParameter(Symbol *name, TemplateParameterList *params, ScopedId *idexp)
	: name(name)
	, typeId(NULL)
	, params(params)
	, idexp(idexp)
{
	FUNCLOG;
}

String 
TypeParameter::toString() const
{
	String s = "TypeParameter(";
	if (name) {
		s += name->toString();
		s += ", ";
	}
	if (typeId) {
		s += typeId->toString();
		s += ", ";
	}
	if (params) {
		s += params->toString();
		s += ", ";
	}
	if (idexp) {
		s += idexp->toString();
	}
	s += ")";
	return s;
}

String 
TypeParameter::toSource() const
{
	String s;
	if (params) {
		s = "template <";
		s += params->toSource();
		s += "> class ";
		if (name) {
			s += name->toString();
		}
		if (idexp) {
			s += " = ";
			s += idexp->toSource();
		}
	} else {
		s = "typename ";
		if (name) {
			s += name->toString();
		}
		if (typeId) {
			s += " = ";
			s += typeId->toSource();
		}
	}
	return s;
}

TemplateId::TemplateId(Symbol *name, TemplateArgumentList *args)
	: name(name)
	, args(args)
{
	FUNCLOG;
	assert(name);
}

String 
TemplateId::toString() const
{
	String s = "TemplateId(";
	s += name->toString();
	if (args) {
		s += ", ";
		s += args->toString();
	}
	s += ")";
	return s;
}

String 
TemplateId::toSource() const
{
	String s = name->toString();
	s += "<";
	if (args) {
		s += args->toSource();
	}
	s += ">";
	return s;
}

TemplateArgumentList::TemplateArgumentList()
 : args()
{
	FUNCLOG;
}

String 
TemplateArgumentList::toString() const
{
	String s = "TemplateArgumentList(";
	s += ::toString(args, ", ");
	s += ")";
	return s;
}

String 
TemplateArgumentList::toSource() const
{
	return ::toSource(args, ", ");
}

TemplateArgument::TemplateArgument(ASTExp *assign)
	: type(ASSIGN_EXP)
	, arg(assign)
{
	FUNCLOG;
	assert(arg);
}

TemplateArgument::TemplateArgument(TypeId *typeId)
	: type(TYPE_ID)
	, arg(typeId)
{
	FUNCLOG;
	assert(arg);
}

TemplateArgument::TemplateArgument(ScopedId *scopedId)
	: type(SCOPED_ID)
	, arg(scopedId)
{
	FUNCLOG;
	assert(arg);
}

String 
TemplateArgument::toString() const
{
	String s = "TemplateArgument(";
	s += arg->toString();
	s += ")";
	return s;
}

String 
TemplateArgument::toSource() const
{
	return arg->toSource();
}

ExplicitInstantiation::ExplicitInstantiation(Declaration *decl)
	: Declaration(EXPLICIT_INST)
	, decl(decl)
{
	FUNCLOG;
	assert(decl);
}

String 
ExplicitInstantiation::toString() const
{
	String s = "ExplicitInstantiation(";
	s += decl->toString();
	s += ")";
	return s;
}

String 
ExplicitInstantiation::toSource() const
{
	return "template " + decl->toSource();
}


ExplicitSpecialization::ExplicitSpecialization(Declaration *decl)
	: Declaration(EXPLICIT_SPECIAL)
	, decl(decl)
{
	FUNCLOG;
	assert(decl);
}

String 
ExplicitSpecialization::toString() const
{
	String s = "ExplicitSpecialization(";
	s += decl->toString();
	s += ")";
	return s;
}

String 
ExplicitSpecialization::toSource() const
{
	return "template <> " + decl->toSource();
}
