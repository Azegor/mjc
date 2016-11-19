class BubbleSort{

	public static void main(String[] args){
		BubbleSort bsort = new BubbleSort();
		int[] list = new int[2];
		list[0] = 2;
		list[1] = 0;
		int[] result = bsort.bubbleSort(list, 2);
    	System.out.println(result[0]);
    	System.out.println(result[1]);
	}


	public int[] bubbleSort(int[] list, int length) {
		int tmp = 0;
		int i = 1;
		while(i < length) {
			int j = 0;
			while(j < length - i) {
				if(list[j] > list[j + 1]) {
					tmp = list[j];
					list[j] = list[j + 1];
					list[j + 1] = tmp;
				}
              j = j + 1;
			}
          	i = i + 1;
		}
		return list;
	}
}
