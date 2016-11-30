class Foo {
  public static void main(String[] args) {
  }
  public boolean foo(int a, int b) {
      return (5 == 5) == false; 
  }
  public boolean bar(int a, int b) {
      return (a == b);
  }
  public boolean baz(int a, int b) {
/*    boolean b = true && false || true; */
    return false;
  }
}
