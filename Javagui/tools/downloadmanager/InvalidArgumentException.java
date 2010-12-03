package tools.downloadmanager;

/** class decscribing an exception related to invalid arguments **/

class InvalidArgumentException extends Exception{
  public  InvalidArgumentException(String message) {
    this.message=message;
  }

  public String toString(){
    return "InvalidArgumentException " + message; 
  }

  private String message;
}
