

class Foo1 {
  public int a;
}

class Foo2 {
  public int b;
}

class Foo3 {
  public static void main(String[] args) {
    Foo1 a = new Foo1();
    Foo2 b = new Foo2();

    boolean c = (a == b);
  }
}
