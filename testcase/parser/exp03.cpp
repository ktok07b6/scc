//postfix expression
void f()
{

a[0];
f(x);
f(x, y, z);
a[0](x);
a[0](x)(y);
a[0, 1, 2];
int(0);
char()++;
a++--;

static_cast<int>(x);
(const signed int)x;
static_cast<char>(static_cast<int>(0));
x.y;
x->y();
}
