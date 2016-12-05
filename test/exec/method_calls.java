class method_calls {
    public static void main(String[] args){
        System.out.println(new method_calls().run2(1, 435));
    }

    public int run(){
        return 1;
    }

    public int run2(int a, int b){
        return a / b * this.run();
    }
}