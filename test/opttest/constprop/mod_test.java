class Test {
  public int foo(){ return 1 % 2;}
  public static void main(String[] args){
    System.out.println(new Test().foo());
  }
}
