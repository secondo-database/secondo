

public class Version{
  int major;
  int minor;
  int subminor;

  public boolean isVersion(int major, int minor, int subminor){
     return (this.major == major) &&
            (this.minor == minor) &&
            (this.subminor == subminor);
  }
  
  public boolean isSmallerOrEqual(int major, int minor, int subminor){
     return (this.major < major) ||
            (this.major==major && this.minor < minor) ||
            (this.major==major && this.minor==minor && this.subminor<=subminor);
  }

}
