class Foobar {
  public static void main(String[] args) {
    boolean[] b = new boolean[2];
    b[1] = true;
    /* This store should not override the value in b[1]! */
    b[0] = false;

    if (b[1]) {
      System.out.println(1);
    }
  }
}
