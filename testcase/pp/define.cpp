#elif 1
#endif

#define A -"B B" 
#define B C C
A B;
CD;

#define FUNC(x) x * x
#define FUNC2(x,y) x * y
FUNC (1);
FUNC(2);
FUNC(FUNC(3));
FUNC;

FUNC2 (1,2);
FUNC2(3,4);
FUNC2(FUNC2(5,6), FUNC2(7,8));
FUNC2;

#undef A
A;
FUNC ( 1 , 2 );//expect error



