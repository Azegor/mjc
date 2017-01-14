class Foobar {
  public static void main(String[] args) {
    int fibLength = 20;

    int[] nums = new int[fibLength];
    nums[0] = 0;
    nums[1] = 1;
    int k = 2;

    while (k < fibLength) {
      nums[k] = nums[k - 1] + nums[k - 2];
      k = k + 1;
    }

    k = 0;
    while (k < fibLength) {
      System.out.println(nums[k]);
      k = k + 1;
    }

  }
}
