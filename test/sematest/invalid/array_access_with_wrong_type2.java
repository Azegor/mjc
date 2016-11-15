class A {
  public static void main(String[] args) {
    A[] a = new A[1];
    a[new A()] = new A();
  }
}