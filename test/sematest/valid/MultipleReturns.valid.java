class MultipleReturns {

	public static void main(String[] args) {
		MultipleReturns multipleReturns=new MultipleReturns();
		int c=multipleReturns.complexFunction(5, false, 12, true);
		int b=multipleReturns.complexFunction(423423, true, -554, false&&true);
		multipleReturns.complexFunction(c, c<b, b*c, c+b*12==12312);
	}

	public int complexFunction(int a, boolean b, int c, boolean d) {
		if (a > c && b != d) {
			if (!b == d || a / 10 <= 15) {
				if (a / 10 < c) {
					return 5 * c;
				}
				return 120;
			}
			return 12 * a + c;
		}
		return 0;
	}

}
