package lib;

public class Math {
  public int pow(int v, int exp){
    if (exp < 0){
      return 1 / pow(v, -exp);
    } else if (exp == 0){
      return 1;
    } else {
      int ret = 1;
      while (exp > 0){
        if (exp % 2 == 0){
          v = v * v;
          exp = exp / 2;
        } else {
          ret = ret * v;
          exp = exp - 1;
        }
      }
      return ret;
    }
  }

  public int factorial(int val){
    int ret = 1;
    int sign = signum(val);
    if (val < 0){
      val = -val;
    }
    if (val == 0){
      return 1;
    }
    while (val > 0){
      ret = ret * val;
      val = val - 1;
    }
    return ret * sign;
  }

  public int min(int s, int t){
    if (s < t){
      return s;
    }
    return t;
  }

  public int max(int s, int t){
    if (s > t){
      return s;
    }
    return t;
  }

  public int lengthInChars(int num){
    int len = 1;
    if (num < 0){
      len = len + 1;
      num = -num;
    }
    while (num > 10){
      num = num / 10;
      len = len + 1;
    }
    return len;
  }

  public int signum(int num){
    if (num == 0){
      return 0;
    }
    if (num < 0){
      return -1;
    }
    return 1;
  }

  public int abs(int num){
    if (num < 0){
      return -num;
    }
    return num;
  }

}
