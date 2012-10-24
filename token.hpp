#ifndef __TOKEN_HPP__
#define __TOKEN_HPP__

#include "String.hpp"
#include "defs.hpp"
#include "ref.hpp"
#include "List.hpp"
#include <cassert>
#include "parserUtils.h"

class TokenValue
{
public:
	TokenValue(const String &s) : s(s) {}
	~TokenValue() {}
	virtual Operator toOp() { assert(0);  return (Operator)0; }
	virtual int toInt() { assert(0); return 0;}
	virtual const String &toString() { return s;}

	virtual bool isOp() { return false; }
	virtual bool isInt() { return false; }
	virtual bool isString() { return false; }
private:
	const String s;
};


class TokenOp : public TokenValue
{
public:
	TokenOp(const String &s, Operator op) : TokenValue(s), op(op) {}
	virtual Operator toOp() { return op; }
	virtual bool isOp() { return true; }
private:	
	const Operator op;
};
class TokenInt : public TokenValue
{
public:
	TokenInt(const String &s, int i) : TokenValue(s), i(i) {}
	virtual int toInt() { return i; }
	virtual bool isInt() { return true; }
private:	
	const int i;
};
class TokenString : public TokenValue
{
public:
	TokenString(const String &s) : TokenValue(s) {}
	virtual bool isString() { return true; }
private:	
	
};

extern String lexstr;
extern ref<TokenValue> lexval;

//follow bits
enum {
	PRIMARY_EXP_FOLLOW        =   0x10000,
	POSTFIX_EXP_FOLLOW        =   0x20000,
	UNARY_EXP_FOLLOW          =   0x40000,
	UNARY_OP_FOLLOW           =   0x80000,
	ASSIGN_OP_FOLOW           =  0x100000,
	SELECTION_STM_FOLLOW      =  0x200000,
	ITERATION_STM_FOLLOW      =  0x400000,
	STORAGE_CLASS_SPEC_FOLLOW =  0x800000,
	FUNC_SPEC_FOLLOW          = 0x1000000,
	TYPE_SPEC_FOLLOW          = 0x2000000,
	DECL_SPEC_FOLLOW          = 0x4000000,
	EXP_FOLLOW_MASK           = PRIMARY_EXP_FOLLOW | POSTFIX_EXP_FOLLOW | UNARY_EXP_FOLLOW,
	DECL_SPEC_FOLLOW_MASK     = TYPE_SPEC_FOLLOW | DECL_SPEC_FOLLOW,
};
/* type-spec-follow
  class struct  enum typename
  const  volatile
  ::
  identifier
  builtin(char int ...)

  - decl-spec-follow
  type-spec-follow
  auto register static extern mutable
  inline virtual explicit
  friend typedef
  
*/
enum {
	_ERROR            = 0x01,
	_DECIMAL_CONSTANT = 0x02 | PRIMARY_EXP_FOLLOW,
	_OCTAL_CONSTANT   = 0x03 | PRIMARY_EXP_FOLLOW,
	_HEX_CONSTANT     = 0x04 | PRIMARY_EXP_FOLLOW,

	_LSHIFT        = 0x05,
	_RSHIFT        = 0x06,
	_INC		   = 0x07 | UNARY_EXP_FOLLOW,
	_DEC		   = 0x08 | UNARY_EXP_FOLLOW,
	_AND		   = 0x09,
	_XOR		   = 0x0a,
	_OR			   = 0x0b,
	_ASSIGN		   = 0x0c | ASSIGN_OP_FOLOW,
	_ADD_ASSIGN	   = 0x0d | ASSIGN_OP_FOLOW,
	_SUB_ASSIGN	   = 0x0e | ASSIGN_OP_FOLOW,
	_MUL_ASSIGN	   = 0x0f | ASSIGN_OP_FOLOW,
	_DIV_ASSIGN	   = 0x10 | ASSIGN_OP_FOLOW,
	_MOD_ASSIGN	   = 0x11 | ASSIGN_OP_FOLOW,
	_LSHIFT_ASSIGN = 0x12 | ASSIGN_OP_FOLOW,
	_RSHIFT_ASSIGN = 0x13 | ASSIGN_OP_FOLOW,
	_AND_ASSIGN	   = 0x14 | ASSIGN_OP_FOLOW,
	_XOR_ASSIGN	   = 0x15 | ASSIGN_OP_FOLOW,
	_OR_ASSIGN	   = 0x16 | ASSIGN_OP_FOLOW,
	_NOT_ASSIGN	   = 0x17 | ASSIGN_OP_FOLOW,

	_EQ		 = 0x18,
	_NE		 = 0x19,
	_LT		 = 0x1a,
	_LE		 = 0x1b,
	_GT		 = 0x1c,
	_GE		 = 0x1d,
	_PLUS	 = 0x1e | UNARY_EXP_FOLLOW,
	_MINUS	 = 0x1f | UNARY_EXP_FOLLOW,
	_STAR	 = 0x20 | UNARY_EXP_FOLLOW,
	_SLASH	 = 0x21,
	_PERCENT = 0x22,
	_BIT_AND = 0x23 | UNARY_EXP_FOLLOW,
	_BIT_XOR = 0x24,
	_BIT_OR	 = 0x25,

