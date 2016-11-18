package lib;

/* Adapted from: https://en.wikipedia.org/wiki/Lehmer_random_number_generator */
public class LehmerRandom {
  public int M; /* 2^31 - 1 (A large prime number) */
  public int A;      /* Prime root of M, passes statistical tests and produces a full cycle */
  public int Q; /* M / A (To avoid overflow on A * seed) */
  public int R; /* M % A (To avoid overflow on A * seed) */
  public int seed;

  public LehmerRandom init(int seed){
    this.M = 2147483647;
    this.A = 16807;
    this.Q = 127773;
    this.R = 2836;
    this.seed = seed;
    return this;
  }

  public int random() {
    int hi = seed / Q;
    int lo = seed % Q;
    int test = A * lo - R * hi;
    if (test <= 0)
      test = test + M;
    seed = test;
    return test;
  }

  public int next(){
    return random();
  }

  /**
   * @param min minimum number
   * @param max exclusive range end
   */
  public int nextRange(int min, int max){
    return next() % (max - min) + min;
  }

  public int[] intArray(int size, int min, int maxEx){
    int[] arr = new int[size];
    int i = 0;
    while (i < size){
      arr[i] = nextRange(min, maxEx);
      i = i + 1;
    }
    return arr;
  }

  public boolean nextBoolean(){
    return next() % 2 == 0;
  }

  public boolean[] booleanArray(int size){
    boolean[] arr = new boolean[size];
    int i = 0;
    while (i < size){
      arr[i] = nextBoolean();
      i = i + 1;
    }
    return arr;
  }

  public void shuffleIntArray(int[] arr, int size) {
    int i = size - 1;
    while (i > 0){
      int index = nextRange(0, i + 1);
      int a = arr[index];
      arr[index] = arr[i];
      arr[i] = a;
      i = i - 1;
    }
  }
}