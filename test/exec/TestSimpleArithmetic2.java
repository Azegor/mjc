
class TestSimpleArithmetic2 {

  public static void main(String[] args) {
    new TestSimpleArithmetic2().run();
  }

  public void run(){
    int maxInt = 2147483647;
    int minInt = -2147483648;
    System.out.println(maxInt + 1);
    System.out.println(minInt + 1);
    System.out.println(minInt - 1);
    System.out.println(maxInt - 1);
    sep();
    System.out.println(maxInt * 1);
    System.out.println(maxInt * 0);
    System.out.println(minInt * 1);
    System.out.println(minInt * 0);
    System.out.println(234234 % -23);
    System.out.println(-234234 % -23);
    System.out.println(-234234 % 23);
    System.out.println(234234 % 23);
    sep();
    System.out.println(maxInt * minInt);
    System.out.println(-maxInt);
    System.out.println(-minInt);
    System.out.println(maxInt / minInt);
    System.out.println(minInt * maxInt);
    sep();
    System.out.println(-2147483647 / -1);
    System.out.println(-2147483648 / -1);
    System.out.println(-2147483647 % -1);
    System.out.println(-2147483648 % -1);
    System.out.println(-1 % -2147483648);
    System.out.println(-(-2147483648));
  }

  public void sep(){
    System.out.println(42424242);
  }

}
