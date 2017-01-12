class Foobar {
  public int a;
  public int b;
  public static void main(String[] args) throws Foobar {
    Foobar f = new Foobar();
    f.b = 1337;
    System.out.println(f.a);
    System.out.println(f.b);
  }
}
