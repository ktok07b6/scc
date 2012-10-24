#define X
#define F(x)
#if defined X
"X is defined"
#endif

#if !defined(Y)
"Y is not defined"
#endif

#define Y

#if defined(Y)
"Y is defined"
#endif

#undef X
#if !defined(X)
"X is not defined"
#endif

#if !defined X && defined Y
"X is not defined and Y is defined"
#endif

#if defined X || defined Y
"X is defined or Y is defined"
#endif

