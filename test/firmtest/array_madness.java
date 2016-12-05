class Test {
  public Test t;
  public Test[] ta;
  public Test[][] taa;

  public int i;
  public int[] ia;
  public int[][] iaa;

  public static void main(String[] args) {
    Test[] t;
    t[0].arrayMadness(t, new int[48]);
    /*new Test().arrayMadness(new Test[42], new int[48]);*/
  }

  public Test arrayMadness(Test[] _t_, int[] _i_) {
    taa[1][2] = _t_[42];
    ia[2] = ta[2].iaa[2][1];
    return taa[ta[1].iaa[1][_t_[_i_[1]].ia[4]]][ta[taa[1][0].ia[0]].iaa[_i_[1]][_i_[2]]];
  }
}
