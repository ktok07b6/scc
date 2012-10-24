#include "Type.hpp"
#include <cassert>
#define ENABLE_FUNCLOG
#include "debug.h"
#include "defs.hpp"

bool equalTypeList(const List<Type*> &l1, const List<Type*> &l2)
{
	if (l1.size() != l2.size()) {
		return false;
	}
	bool equal_all_arg = true;
	List<Type*>::const_iterator it1 =  l1.begin();
	List<Type*>::const_iterator it2 =  l2.begin();
	while (it1 != l1.end()) {
		Type *t1 = *it1;
		Type *t2 = *it2;
		if (!t1->equal(t2)) {
			equal_all_arg = false;
			break;
		}
		++it1;
		++it2;
	}
	return equal_all_arg;
}

String 
Type::declSpecifierString() const
{
	String s;
	//FIXME: max index
	for (int i = 0; i < 12; ++i) {
		if (hasDeclSpec(static_cast<DeclSpecifierID>(i))) {
			s += DeclSpecifierString[i];
			s += ", ";
		}
	}
	return s;
}

TypeList::TypeList()
	: Type(TYPELIST_T)
{
}

bool 
TypeList::coerceTo(Type *t) const
{
	if (t->is(Type::TYPELIST_T)) {
		TypeList *other = t->as<TypeList>();

		List<Type*>::const_iterator it1 = list.begin();
		List<Type*>::const_iterator it2 = other->list.begin();
		while (it1 != list.end() && it2 != other->list.end()) {
			Type *t1 = *it1;
			Type *t2 = *it2;
			if (!t1->coerceTo(t2)) {
				return false;
			}
			++it1;
			++it2;
		}
	} else {
		List<Type*>::const_iterator it = list.begin();
		while (it != list.end()) {
			Type *t1 = *it;
			if (!t1->coerceTo(t)) {
				return false;
			}
			++it;
		}
	}
	return true;
}

bool TypeList::equal(Type *t) const
{
	if (!t->is(Type::TYPELIST_T)) return false;
	TypeList *other = t->as<TypeList>();
	return equalTypeList(list, other->list);
}

bool TypeList::contain(Type *t) const
{
	List<Type*>::const_iterator it = list.begin();
	while (it != list.end()) {
		Type *t1 = *it;
		DBG("$$$$ %s equal? %s", t1->toString().c_str(), t->toString().c_str());
		if (t1->equal(t)) {
			return true;
		}
		++it;
	}
	return false;
}

String
TypeList::toString() const
{
	String s = "TypeList(";
	List<Type*>::const_iterator it =  list.begin();
	while (it != list.end()) {
		Type *t = *it;
		s += t->toString() + ", ";
		++it;
	}
	s += ")";
	return s;
}

String
TypeList::mangledString() const
{
	String s;
	List<Type*>::const_iterator it =  list.begin();
	while (it != list.end()) {
		Type *t = *it;
		s += t->mangledString() + "_";
		++it;
	}
	return s;
}

void 
TypeList::merge(TypeList *tl)
{
	List<Type*>::const_iterator it = tl->list.begin();
	while (it != tl->list.end()) {
		Type *t = *it;
		if (!contain(t)) {
			list.push_back(t);
		}
		++it;
	}
}

IntType::IntType(int bitWidth, bool sign)
	: Type(INT_T)
	, bitWidth(bitWidth)
	, sign(sign)
	, nullable(false)
{
}

bool 
IntType::coerceTo(Type *t) const
{
	if (t->is(Type::INT_T)) {
		return true;
	} else 	if (t->is(Type::ENUM_T)) {
		return true;
	}
	return false;
}

bool 
IntType::equal(Type *t) const
{
	if (!t->is(Type::INT_T)) return false;
	IntType *intt = t->as<IntType>();
	return (bitWidth == intt->bitWidth &&
			sign == intt->sign);
}

String
IntType::toString() const
{
	char buf[32];
	sprintf(buf, "IntType(%d", bitWidth);
	String s = buf;
	if (rdonly) {
		s += ", const";
	}
	if (sign) {
		s += ", signed";
	} else {
		s += ", unsigned";
	}
	s += ")";
	return s;
}

String
IntType::mangledString() const
{
	char buf[32];
	sprintf(buf, "%sI%d%s", rdonly ? "<c>":"", bitWidth, sign?"S":"U");
	String s = buf;
	return s;
}

