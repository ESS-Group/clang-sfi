#define meinmakro (length(), length())
#define five5(a) x(a)
#define four strtest

#define five6 x

#define fullmacro x(strtest)

#define encompassingmacro int ea = 5; \
	int eb = 5; \
	x(ea);

#define encompassingmacrofunction() int ec = 5; \
	int ed = 6; \
	x(ec);

int length() {
	return 1;
}
bool x(int str) {
	return str != 0;
}
void getStr(int &str) {
	str = 5;
}
int main(int argc, char ** argv) {
	int a[1];
	a[0] = 1;
	int b = 5, c = 6;
	int e;
	e = a[0] + b + c;
	int size = 99;
	for (int i = 0, size = length(); i < size; i++) {
		a[i] = 5;
	}
	int strtest = 2;
	x(strtest);
	five5(four); // Should not be removed
	five6(four); // Should not be removed
	five6(strtest); // Should not be removed
	x(four); // Should be removed
	fullmacro; // Should be removed
	encompassingmacro; // Should be removed
	encompassingmacrofunction(); // Should be removed

	return a[0];
}
