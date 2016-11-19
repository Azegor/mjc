import lib.LCG;
import lib.LehmerRandom;

/**
 * Calculate the tensor product for a dimension of 7
 */
class BigTensorProduct {

  public static void main(String[] args) {
    BigTensorProduct prod = new BigTensorProduct();
    prod.run(7, true);
    prod.run(12, false);
    prod.runWithNumbers(4, true);
    prod.runWithNumbers(5, false);
  }

  public void run(int n, boolean outputMatrix){
    LCG random = new LCG().initWithDefault2();
    int sum = 0;
    int[] indezes = new int[7];
    int j = 0;
    while (j < 7){
      indezes[j] = 0;
      j = j + 1;
    }
    int[][][][][][][] arr = randomIntArray(n);
    int[][] vectors = randomMatrix(7, n);
    while (indezes[0] < n) {
      while (indezes[1] < n) {
        while (indezes[2] < n) {
          while (indezes[3] < n) {
            while (indezes[4] < n) {
              while (indezes[5] < n) {
                while (indezes[6] < n) {
                  int val = 1;
                  int i = 0;
                  while (i < 7){
                    val = val * vectors[i][indezes[i]];
                    i = i + 1;
                  }
                  if (outputMatrix){
                    System.out.println(arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]]);
                  }
                  sum = sum + val * arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]];
                  indezes[6] = indezes[6] + 1;
                }
                indezes[6] = 0;
                indezes[5] = indezes[5] + 1;
              }
              indezes[5] = 0;
              indezes[4] = indezes[4] + 1;
            }
            indezes[4] = 0;
            indezes[3] = indezes[3] + 1;
          }
          indezes[3] = 0;
          indezes[2] = indezes[2] + 1;
        }
        System.out.println(sum);
        indezes[2] = 0;
        indezes[1] = indezes[1] + 1;
      }
      indezes[1] = 0;
      indezes[0] = indezes[0] + 1;
    }
    System.out.println(sum);
  }

  public void runWithNumbers(int n, boolean outputMatrix){
    LCG random = new LCG().initWithDefault2();
    Number sum = new Number().init(0);
    int[] indezes = new int[7];
    int j = 0;
    while (j < 7){
      indezes[j] = 0;
      j = j + 1;
    }
    Number[][][][][][][] arr = randomNumberArray(n);
    Number[][] vectors = randomNumberMatrix(7, n);
    while (indezes[0] < n) {
      while (indezes[1] < n) {
        while (indezes[2] < n) {
          while (indezes[3] < n) {
            while (indezes[4] < n) {
              while (indezes[5] < n) {
                while (indezes[6] < n) {
                  Number val = new Number().init(1);
                  int i = 0;
                  while (i < 7){
                    val = val.mul(vectors[i][indezes[i]]);
                    i = i + 1;
                  }
                  if (outputMatrix){
                    System.out.println(arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]].val);
                  }
                  sum = sum.add(val.mul(arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]]));
                  indezes[6] = indezes[6] + 1;
                }
                indezes[6] = 0;
                indezes[5] = indezes[5] + 1;
              }
              indezes[5] = 0;
              indezes[4] = indezes[4] + 1;
            }
            indezes[4] = 0;
            indezes[3] = indezes[3] + 1;
          }
          indezes[3] = 0;
          indezes[2] = indezes[2] + 1;
        }
        System.out.println(sum.val);
        indezes[2] = 0;
        indezes[1] = indezes[1] + 1;
      }
      indezes[1] = 0;
      indezes[0] = indezes[0] + 1;
    }
    System.out.println(sum.val);
  }

  public int[][] randomMatrix(int number, int n){
    LehmerRandom random = new LehmerRandom().initWithDefault();
    int[][] ret = new int[number][];
    int i = 0;
    while (i < number){
      ret[i] = new int[n];
      int j = 0;
      while (j < n){
        ret[i][j] = random.next();
        j = j + 1;
      }
      i = i + 1;
    }
    return ret;
  }

  public Number[][] randomNumberMatrix(int number, int n){
    LehmerRandom random = new LehmerRandom().initWithDefault();
    Number[][] ret = new Number[number][];
    int i = 0;
    while (i < number){
      ret[i] = new Number[n];
      int j = 0;
      while (j < n){
        ret[i][j] = new Number().init(random.next());
        j = j + 1;
      }
      i = i + 1;
    }
    return ret;
  }

  public int[][][][][][][] randomIntArray(int n){
    LCG lcg = new LCG().initWithDefault();
    int[][][][][][][] arr = new int[n][][][][][][];
    int[] indezes = new int[7];
    while (indezes[0] < n) {
      arr[indezes[0]] = new int[n][][][][][];
      while (indezes[1] < n) {
        arr[indezes[0]][indezes[1]] = new int[n][][][][];
        while (indezes[2] < n) {
          arr[indezes[0]][indezes[1]][indezes[2]] = new int[n][][][];
          while (indezes[3] < n) {
            arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]] = new int[n][][];
            while (indezes[4] < n) {
              arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]] = new int[n][];
              while (indezes[5] < n) {
                arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]] = new int[n];
                while (indezes[6] < n) {
                  arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]] = lcg.nextVal();
                  indezes[6] = indezes[6] + 1;
                }
                indezes[6] = 0;
                indezes[5] = indezes[5] + 1;
              }
              indezes[5] = 0;
              indezes[4] = indezes[4] + 1;
            }
            indezes[4] = 0;
            indezes[3] = indezes[3] + 1;
          }
          indezes[3] = 0;
          indezes[2] = indezes[2] + 1;
        }
        indezes[2] = 0;
        indezes[1] = indezes[1] + 1;
      }
      indezes[1] = 0;
      indezes[0] = indezes[0] + 1;
    }
    return arr;
  }

  public Number[][][][][][][] randomNumberArray(int n){
    LCG lcg = new LCG().initWithDefault();
    Number[][][][][][][] arr = new Number[n][][][][][][];
    int[] indezes = new int[7];
    while (indezes[0] < n) {
      arr[indezes[0]] = new Number[n][][][][][];
      while (indezes[1] < n) {
        arr[indezes[0]][indezes[1]] = new Number[n][][][][];
        while (indezes[2] < n) {
          arr[indezes[0]][indezes[1]][indezes[2]] = new Number[n][][][];
          while (indezes[3] < n) {
            arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]] = new Number[n][][];
            while (indezes[4] < n) {
              arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]] = new Number[n][];
              while (indezes[5] < n) {
                arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]] = new Number[n];
                while (indezes[6] < n) {
                  arr[indezes[0]][indezes[1]][indezes[2]][indezes[3]][indezes[4]][indezes[5]][indezes[6]] = new Number().init(lcg.nextRange(-1000, 1000));
                  indezes[6] = indezes[6] + 1;
                }
                indezes[6] = 0;
                indezes[5] = indezes[5] + 1;
              }
              indezes[5] = 0;
              indezes[4] = indezes[4] + 1;
            }
            indezes[4] = 0;
            indezes[3] = indezes[3] + 1;
          }
          indezes[3] = 0;
          indezes[2] = indezes[2] + 1;
        }
        indezes[2] = 0;
        indezes[1] = indezes[1] + 1;
      }
      indezes[1] = 0;
      indezes[0] = indezes[0] + 1;
    }
    return arr;
  }
}

class Number {

  public int val;

  public Number init(int val){
    this.val = val;
    return this;
  }

  public Number mul(Number other){
    return new Number().init(other.val * val);
  }

  public Number add(Number other){
    return new Number().init(other.val + val);
  }
}
