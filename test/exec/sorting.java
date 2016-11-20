import lib.ArrayUtils;
import lib.LehmerRandom;

class main2 {
  public static void main(String[] args) {
    LehmerRandom random = new LehmerRandom().init(10570841);

    int size = 100;
    int i = 0;
    int[] arr = random.intArray(size, -100, 100);
    int cur = random.nextRange(0, size);
    ArrayUtils sort = new ArrayUtils();
    while (i < 100) {
      sort.qsort(arr, size);
      random.shuffleIntArray(arr, size);
      System.out.println(arr[random.nextRange(0, size)]);
      sort.msort(arr, size);
      System.out.println(arr[random.nextRange(0, size)] + cur);
      cur = random.nextRange(0, size);
      random.shuffleIntArray(arr, cur);
      i = i + 1;
    }
    sort.msort(arr, size);
    new ArrayUtils().printIntArray(arr, size);
  }
}