class B {
    public B b;
    public int c;

    public static void main(String[] a) {
        B b = new B();
        b.b = new B();
        b.b.b = new B();
        System.out.println(b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.b.c);
    }
}
