#include <iostream>

int length() {
	return 1;
}
bool checkStr(std::string str) {
	return str != "STOP";
}
void getStr(std::string &str) {
	std::cin >> str;
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
	b = (length(), length());

	std::string str;
	while (getStr(str), checkStr(str)) {
		a[0] = 5;
	}

	return a[0];
}
