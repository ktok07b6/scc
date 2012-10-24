#include <cstdio>
#include <cstring>
#include "debug.h"
#include <cassert>
#include "String.hpp"
#include "SourceReader.hpp"

/*
int tokenStartPos;
int tokenLength;
int tokenNextStartPos;
*/
void beginToken(const String &t) 
{
	/*
	tokenStartPos = tokenNextStartPos;
	tokenLength = t.size();
	tokenNextStartPos = SourceReader.getPos(); // + 1;
	*/
	/*	
	location.first_line   = SourceReader.getRow();
	location.first_column = tokenStartPos;
	location.last_line    = SourceReader.getRow();
	location.last_column  = tokenStartPos + tokenLength - 1;
	DBG("beginToken %s (line[%d]:%d-%d)", t.c_str(), 
		 location.first_line, 
		 location.first_column, 
		 location.last_column);
	*/
}


void printError(const char *errorstring, ...) 
{
	static char errmsg[10000];
	va_list args;

	//int start = tokenStartPos;
	//int end = start + tokenLength - 1;

	/*================================================================*/
	/* simple version ------------------------------------------------*/
	/*
	  fprintf(stdout, "...... !");
	  for (i=0; i<nBuffer; i++)
      fprintf(stdout, ".");
	  fprintf(stdout, "^\n");
	*/

	/*================================================================*/
	/* a bit more complicate version ---------------------------------*/
	/* */

	SourceReader.printLine();
	/* TODO:
	if (SourceReader.isEOF()) {
		fprintf(stdout, "...... !");
		for (int i = 0; i < SourceReader.getLength(); ++i) {
			fprintf(stdout, ".");
		}
		fprintf(stdout, "^-EOF\n");
	}
	else {
		fprintf(stdout, "...... !");
		for (int i = 1; i < start; ++i) {
			fprintf(stdout, ".");
		}
		for (int i = start; i <= end; ++i) {
			fprintf(stdout, "^");
		}
		for (int i = end+1; i < SourceReader.getLength(); ++i) {
			fprintf(stdout, ".");
		}
		fprintf(stdout, "   token%d:%d\n", start, end);
	}
	*/
	/* */
	
	/*================================================================*/
	/* print it using variable arguments -----------------------------*/
	va_start(args, errorstring);
	vsprintf(errmsg, errorstring, args);
	va_end(args);
	
	fprintf(stdout, "%s\n", errmsg);
}
