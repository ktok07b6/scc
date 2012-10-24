#ifndef __TYPE_CREATOR_HPP__
#define __TYPE_CREATOR_HPP__

#include "defs.hpp"

struct TypeSpecifier;
class Type;
class TypeCheck;
class Symbol;

class TypeCreator
{
public:
	TypeCreator();
	int add(TypeCheck *typeCheck, TypeSpecifier *typeSpec);
	int add(TypeCheck *typeCheck, DeclSpecifierID declSpecId);
	int getType(Type **);

private:
	bool isInteger(int type);
	Type *resultType;
	Type *preCheckedType;
	int baseType;
	int sign;
	bool constant;
	int intCount;
	Symbol *name;
	unsigned int declSpecifierFlags;
};
#endif
