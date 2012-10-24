//function definition

void f1(int a) { 
	return 0; 
}

void f2(int *x, int &y) {
     int a;
	 //return a;
}

int f3(int &(*x)(const unsigned int)) {
	 return x;
}

int f4(const int &x = int());

namespace X {
	class Y {};
}
int f5(const X::Y *x) {
	return 0;
}

void f6(int, int *, int *[3], int (*)[3], int *(), int(*)(char*));
void f7(int i, int *pi, int *p[3], int (*p3i)[3], int *f(), int(*pf)(char*));

