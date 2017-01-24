class Foobar {
  public void foo(int k) {
    int a = 2;
    a = a * (a = k);
    System.out.println(a);
  }

  public static void  main (String[] args) {
    Foobar f = new Foobar();
    f.foo(10);
  }

}
