
 class NestedScopes {

	 
	 public static void main(String[] args) {
		
		 int a=0;
		 while(a<5)
		 {
			 if(a>2)
			 {
				 int b=a;
				 {
					 b=b*5;
					 {
						 if(b>a)
						 {
							 b=a;
						 }
					 }
				 }
			 }
			 a=a+1;
		 }		 
	}
	 
}
