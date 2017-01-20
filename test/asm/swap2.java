class Foobar {
    public int a;

    public void inc() {
        a = a + 1;
    }

    public static void main(String[] args){
      Foobar f1 = new Foobar();
      Foobar f2 = new Foobar();

      f1.a = 0;
      f2.a = 999;

      int i = 0;
      while(i < 1) {
        Foobar tmp = f1;
        f1 = f2;
        f2 = tmp;

        f2.inc();

        i = i + 1;
        System.out.println(1337);
      }

      System.out.println(f1.a);
      System.out.println(f2.a);
    }
}
