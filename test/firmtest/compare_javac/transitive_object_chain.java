class transitive_object_chain {
    public static void main(String[] args) {
        A_2 a = new A_2();
        A_2 b = new A_2();
        a.ref = b;
        b.ref = a;
        a.n = 5;
        System.out.println(a.ref.ref.n);
    }
}

class A_2 {
    public A_2 ref;
    public int n;
}
