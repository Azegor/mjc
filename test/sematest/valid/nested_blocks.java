class A {
  public int a;
  public static void main(String[] args) {
    int a = 4;
    {
      {
        {
          {
            a = 5;
            {
              {
                {
                  {
                    {
                      {
                        {
                          {
                            {
                              System.out.println(a);
                              System.out.println(this.a);
                              {
                                {
                                  {
                                    {
                                      if(true){
                                        System.out.println(a);
                                        {
                                          {
                                            {
                                              a = 34;
                                            }
                                          }
                                        }
                                        return;
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}