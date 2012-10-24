#include "SymbolTable.hpp"
#include "Type.hpp"
#include "Symbol.hpp"
#include <cassert>
#include "debug.h"

using namespace std;

class SymbolTable SymbolTable;

SymbolTable::SymbolTable()
{
}

void
SymbolTable::init()
{
	cleanup();
	Symbol *sym = Symbol::symbol("*top*");
	currentScope = new Scope(GLOBAL, sym, NULL);
	scopes.insert(make_pair(sym, currentScope)); 
	/*
	currentScope->addType(new IntType(8, true), Symbol::symbol("char"));
	currentScope->addType(new IntType(8, false), Symbol::symbol("bool"));
	currentScope->addType(new IntType(16, true), Symbol::symbol("short"));
	currentScope->addType(new IntType(32, true), Symbol::symbol("int"));
	currentScope->addType(new IntType(32, true), Symbol::symbol("long"));
	currentScope->addType(new IntType(32, true), Symbol::symbol("signed"));
	currentScope->addType(new IntType(32, false), Symbol::symbol("unsigned"));
	currentScope->addType(new VoidType(), Symbol::symbol("void"));
	*/
}

void
SymbolTable::cleanup()
{
	/*
	if (currentScope) {
		currentScope->cleanup();
		currentScope = NULL;
	}
	*/
	map<Symbol*,Scope*>::iterator it = scopes.begin();
	while (it != scopes.end()) {
		Scope *scope = it->second;
		delete scope;
		++it;
	}
	scopes.clear();
}

bool
SymbolTable::addVar(Type *type, Symbol *sym, bool isOuter, Type **added)
{
	assert(type);
	assert(sym);
	if (isOuter) {
		assert(currentScope->outer);
		return currentScope->outer->addVar(type, sym, added);
	} else {
		return currentScope->addVar(type, sym, added);
	}
}

Type *
SymbolTable::findVar(Symbol *sym)
{
	return currentScope->findVar(sym);
}

bool
SymbolTable::addType(Type *type, Symbol *sym, bool isOuter)
{
	assert(type);
	assert(sym);
	if (isOuter) {
		assert(currentScope->outer);
		return currentScope->outer->addType(type, sym);
	} else {
		return currentScope->addType(type, sym);
	}
}

Type *
SymbolTable::findType(Symbol *sym, Symbol *scope)
{
	if (scope == NULL) {
		return currentScope->findType(sym);
	} else {
		Scope *base = currentScope;
		while (base) {
			DBG("$ %s", scope->toString().c_str());
			Symbol *scopeSymbol = makeScopeSymbol(scope, base);
			map<Symbol*,Scope*>::iterator it = scopes.find(scopeSymbol);
			if (it != scopes.end()) {
				DBG("findType in %s", scopeSymbol->toString().c_str());
				return it->second->findType(sym);
			}
			base = base->outer;
		}
		return NULL;
	}
}

void
SymbolTable::enterScope(ScopeType type, Symbol *sym)
{
	Symbol *scopeSymbol = makeScopeSymbol(sym, currentScope);
	DBG("enterScope %s", scopeSymbol->toString().c_str());
	map<Symbol*,Scope*>::iterator it = scopes.find(scopeSymbol);
	if (it != scopes.end() && it->second->type == type) {
		DBG("already created scope");
		currentScope = it->second;
	} else {
		currentScope = new Scope(type, scopeSymbol, currentScope);
		scopes.insert(make_pair(scopeSymbol, currentScope)); 
	}
}

void
SymbolTable::leaveScope()
{
	DBG("leaveScope %s", currentScope->name->toString().c_str());
	Scope *scope = currentScope;
	currentScope = currentScope->outer;
}

void 
SymbolTable::updateScope(Scope *scope, Symbol *newsym)
{
	map<Symbol*, Scope*>::iterator it = scopes.find(scope->name);
	if (it != scopes.end() && it->second == scope) {
		scopes.erase(it);
		scope->name = makeScopeSymbol(newsym, scope->outer);

		map<Symbol*,Scope*>::iterator it = scopes.find(scope->name);
		if (it != scopes.end() && it->second->type == scope->type) {
			it->second->merge(scope);
		} else {
			scopes.insert(make_pair(scope->name, scope));
		}
	}
}

Symbol *
SymbolTable::makeScopeSymbol(Symbol *sym, SymbolTable::Scope *base)
{
	//String scopeName = sym->toString();
	//Scope *scope = currentScope;
	String scopeName = base->name->toString() + "::" + sym->toString();//scopeName;
	return Symbol::symbol(scopeName);
}

SymbolTable::Scope *
SymbolTable::getCurrentFunctionScope()
{
	return getCurrentXScope(FUNCTION);
}

SymbolTable::Scope *
SymbolTable::getCurrentClassScope()
{
	return getCurrentXScope(CLASS);
}

SymbolTable::Scope *
SymbolTable::getCurrentNamespaceScope()
{
	return getCurrentXScope(NAMESPACE);
}

