/* Line 7 is valid syntax, bordercase, pointing out SLL(3) decision between LocalVariableDeclarationStatement and Statement */
class Clazz {
    public boolean a;
    public int b;

    public static void main(String[] args) {
        if (true) {
            Clazz2[] myArray;
        }
    }
}

class Clazz2 {}