class Foobar {
  public boolean[] a;
  public boolean[] b;

  public void foo() {
    this.b[42] = false;
  }
  public void bar() {
    b[42] = true;
  }

  public boolean baz(int idx) {
    return a[idx] = b[idx];
  }


  public static void main(String[] args) {
  }
}
