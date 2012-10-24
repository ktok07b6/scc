#ifndef __SOURCE_READER_HPP__
#define __SOURCE_READER_HPP__

#include <cstdio>
#include "debug.h"

class SourceReader
{
public:
	SourceReader();
	~SourceReader();

	bool load(FILE *);
	char getNextChar();
	bool isEOF();
	int getPos();
	int getLength();
	int getRow();
	int getColumn();
	void acceptToken() {
		prevStart = column;
	}
	int getPrevStart();
	void printLine();
private:
	char *buf;
	int length;
	int pos;
	bool eof;
	int row;
	int column;
	int prevStart;
};

extern class SourceReader SourceReader;
#endif
