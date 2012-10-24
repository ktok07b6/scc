#include "SCParser.hpp"
#include "AST.hpp"
#include "TypeCheck.hpp"
#include <cstdio>
#include <cstring>
#include "error.hpp"
#include "SymbolTable.hpp"

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
	AST *ast = parser.parse("/tmp/tmp");

	assert(ast);

	DBG("%s", ast->toString().c_str());
	DBG("\n");
	DBG("%s", ast->toSource().c_str());

	TypeCheck typeCheck;
	ast->accept(&typeCheck);

	DBG("%s\n", SymbolTable.toString().c_str());

	int err = typeCheck.getLastError();
	return err;
}

void testFile(const char *file)
{
	SCParser parser;

	AST *ast = parser.parse(file);
	if (ast) {
		DBG("%s", ast->toString().c_str());
		DBG("\n");
		DBG("%s", ast->toSource().c_str());
	}
	TypeCheck typeCheck;
	ast->accept(&typeCheck);
	//int err = typeCheck.getLastError();
}

int main(int argc, char **argv)
{
	if (argc == 2) {
		testFile(argv[1]);
		return 0;
	}
#if 0
	//declaration
	expected = 0;
	etest("int a, b, c;");
	etest("int a = 0, b = 0;");
	etest("const int *a = 0;");
	etest("int const * const a = 0;");
	etest("char a;");
	etest("signed char a;");
	etest("char signed a;");
	etest("const signed char a;");
	etest("char unsigned const a;");
	etest("short int unsigned const a;");
	etest("long long int unsigned const a;");
	etest("const char *s = \"string\";");
	etest("int *f(int*, char**);");
	etest("int *(*pf)(int);");
	etest("void f() {int a, b; a = b;}");
	etest("void f() {const int * const a = 0;}");

	expected = SE_UNDEFINED_SYMBOL;
	etest("int a = b;");
	etest("void f() {int a; a = b;}");

	expected = 0;
	etest("int a; char b = a;");
	etest("int a; char b = a;");
	expected = SE_CAN_NOT_CONVERT;
	etest("int a; int *b = a;");
	etest("int *a; int b = a;");

	//read-only
	expected = 0;
	etest("void f() {int *a = 0; int *b; a = b;}");
	expected = SE_ASSIGN_READONLY;
	etest("void f() {const int a = 0; a = 1;}");
	etest("void f() {int * const a = 0; int *b; a = b;}");
	etest("void f() {int * const a = 0; a = 0;}");
	etest("void f() {const int a[1] = {0}; a[0] = 1;}");
	etest("void f() {const int *a; a[0] = 1;}");


	expected = SE_TWO_OR_MORE_BASETYPE;
	etest("const int int a = 0;");
	etest("long long long int a = 0;");
	//etest("const const a = 0;");//PA_DECL_SPEC_BASE_TYPE_NOT_SPECIFIED;

	expected = SE_SIGN_SPECIFIER;
	etest("signed bool a;");
	etest("signed void a;");

	expected = 0;
	etest("void f();");
	expected = SE_VOID_DECL;
	etest("void a;");
	expected = SE_INVALID_CONST;
	etest("const void f() {};");

	expected = 0;
	etest("int a[3];");
	etest("int a[3] = {0, 1, 2};");
	etest("int a[] = {0, 1, 2};");
	etest("void f() {int a[3]; a[0] = 1;}");
	etest("void f() {int a[3]; int *pa; pa = a;}");
	etest("void f() {int a = 0; int b = 0; int c = a+b;}");

	//etest("void f() { int a = int(100); }");

	//etest("void f() {int a[3]; a[\"str\"] = 1;}");

	expected = SE_RETURN_VALUE_IN_VOID_FUNC;
	etest("void f(){return a;}");
	expected = 0;
	etest("void f(){return;}");
	
	expected = SE_NUMBER_OF_FUNC_PARAMS_NOT_VALID;
	etest("int f(int a){return a;} void g() {f();}");
	etest("int f(int a){return a;} void g() {f(1, 2);}");

	expected = SE_FUNC_PARAM_TYPE_NOT_VALID;
	etest("void f(const char *a){} void g() {f(1);}");
	expected = 0;//0 as null
	etest("void f(const char *a){} void g() {f(0);}");
	
	//function overload
	expected = 0;
	etest("void f(){} void f(int x) {} void g(){f(); f(1);}");
	etest("void f(int x){} void f(int x, char *y) {} void g(){f(1); f(1, \"aa\");}");
	etest("void f(int); void f(int a) {a;}");

	//initializer
	expected = SE_CAN_NOT_CONVERT;
	etest("void f() { int a[1] = {0, 1, 2}; }");
	expected = 0;
	etest("void f() { int a[] = {0, 1, 2}; }");

	//struct
	expected = 0;
	etest("struct S {int x;}; S s = {0};");
	etest("struct S {int x; int y;}; S s = {0, 0};");
	expected = SE_CAN_NOT_CONVERT;
	etest("struct S {int x; int y;}; S s = {0, 0, 0};");
	expected = 0;
	etest("struct S {int x; const char *y;}; S s = {0, \"str\"};");
	expected = SE_CAN_NOT_CONVERT;
	etest("struct S {int x; const char *y;}; S s = {\"str\", 0};");
	
	expected = 0;
	etest("struct S {int x = 0; int *px = 0;}; void main() {S s; s.x = 0; s.px = &s.x;}");
	etest("struct S {int f(int x); int f(int x) {return x;} }; void main() {S s; s.f(100);}");
	etest("struct S {int m_x; int get() {return m_x;} }; void main() {S s; int x = s.get();}");
	
	etest("struct S {struct T {int tx;}a, b; T t;}; void main() {S s; int x = s.t.tx;}");
	etest("struct S {struct T {int tx;};}; void main() {S::T t; int x = t.tx;}");

	expected = 0;
	etest("int f0() {return 0;} int f1() {return 1;}");
	etest("int f() {} struct S{ int f() {} };");
	expected = SE_DUPLICATE_FUNC_DEF;
	etest("int f() {return 0;} int f() {return 1;}");
	etest("int f() {return 0;} void f() {}");
	etest("struct S{ int f() {return 0;} int f() {return 1;} };");

	expected = SE_DUPLICATE_TYPE;
	etest("struct S{}; struct S{};");
	etest("class S{}; struct S{};");
	expected = 0;
	etest("class A{}; void A();");
	etest("class A{}; int A;");

	expected = SE_DUPLICATE_SYMBOL;
	etest("int a; int a;");
	etest("int a; int c; char a;");
	etest("void f(); int f;");
	etest("int f; void f();");

	//enum
	expected = 0;
	etest("enum E{A, B, C};");

	expected = SE_DUPLICATE_SYMBOL;
	etest("enum E{A, B, C, A};");
	expected = SE_DUPLICATE_TYPE;
	etest("enum S{}; struct S{};");

	expected = 0;
	etest("enum {E1, E2, E3};");
	etest("enum {E1, E2, E3}; int e = E1;");
	expected = SE_ENUM_ASSIGN;
	etest("enum {E1 = \"x\"}; ");
	expected = 0;
	etest("enum {E1 = 0}; ");
	etest("enum {E1, E2 = E1}; ");
	etest("enum {E1}; enum {E2 = E1};");

	expected = SE_CAN_NOT_CONVERT;
	etest("enum E{E1}; E e = 0;");

	//address
	expected = 0;
	etest("int a; int *pa = &a; int **ppa = &pa;");
	etest("int a; int *pa = &a; int b = *pa;");
	etest("struct S { int i;}s; int *pi = &s.i;");
	etest("extern int f(int); int (*f_addr)(int x) = &f; void main() {int x = (*f_addr)(0);}");
	//pointer arithmetic is not allowed
	expected = SE_CAN_NOT_CONVERT;
	etest("int *p1; int *p2 = p1+1;");
	etest("int p1; int *p2 = &p1+1;");
	
	//switch case
	expected = 0;
	etest("void main() { int i; switch (i) { case 0: break; case 1: break;}}");
	etest("int f() {return 0;} void main() { switch (f()) { case 0: break; case 1: break;}}");
	expected = SE_SWITCH_COND_MUSTBE_INT;
	etest("void main() { int *i; switch (i) { case 0: break; case 1: break;}}");
	expected = SE_EXPECTED_CONST_EXP;
	etest("void main() { int i; switch (i) { case i: break;}}");
 	expected = SE_CAN_NOT_CONVERT;
	etest("void main() { int i; switch (i) { case \"0\": break;} }");
	expected = 0;
	etest("void main() { int i; switch (i) { default: break;} }");
	//expected = SE_SWITCH_DEFAULT;
	//etest("void main() { int i; switch (i) { default: break; default: break;} }");

	//if
	expected = 0;
	etest("void main() { int i; if (i) 0; else 1;}");
	etest("void main() { if (\"0\") 0; else 1;}");
	etest("void main() { if (int a = 0) 0; else 1;}");
	etest("int f(); void main() { if (f()) 0; else 1;}");
	etest("int f(); void main() { int a; if ((a = f()) == 0) 0; else 1;}");
	etest("void main() { if (0) 0; else if (1) 1; else if(2) 2; else 3;}");
	//loop
	expected = 0;
	etest("int main() { int n; for (int i = 0; i < n; ++i) 0;}");
	etest("int main() { int n; for (int i = 0, j = 0; i < n; ++i, --j) 0;}");
	etest("int f(); int g(); void h(); int main() { for (int i = f(); i < g(); h()) 0;}");
	etest("int main() {while (true);}");
	etest("int main() {while (true) {} }");
	etest("int main() {while (true) {if (1) continue; else break;} }");

#endif	
	//goto/label
	expected = 0;
	etest("int main() {start: goto end; end: return;}");
	expected = SE_UNDEFINED_LABEL;
	etest("int main() {start: goto end; return;}");
	expected = SE_DUPLICATE_LABEL;
	etest("int main() {start: int a; start: return;}");
	return 0;
}

namespace X {
	namespace Z {
		int z;
	}
	int x;
}

namespace A {
	int a = 0;
	namespace X {
		namespace Z {
			int z;
		}
		int x1;
	}

	namespace Y {
		namespace Z {
			int z1;
			namespace Z {
				int z2;
				namespace ZZ {
					int z3;
				}
			}
			int *pz = &Z::z2;
		}
		int y;
	}
}

