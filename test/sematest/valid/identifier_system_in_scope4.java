class A {
  public static void main(String[] ab) {
    int System = 4;
    A a = new A();
    a.print();
    B b = new B();
    b.System();
  }

  public void print(){
    System.out.println(45);
    B b = new B();
    if (b.System())
      System.out.println(1);
  }
}

class B {
  public boolean System(){
    return true;
  }
}