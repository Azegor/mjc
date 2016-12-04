class A {
  public static void main(String[]s) {
    A a = new A();
    a.baz() != a.baz();
  }
  public void baz() {}
}
