
class Foo {
  public int a() {
   int k = 10;
   return k;
  }

  public int b () {
    return a();
  }

  public int c () {
    return b();
  }

  public static void main (String[] args) {

  }
}
