
class short_circuiting {

    public int counter;

    public void print(){
        System.out.println(counter = counter + 1);
    }

    public boolean printWithReturn(boolean ret){
        System.out.println(counter = counter + 1);
        return ret;
    }

    public void printWithIncr(int incr){
        System.out.println(counter = counter + incr);
    }

    public void run(){
        if (printWithReturn(true)){
            System.out.println(34);
        } else if (!printWithReturn(false) || true){
            System.out.println(435345);
        }
        if (!printWithReturn(false) || true){
            System.out.println(435345);
        }
        if (printWithReturn(true) || printWithReturn(false)){
            System.out.println(345);
        }
        if (printWithReturn(false) || printWithReturn(true)){
            System.out.println(34552);
        }
        if (printWithReturn(true) && printWithReturn(false)){
            System.out.println(34522);
        }
        if (printWithReturn(false) && printWithReturn(true)){
            System.out.println(3455);
        }
        if (printWithReturn(false) && printWithReturn(false)){
            System.out.println(34555);
        }
        if (printWithReturn(true) && printWithReturn(true)){
            System.out.println(3455555);
        }
        if ((printWithReturn(false) || (printWithReturn(true) || !printWithReturn(false))) && printWithReturn(true)){
            System.out.println(42);
        }
        boolean b = printWithReturn(true) || printWithReturn(false);
        short_circuiting s;
        boolean c = (s = new short_circuiting().init(39)).counter == 3 && (s = new short_circuiting().init(22)).counter == 22;
    }

    public short_circuiting init(int counterStart){
        counter = counterStart;
        return this;
    }

    public static void main(String[] args) {
        new short_circuiting().run();
    }
}
