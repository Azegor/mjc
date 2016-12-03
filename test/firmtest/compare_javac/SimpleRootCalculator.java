class SimpleRootCalculator {

	public static void main(String[] args) {

		Rooter r = new Rooter();
		
		System.out.println(r.sqaureRoot(16));
		System.out.println(r.sqaureRoot(70756));
		System.out.println(r.sqaureRoot(347));
	}

}

class Rooter {

	public int sqaureRoot(int number) {
		int i = 1;
		while (i * i != number) {
			if (i >= number) {
				return -1;
			}
			i = i + 1;
		}
		return i;
	}

}