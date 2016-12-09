class BitOps32 {

  public static void main(String[] args) {
    BitOps32 b = new BitOps32();
    System.out.println(b.i32and(4, 7));
  }

  public int i32and(int a, int b) {
    int r = 0;
    if (r < 1) {
      r = a*b;
    }
    return r;
  }
}
