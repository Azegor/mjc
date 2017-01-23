class Foobar {
  public static void  main (String[] args) {
    int a = 10;
    int e = 50;

    int i = 0;
    while (i < 1) {
      int tmp = a;
       a = 3;
       e = tmp;

       i = i + 1;
    }

    System.out.println(a);
    System.out.println(e);
  }
}
