#include "library.h"

int libraryfunction(int a) {
	int b = 1;
	int c = 2;
	int e = 3;
	e = b + c;
	b = e + a;
	removablefunctioncall();
	return 5;
}

int removablefunctioncall() {
	return 5;
}
