class LCG {
  public static void main(String[] args) {
    LCG lcg = new LCG();
    lcg.init(2147483629, 2147483587, 2147483647);
    lcg.runTest();
  }

  public int a;
  public int c;
  public int m;
  public int val;

  public void init(int a_, int c_, int m_) {
    a = a_;
    c = c_;
    m = m_;
    val = 0;
  }

  public int nextVal() {
    return val = (a * val + c) % m;
  }

  /* util functions */
  public int abs(int v) {
    if (v >= 0)
      return v;
    return -v;
  }

  public void runTest() {
    int i = 0;
    while (i < 100) {
      i = i + 1;
      System.out.println(abs(nextVal()));
    }
    /*System.out.println(-2147483648 / -1);*/
  }
}
