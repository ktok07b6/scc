#include "Scanner.hpp"
#include <cstdio>
#include <cstring>
#include "String.hpp"
#include "debug.h"
#include "SourceReader.hpp"

String lexstr;
ref<TokenValue> lexval;

extern int yylex();
extern FILE *yyin;
extern FILE *yyout;
extern char *yytext;

int tokenStartPos;
int tokenLength;
int tokenNextStartPos;

bool
Scanner::scan(const char *file, List<Token> &tokens)
{
 	yyin = NULL;
	yyout = stderr;
	if (file) {
		yyin = fopen(file, "rb");
		if (yyin == NULL) {
			ERROR("open fail %s (%s)\n", strerror(errno), file);
			return false;
		}
	} else {
		yyin = stdin;
	}
	SourceReader.load(yyin);

	int tokenid;
	tokenNextStartPos = 1;
	while ((tokenid = yylex()) != 0) {
		if (tokenid == _ERROR) {
			return false;
		}
		Token t;
		t.id = tokenid;
		t.val = lexval;

#if 1 //FIXME
		int row = SourceReader.getRow();
		int col = SourceReader.getColumn();

		tokenStartPos = SourceReader.getPrevStart();
		tokenLength   = t.val->toString().size();
		SourceReader.acceptToken();
		//tokenNextStartPos = col; // + 1;
		t.loc.first_line   = row;
		t.loc.first_column = tokenStartPos;
		t.loc.last_line    = row;
		t.loc.last_column  = tokenStartPos + tokenLength - 1;

		VDBG("token %s: %x: line %d:%d.%d", 
			lexval->toString().c_str(), 
			tokenid, row, t.loc.first_column, t.loc.last_column);
#endif
		tokens.push_back(t);
	}
	if (file && yyin) fclose(yyin);
	return true;
}
