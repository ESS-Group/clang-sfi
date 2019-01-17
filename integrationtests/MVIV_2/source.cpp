int main(int argc, char ** argv) {
	// MVIV should not match here, because const variables must always be initialized.
	const int b = 0;
	return b;
}
