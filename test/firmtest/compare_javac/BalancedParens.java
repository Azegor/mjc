class BalancedParens {
  public int size;
  public int[] parens;

  public boolean check() {
    return size % 2 == 0 && sameOpenAsClose() && isBalanced();
  }

  public boolean sameOpenAsClose() {
    int open; int close;
    int i = open = close = 0;
    while (i < size) {
      if (parens[i] == 1) {
        open = open + 1;
      } else {
        close = close + 1;
      }
      i = i + 1;
    }
    return open == close && open == size / 2;
  }

  public boolean isBalanced() {
    int i = 0;
    while (i >= 0 && i < size)
      i = recursiveCheck(i);
    return i == size;
  }

  /**
   * @return -1 if we run out of parens to check or the position where
   *         the paren at pos is closed + 1
   */
  public int recursiveCheck(int pos) {
    if (pos + 1 >= size)
      return -1;
    if (parens[pos] != 1)
      return -1;
    pos = pos + 1;
    while (pos >= 0 && parens[pos] != 0)
      pos = recursiveCheck(pos);
    return pos + 1;
  } 

  public static void main(String[] args) {
    BalancedParens test = new BalancedParens();
    int size = 20;
    test.size = size;
    test.parens = new int[size];
    /* Create input data */
    test.parens[ 0] = 1; /* (                      */
    test.parens[ 1] = 1; /* ((                     */
    test.parens[ 2] = 0; /* (()                    */
    test.parens[ 3] = 1; /* (()(                   */
    test.parens[ 4] = 1; /* (()((                  */
    test.parens[ 5] = 0; /* (()(()                 */
    test.parens[ 6] = 0; /* (()(())                */
    test.parens[ 7] = 0; /* (()(()))               */
    test.parens[ 8] = 1; /* (()(()))(              */
    test.parens[ 9] = 0; /* (()(()))()             */
    test.parens[10] = 1; /* (()(()))()(            */
    test.parens[11] = 0; /* (()(()))()()           */
    test.parens[12] = 1; /* (()(()))()()(          */
    test.parens[13] = 1; /* (()(()))()()((         */
    test.parens[14] = 1; /* (()(()))()()(((        */
    test.parens[15] = 1; /* (()(()))()()((((       */
    test.parens[15] = 0; /* (()(()))()()(((()      */
    test.parens[15] = 0; /* (()(()))()()(((())     */
    test.parens[16] = 0; /* (()(()))()()(((()))    */
    test.parens[17] = 1; /* (()(()))()()(((()))(   */
    test.parens[18] = 0; /* (()(()))()()(((()))()  */
    test.parens[19] = 0; /* (()(()))()()(((()))()) */
    /* Print 1 */
    if (test.check())
      System.out.println(1);
    else
      System.out.println(0);
  }
}
