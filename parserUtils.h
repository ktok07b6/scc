#ifndef __PARSER_UTILS_H__
#define __PARSER_UTILS_H__

#include "String.hpp"

void beginToken(const String &t);
void printError(const char *errorstring, ...);

#endif //PARSER_UTILS_H