SymbolTable::Scope *
SymbolTable::getCurrentXScope(ScopeType type)
{
	Scope *scope = currentScope;
	while (scope) {
		if (scope->type == type) {
			return scope;
		} else {
			scope = scope->outer;
		}
	}
	return NULL;
}

String 
SymbolTable::toString() const
{
	String s = "\n";
	map<Symbol*,Scope*>::const_iterator it = scopes.begin();
	while (it != scopes.end()) {
		Scope *scope = it->second;
		s += scope->toString();
		s += "----------\n";
		++it;
	}
	return s;
}


SymbolTable::Scope::Scope(ScopeType type, Symbol *sym, SymbolTable::Scope *outer)
	: type(type)
	, name(sym) 
	, outer(outer)
{
}

bool
SymbolTable::Scope::addVar(Type *type, Symbol *sym, Type **added)
{
	DBG("SymbolTable::Scope::addVar %s %s::%s", type->toString().c_str(), name->toString().c_str(), sym->toString().c_str());
	std::map<Symbol *, Type *>::iterator it = varTable.find(sym);
	if (it != varTable.end()) {
		// is overloaded function?
		if (type->is(Type::FUNC_T) && it->second->is(Type::FUNC_T)) {
			FuncType *fttop = it->second->as<FuncType>();
			FuncType *new_ft = type->as<FuncType>();
			FuncType *ft = fttop;
			while (ft) {
				if (ft->equal(new_ft)) {
					DBG("!!! %s is already added", sym->toString().c_str());
					if (added) {
						*added = ft;
					}
					return false;
				}
				ft = ft->nextOverload;
			}
			DBG("function %s is overloaded", sym->toString().c_str());
			fttop->addOverload(new_ft);
			return true;
		} else {
			DBG("!!! %s is already added", sym->toString().c_str());
			if (added) {
				*added = it->second;
			}
			return false;
		}
	} else {
		varTable.insert(make_pair(sym, type));
		return true;
	}
}

Type * 
SymbolTable::Scope::findVar(Symbol *sym)
{
	DBG("SymbolTable::Scope::findVar %s in %s", sym->toString().c_str(), name->toString().c_str());
	map<Symbol *, Type *>::iterator it;
	it = varTable.find(sym);
	if (it != varTable.end()) {
		DBG("found %s in %s", sym->toString().c_str(), name->toString().c_str());
		return it->second;
	}
	if (outer) {
		return outer->findVar(sym);
	}
	return NULL;
}
bool
SymbolTable::Scope::addType(Type *type, Symbol *sym)
{
	DBG("SymbolTable::Scope::addType %s %s::%s", type->toString().c_str(), name->toString().c_str(), sym->toString().c_str());
	std::map<Symbol *, Type *>::iterator it = typeTable.find(sym);
	if (it != typeTable.end()) {
		DBG("!!! %s is already added", sym->toString().c_str());
		return false;
	} else {
		typeTable.insert(make_pair(sym, type));
	}
	return true;
}

Type * 
SymbolTable::Scope::findType(Symbol *sym)
{
	DBG("SymbolTable::Scope::findType %s in %s", sym->toString().c_str(), name->toString().c_str());
	map<Symbol *, Type *>::iterator it;
	it = typeTable.find(sym);
	if (it != typeTable.end()) {
		DBG("found %s in %s", sym->toString().c_str(), name->toString().c_str());
		return it->second;
	}
	if (outer) {
		return outer->findType(sym);
	}
	DBG("%s is not found", sym->toString().c_str());
	return NULL;
}

void
SymbolTable::Scope::cleanup()
{
	/*
	if (outer) {
		outer->cleanup();
		delete outer;
		outer = NULL;
	}
	varTable.clear();
	typeTable.clear();
	*/
}

void 
SymbolTable::Scope::merge(Scope *other)
{
	varTable.insert(other->varTable.begin(), other->varTable.end());
	typeTable.insert(other->typeTable.begin(), other->typeTable.end());	
}

bool 
SymbolTable::Scope::addLabel(Symbol *label)
{
	std::set<Symbol*>::iterator it = labels.find(label);
	if (it != labels.end()) {
		return false;
	}
	labels.insert(label);
	return true;
}

String 
SymbolTable::Scope::toString() const
{
	assert(name);
	String s = "name   : " + name->toString() + "\n";
	if (outer) {
		s += "parent : " + outer->name->toString() + "\n";
	}

	s += "vars   : \n";
	std::map<Symbol *, Type *>::const_iterator it = varTable.begin();
	while (it != varTable.end()) {
		s += "    ";
		s += "\"" + it->first->toString() + "\"\n";
		s += "    ";
		s += it->second->toString() + "\n";
		++it;
	}
	s += "types  : \n";
	it = typeTable.begin();
	while (it != typeTable.end()) {
		s += "    ";
		s += "\"" + it->first->toString() + "\"\n";
		s += "    ";
		s += it->second->toString() + "\n";
		++it;
	}

	return s;
}

