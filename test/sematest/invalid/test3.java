/* -*- coding:utf-8; mode:java; -*- */

class Test {

	public static void main(String[] args) { }

	public void f() {
		return f();  /* Valid C++, invalid (Mini)Java. */
	}

}