	_LPAREN		= 0x26 | PRIMARY_EXP_FOLLOW,
	_RPAREN		= 0x27,
	_LBRACKET	= 0x28,
	_RBRACKET	= 0x29,
	_LBRACE		= 0x2a,
	_RBRACE		= 0x2b,
	_COMMA		= 0x2c,
	_DOT		= 0x2d,
	_COLON		= 0x2e,
	_SEMICOLON	= 0x2f,
	_BANG		= 0x30 | UNARY_EXP_FOLLOW,
	_TILDE      = 0x31 | UNARY_EXP_FOLLOW,
	_QUESTION	= 0x32,
	_ARROW		= 0x33,
	_ARROW_STAR = 0x34,	//->*
	_ELIPSIS	= 0x35,
	_SCOPE		= 0x36 | PRIMARY_EXP_FOLLOW | TYPE_SPEC_FOLLOW,	//::

	_STRING_CONSTANT = 0x37 | PRIMARY_EXP_FOLLOW,
	_IDENT			 = 0x38 | PRIMARY_EXP_FOLLOW | TYPE_SPEC_FOLLOW,


	//keywords
	_CHAR	  = 0x80 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_BOOL	  = 0x81 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_SHORT	  = 0x82 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_INT	  = 0x83 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_LONG	  = 0x84 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_SIGNED	  = 0x85 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_UNSIGNED = 0x86 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_VOID	  = 0x87 | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_TRUE	  = 0x88 | PRIMARY_EXP_FOLLOW,
	_FALSE	  = 0x89 | PRIMARY_EXP_FOLLOW,
	_ENUM	  = 0x8a | TYPE_SPEC_FOLLOW,
	_CONST	  = 0x8b | TYPE_SPEC_FOLLOW,
	_VOLATILE = 0x8c | TYPE_SPEC_FOLLOW,
	_BREAK	  = 0x8d,
	_CONTINUE = 0x8e,
	_RETURN	  = 0x8f,
	_GOTO	  = 0x90,
	_DO		  = 0x91,
	_FOR	  = 0x92,
	_WHILE	  = 0x93,
	_IF		  = 0x94,
	_ELSE	  = 0x95,
	_SWITCH	  = 0x96,
	_CASE	  = 0x97,
	_DEFAULT  = 0x98,
	_STRUCT	  = 0x99 | TYPE_SPEC_FOLLOW,
	_CLASS	  = 0x9a | TYPE_SPEC_FOLLOW,

	_PRIVATE   = 0x9b,
	_PROTECTED = 0x9c,
	_PUBLIC	   = 0x9d,

	_THIS     = 0x9e  | PRIMARY_EXP_FOLLOW,
	_OPERATOR = 0x9f  | PRIMARY_EXP_FOLLOW,

	_TYPEDEF = 0xa0 | DECL_SPEC_FOLLOW,
	_FRIEND	 = 0xa1 | DECL_SPEC_FOLLOW,

	_AUTO	   = 0xa2 | DECL_SPEC_FOLLOW,
	_REGISTER  = 0xa3 | DECL_SPEC_FOLLOW,
	_STATIC	   = 0xa4 | DECL_SPEC_FOLLOW,
	_EXTERN	   = 0xa5 | DECL_SPEC_FOLLOW,
	_MUTABLE   = 0xa6 | DECL_SPEC_FOLLOW,
	_INLINE	   = 0xa7 | DECL_SPEC_FOLLOW,
	_VIRTUAL   = 0xa8 | DECL_SPEC_FOLLOW,
	_EXPLICIT  = 0xa9 | DECL_SPEC_FOLLOW,
	_NAMESPACE = 0xaa,
	_USING	   = 0xab,
	_TEMPLATE  = 0xac,
	_TYPENAME  = 0xad | POSTFIX_EXP_FOLLOW | TYPE_SPEC_FOLLOW,
	_EXPORT	   = 0xae,

	_STATIC_CAST	  = 0xaf | POSTFIX_EXP_FOLLOW,
	_DYNAMIC_CAST	  = 0xb0 | POSTFIX_EXP_FOLLOW,
	_REINTERPRET_CAST = 0xb1 | POSTFIX_EXP_FOLLOW,
	_CONST_CAST		  = 0xb2 | POSTFIX_EXP_FOLLOW,

	_WAIT  = 0xb3,
	_WRITE = 0xb4,

	_COMMENT = 0xb5,
	_SPC     = 0xb6,
	_QUOTE_S = 0xb7, // '
	_QUOTE_D = 0xb8, // "
	_NEW_LINE = 0xb9,

	_PP_DEFINE		 = 0xc0,
	_PP_UNDEF		 = 0xc1,
	_PP_INCLUDE		 = 0xc2,
	_PP_INCLUDE_NEXT = 0xc3,
	_PP_IF			 = 0xc4,
	_PP_ELIF		 = 0xc5,
	_PP_DEFINED		 = 0xc6,
	_PP_IFDEF		 = 0xc7,
	_PP_IFNDEF		 = 0xc8,
	_PP_ELSE		 = 0xc9,
	_PP_ENDIF		 = 0xca,
	_PP_ERROR		 = 0xcb,
	_PP_WARNING		 = 0xcc,
	_PP_LINE		 = 0xcd,
	_PP_PRAGMA		 = 0xce,
	_PP_OP_CONCAT    = 0xcf,
	_PP_OP_TO_STR    = 0xd0,
	_PP_HEADER_NAME  = 0xd1,
	_PP_MACRO_PARAM  = 0xd2,
	_PP_END			 = 0xd3,

	_SOURCE_STRING = 0xf0
};


class AcceptResultCache;

struct Location
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
};

struct Token
{
	int id;
	//String valstr;
	ref<TokenValue> val;
	Location loc;
	List<AcceptResultCache*> resultCache;
	bool operator ==(const Token &other) const {
		return val.get() == other.val.get();
	}
};
#endif
