#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include "String.hpp"


struct Symbol {
	static Symbol *symbol(const String &n);
	static Symbol *gensym(const String &tag);
	static bool exist(const String &n);

	const String &toString() const {
		return name;
	}
	/*
	operator const char *()	{
		return name.c_str();
	}
	*/
	String name;
private:
	Symbol(const String &s);
};

/*
struct Scope {
	Scope(ScopeName *scopeName);
	void addOuter(ScopeName *scopeName);
	virtual String toString() const;
	virtual String toSource() const;

	List<ScopeName*> scopeNames;
	ACCEPTABLE
};
*/

#endif //__SYMBOL_H__
