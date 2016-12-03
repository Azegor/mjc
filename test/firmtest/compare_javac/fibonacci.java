/*
 * fibonacci.mj -- Computes the 100000'th Fibonacci number.
 *
 * This uses the naive linear algorithm which is rather slow.  But implementing
 * a fast multiplication routine for multi-precision integers seemed like too
 * much work for now.
 *
 * The output is in the same format as explained in `hello_world.mj`.
 *
 */

class fib_Main {

	public static void main(String[] args) {
		int n = 100;
		int i = 2;
		MPI10 x = new MPI10();
		MPI10 y = new MPI10();
		x.init(0);
		y.init(1);
		while (i <= n) {
			x.plusEq(y);
			MPI10 temp = x;
			x = y;
			y = temp;
			i = i + 1;
		}
		y.print();
	}

}

class MPI10 {

	public void init(int value) {
		_M_size = 1;
		_M_capacity = 1;
		_M_data = new int[_M_size];
		_M_data[0] = value;
	}

	public void plusEq(MPI10 other) {
		int ndigits = 1;
		if (other._M_size > _M_size) {
			ndigits = ndigits + other._M_size;
		} else {
			ndigits = ndigits + _M_size;
		}
		_M_grow(ndigits);
		other._M_grow(ndigits);
		int i = 0;
		int carry = 0;
		int sum;
		while (i < ndigits) {
			sum = _M_data[i] + other._M_data[i] + carry;
			carry = sum / 10;
			_M_data[i] = sum % 10;
			i = i + 1;
		}
	}

	public void print() {
		int i = _M_size - 1;
		while ((i > 0) && (_M_data[i] == 0)) {
			i = i - 1;
		}
		while (i >= 0) {
			System.out.println(48 + _M_data[i]);
			i = i - 1;
		}
		System.out.println(10);
	}

	public int[] _M_data;
	public int _M_size;
	public int _M_capacity;

	public void _M_grow(int size) {
		if (size <= _M_capacity) {
			_M_size = size;
			return;
		}
		int capacity = 2 * _M_capacity;
		int[] temp = new int[capacity];
		int i = 0;
		while (i < _M_size) {
			temp[i] = _M_data[i];
			i = i + 1;
		}
		while (i < size) {
			temp[i] = 0;
			i = i + 1;
		}
		_M_data = temp;
		_M_size = size;
		_M_capacity = capacity;
	}

}
