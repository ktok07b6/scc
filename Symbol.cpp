#include "Symbol.hpp"
#include <map>
#include "debug.h"
//#include "HeapManager.h"

using namespace std;

namespace {
	typedef map<string, Symbol*> SymbolDictionary;
	SymbolDictionary dict;
}

Symbol::Symbol(const String &s)
	: name(s)
{
}

bool
Symbol::exist(const String &n)
{
	//Symbol *s = NULL;
	SymbolDictionary::iterator it;
	it = dict.find(n);
	bool r = (it != dict.end());
	//DBG("exist %s:%d", n.c_str(), r);
	return r;
}

Symbol *
Symbol::symbol(const String &n)
{
	//DBG("gensym %s", n.c_str());
	Symbol *s = NULL;
	SymbolDictionary::iterator it;
	it = dict.find(n);
	if (it != dict.end()) {
		s = it->second;
	} else {
		s = new Symbol(n);
		dict.insert(make_pair(n, s));
	}

	return s;
}

Symbol *
Symbol::gensym(const String &tag)
{
	static int count;

	char buf[64];
	sprintf(buf, "#%d", count++);

	String s = tag;
	s += buf;
	return symbol(s);
}

