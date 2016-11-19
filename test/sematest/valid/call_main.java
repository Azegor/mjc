class Foo {
    public void main(int x) {
        System.out.println(x);
    }
    public void foo() {
        main(42);
    }
}

class Bar {
    public static void main(String[] args) {
        Foo f = new Foo();
        f.foo();
    }
}
