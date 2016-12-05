class HitboxTest {

	public static void main(String[] args) {

		Position p = new Position();
		p.x = 100;
		p.y = 50;

		Hitbox b = new Hitbox();
		b.x = 160;
		b.y = 90;
		b.w = 30;
		b.h = 20;

		while (!b.contains(p)) {
			p.move(2, 1);
		}
		System.out.println(p.x);
		System.out.println(p.y);
	}

}

class Position {
	public int x;
	public int y;

	public void move(int xstep, int ystep) {
		x = x + xstep;
		y = y + ystep;
	}
}

class Hitbox {
	public int x;
	public int y;
	public int w;
	public int h;

	public boolean contains(Position p) {
		return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
	}
}
