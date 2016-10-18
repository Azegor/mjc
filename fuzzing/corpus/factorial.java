class Factorial {
public int fac(int n) {
if (n < 2)
return 1;
return n ∗ fac(n−1);
}
}
class Prog3 {
public static void main() {
Factorial f = new Factorial();
int n = f.fac(42);
System.out.println(n);
}
}
