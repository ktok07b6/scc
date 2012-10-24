#include "SCParser.hpp"
#include "AST.hpp"
#include <typeinfo>
#include <cstring>
#include "error.hpp"

void coverage();
void errorTest();

void testFile(const char *file)
{
	SCParser parser;
	AST *ast = parser.parse(file);
	if (ast) {
		DBG("%s", ast->toString().c_str());
		DBG("\n");
		DBG("%s", ast->toSource().c_str());
	}
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		coverage();
		return 0;
	}
	if (argc == 2 && strcmp(argv[1], "err")==0) {
		errorTest();
		return 0;
	}
	testFile(argv[1]);
	return 0;
}

#define TESTCASE_DIR "testcase/parser/"
void coverage()
{
	testFile(TESTCASE_DIR"exp01.cpp");
	testFile(TESTCASE_DIR"exp02.cpp");
	testFile(TESTCASE_DIR"exp03.cpp");
	testFile(TESTCASE_DIR"exp04.cpp");
	testFile(TESTCASE_DIR"exp05.cpp");

	testFile(TESTCASE_DIR"stm01.cpp");
	testFile(TESTCASE_DIR"stm02.cpp");
	testFile(TESTCASE_DIR"stm03.cpp");
	testFile(TESTCASE_DIR"stm04.cpp");
	testFile(TESTCASE_DIR"stm05.cpp");

	testFile(TESTCASE_DIR"decl01.cpp");
	testFile(TESTCASE_DIR"decl02.cpp");
	testFile(TESTCASE_DIR"decl03.cpp");
	testFile(TESTCASE_DIR"decl04.cpp");
	testFile(TESTCASE_DIR"decl05.cpp");
	testFile(TESTCASE_DIR"decl06.cpp");

}


int expected;

#define etest(program)													\
	do {																\
		int actual = __etest(program);									\
		if (actual != expected) {										\
			printf("!!!!! expected 0x%x : actual 0x%x !!!!!\n", expected, actual);	\
			assert(!program);													\
		}																\
	}while (0)

int __etest(const char *program)
{
	FILE *f = fopen("/tmp/tmp", "w");
	if (f == NULL) {
		//fprintf(stderr, "open fail %s (%s)\n", strerror(errno), argv[1]);
		exit(-1);
	}
	fwrite(program, 1, strlen(program), f);
	fclose(f);

	SCParser parser;
	parser.parse("/tmp/tmp");
	int err = parser.getLastError();
	return err;
}


void errorTest()
{
	expected = PA_PRIMARY_EXP_PARENTHESIS_NOT_CLOSE;
	etest("void f() { (1;}");

	expected = PA_SCOPE_IDENTIFIER;
	etest("void f() { ::1;}");

	expected = PA_CXXCAST_NOT_VALID;
	etest("void f() { static_cast<*>();}");

	expected = PA_POSTFIX_EXP_BRACKET_NOT_CLOSE;
	etest("void f() { a[1;}");

	expected = PA_POSTFIX_EXP_PARENTHESIS_NOT_CLOSE;
	etest("void f() { g(;}");

	expected = PA_DOT_OP_NOT_VALID;
	etest("void f() { x.*;}");

	expected = PA_ARROW_OP_NOT_VALID;
	etest("void f() { x->+;}");

	expected = PA_EXP_EXPECTED;
	etest("void f() { g(1, 2, );}");

	expected = PA_UNARY_EXP_NOT_VALID;
	etest("void f() { !;}");
	etest("void f() { !!;}");

	expected = PA_BINARY_EXP_NOT_VALID;
	etest("void f() { 1+;}");
	etest("void f() { 1+2*;}");

	expected = PA_CONDITIONAL_EXP_NOT_VALID;
	etest("void f() { bool x = (true)?;}");
	etest("void f() { bool x = (true)?1:;}");

	expected = PA_ASSIGNMENT_EXP_NOT_VALID;
	etest("void f(int x) {x = ;}");
	etest("void f(int x) {x = y = ;}");

	expected = PA_LABEL_STATEMENT_NOT_VALID;
	etest("void f() {label:}");

	expected = PA_CASE_STATEMENT_NOT_VALID;
	etest("void f() {switch (x) { case:}");
	etest("void f() {switch (x) { case:0}");

	expected = PA_DEFAULT_STATEMENT_NOT_VALID;
	etest("void f() {switch (x) { default:}");
	etest("void f() {switch (x) { default:0}");

	expected = PA_COMPOUND_STATEMENT_NOT_CLOSE;
	etest("void f() {");
	etest("void f() {{}");

	expected = PA_IF_STATEMENT_NOT_VALID;
	etest("void f() {if}");
	etest("void f() {if (1)}");

	expected = PA_ELSE_STATEMENT_NOT_VALID;
	etest("void f() { if (1) x; else }");
	etest("void f() { if (1) {} else }");

	expected = PA_SWITCH_STATEMENT_NOT_VALID;
	etest("void f() {switch}");

	expected = PA_CONDITION_NOT_VALID;
	etest("void f() {switch(int x)}");
	etest("void f() {switch(int x=)}");

	expected = PA_WHILE_STATEMENT_NOT_VALID;
	expected = PA_DO_STATEMENT_NOT_VALID;
	expected = PA_FOR_STATEMENT_NOT_VALID;
	expected = PA_JUMP_STATEMENT_NOT_VALID;
	expected = PA_CONTINUE_STATEMENT_NOT_VALID;
	expected = PA_RETURN_STATEMENT_NOT_VALID;
	expected = PA_GOTO_STATEMENT_NOT_VALID;
	//expected = PA_DECLARATION_SEMICOLON;
	//etest("int {};");
	expected = PA_DECLARATION_LBRACE_FOLLOW;
	
	expected = PA_DECLARATION_NOT_VALID;
	etest("INT a;");
	etest("void f() {INT a;}");

}


class C1 {
public:	int x;
};
class C2 : public C1{
};
/*
enum X { X1 };
enum Y { Y1 };
typedef int (&pf)(X, Y);
typedef int (Int);
int f(X, Y) {
	return 0;
}
pf getf(double c) {
	int (*p)(X, Y) = &f;
	return *p;
}
Int geti(float) {
	return 0;
}
void typetest()
{
	int i;
	printf("%s\n", typeid(i).name());

	int (i2);
	printf("%s\n", typeid(i2).name());

	int *(i3);
	printf("%s\n", typeid(i3).name());

	int (*i4);
	printf("%s\n", typeid(i4).name());
	
	int *a[3];
	printf("%s\n", typeid(a).name());
	int (*ap)[3];
	printf("%s\n", typeid(ap).name());

	int (*aap(double))(char);
	printf("%s\n", typeid(aap).name());

	
	
	printf("enum X %s\n", typeid(X).name());
	printf("int (&pf)(X, Y) %s\n", typeid(pf).name());
	
	printf("int f(X, Y) %s\n", typeid(f).name());
	printf("Int %s\n", typeid(Int).name());
	printf("getf %s\n", typeid(getf).name());
	printf("geti %s\n", typeid(geti).name());
	printf("%s\n", typeid(&getf).name());
	int (&(*pgetf)(double))(X, Y) = &getf;
	typedef int (*pgeti)(float);
	//pgetf *pg = &getf;
	printf("%s\n", typeid(pgetf).name());
	pgeti pgi = & geti;
}
*/
