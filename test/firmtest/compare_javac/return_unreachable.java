class return_unreachable {
  public static void main(String[] args) {
    if (true) {
      return;
    } 
    System.out.println(1);
  }
}
