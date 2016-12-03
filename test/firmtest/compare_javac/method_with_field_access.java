class method_with_field_access {

  public int x;

  public int m() {
    return x;
  }

  public static void main(String[] args) {
    System.out.println(new method_with_field_access().m());
  }

}
