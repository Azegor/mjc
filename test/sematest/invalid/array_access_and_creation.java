class A {
  public A[] as;
  public A[][][] as3;
  public int[] ints;
  public int[][] ints2;
  public int[][][] ints3;

  public A init(){
    int size = 10;
    as = new A[10];
    as[1] = this;
    as3 = new A[10][][];
    as3[1] = new A[234231][];
    as3[1][1] = new A[2];
    as3[1][1][1] = this;
    int[] ints = new int[size * 3 / 4];
    this.ints = ints;
    ints2 = new int[11][];
    ints2[1] = ints;
    ints3 = new int[102][];
    ints3[1] = ints2;
  }

  public static void main(String[] args) {
    A a = new A();
    a.init();
    a.as3[1][1][1].as[1].ints3[3 * 4 / 12][2] = a.as3[1][1][1].ints2;
  }
}