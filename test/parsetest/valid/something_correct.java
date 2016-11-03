class Valid4 {

    public int a;
    public boolean b;
    public ClassA x;

	public static void main(String[] args) { }

    public void fun1() {
        a = 0;
        a = 10;
        b = true;
        b = false;
        x = new ClassA();
        x = null;
    }
    
    public int fun2() { return 2; }
    public boolean fun3t() { return true; }
    public boolean fun3f() { return false; }

    public void fun4(int a, boolean b, ClassA c) { }
}

class ClassA { }