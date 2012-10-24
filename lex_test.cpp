#include <cstdio>
#include <errno.h>
#include <stdlib.h>
#include "token.hpp"

String lexstr;
extern int yylex();
extern FILE *yyin;
extern FILE *yyout;
extern char *yytext;

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
	int token;
	while ((token = yylex()) != 0) {
		printf("%2d: %s\n", token, lexstr.c_str());
	}

	return 0;
}
