
class InvalidReturns {

	
	public static void main(String[] args) {
		
	}
	
	public void returner1()
	{
		return 12; 
	}
	
	public int returner2()
	{
		if(false)
		{
			return 1;
		}
		return true;
	}
	
}
