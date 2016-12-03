/*
 * sudoku.mj -- A simple back-tracking Sudoku solver.
 *
 * By default, this program outputs a solution for an empty 4 x 4 x 4 Sudoku.
 * If you want to solve the one from today's newspaper instead, change the
 * argument to `sudoku.init` from 4 to 3 and set the constraints by calling
 * `sudoku.fix(row, col, value)` as often as needed.
 *
 * The output is in the same format as explained in `hello_world.mj`.
 *
 */

class Main {

	public static void main(String[] args) {
		Sudoku sudoku = new Sudoku();
		sudoku.init(4);
		/* sudoku.fix(0, 0, 1); */
		if (sudoku.solve()) {
			sudoku.print();
		}
	}

}


class Sudoku {

	public void init(int dim) {
		_M_dim = dim;
		int size = _M_dim * _M_dim * _M_dim * _M_dim;
		_M_board = new int[size];
		_M_fixed = new boolean[size];
		int idx = 0;
		while (idx < size) {
			_M_board[idx] = 0;
			_M_fixed[idx] = false;
			idx = idx + 1;
		}
	}

	public void fix(int i, int j, int value) {
		int idx = _M_getIndex(i, j);
		_M_board[idx] = value;
		_M_fixed[idx] = true;
	}

	public void print() {
		int digits = _M_dim * _M_dim;
		int i = 0;
		while (i < digits) {
			if (i % _M_dim == 0) {
				System.out.println(10);
			}
			int j = 0;
			while (j < digits) {
				if (j % _M_dim == 0) {
					System.out.println(32);
				}
				System.out.println(32);
				int d = _M_board[_M_getIndex(i, j)];
				if (d < 10) {
					d = d + 48;
				} else {
					d = 97 + d - 10;
				}
				System.out.println(d);
				j = j + 1;
			}
			System.out.println(10);
			i = i + 1;
		}
	}

	public int _M_dim;
	public int[] _M_board;
	public boolean[] _M_fixed;

	public void _M_printRow() {
		int i = 0;
		while (i < _M_dim) {
			System.out.println(43);
			int j = 0;
			while (j < _M_dim) {
				System.out.println(45);
				System.out.println(45);
				j = j + 1;
			}
			i = i + 1;
		}
		System.out.println(43);
		System.out.println(10);
	}

	public boolean solve() {
		int digits = _M_dim * _M_dim;
		int size = digits * digits;
		int idx = 0;
		while (idx < size) {
			int i = idx / digits;
			int j = idx % digits;
			if (_M_fixed[idx]) {
				idx = idx + 1;
			} else {
				int k = _M_board[idx];
				if (k == 0) {
					k = 1;
				}
				while (_M_isInRow(i, k) || _M_isInColumn(j, k) || _M_isInSquare(i / _M_dim, j / _M_dim, k)) {
					k = k + 1;
				}
				if (k <= digits) {
					_M_board[idx] = k;
					idx = idx + 1;
				} else {
					_M_board[idx] = 0;
					if (idx == 0) { return false; }
					idx = idx - 1;
					while (_M_fixed[idx]) {
						if (idx == 0) { return false; }
						idx = idx - 1;
					}
				}
			}
		}
		return true;
	}

	public int _M_getIndex(int i, int j) {
		return _M_dim * _M_dim * i + j;
	}

	public boolean _M_isInRow(int row, int value) {
		int digits = _M_dim * _M_dim;
		int j = 0;
		while (j < digits) {
			if (_M_board[_M_getIndex(row, j)] == value) {
				return true;
			}
			j = j + 1;
		}
		return false;
	}

	public boolean _M_isInColumn(int col, int value) {
		int digits = _M_dim * _M_dim;
		int i = 0;
		while (i < digits) {
			if (_M_board[_M_getIndex(i, col)] == value) {
				return true;
			}
			i = i + 1;
		}
		return false;
	}

	public boolean _M_isInSquare(int n, int m, int value) {
		int i = n * _M_dim;
		while (i < n + _M_dim) {
			int j = m * _M_dim;
			while (j < m + _M_dim) {
				if (_M_board[_M_getIndex(i, j)] == value) {
					return true;
				}
				j = j + 1;
			}
			i = i + 1;
		}
		return false;
	}

}
