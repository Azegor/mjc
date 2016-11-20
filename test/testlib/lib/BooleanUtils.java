package lib;

public class BooleanUtils {

  public int toInt(boolean value){
    if (value){
      return 1;
    } else {
      return 0;
    }
  }

  public BooleanUtils println(boolean value){
    System.out.println(toInt(value));
    return this;
  }
}
