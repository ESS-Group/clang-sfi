int g() {
    return 0;
}

int f() {
    int a = g();
	int b = 5, c = 6;
	int e;
	e = a + b + c;
	g();
	if (e) {
		return 0;
	} else {
		return 1;
	}
}
