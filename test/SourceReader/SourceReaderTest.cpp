#include "SourceReader.hpp"
#include <cstdio>


int main(int argc, char **argv)
{
	const char * file = "../../testcase/parser/decl01.cpp";
	FILE *fp = fopen(file, "r");

	SourceReader.load(fp);

	for (int i = 0; i < 30; ++i) {
		char c = SourceReader.getNextChar();
		printf("%c", c);
	}
	printf("\n");
	SourceReader.printLine();

	return 0;
}
