class out {
  public void println(boolean a){
    if (a){
      System.out.println(1);
    } else {
      System.out.println(0);
    }
  }
}
class blub {
  public out out;

  public blub init(){
    out = new out();
    return this;
  }
}

class A {
  public blub System;

  public A init(){
    System = new blub().init();
    return this;
  }

  public static void main(String[] ab) {
    int System = 4;
    A a = new A().init();
    a.print();
    B b = new B();
    b.System();
  }

  public void print(){
    System.out.println(true);
    B b = new B();
    if (b.System())
      System.out.println(false);
  }
}

class B {
  public boolean System(){
    return true;
  }
}