package lib;

import lib.BooleanUtils;
import lib.ASCII;

public class ArrayUtils {
  public ArrayUtils printIntArray(int[] arr, int size) {
    int i = 0;
    while (i < size) {
      System.out.println(arr[i]);
      i = i + 1;
    }
    return this;
  }

  public ArrayUtils printBooleanArray(boolean[] arr, int size) {
    int i = 0;
    while (i < size) {
      new BooleanUtils().println(arr[i]);
      i = i + 1;
    }
    return this;
  }

  public int[] copyIntArray(int[] src, int start, int length) {
    int[] ret = new int[length];
    int i = start;
    while (i < start + length) {
      ret[i - start] = src[i];
      i = i + 1;
    }
    return ret;
  }

  public int toInt(int[] chars, int size) {
    int val = 0;
    size = size - 1;
    while (size >= 0) {
      val = val * 10 + chars[size] - new ASCII().init()._0;
      size = size - 1;
    }
    return val;
  }

  public int[] qsort(int[] a, int size) {
    _qsort(a, 0, size - 1);
    return a;
  }

  /** Adapted from http://stackoverflow.com/a/29610583 */
  public void _qsort(int[] a, int left, int right) {
    if (right > left) {
      int i = left;
      int j = right;
      int tmp;

      int v = a[right];
      boolean breakLoop = false;
      while (!breakLoop) {
        while (a[i] < v) i = i + 1;
        while (a[j] > v) j = j - 1;

        if (i <= j) {
          tmp = a[i];
          a[i] = a[j];
          a[j] = tmp;
          i = i + 1;
          j = j - 1;
        }
        if (i > j) {
          breakLoop = true;
        }
      }
      if (left < j) _qsort(a, left, j);

      if (i < right) _qsort(a, i, right);
    }
  }

  public int[] _marr;

  public int[] msort(int[] arr, int size) {
    this._marr = arr;
    _msort(0, size - 1);
    return _marr;
  }

  public void _msort(int low, int high) {
    if (low < high) {
      int mid = ((low + high) / 2);
      _msort(low, mid);
      _msort(mid + 1, high);
      _msort_merge(low, mid, high);
    }
  }

  /*
  Adapted from http://stackoverflow.com/a/20039399
  */
  public void _msort_merge(int low, int mid, int high) {
    int[] temp = new int[high - low + 1];
    int left = low;
    int right = mid + 1;
    int index = 0;

    while (left <= mid && right <= high) {
      if (_marr[left] < this._marr[right]) {
        temp[index] = _marr[left];
        left = left + 1;
      } else {
        temp[index] = _marr[right];
        right = right + 1;
      }
      index = index + 1;
    }

    while (left <= mid || right <= high) {
      if (left <= mid) {
        temp[index] = _marr[left];
        left = left + 1;
        index = index + 1;
      } else if (right <= high) {
        temp[index] = _marr[right];
        right = right + 1;
        index = index + 1;
      }
    }
    int i = 0;
    while (i < high - low + 1) {
      _marr[low + i] = temp[i];
      i = i + 1;
    }
  }
}
