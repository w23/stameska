int testShaderSource();
int testRenderDesc();

int main() {
	int failed = 0;

	failed += testShaderSource();
	failed += testRenderDesc();

	return failed;
}
