class Foobar {

  public void sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    System.out.println(a +
                       b +
                       c +
                       d +
                       e +
                       f +
                       g +
                       h +
                       i +
                       j);
  }


  public static void main(String[] args) {
    Foobar f = new Foobar();
    f.sum(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  }
}

