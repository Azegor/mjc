class SimpleMethods {

	public static void main(String[] args) {
		SimpleMethods simpleMethods = new SimpleMethods();
		simpleMethods.isTrue(simpleMethods.isBiggerThanTwelve(simpleMethods.multiply(12, 2)));
	}

	public int multiply(int x, int y) {
		return x * y;
	}

	public boolean isBiggerThanTwelve(int x) {
		return x > 12;
	}

	public boolean isTrue(boolean b) {
		return b == true;
	}

}
