class A {
  public A a;
  public int b;
  public static void main(String[] args) {

  }

  public void blub(A a, int n){
    this.a = a;
    this.b = n;
    blub(a, n);
    this.blub(a, n);
  }
}

