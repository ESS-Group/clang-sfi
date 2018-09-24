int f() {
	return 1;
}
int main(int argc, char ** argv) {
	int a = f();
	int b = 5, c = 6;
	int e;
	e = a + b + c;
	f();
	if (e) {
		return 0;
	} else {
		return 1;
	}
}
