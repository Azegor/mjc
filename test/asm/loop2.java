class Foobar {
  public static void main (String[] foobar) {
    int k = 0;
    while (k < 100) {
      if (k > 50) {
        /* Should only print 0-50(inclusive) */
        return;
      }
      System.out.println(k);
      k = k + 1;
    }
  }
}
