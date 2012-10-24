#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include "List.hpp"
#include "ref.hpp"
#include "Symbol.hpp"
#include "defs.hpp"

class Type
{
public:
	enum ID {
		INT_T,
		VOID_T,
		POINTER_T,
		CLASS_T,
		ENUM_T,
		ARRAY_T,
		ALIAS_T,
		FUNC_T,
		METHOD_T,
		FIELD_T,
		TEMPLATE_PARAM_T,
		TEMPLATE_CLASS_T,
		TEMPLATE_FUNC_T,
		INITIALIZER_T,
		INCOMPLETE_T,
		TYPELIST_T
	};

	Type(ID id) : id(id), rdonly(false), declSpecifierFlags(0) {}

	const ID id;
	virtual Type *actual() {return this;}
	virtual bool coerceTo(Type *t) const {return false;}
	virtual bool equal(Type *t) const {return id == t->id;}
	virtual String toString() const = 0;
	virtual String mangledString() const {return "";}
	virtual bool canImplicitCast(Type *t) const {
		return (t->is(this->id));
	}

	void readonly(bool b) {
		rdonly = b;
	}
	bool isReadonly() const {
		return rdonly;
	}
	void setFlags(unsigned int flags) {
		declSpecifierFlags = flags;
	}
	unsigned int getFlags() {
		return declSpecifierFlags;
	}
	void declSpec(DeclSpecifierID id) {
		declSpecifierFlags |= (1<<id);
	}
	bool hasDeclSpec(DeclSpecifierID id) const {
		return declSpecifierFlags & (1<<id);
	}

	//void isReadOnly
	bool is(ID id) const {
		return this->id == id;
	}

	template<typename T>
	T *as() {
		if (T::tid == (int)this->id) {
			return static_cast<T*>(this);
		} else {
			if ((ID)T::tid == FUNC_T && this->id == METHOD_T) {
				return static_cast<T*>(this);
			}
			return NULL;
		}
	}

	template<typename T>
	const T *as() const{
		if (T::tid == (int)this->id) {
			return static_cast<const T*>(this);
		} else {
			return NULL;
		}
	}

	String declSpecifierString() const;
	bool rdonly;
	unsigned int declSpecifierFlags;
};

class TypeList : public Type
{
public:
	enum {tid = TYPELIST_T};

	TypeList();
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	bool contain(Type *t) const;
	void merge(TypeList *tl);

	List<Type*> list;
};


class IntType : public Type
{
public:
	enum {tid = INT_T};

	IntType(int bitWidth, bool sign);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	void setBitWidth(int width) {
		bitWidth = width;
	}

	void setSign(bool b) {
		sign = b;
	}

	void setNullable(bool b) {
		nullable = b;
	}
	int bitWidth;
	bool sign;
	bool nullable;
};

class VoidType : public Type
{
public:
	enum {tid = VOID_T};

	VoidType();
	virtual bool coerceTo(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
};

class PointerType : public Type
{
public:
	enum {tid = POINTER_T};

	PointerType();
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;

	Type *reference;
};

class MethodType;
class FieldType;

class ClassType : public Type
{
public:
	enum {tid = CLASS_T};

	ClassType(Symbol *name, bool clazz);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	//void getMethods(Symbol *name, List< Type * > &methods);
	Type *findMember(Symbol *findname);
	void addMethod(MethodType *mt);
	void addField(FieldType *ft);
	Symbol *name;
	TypeList methods;
	TypeList fields;
	bool clazz;
};

class EnumType : public Type
{
public:
	enum {tid = ENUM_T};

	EnumType(Symbol *name);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	virtual bool canImplicitCast(Type *t) const;
	void addMember(Symbol *sym);

	Symbol *name;
	List<Symbol*> members;
};

class ArrayType : public Type
{
public:
	enum {tid = ARRAY_T};

	ArrayType(Type *elem, int size);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	virtual bool canImplicitCast(Type *t) const;

	Type *element;
	mutable int size;
};

//'typedef'ed type
class AliasType : public Type
{
public:
	enum {tid = ALIAS_T};

	AliasType(Symbol *name, Type *binding);
	virtual Type *actual() {return binding->actual();}
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;

	Symbol *alias;
	Type *binding;
};

class FuncType : public Type
{
public:
	enum {tid = FUNC_T};

	FuncType(Symbol *name, Type *ret);
	virtual Type * actual() {return returnType->actual();}
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	void addOverload(FuncType *ft);
	void mergeParams(FuncType *ft);
	Symbol *genMangledSymbol() const;
	
	Symbol *name;
	Type *returnType;
	TypeList args;
	FuncType *nextOverload;
	bool hasBody;
protected:
	FuncType(Symbol *name, Type *ret, ID id);
};

class MethodType : public FuncType
{
public:
	enum {tid = METHOD_T};

	MethodType(Symbol *name, Type *ret);
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;
	static MethodType *fromFuncType(FuncType *);

	ClassType *parent;
	AccessSpecifierID access;
};

class FieldType : public Type
{
public:
	enum {tid = FIELD_T};

	FieldType(Symbol *name, Type *binding);
	virtual Type *actual() {return binding->actual();}
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;

	ClassType *parent;
	Symbol *name;
	Type *binding;
	AccessSpecifierID access;
};


class TemplateParamType : public Type
{
public:
	enum {tid = TEMPLATE_PARAM_T};

	TemplateParamType(Symbol *name);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;

	Symbol *name;//normally "T", "U" etc...
	Type *binding;
};

class TemplateClassType : public Type
{
public:
	enum {tid = TEMPLATE_CLASS_T};

	TemplateClassType(Symbol *name);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;

	Symbol *name;
	TypeList typeParams;
	TypeList members;
};

class TemplateFuncType : public Type
{
public:
	enum {tid = TEMPLATE_FUNC_T};
	TemplateFuncType(Symbol *name, Type *ret);
	virtual bool coerceTo(Type *t) const;
	virtual bool equal(Type *t) const;
	virtual String toString() const;
	virtual String mangledString() const;

	Symbol *name;
	TypeList typeParams;
	Type *returnType;
	TypeList args;
};

class InitializerType : public Type
{
public:
	enum {tid = INITIALIZER_T};

	InitializerType();
	virtual String toString() const;

	TypeList subTypes;
};

class IncompleteType : public Type
{
public:
	enum {tid = INCOMPLETE_T};

	IncompleteType();
	virtual String toString() const;
};

/*
extern IntType *int8Type;
extern IntType *int16Type;
extern IntType *int32Type;
extern IntType *int64Type;
extern IntType *uint8Type;
extern IntType *uint16Type;
extern IntType *uint32Type;
extern IntType *uint64Type;
extern VoidType *voidType;
*/

#endif
