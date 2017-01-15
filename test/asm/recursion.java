class Foobar {
  public int value;

  public void rec() {
    value = value + 1;
    if (value < 20)
      this.rec();
  }


  public static void main(String[] args) {
    Foobar f = new Foobar();
    f.value = 0;
    f.rec();
    System.out.println(f.value);
  }
}

