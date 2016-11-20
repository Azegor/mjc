package lib;

public class ArrayUtils {
  public ArrayUtils printIntArray(int[] arr, int size){
    int i = 0;
    while (i < size){
      System.out.println(arr[i]);
      i = i + 1;
    }
    return this;
  }

  public int[] copyIntArray(int[] src, int start, int length){
    int[] ret = new int[length];
    int i = start;
    while (i < start + length){
      ret[i - start] = src[i];
      i = i + 1;
    }
    return ret;
  }

  public int toInt(int[] chars, int size){
    int val = 0;
    size = size - 1;
    while (size >= 0){
      val = val * 10 + chars[size] - new ASCII().init()._0;
      size = size - 1;
    }
    return val;
  }
}

