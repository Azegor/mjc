
class infinite_loop3 {
    public static void main(String[] args) {
        System.out.println(41);
        while (true) {
            if (false)
                ;
            else
                ;
            if (false)
                ;
            else
                System.out.println(22);
            if (true){
                System.out.println(33);
            } else {

            }
        }
    }
}
