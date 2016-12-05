class Test {
  public static void main(String[] args) {}

  public void illegal() {
    this = new Test();
  }
}
