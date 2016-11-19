package lib;

/**
 * Linear congruence generator for pseudo random numbers.
 */
public class LCG {

  public int a;
  public int c;
  public int m;
  public int val;

  public LCG initWithDefault(){
    return init(2147483629, 2147483587, 2147483647);
  }

  public LCG initWithDefault2(){
    return init(2147480707, 2147480707, 2147482367);
  }

  public LCG init(int a_, int c_, int m_) {
    a = a_;
    c = c_;
    m = m_;
    val = 0;
    return this;
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

  /**
   * @param min minimum number
   * @param max exclusive range end
   */
  public int nextRange(int min, int max){
    return nextVal() % (max - min) + min;
  }
}