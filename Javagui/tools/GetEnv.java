
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

  /** Declaration of the c implementation **/
  private static native String getEnvFromC(String name);

  /** Returns the content of the environment variable with the
    * specified name 
    **/
  public static String getEnv(String name){
        return System.getenv(name);
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

}

