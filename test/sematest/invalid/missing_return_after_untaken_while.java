class Test {
    public static void main(String[] args) {
        Test t = new Test();
        t.test();
    }

    public int test() {
        while (false) {
            return 42;
        }
    }
}
