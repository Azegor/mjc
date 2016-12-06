class Test {
  public int foo(){ return b2i(!true);}
  public static void main(String[] args){
    System.out.println(new Test().foo());
  }
  public int b2i(boolean b) {
    if (b) return 1; return 0;
  }
}
