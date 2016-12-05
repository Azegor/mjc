class A1 {
  public A2 a;
  public int i;

  public A1 init() {
    this.i = 1;
    this.a = new A2();
    return this;
  }
}

class A2 {
  public A3 a;
  public int i;

  public A2 init() {
    this.i = 2;
    this.a = new A3();
    return this;
  }
}

class A3 {
  public A4 a;
  public int i;

  public A3 init() {
    this.i = 3;
    this.a = new A4();
    return this;
  }
}

class A4 {
  public A5 a;
  public int i;

  public A4 init() {
    this.i = 4;
    this.a = new A5();
    return this;
  }
}

class A5 {
  public A6 a;
  public int i;

  public A5 init() {
    this.i = 5;
    this.a = new A6();
    return this;
  }
}

class A6 {
  public A7 a;
  public int i;

  public A6 init() {
    this.i = 6;
    this.a = new A7();
    return this;
  }
}

class A7 {
  public A8 a;
  public int i;

  public A7 init() {
    this.i = 7;
    this.a = new A8();
    return this;
  }
}

class A8 {
  public A9 a;
  public int i;

  public A8 init() {
    this.i = 8;
    this.a = new A9();
    return this;
  }
}

class A9 {
  public A10 a;
  public int i;

  public A9 init() {
    this.i = 9;
    this.a = new A10();
    return this;
  }
}

class A10 {
  public A1 a;
  public int i;

  public static void main(String[] args) {
    System.out.println(
        new A10()
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init().i);
    System.out.println(
        new A2()
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .init()
            .a
            .i);
  }

  public A10 init() {
    this.i = 50;
    this.a = new A1();
    return this;
  }
}
