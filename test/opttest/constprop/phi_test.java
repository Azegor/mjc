class Test {
  public int foo(boolean b){
    int res;
    if (b) {
      res = 42;
    } else {
      res = 42;
    }
    return res;
  }
  public static void main(String[] args){
    System.out.println(new Test().foo(true));
  }
}
