class Foo {
  public static void main(String[] args) {
  }
  public boolean foo(boolean b) {
    while (b) {
      if (b) {
        b = false;
      } else {
        b = true;
      }
    }
    return true;
  }
}
