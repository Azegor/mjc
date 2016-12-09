class Math {
  public static void main(String[] args) {}
  public int signum(int v) {return 0;}
  public int factorial(int val){
    int ret = 1;
    if (val < 0){
      val = -val;
    }
    while (val > 0){
      val = val - 1;
    }
    return ret;
  }
}
