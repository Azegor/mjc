class C1 {}
class C2 {}

class Test {
  public static void main(String[] args) {
    new Test().test();
  }

  public int test() {
    if (new C1() == new C2()) {
      return 1;
    }
    return 0;
  }
}
