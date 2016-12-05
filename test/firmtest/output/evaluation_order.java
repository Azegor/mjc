class Test {
  public static void main(String[] args){
    int a = 1;
    int x = (a = 3) * a;
    System.out.println(x);
    /* -> 9 */
  }
}
