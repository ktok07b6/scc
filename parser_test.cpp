#include <cstdio>
#include <errno.h>
#include <stdlib.h>
#include "AST.h"
#include "parser.hpp"

extern int yyparse();
extern int yydebug;
extern FILE *yyin;
extern FILE *yyout;

AST *result_ast;

int main(int argc, char **argv)
{
	yyin = NULL;
	yyout = stderr;

	if (argc==2) {
		yyin = fopen(argv[1], "rb");
		if (yyin == NULL) {
			//fprintf(stderr, "open fail %s (%s)\n", strerror(errno), argv[1]);
			exit(-1);
		}
	} else {
		yyin = stdin;
	}

	//#ifdef YYDEBUG
	yydebug = 1;
	//#endif
	yyparse();
	
	if (result_ast) {
		printf("%s\n", result_ast->toString().c_str());
		printf("OK\n");
	}
	
	return 0;
}
