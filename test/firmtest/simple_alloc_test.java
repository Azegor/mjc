class Test {
  public int[] test;
  public static void main(String[] args) {
    new Test().test(7);
  }

  public int[][] test(int i) {
    int[][] res = new int[10][];
    res[0] = new int[i + 0];
    res[1] = new int[i + 1];
    res[2] = new int[i + 2];
    res[3] = new int[i + 3];
    res[4] = new int[i + 4];
    res[5] = new int[i + 5];
    res[6] = new int[i + 6];
    res[7] = new int[i + 7];
    res[8] = new int[i + 8];
    res[9] = new int[i + 9];
    return res;
  }
}
