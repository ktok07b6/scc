// id_expression
namespace A {
	int b;
	namespace B {
		int c;
	}
	template <int I> class T {
		int c;
	};
}
int a;
void f()
{
	A::b;
	A::B::c;
	::a;
	::A::b;
	::operator +;
	operator +;
	operator char *;
	A<int>;
	A::template T::c;

}
