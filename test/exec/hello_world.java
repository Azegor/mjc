/*
 * hello_world.mj -- Prints ASCII codes of "hello, world" message.
 *
 * This program is there to help you test your setup for the more complicated
 * examples that will follow.  It outputs the classical "hello, world" message,
 * followed by a new-line character.  Because MiniJava's I/O capabilities are
 * extremely limited, this is not directly possible.  Instead, the program
 * prints the decimal numbers of the bytes that it *would* print.  In order to
 * see the message, pipe the output to a program that reads decimal numbers --
 * one per line -- and for each integer read, outputs a byte with that value.
 * You can use the Java program in `Filter.java` for that purpose.
 *
 * I think it would be reasonable to amend the MiniJava specification by a
 * `System.out.write` builtin that would be trivial to implement and would
 * make this whole nonsense redundant.
 *
 */

class Main {

	public static void main(String[] args) {
		int[] salutation = new int[13];
		salutation[ 0] = 104;
		salutation[ 1] = 101;
		salutation[ 2] = 108;
		salutation[ 3] = 108;
		salutation[ 4] = 111;
		salutation[ 5] =  44;
		salutation[ 6] =  32;
		salutation[ 7] = 119;
		salutation[ 8] = 111;
		salutation[ 9] = 114;
		salutation[10] = 108;
		salutation[11] = 100;
		salutation[12] =  10;
		int i = 0;
		while (i < 13) {
			System.out.println(salutation[i]);
			i = i + 1;
		}
	}

}
