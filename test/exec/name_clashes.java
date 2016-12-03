class name_clashes {


}

class ZZ {
    public ZZ ZZ;
    public void main(int x) {
        System.out.println(x);
    }
    public void foo() {
        main(42);
    }
}

class BB {
    public int BB;
    public void BB(){

    }
    public void main(int x) {
        System.out.println(x);
    }
    public void foo() {
        main(42);
    }

}

class EqualIdentifiers {

    public int counter;

    public static void main(String[] args) {

        EqualIdentifiers equalIdentifiers = new EqualIdentifiers();
        equalIdentifiers.equalIdentifiers(equalIdentifiers.equalIdentifiers(new EqualIdentifiers()));
    }

    public EqualIdentifiers equalIdentifiers(EqualIdentifiers equalIdentifiers) {
        while (count() < 10) {
            System.out.println(equalIdentifiers.count());
        }
        return equalIdentifiers;
    }

    public int count() {
        counter = counter + 1;
        return counter;
    }
}