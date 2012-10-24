#include "Reporter.hpp"
#include "parserUtils.h"
#include "error.hpp"
#include <cassert>

class Reporter Reporter;

Reporter::Reporter()
	: errCode(NO_ERROR)
{
}

void 
Reporter::error(int code, ...)
{
	errCode = code;
	const char *kind;
	if (code < SCANNER_ERROR) {
		kind = "preprocessor error";
	} else if (code < PARSER_ERROR) {
		kind = "scanner error";
	} else if (code < SEMANT_ERROR) {
		kind = "parser error";
	} else {
		kind = "semantic error";
	}

	printError("%s: %x", kind, code);
}

void 
Reporter::warning(int code, ...)
{
}

void 
Reporter::info(int code, ...)
{
}

int
Reporter::getLastError()
{
	return errCode;
}

void
Reporter::reset()
{
	errCode = NO_ERROR;
}
