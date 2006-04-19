
package tools;

/** This class provides the getenv mechanism.
  * The most java versions support getenv in the
  * System class. Unfortunately, the default Java version
  * of Secondo (1.4) is the only version not supporting the
  * reading of environments variables.
  * This class will call the standard mechanism for all
  * Java versions except the version 1.4. For this version,
  * a c routine is called using JNI.
  * Note that the shared object GetEnv must be in the
  * library path of the Java VM.
  **/
public class GetEnv{
  /** variable denoting the possibility to use the 
    * mechanism embedded in Java. 
    **/
  private static boolean simple = isSimple();

  /** Declaration of the c implementation **/
  private static native String getEnvFromC(String name);

  /** Returns the content of the environment variable with the
    * specified name 
    **/
  public static String getEnv(String name){
    if(simple){
        return System.getenv(name);
    } else{
        return getEnvFromC(name);
    }
  }

  /** checks for version 1.4 **/
  private static boolean isSimple(){
     try{
        return !System.getProperty("java.version").startsWith("1.4"); 
     }catch(Exception e){
        Reporter.writeError("cannot find out the currently used java version");
        return false;
     }
  }

  /** function for testing this class */
  public static void main(String[] args){
    for(int i=0;i<args.length;i++){
      try{
         System.out.println("secure "+ args[i] +" : " + getEnv(args[i]));
         System.out.println("c "+ args[i] +" : " + getEnvFromC(args[i]));
      }   catch(Exception e){}
    }

  }

  static { 
    System.loadLibrary("GetEnv");
  }

}