VoidType::VoidType()
	: Type(VOID_T) 
{
}

bool 
VoidType::coerceTo(Type *t) const
{
	if (t->is(Type::VOID_T)) {
		return true;
	}
	return false;
}

String
VoidType::toString() const
{
	return "VoidType";
}

String
VoidType::mangledString() const
{
	return "V";
}

PointerType::PointerType() 
	: Type(POINTER_T) 
	, reference(NULL)
{
}


bool 
PointerType::coerceTo(Type *t) const
{
	if (t->is(Type::POINTER_T)) {
		PointerType *other = t->as<PointerType>();
		return reference->coerceTo(other->reference);
	}
	if (t->is(Type::INT_T)) {
		return t->as<IntType>()->nullable;
	}
	//convert from ArrayType
	if (t->is(Type::ARRAY_T)) {
		Type *elem = t->as<ArrayType>()->element;
		return reference->coerceTo(elem);
	}

	return false;
}

bool 
PointerType::equal(Type *t) const
{
	if (!t->is(Type::POINTER_T)) return false;

	PointerType *pt = t->as<PointerType>();
	return reference->equal(pt->reference);
}

String
PointerType::toString() const
{
	//TODO:
	String s = "PointerType";
	if (rdonly) {
		s += "(const)->"; 
	} else {
		s += "()->"; 
	}
	s += reference->toString();
	return s;
}

String
PointerType::mangledString() const
{
	String s = rdonly ? "c_P" : "P";
	s += reference->mangledString();
	return s;
}

ClassType::ClassType(Symbol *name, bool clazz) 
	: Type(CLASS_T)
	, name(name) 
	, clazz(clazz)
{
}

bool 
ClassType::coerceTo(Type *t) const
{
	if (t->is(Type::INITIALIZER_T)) {
		InitializerType *initType = t->as<InitializerType>();
		if (fields.list.size() < initType->subTypes.list.size()) {
			return false;
		}
		return fields.coerceTo(&initType->subTypes);
	}
	return false;
}

bool 
ClassType::equal(Type *t) const
{
	if (!t->is(Type::CLASS_T)) return false;

	ClassType *ct = t->as<ClassType>();
	//TODO: namespace check
	return name == ct->name;
}

String
ClassType::toString() const
{
	String s = "ClassType(";
	s += name->toString();
	s += ", ";
	s += methods.toString();
	s += ", ";
	s += fields.toString();
	s += ")";
	return s;
}

String
ClassType::mangledString() const
{
	//TODO: scope
	String s = name->toString();
	return s;
}

Type *
ClassType::findMember(Symbol *findname)
{
	DBG("findMember %s in %s", findname->toString().c_str(), name->toString().c_str());
	List<Type*>::const_iterator it = methods.list.begin();
	while (it != methods.list.end()) {
		Type *t = *it;
		MethodType *mt = t->as<MethodType>();
		assert(mt);
		if (mt->name == findname) {
			return mt;
		}
		++it;
	}

	it = fields.list.begin();
	while (it != fields.list.end()) {
		Type *t = *it;
		FieldType *ft = t->as<FieldType>();
		assert(ft);
		if (ft->name == findname) {
			return ft;
		}
		++it;
	}
	return NULL;
}

void
ClassType::addMethod(MethodType *mt)
{
	FUNCLOG;
	mt->parent = this;
	if (!methods.contain(mt)) {
		methods.list.push_back(mt);
	} else {
		DBG("already added method");
	}
}

void
ClassType::addField(FieldType *ft)
{
	ft->parent = this;
	if (!fields.contain(ft)) {
		fields.list.push_back(ft);
	}
}

EnumType::EnumType(Symbol *name) 
	: Type(ENUM_T)
	, name(name) 
{
}

bool 
EnumType::coerceTo(Type *t) const
{
	if (t == this) {
		return true;
	}
	return false;
}

bool 
EnumType::equal(Type *t) const
{
	if (!t->is(Type::ENUM_T)) return false;

	EnumType *et = t->as<EnumType>();
	//TODO: namespace check
	return name == et->name;
}

String
EnumType::toString() const
{
	String s = "EnumType(";
	s += name->toString();
	s += ")";
	return s;
}

String
EnumType::mangledString() const
{
	return "E" + name->toString();
}

