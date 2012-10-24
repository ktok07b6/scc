#ifndef __TOKEN_ITERATOR_HPP__
#define __TOKEN_ITERATOR_HPP__

#include "List.hpp"


class TokenIterator
{
public:
	TokenIterator()
		: tokens(NULL)
		, it()
	{}
	
	void setTokens(List<Token> *tokens)
	{
		this->tokens = tokens;
		it = tokens->begin();
	}

	TokenIterator(const TokenIterator &other)
		: tokens(other.tokens)
		, it(other.it)
	{
	}

	void operator=(const TokenIterator &other)
	{
		tokens = other.tokens;
		it = other.it;
	}

	bool operator==(const TokenIterator &other) const 
	{
		return it == other.it;
	}

	int operator-(const TokenIterator &other) const 
	{
		return std::distance(other.it, it);
	}

	void operator+=(size_t n) 
	{
		std::advance(it, n);
	}

	bool eat(int tid) 
	{
		if (is(tid)) {
			next();
			return true;
		} else {
			return false;
		}
	}
	
	bool is(int tid) 
	{
		if (it != tokens->end() && tid == it->id) {
			return true;
		} else {
			return false;
		}
	}

	void next(bool skipspc = true) 
	{
		if (it != tokens->end()) {
			++it;
		}
		if (skipspc) {
			skip(_SPC);
		}
	}

	void prev() 
	{
		if (it != tokens->begin()) {
			--it;
		}
	}
	
	int skip(int id) {
		int count = 0;
		while (it != tokens->end() && it->id == id) {
			++it;
			++count;
		}
		return count;
	}

	ref<TokenValue> val() {
		if (it != tokens->end()) {
			return it->val;
		} else {
			return ref<TokenValue>();
		}
	}

	bool isFollow(unsigned int mask) {
		return (it->id & mask);
	}

	bool eof() {
		return it == tokens->end();
	}

	void pushAcceptResult(AcceptResultCache *arc) {
		it->resultCache.push_front(arc);
	}

	const Token &get(int offset = 0) const {
		//assert(it != tokens->end());
		if (0 == offset) return *it;
		List<Token>::iterator newit = it;
		std::advance(newit, offset);
		return *newit;
	}

	Token &get(int offset = 0) {
		//assert(it != tokens->end());
		if (0 == offset) return *it;
		List<Token>::iterator newit = it;
		std::advance(newit, offset);
		return *newit;
	}

	int skipUntil(const std::vector<int> &targets, bool consume) {
		int foundTargetId = -1;
		List<Token>::iterator i = it;
		while (i != tokens->end()) {
			const Token &t = *i;
			for (size_t j=0;j<targets.size();++j) {
				if (t.id == targets[j]) {
					foundTargetId = t.id;
					goto done;
				}
			}
			++i;
		}
	done:
		if (consume) {
			it = i;
		}
		return foundTargetId;
	}

	void erase(size_t count) {
		List<Token>::iterator end = it;
		std::advance(end, count);
		it = tokens->erase(it, end);
	}

	void insert(const List<Token> &tks) {
		size_t pos = std::distance(tokens->begin(), it);
		List<Token> ts = tks;
		tokens->splice(it, ts);
		it = tokens->begin();
		std::advance(it, pos);
	}
	void insert(const Token &t, size_t offset = 0) {
		List<Token>::iterator pos = it;
		std::advance(pos, offset);
		tokens->insert(pos, t); 
	}

protected:
	List<Token> *tokens;
	List<Token>::iterator it;
};

#endif
