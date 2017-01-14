class Foobar {
  public static void main(String[] args) {
    int[][] ints = new int[1][];
    ints[0] = new int[2];
    ints[0][0] = 1337;
    ints[0][1] = 10;

    System.out.println(ints[0][0]);
    System.out.println(ints[0][1]);
  }
}

