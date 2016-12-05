class Test {
  public static void main(String[] args) {}

  public int foo(int i) {
    int a = i;
    int b = a + i;
    int c = a * b * i;
    int d = (d = 4) + a + b + c;
    return a = b = c = d;
  }

  public int x;
  public int y;
  public int z;
  public int w;

  public int bar(int i) {
    int x = 1;
    int y = x + i;
    int z = x * y * i;
    int w = (w = 4) + x + y + z;
    return x = y = z = w;
  }
}
