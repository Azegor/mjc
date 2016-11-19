/* -*- coding:utf-8; mode:java; -*- */

class Test {

	public static void main(String[] args) { }

	public int test() {
		/* Not valid Java but valid MiniJava. */
		return new int[3][new int[2][new int[1][0]]];
	}

}
