class B {

}

class A {
  public static void main(String[] args) {
    new A() == new A();
    new A() == new B();
    new A() != new A();
    new A() != new B();
    new int[1] == new int[2];
    new A() == new boolean[1];
    1 < 2;
    true && false;
    false && false;
    false || false;
    3 <= 4;
    5 >= 5;
    (new A() == null) && (new B() == null);
  }
}