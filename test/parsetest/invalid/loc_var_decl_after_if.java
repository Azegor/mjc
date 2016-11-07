/*Line 5: LocalVariableDeclarationStatement must be inside a block*/
class Main{
	public void main(String[] args){
		if(0 != 1)
			int a = 1;
		return;
	}
}