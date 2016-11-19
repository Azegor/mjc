class SysOut {
    public static void main (String[] args){
        Sys System = new Sys();
        System.out.println(1);
    }
}


class Sys {
	public Out out;
}


class Out {
    public void println(int number){
  	    System.out.println(number);
    }
}
