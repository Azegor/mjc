import lib.MersenneTwisterRNG;

public class mt_test {
  public static void main(String[] args) {
    MersenneTwisterRNG mt = new MersenneTwisterRNG().init();
    int i = 0;
    while (i < 100) {
      int j = 1;
      while (j <= 32) {
        System.out.println(mt.next(j));
        j = j + 1;
      }
      i = i + 1;
    }
  }
}
