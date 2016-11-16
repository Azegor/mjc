class Test {
  public int test() {
    int name;
    foo();
    int name; /* illegal redefinition */
  }
}
