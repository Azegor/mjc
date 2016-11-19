class Foo {
    public void foo() {
        System.out.println(42);
    }

    public static void main(String[] args) {
        Foo f = new Foo();
        f.foo = 42;
    }
}
