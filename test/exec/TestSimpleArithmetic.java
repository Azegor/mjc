import lib.LehmerRandom;

class TestSimpleArithmetic {

  public static void main(String[] args) {
    new TestSimpleArithmetic().run();
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
    sep();
    System.out.println(maxInt * minInt);
    System.out.println(-maxInt);
    System.out.println(-minInt);
    int i = 10;
    int j = 10;
    LehmerRandom random = new LehmerRandom().init(10569345);
    while ((i = i-1) >= 0){
      while ((j = j-1) >= 0){
        sep();
        System.out.println(random.next() * random.nextRange(maxInt, minInt));
        System.out.println(random.next() / random.nextRange(1, 100));
        System.out.println(random.next() / random.nextRange(-100, 1));
        System.out.println(random.next() % random.nextRange(-100, 100));
        System.out.println(random.next() * -random.next());
        System.out.println(random.next() + random.next());
        System.out.println(random.next() - random.next());
      }
    }
    sep();
    System.out.println(maxInt / minInt);
    System.out.println(minInt * maxInt);
    sep();
    System.out.println(-2147483647 / -1);
    System.out.println(-2147483648 / -1);
  }

  public void sep(){
    System.out.println(42424242);
  }

}
