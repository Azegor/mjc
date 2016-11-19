class Test {
    public static void main(String[] args) {
        Test t = new Test();
        t.test();
    }

    public boolean test() {
        if (1 == 2) {
            return true;
        } else {
            if (1 != 1) {
                int b = 55;
            } else {
                return false;
            }
        }
    }
}
