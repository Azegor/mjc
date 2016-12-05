class Fibonacci {
    public int a;
    public int b;

    public static void main(String[] args) {
        Fibonacci f = new Fibonacci();

        f.calcNNumbers(20);
    }

    public void calcNNumbers(int n) {
        this.a = 1;
        this.b = 1;

        System.out.println(1);
        System.out.println(1);

        int i = 0;

        while (i < n) {
            System.out.println(calcNextNumber());
            i = i + 1;
        }
    }

    public int calcNextNumber() {
        int c = a;

        this.a = b;
        b = c + b;

        return b;
    }
}