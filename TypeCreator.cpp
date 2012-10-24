#include "TypeCreator.hpp"
#include "AST.hpp"
#include "Type.hpp"
#include <cassert>
#include "error.hpp"
#include "TypeCheck.hpp"
#include "SymbolTable.hpp"

#define ENABLE_FUNCLOG
#include "debug.h"


TypeCreator::TypeCreator()
	: resultType(NULL)
	, preCheckedType(NULL)
	, baseType(-1)
	, sign(-1)
	, constant(false)
	, intCount(0)
	, name(NULL)
	, declSpecifierFlags(0)
{
}

int 
TypeCreator::add(TypeCheck *typeCheck, TypeSpecifier *typeSpec)
{
	FUNCLOG;
	DBG("typespec %s", typeSpec->toString().c_str());
	assert(typeSpec);

	switch (typeSpec->typeSpecId) {
	case CHAR:	
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		baseType = CHAR;
		break;

	case BOOL:
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		baseType = BOOL;
		break;

	case SHORT:	
		if (baseType != -1 && baseType != INT) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		baseType = SHORT;
		break;

	case INT:	
		if (baseType != -1) {
			if (baseType != SHORT && 
				baseType != LONG && 
				baseType != LONG_LONG) {
				//two or more base type is specified
				return SE_TWO_OR_MORE_BASETYPE;
			}
		}
		if (baseType != SHORT && 
			baseType != LONG && 
			baseType != LONG_LONG) {
			baseType = INT;	
		}
		//if 'long' or 'short' is found before 'int', we can't find 'int' after that.
		//therefore we should count the number of 'int'.
		++intCount;
		if (1 < intCount) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		break;

	case LONG:
		if (baseType != -1) {
			if (baseType != INT && baseType != LONG) {
				//two or more base type is specified
				return SE_TWO_OR_MORE_BASETYPE;
			}
		}
		//resultType = new IntType(32, sign!=UNSIGNED);
		if (baseType == LONG) {
			baseType = LONG_LONG;
		} else {
			baseType = LONG; 
		}
		break;

	case SIGNED:
		sign = SIGNED;	
		break;

	case UNSIGNED:	
		sign = UNSIGNED;	
		break;

	case VOID:	
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		baseType = VOID;
		break;

	case CONST:
		if (constant) {
			//two or more 'const' is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		constant = true;	
		break;

	case VOLATILE: /*ignore*/
		break;

	case STRUCT: {
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		ClassSpecifier *classspec = (ClassSpecifier*)typeSpec;
		preCheckedType = classspec->accept(typeCheck);

		if (classspec->name->isTemplateId()) {
			//TODO:
			assert(0);
		} else {
			name =  classspec->name->name;
		}
		baseType = STRUCT;
	}
		break;

	case CLASS: {
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		ClassSpecifier *classspec = (ClassSpecifier*)typeSpec;
		preCheckedType = classspec->accept(typeCheck);

		if (classspec->name->isTemplateId()) {
			//TODO:
			assert(0);
		} else {
			name =  classspec->name->name;
		}
		baseType = CLASS;
	}
		break;

	case ENUM: {
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		EnumSpecifier *enumspec = (EnumSpecifier*)typeSpec;
		preCheckedType = enumspec->accept(typeCheck);
		name = enumspec->name;
		baseType = ENUM;
	}
		break;

	case USER_TYPE: {
		if (baseType != -1) {
			//two or more base type is specified
			return SE_TWO_OR_MORE_BASETYPE;
		}
		UserTypeSpecifier *userspec = (UserTypeSpecifier*)typeSpec;
		preCheckedType = userspec->accept(typeCheck);
		assert(preCheckedType);
		if (userspec->name->isTemplateId()) {
			//TODO:
			assert(0);
		} else {
			name =  userspec->name->name;
			assert(name);
		}
		baseType = USER_TYPE;
	}
		break;

	default:
		assert(0);
	}
	return NO_ERROR;
}

int 
TypeCreator::add(TypeCheck *typeCheck, DeclSpecifierID declSpecId)
{
	FUNCLOG;
	DBG("$$$$$ declSpec %d", declSpecId);
	//TODO:
	switch (declSpecId) {
	case AUTO:
	case REGISTER:
	case STATIC:
	case EXTERN:
	case MUTABLE:
	case INLINE:
	case VIRTUAL:
	case EXPLICIT:
	case FRIEND:
	case TYPEDEF:
		declSpecifierFlags |= (1<<declSpecId);
		break;
	default:
		assert(0);
		break;
	}
	return NO_ERROR;
}

int
TypeCreator::getType(Type **result)
{
	if (sign != -1) {
		if (baseType == -1) {
			//in this case(only 'signed' or 'unsigned' is specified), type is set to 'int'.
			baseType = INT;
		} else if (!isInteger(baseType)) {
			//sign specifier only can specify with integer type 
			return SE_SIGN_SPECIFIER;
		}
	}

	switch (baseType) {
	case CHAR:
		resultType = new IntType(8, sign!=UNSIGNED);
		break;

	case BOOL:
		resultType = new IntType(8, false);
		break;

	case SHORT:
		resultType = new IntType(16, sign!=UNSIGNED);
		break;

	case INT:
		resultType = new IntType(32, sign!=UNSIGNED);
		break;

	case LONG:
		resultType = new IntType(32, sign!=UNSIGNED);
		break;

	case LONG_LONG:
		resultType = new IntType(64, sign!=UNSIGNED);
		break;

	case VOID:
		if (constant) {
			//'void' can't be 'const'
			return SE_INVALID_CONST;
		}
		resultType = new VoidType();
		break;

	case STRUCT:
		if (constant) {
			//'struct' can't be 'const'
			return SE_INVALID_CONST;
		}
		resultType = preCheckedType;
		SymbolTable.addType(resultType, name);
		break;

	case CLASS:
		if (constant) {
			//'class' can't be 'const'
			return SE_INVALID_CONST;
		}
		resultType = preCheckedType;
		SymbolTable.addType(resultType, name);
		break;

	case ENUM:
		if (constant) {
			//'enum' can't be 'const'
			return SE_INVALID_CONST;
		}
		resultType = preCheckedType;
		break;

	case USER_TYPE: {
		//Type *binding = SymbolTable.findType(name);
		//resultType = new NamedType(name, binding);
		DBG("find %s", name->toString().c_str());
		resultType = preCheckedType;
	}
		break;

	default:
		//base type is not specified
		resultType = new IncompleteType();
		break;
	}

	assert(resultType);
	resultType->readonly(constant);
	resultType->setFlags(declSpecifierFlags);
	*result = resultType;
	return NO_ERROR;
}

bool 
TypeCreator::isInteger(int type)
{
	return (type == CHAR || 
			type == SHORT || 
			type == INT || 
			type == LONG || 
			type == LONG_LONG);
}
