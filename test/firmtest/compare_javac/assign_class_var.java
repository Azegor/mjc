class assign_class_var {

  public int x;

  public static void main(String[] args) {
    assign_class_var[] arr = new assign_class_var[3];
    arr[2] = new assign_class_var();
    arr[2].x = 5;
    System.out.println(arr[2].x);
  }

}
