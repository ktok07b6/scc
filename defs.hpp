#ifndef __DEFS_HPP__
#define __DEFS_HPP__

#include "String.hpp"

enum DeclSpecifierID {
	NO_DECL_SPEC,
	AUTO,
	REGISTER,
	STATIC,
	EXTERN,
	MUTABLE,
	
	INLINE,
	VIRTUAL,
	EXPLICIT,

	FRIEND,
	TYPEDEF,
	PURE_VIRTUAL,
};
extern const char *DeclSpecifierString[];

enum CVQualifierID {
	NO_TYPE_QUAL,
	//CONST,
	//VOLATILE
};
extern const char *CVQualifierString[];

enum TypeSpecifierID {
	CHAR,
	BOOL,
	SHORT,
	INT, 
	LONG,
	LONG_LONG,
	SIGNED,
	UNSIGNED,
	VOID,
	CONST,
	VOLATILE,
	STRUCT,
	CLASS,
	ENUM,
	
	USER_TYPE,
	TEMPLATE_TYPE,
	MAX_TYPE_SPECIFIER_ID
};
extern const char *TypeSpecifierString[];

enum AccessSpecifierID {
	DEFAULT_ACCESS_SPEC,
	PRIVATE,
	PROTECTED,
	PUBLIC
};
extern const char *AccessSpecifierString[];

enum JumpType {
	BREAK,
	CONTINUE,
	RETURN,
	GOTO
};
extern const char *JumpTypeString[];

enum CastType {
	STATIC_CAST,
	DYNAMIC_CAST,
	REINTERPRET_CAST,
	CONST_CAST,
	C_CAST
};
extern const char *CastTypeString[];

enum Operator {
	PRE_INC, PRE_DEC, 
	POST_INC, POST_DEC,
	UNARY_PLUS, UNARY_MINUS,
	UNARY_STAR, UNARY_AND,
	BANG, TILDE,

	STAR, SLASH, PERCENT,
	PLUS, MINUS,
	LSHIFT, RSHIFT,
	BIT_OR, BIT_XOR, BIT_AND,

	EQ, NE,	LT, LE, GT, GE,

	AND, XOR, OR,

	ASSIGN,
	MUL_ASSIGN,
	DIV_ASSIGN,
	MOD_ASSIGN,
	ADD_ASSIGN,
	SUB_ASSIGN, 
	LSHIFT_ASSIGN,
	RSHIFT_ASSIGN,
	AND_ASSIGN,
	XOR_ASSIGN,
	OR_ASSIGN,
	NOT_ASSIGN,

	CONDITIONAL, //exp ? exp : exp
	FUNC_CALL,   //ident(exp_list)
	CTOR_CALL,   //int(exp)
	SUBSCRIPT,   //x[y]
	MEMBER_SELECT,//x.y
	ARROW, //x->y
	ARROW_STAR,

	PARENTHESIS, //operator ()
	BRACKETS, //operator []
	CAST, 
	COMMA,

	SCOPE, //::
	ELIPSIS, //...
	PRIMARY_ID,
	PRIMARY_INT,
	PRIMARY_BOOL,
	PRIMARY_STR,
	PRIMARY_EXP,
	SEQ
};

enum OperatorPriority {
	MULTIPLICATIVE_PRIO,
	ADDTIVE_PRIO,
	SHIFT_PRIO,
	RELATION_PRIO,
	EQUALITY_PRIO,
	AND_PRIO,
	XOR_PRIO,
	OR_PRIO,
	LOG_AND_PRIO,
	LOG_OR_PRIO
};

extern const char * OperatorString[];

#endif
