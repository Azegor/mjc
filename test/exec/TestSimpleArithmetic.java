import lib.LehmerRandom;

class TestSimpleArithmetic {

  public static void main(String[] args) {
    new TestSimpleArithmetic().run();
  }

  public void run(){
    int maxInt = 2147483647;
    int minInt = -2147483648;
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
  }

  public void sep(){
    System.out.println(42424242);
  }

}
