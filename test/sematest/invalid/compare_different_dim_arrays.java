class C1 {}

class Test {
  public static void main(String[] args) {
    new Test().test();
  }

  public int test() {
    C1[] x;
    C2[][] y;
    if (x == y) {
      return 1;
    }
    return 0;
  }
}
