package lib;

public class ASCII {
  public int ws;
  public int _0;
  public int _9;
  public int lparen;
  public int rparen;
  public int plus;
  public int minus;
  public int star;
  /**
   * /
   */
  public int slash;
  /**
   * !
   */
  public int exclm;
  /**
   * %
   */
  public int mod;

  public ASCII init(){
    this.ws = 32;
    lparen = 40;
    rparen = 41;
    this._0 = 48;
    this._9 = 57;
    star = 42;
    plus = 43;
    minus = 45;
    slash = 47;
    exclm = 33;
    mod = 37;
    return this;
  }
}