#define TEST_1 5
#define TEST_2 6
#define CLOSING )

#define mymakro f(TEST_1 | TEST_2 CLOSING;
#define mymakro2 f(TEST_1 | TEST_2);

int f(int a) {
	return a;
}
int main(int argc, char ** argv) {
	int a = f(TEST_1 | TEST_2 CLOSING;
	int b = mymakro;
	int c = f(TEST_1 | TEST_2);
	int d = mymakro2;
	int e;
	e = a + b + c;
	if (e) {
		return 0;
	} else {
		return 1;
	}
}
