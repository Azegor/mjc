class B {
    public void f(int x) {}
}

class A {
    public int field;
    public static void main(String[] args) {
        B b = new B();
        b.f(field);
    }
}
