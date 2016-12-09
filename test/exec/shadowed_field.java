class Test {
  public static void main(String[] args) {
    Test t = new Test();
    t.name = 2;
    System.out.println(t.test());
  }

  public int name;
  public int test() {
    int name = 42;
    return name;
  }
}