bool 
EnumType::canImplicitCast(Type *t) const
{
	if (t->is(Type::INT_T)) {
		return true;
	}
	return Type::canImplicitCast(t);
}

void 
EnumType::addMember(Symbol *sym)
{
	members.push_back(sym);
}

ArrayType::ArrayType(Type *elem, int size) 
	: Type(ARRAY_T)
	, element(elem)
	, size(size) 
{
}

bool 
ArrayType::coerceTo(Type *t) const
{
	if (t->is(Type::INITIALIZER_T)) {
		InitializerType *initType = t->as<InitializerType>();
		if (size) {
			if (size < initType->subTypes.list.size()) {
				return false;
			}
		} else {
			size = initType->subTypes.list.size();
		}
		
		return initType->subTypes.coerceTo(element);
	}
	//TODO:
	return false;
}

bool 
ArrayType::equal(Type *t) const
{
	if (!t->is(Type::ARRAY_T)) return false;

	ArrayType *at = t->as<ArrayType>();
	if (size != at->size) return false;

	return element->equal(at->element);
}

String
ArrayType::toString() const
{
	String s = "ArrayType(";
	s += element->toString() + ", ";

	char buf[32];
	sprintf(buf, "%d", size);
	s += buf;
	s += ")";
	return s;
}

String
ArrayType::mangledString() const
{
	String s = element->mangledString()+"[]";
	return s;
}

bool 
ArrayType::canImplicitCast(Type *t) const
{
	//TODO:
	return Type::canImplicitCast(t);
}

AliasType::AliasType(Symbol *alias, Type *binding) 
	: Type(ALIAS_T)
	, alias(alias)
	, binding(binding) 
{
}

bool 
AliasType::coerceTo(Type *t) const
{
	return 	binding->coerceTo(t);
}

bool 
AliasType::equal(Type *t) const
{
	if (!t->is(Type::ALIAS_T)) return false;

	AliasType *at = t->as<AliasType>();
	if (alias != at->alias) return false;
	
	return binding->equal(at->binding);
}

String
AliasType::toString() const
{
	String s = "AliasType";
	s += "(" + alias->toString() + ")";
	s += "->" + binding->toString();
	return s;
}

String
AliasType::mangledString() const
{
	return binding->mangledString();
}


FuncType::FuncType(Symbol *name, Type *ret) 
	: Type(FUNC_T)
	, name(name)
	, returnType(ret) 
	, nextOverload(NULL)
	, hasBody(false)
{
}

//for MethodType
FuncType::FuncType(Symbol *name, Type *ret, ID id) 
	: Type(id)
	, name(name)
	, returnType(ret) 
	, nextOverload(NULL)
{
}

bool 
FuncType::coerceTo(Type *t) const
{
	return returnType->coerceTo(t);
}

bool 
FuncType::equal(Type *t) const
{
	if (!t->is(Type::FUNC_T)) return false;

	FuncType *ft = t->as<FuncType>();
	//TODO: check namespace
	if (name != ft->name) return false;
	
	return args.equal(&ft->args);
}

String
FuncType::toString() const
{
	String s = "FuncType(";
	s += "\"" + name->toString() + "\"";
	s += ", ";
	s += "\"" + genMangledSymbol()->toString() + "\"";
	s += ", ";
	if (rdonly) {
		s += "const, ";
	}
	s += declSpecifierString();
	s += returnType->toString() + ", ";
	s += args.toString();
	s+= ")";
	return s;
}

String
FuncType::mangledString() const
{
	String s = "F" + name->toString();
	return s;
}

void 
FuncType::addOverload(FuncType *overload)
{
	FuncType *ft = this;
	while (ft->nextOverload) {
		ft = ft->nextOverload;
	}
	ft->nextOverload = overload;
}

void 
FuncType::mergeParams(FuncType *ft)
{
	args.merge(&ft->args);
}

Symbol *
FuncType::genMangledSymbol() const
{
	String s;
	s = name->toString() + "_";
	s += returnType->mangledString() + "_";
	s += args.mangledString();
	return Symbol::symbol(s);
}

MethodType::MethodType(Symbol *name, Type *ret)
	: FuncType(name, ret, METHOD_T)
{
}

bool 
MethodType::equal(Type *t) const
{
	if (!t->is(Type::METHOD_T)) return false;

	MethodType *mt = t->as<MethodType>();
	//TODO: check namespace
	if (name != mt->name) return false;
	if (parent != mt->parent) return false;
	return args.equal(&mt->args);
}

