class O {
  	public void println(int i) {
    	System.out.println(i - 1);
  	}
}
class S {
  	public O out;
}
class C {
	public S System;
	public static void main(String[] args) {
		C c = new C();
		c.System = new S();
		c.System.out = new O();
		c.doit();
	}
	public void doit() {
		System.out.println(1);
	}
}
