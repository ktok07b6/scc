#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

#include <set>
#include <map>
#include "String.hpp"

class Type;
class Symbol;

class SymbolTable
{
public:
	SymbolTable();
	void init();
	void cleanup();
	bool addVar(Type *type, Symbol *sym, bool isOuter = false, Type **added = NULL);
	Type *findVar(Symbol *sym);
	bool addType(Type *type, Symbol *sym, bool isOuter = false);
	Type *findType(Symbol *sym, Symbol *scope = NULL);

	enum ScopeType {
		FUNCTION,
		CLASS,
		NAMESPACE,
		LOOP,
		UNNAMED,
		GLOBAL
	};

	struct Scope {
		Scope(ScopeType type, Symbol *sym, Scope *outer);
		bool addVar(Type *type, Symbol *sym, Type **added = NULL);
		Type *findVar(Symbol *sym);
		bool addType(Type *type, Symbol *sym);
		Type *findType(Symbol *sym);
		void cleanup();
		void merge(Scope *other);
		bool addLabel(Symbol *label);
		String toString() const;

		ScopeType type;
		Symbol *name;
		Scope *outer;
		std::map<Symbol *, Type *> varTable;
		std::map<Symbol *, Type *> typeTable;
		std::set<Symbol *> labels;
	};
	void enterScope(ScopeType, Symbol *);
	void leaveScope();
	void updateScope(Scope *scope, Symbol *newsym);
	Symbol *makeScopeSymbol(Symbol *sym, Scope *base);
	
	Scope *getCurrentFunctionScope();
	Scope *getCurrentClassScope();
	Scope *getCurrentNamespaceScope();
	Scope *getCurrentXScope(ScopeType type);

	String toString() const;

	Scope *currentScope;
	std::map<Symbol *, Scope *> scopes;
};

extern class SymbolTable SymbolTable;

#endif
