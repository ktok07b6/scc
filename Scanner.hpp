#ifndef __SCANNER_HPP__
#define __SCANNER_HPP__

#include "List.hpp"
#include "token.hpp"

class Scanner
{
public:
	bool scan(const char *file, List<Token> &tokens);
};
#endif
