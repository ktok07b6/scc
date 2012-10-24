#define CAT(x,y)     x ## y

CAT(a,b);

#define TOSTR(s) # s
TOSTR(CAT(he, llo))

#define MULTI_LINE() \
	do {			 \
	printf(TOSTR(CAT(he, llo))); \
	} while (false)

MULTI_LINE();

