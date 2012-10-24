//selection
void f()
{
    if (x) {
    }

    if (y==0) {
        a = 0;
    } else {
        a = 1;
    }

    if (z==0, z==1) {
        b = 0;
    } else if (z==1) {
        b = 1;
    }

    if (int a = func()) {
    }
}

void g()
{
    switch (c, d) {
    case 0:
    case 1:
        break;
    case 2:
        z = 0;
        break;
    case 3: {
        int a;
    } break;
	default:
		break;
    }
}

