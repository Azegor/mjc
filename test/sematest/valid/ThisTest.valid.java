
class ThisTest {

	public int t;
	
	public static void main(String[] args) {
		ThisTest thisTest=new ThisTest();
		thisTest.t=5;
		thisTest.thisTester(12);
	}
	
	public void thisTester(int t)
	{
		if(t==t&&this.t==this.t)
		{
			this.t=t*2;
			if(t==this.t)
			{
				/* error */
			}
		}
	}
	
	
}
