class A {
  public static void main(String[] args) {
    boolean a = true && false;
    boolean b = a == false || true;
    b = true && false == (4 < 5 && 5 > 5);
    if (b)
      System.out.println(435);
    else
      System.out.println(4);
  }
}