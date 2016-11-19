class MatrixMultiplication {
    public static void main (String[] args){}
    public int[][] calculate(int[][] left, int[][] right, int length) {
        int[][] result = new int[length][];
        int i = 0;
        while(i < length) {
            i = i + 1;
            int j = 0;
            while(j < length) {
                j = j + 1;
                int k = 0;
                while(k < length) {
                    k = k + 1;
                    result[i][j] = result[i][j] + left[i][k] * right[k][j];
                }
            }
        }
        return result;
    }
}
