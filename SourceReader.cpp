#include "SourceReader.hpp"
#include "debug.h"
#include <cstring>
#include <cassert>

class SourceReader SourceReader;

SourceReader::SourceReader()
	: buf(NULL)
	, length(0)
	, pos(0)
	, eof(false)
	, prevStart(1)
{
}

bool
SourceReader::load(FILE *fp)
{
	assert(fp);
	if (buf) delete [] buf;

	const int orig = ftell(fp);
	fseek(fp, 0, SEEK_END);
	const size_t sz = ftell(fp);
	fseek(fp, orig, SEEK_SET);//revert

	buf = new char[sz];
	size_t rd = fread(buf, 1, sz, fp);
	if (rd != sz) {
		if (ferror(fp)) {
			ERROR("Failed to read line");
		}
		eof = true;
		VDBG("EOF");
		return false;
	}
	length = sz;
	pos = 0;
	column = 0;
	row = 1;
	return true;
}

SourceReader::~SourceReader()
{
	delete [] buf;
}

char
SourceReader::getNextChar()
{
	if (pos < length) {
		char c = buf[pos];
		++pos;
		++column;
		//TODO: crlf? 
		if (c == '\n') {
			++row;
			column = 0;
		}
		return c;
	} else {
		eof = true;
		return 0;
	}
}

bool
SourceReader::isEOF()
{
	return eof;
}

int
SourceReader::getPos()
{
	return pos;
}

int
SourceReader::getLength()
{
	return length;
}

int
SourceReader::getRow()
{
	return row;
}

int
SourceReader::getColumn()
{
	return column;
}

int
SourceReader::getPrevStart()
{
	return prevStart;
}

void
SourceReader::printLine()
{
	if (buf[pos] != '\n') {
		int p = pos;
		while (buf[p] != '\n' && 0 <= p) --p;
		const int head = p+1;
		p = pos;
		while (buf[p] != '\n' && p < length) ++p;
		const int tail = p;
		const int linelen = tail-head;
		char *line = new char[linelen+1];
		strncpy(line, buf+head, linelen);
		line[linelen] = '\0';
		fprintf(stdout, "%6d| %.*s", row, length, line);
		fprintf(stdout, "\n");
		delete [] line;
	} else {
		fprintf(stdout, "\n");
	}
}


