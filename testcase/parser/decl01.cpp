int a, b;
int &x;
int * const x[a];
static int a;
int x(int a, int b);
int x(int a, int b, ...) const;
int *(*a(int))[3];
int x[2] = {1, 2};
int y[1] = {};
char *s = ("...");
int aa(a());

class X{class Y{};};
X const x = y;
const X x = y;

template <class T> class XT{};
const XT<int> x;

namespace zzz {
	template <class T> class XT {};
} 
const zzz::template XT<int> x;

typedef int INT, *PINT;
INT a;
PINT pa;

int a, ::X::Y;

