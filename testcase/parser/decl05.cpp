
template <class T>
void f(T a);

template <typename T, class U>
class X1
{
};

template <typename T>
class X1<T, int>
{
};

namespace z{
	class X{};
	template <class T> class Y {};
}
template <class T>
class X2
{
	T a;
	T *pa;
	typename z::X x;	
	typename z::template Y<void> y;
};


template <int I> 
class X3 
{
public:
	enum { Val = I };
};

X3<(16 > 1)> a;       // works
X3<(int(16) > 1)> b;  // works
X3<(16 >> 1)> c;      // works
X3<(int(16) >> 1)> d; // works
//X3<16 > 1> e;       // fails
//X3<int(16) > 1> f;  // fails
X3<16 >> 1> g;        // works (">>" is not a ">" token)
X3<int(16) >> 1> h;   // works (">>" is not a ">" token).
X3<static_cast<int>(1)> i; //works
X3<X3<10>::Val> j; //works

template <class T> class Y {};
template < template<class T> class T1 >
class X4 {};


template <class T1, class T2 = int, template<class T> class T3 = Y>
class X5 {};

template <int *PI, const X1<int, int> &x>
class X6 {};

//B<int> b;
