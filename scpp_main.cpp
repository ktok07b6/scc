#include "PreProcessor.hpp"
#include "String.hpp"
#include "token.hpp"

int main(int argc, char **argv)
{
	if (argc != 3) {
		return -1;
	}
	PreProcessor pp;
	pp.preprocess(argv[1], argv[2]);
	return 0;
}
