class Foobar {
    public int a;
    public static void main(String[] args){
      int a = 10;
      int b = 20;

      int i = 0;
      while(i < 1) {
        int tmp = a;
        a = b;
        b = tmp;

        i = i + 1;
        System.out.println(1337);
      }

      System.out.println(a);
      System.out.println(b);
    }
}
