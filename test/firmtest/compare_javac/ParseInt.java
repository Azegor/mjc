class ParseInt {
  public static void main(String[] args) {
    int[] places = new int[9];
    int i = 0;
    /* Generate input data:
       [2, 4, 6, 1, 3, 5, 0, 2, 4] */
    while (i < 9) {
      places[i] = (9 * (i + 1)) % 7;
      i = i + 1;
    }
    /* Do the actual parsing */
    i = 0;
    int res = 0;
    int pow = 1;
    while (i < 9) {
      res = res + places[8 - i] * pow;
      pow = pow * 10;
      i = i + 1;
    }
    /* 246135024 */
    System.out.println(res);
  }
}
