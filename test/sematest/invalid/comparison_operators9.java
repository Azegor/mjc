class B {}

class A {
  public static void main(String[] args) {
    new A() == new B();
    new A() != new B();
    new A() == new boolean[1];
  }
}
