class Foobar {
  public static void main(String[] args) {
    boolean a = System.in.read() == 97;
    boolean b = System.in.read() == 98;
    boolean c = System.in.read() == 99;

    if (a && b && c) {
      System.out.println(1);
    } else {
      System.out.println(0);
    }
  }
}
