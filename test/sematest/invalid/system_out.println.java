class A {
  public A a;
  public int blub;
  public static void main(String[] args) {
    System.out.println(new A().bla(new A()));
  }

  public A bla(A b){
    A a = new A();
    a.a = a;
    return a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a;
  }
}