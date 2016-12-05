class ArrayListTest {

	public static void main(String[] args) {

		List list = new List();
		list.init();

		int i = 0;
		while (i < 34) {
			list.add(i);
			i = i + 1;
		}
		System.out.println(list.numberOfItems());
		System.out.println(list.actualSize());
		System.out.println(list.get(5));

		i = 0;
		while (i < 10) {
			list.remove(5);
			i = i + 1;
		}
		System.out.println(list.numberOfItems());
		System.out.println(list.actualSize());
		System.out.println(list.get(5));
	}

}

class List {

	public int size;
	public int items;
	public int threshold;
	public int[] list;

	public void init() {
		size = 0;
		items = 0;
		threshold = 10;
		size = threshold;
		items = 0;
		list = new int[size];
	}

	public void add(int item) {
		if (items < size) {
			list[items] = item;
			items = items + 1;
		} else {
			int[] array = new int[size + threshold];
			int i = 0;
			while (i < size) {
				array[i] = list[i];
				i = i + 1;
			}
			list = array;
			size = size + threshold;
			add(item);
		}
	}

	public int get(int nr) {
		if (nr < items) {
			return list[nr];
		}
		return -1;
	}

	public void remove(int nr) {
		int i = nr;
		while (i < items - 1) {
			list[i] = list[i + 1];
			i = i + 1;
		}
		items = items - 1;
		if (items <= size - threshold) {
			int[] array = new int[size - threshold];
			i = 0;
			while (i < items) {
				array[i] = list[i];
				i = i + 1;
			}
			list = array;
			size = size - threshold;
		}
	}

	public int numberOfItems() {
		return items;
	}

	public int actualSize() {
		return size;
	}

}
