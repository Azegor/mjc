class Foobar {
  public int a;
  public int b;

  public int sum() {
    return this.a + this.b;
  }


  public static void main(String[] args) {
    Foobar f = new Foobar();
    f.a = 8;
    f.b = 4;
    Foobar f2 = new Foobar();
    f2.a = 16;
    f2.b = 32;

    int a = f.sum();
    System.out.println(a);
    int b = f2.sum();
    System.out.println(b);
  }
}
