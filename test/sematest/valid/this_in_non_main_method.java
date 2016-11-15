class A {
  public int bla(){
    return this.bla();
  }

  public static void main(String[] args) {
    new A().bla();
  }
}