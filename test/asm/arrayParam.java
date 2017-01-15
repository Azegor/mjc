class Foobar {
  public void sum(int[] numbers, int n) {
    System.out.println(numbers[0] + numbers[1]);
  }


  public static void main(String[] args) {
    Foobar f = new Foobar();
    int[] nums = new int[2];
    nums[0] = 12;
    nums[1] = -48;
    f.sum(nums, 2);
  }
}

