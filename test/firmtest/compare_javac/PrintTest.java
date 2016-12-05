class PrintTest {

	public static void main(String[] args) {

		Printer printer = new Printer();

		int l = 9;
		int i = 0;

		while (i < l) {
			printer.startPrint();
			int h = l - 1;
			while (h >= 0) {
				printer.print(i > h);
				h = h - 1;
			}
			printer.endPrint();
			i = i + 1;
		}

		i = 0;
		while (i < l) {
			printer.startPrint();
			int h = l - 1;
			while (h >= 0) {
				printer.print(i < h);
				h = h - 1;
			}
			printer.endPrint();
			i = i + 1;
		}

	}

}

class Printer {

	public int print;

	public void startPrint() {
		print = 0;
	}

	public void print(boolean print1) {
		print = print * 10;
		if (print1) {
			print = print + 1;
		} else {
			print = print + 8;
		}
	}

	public void endPrint() {
		System.out.println(print);
	}
}