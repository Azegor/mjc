class BubbleSort {
    public static void main(String[] args) {
        int len = 9;
        int[] input = new int[len];
        input[0] = 4;
        input[1] = 2;
        input[2] = 9;
        input[3] = 6;
        input[4] = 23;
        input[5] = 12;
        input[6] = 34;
        input[7] = 0;
        input[8] = 1;

        int n = len;
        int k;
        int m = n;
        while (m >= 0)
        {
            int i = 0;
            while (i < n - 1) {
                k = i + 1;
                if (input[i] > input[k]) {
                  /* swapNumbers(i, k, array) */
                  int temp = input[i];
                  input[i] = input[k];
                  input[k] = temp;
                }
                i = i + 1;
            }
            m = m - 1;
        }

        /* Print Numbers */
        int i = 0;
        while (i < len) {
            System.out.println(input[i]);
            i = i + 1;
        }
    }
}