String 
MethodType::toString() const
{
	String s = "MethodType";
	if (rdonly) {
		s += "(const, ";
	} else {
		s += "(";
	}
	s += declSpecifierString();
	s += returnType->toString() + ", ";
	s += args.toString();
	s+= ")";
	return s;
}

String 
MethodType::mangledString() const
{
	String s = "MF" + name->toString();
	return s;
}

MethodType *
MethodType::fromFuncType(FuncType *ft)
{
	MethodType *mt = new MethodType(ft->name, ft->returnType);
	//TODO:
	DBG("ft %u", ft->getFlags());
	DBG("%s", ft->toString().c_str());
	
	mt->setFlags(ft->getFlags());
	mt->args.list = ft->args.list;
	mt->nextOverload = ft->nextOverload;
	mt->parent = NULL;
	return mt;
}

FieldType::FieldType(Symbol *name, Type *binding) 
	: Type(FIELD_T)
	, name(name)
	, binding(binding) 
{
}

bool 
FieldType::coerceTo(Type *t) const
{
	return 	binding->coerceTo(t);
}

bool 
FieldType::equal(Type *t) const
{
	if (!t->is(Type::FIELD_T)) return false;

	FieldType *ft = t->as<FieldType>();
	if (name != ft->name) return false;
	
	return binding->equal(ft->binding);
}

String
FieldType::toString() const
{
	String s = "FieldType";
	s += "(\"" + name->toString() + "\")";
	s += "->" + binding->toString();
	return s;
}

String
FieldType::mangledString() const
{
	String s = "M" + binding->mangledString();
	return s;
}

TemplateParamType::TemplateParamType(Symbol *name) 
	: Type(TEMPLATE_PARAM_T) 
{
}

bool 
TemplateParamType::coerceTo(Type *t) const
{
	//TODO:
	return false;
}

bool 
TemplateParamType::equal(Type *t) const
{
	if (!t->is(Type::TEMPLATE_PARAM_T)) return false;

	TemplateParamType *tt = t->as<TemplateParamType>();
	if (name != tt->name) return false;

	return binding->equal(tt->binding);
}

String
TemplateParamType::toString() const
{
	//TODO:
	return "TemplateParamType";
}

TemplateClassType::TemplateClassType(Symbol *name) 
	: Type(TEMPLATE_CLASS_T)
	, name(name) 
{
}

bool 
TemplateClassType::coerceTo(Type *t) const
{
	//TODO:
	return false;
}

bool 
TemplateClassType::equal(Type *t) const
{
	if (!t->is(Type::TEMPLATE_CLASS_T)) return false;

	TemplateClassType *tt = t->as<TemplateClassType>();
	//TODO: check name space
	if (name != tt->name) return false;

	//TODO: template specilization
	return typeParams.equal(&tt->typeParams);
}

String
TemplateClassType::toString() const
{
	//TODO:
	return "TemplateClassType";
}

String
TemplateClassType::mangledString() const
{
	//TODO:
	return "TC";
}

TemplateFuncType::TemplateFuncType(Symbol *name, Type *ret) 
	: Type(TEMPLATE_FUNC_T)
	, name(name)
	, returnType(ret) 
{
}

bool 
TemplateFuncType::coerceTo(Type *t) const
{
	//TODO:
	return false;
}

bool 
TemplateFuncType::equal(Type *t) const
{
	if (!t->is(Type::TEMPLATE_FUNC_T)) return false;

	TemplateFuncType *tt = t->as<TemplateFuncType>();
	//TODO: check name space
	if (name != tt->name) return false;

	//TODO: template specilization
	if (!typeParams.equal(&tt->typeParams)) {
		return false;
	}
	return args.equal(&tt->args);
}

String
TemplateFuncType::toString() const
{
	//TODO:
	return "TemplateFuncType";
}

String
TemplateFuncType::mangledString() const
{
	//TODO:
	return "TF";
}

InitializerType::InitializerType()
	: Type(INITIALIZER_T)
{
}

String 
InitializerType::toString() const
{
	//TODO:
	return "InitializerType";
}

IncompleteType::IncompleteType()
	: Type(INCOMPLETE_T)
{
}

String
IncompleteType::toString() const
{
	//TODO:
	return "IncompleteType";
}
