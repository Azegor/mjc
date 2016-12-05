class RectangleTest {

	public static void main(String[] args) {

		Rectanlge rectanlge = new Rectanlge();
		rectanlge.width = 15;
		rectanlge.height = 10;
		rectanlge.printSize();
	}

}

class Rectanlge {

	public int width;
	public int height;

	public void printSize() {
		System.out.println(width * height);
	}

}
