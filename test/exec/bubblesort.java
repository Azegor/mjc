class BubbleSort {
    public void sort(int[] array, int len) {
        int n = len;
        int k;
        int m = n;
        while (m >= 0)
        {
            int i = 0;
            while (i < n - 1) {
                k = i + 1;
                if (array[i] > array[k]) {
                    swapNumbers(i, k, array);
                }
                i = i + 1;
            }
            printNumbers(array, len);
            m = m - 1;
        }
    }

    public void swapNumbers(int i, int j, int[] array) {
        int temp;
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }

    public void printNumbers(int[] input, int len) {
        int i = 0;
        while (i < len) {
            System.out.println(input[i]);
            i = i + 1;
        }
    }

    public static void main(String[] args) {
        int[] input = new int[9];
        input[0] = 4;
        input[1] = 2;
        input[2] = 9;
        input[3] = 6;
        input[4] = 23;
        input[5] = 12;
        input[6] = 34;
        input[7] = 0;
        input[8] = 1;
        BubbleSort b = new BubbleSort();
        b.sort(input, 9);
    }
}
