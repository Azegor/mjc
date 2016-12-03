class if_all_true {
	public static void main(String[] args) {
		if (true && (1*1 == 1))
			System.out.println(1);
		if (true || false)
			System.out.println(2);
		if (1 < 5)
			System.out.println(3);
		if (123 <= 123)
			System.out.println(4);
		if (7 > 3)
			System.out.println(5);
		if (5 >= 5)
			System.out.println(6);
		if (null == null)
			System.out.println(7);
		if (3 == 3)
			System.out.println(8);
		if (null != new if_all_true())
			System.out.println(9);
		if (5 != 6)
			System.out.println(10);
		if (1 + 2 == 4 - 1)
			System.out.println(11);
		if (2 * 3 == 12 / 2)
			System.out.println(12);
		if (7 % 2 == 1)
			System.out.println(13);
	}
}
