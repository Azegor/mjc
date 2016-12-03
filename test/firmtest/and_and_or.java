class Test {
  public static void main(String[] args) {}

  public boolean foo(boolean a, boolean b) {
    if (a) {
      if (a && b) {
        return a && b;
      }
    } else {
      if (a || b) {
        return a || b;
      }
    }
    while (a && b || b && a) {
      return (a || b) && (b || a);
    }
    return false;
  }
}
