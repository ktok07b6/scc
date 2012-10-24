//unary & binary expression
void f()
{
	+1;
	-0;
	-(1-2);
	!x;
	~x;
	*x;
	&x;
	-x++;
	++x++;

	1+-2;
	1-2-3;
	1+2*3;
	1*2+3;
	
	a<<0;
	b>>1;
	
	a<b == a>b; a<=b != a>=b;
	a==b & a!=b;
	a & b | c ^ d;
	a&a && b|b || c<<~c;
}
