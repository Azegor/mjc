class B {

}

class A {
  public static void main(String[] args) {
    new A() == new A();
    new A() != new A();
    new int[1] == new int[2];
    1 < 2;
    true && false;
    false && false;
    false || false;
    3 <= 4;
    5 >= 5;
    (new A() == null) && (new B() == null);
  }
}
