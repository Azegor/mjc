class Test {
  public static void main(String[] args) {}

  public void foo(boolean a, boolean b) {
    if (a) {
      if (a && b) {
        return;
      }
    } else {
      if (a || b) {
        return;
      }
    }
  }
}
