//jump
void f()
{
/*
	break ;
	| continue ;
	| return [ expression] ;
	| goto label-name ;
*/
 L1:

	while (true) {
		for (int i = 0; i < x; ++i) {
			break;
		}
		continue;
	}
	goto L1;

	return;
}

int g()
{
 start:
	return 0;
}

int h()
{
 start:
	int a;
	goto end;
 end:
	return 0, 1;
}
