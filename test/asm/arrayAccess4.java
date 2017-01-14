class Foobar {
  public static void main(String[] args) {
    int[] a = new int[10];
    int[] b = new int[1];

    a[9] = 1337;
    System.out.println(a[9]);
    b[0] = a[9] - 1;
    System.out.println(b[0]);
  }
}
