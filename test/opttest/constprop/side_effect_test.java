class Test {
  public int foo(){
    int x;
    if (42 == 42) {
      x = 7;
      System.out.println(1);
    } else {
      x = 9;
      System.out.println(0);
    }
    return x;
  }
  public static void main(String[] args){
    System.out.println(new Test().foo());
  }
}
