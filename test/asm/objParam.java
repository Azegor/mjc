
class Foobar {
  public int value;

  public boolean cmp(Foobar other) {
    return other.value > value;
  }


  public static void main(String[] args) {
    Foobar f = new Foobar();
    f.value = 10;
    Foobar f2 = new Foobar();
    f2.value = 20;

    if (f.cmp(f2)) {
      System.out.println(1);
    } else {
      System.out.println(0);
    }


    f.value = 1000;
    f2.value = -400;

    if (f.cmp(f2)) {
      System.out.println(1);
    } else {
      System.out.println(0);
    }


  }
}

