

class Foo {

  public int a () {
    if (true) {
      return true;
    } else if (false) {
      return true;
    } else if (!true) {
      return true;
    } else if (!false) {
      return true;
    } else {
      return true;
    }
    return false;
  }

  public static void main(String[] args) {
  }
}
