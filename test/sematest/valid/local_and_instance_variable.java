class A {
  public static void main(String[] args) {
    new A().run();
  }

  public int x;

  public void run(){
    int x = (x = 2) * 2;
  }
}