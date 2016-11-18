class C1 {}
class C2 {}

class Test {
  public static void main(String[] args) {
    new Test().test();
  }

  public int test() {
    if (new C1[42] == new C2[42]) {
      return 1;
    }
    return 0;
  }
}
