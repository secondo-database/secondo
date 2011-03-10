

package tools;

/**
 This class holds some static variales.

**/


public class Environment{
/** forces more outputs when errors occur **/
public static boolean DEBUG_MODE = true;
/** enables the output of time measure at different places **/
public static boolean MEASURE_TIME = true;
/** enables the printing of the memery state before
  * and after some operations 
  **/
public static boolean MEASURE_MEMORY = false;
/** Flag indicating colorized outputs in the console **/
public static boolean FORMATTED_TEXT = false;


  /** Use a font encoding **/
  public static String ENCODING=null;



  /*constants denoting various testmodes */
  public static final int NO_TESTMODE=0;
  public static final int SIMPLE_TESTMODE=1;
  public static final int EXTENDED_TESTMODE=2;
  public static final int TESTRUNNER_MODE=4;


  /** currently used testmode.
    * the value may be a one of the testmodes specified above 
    * or an OR-connection of them.
    **/
  public static int TESTMODE = NO_TESTMODE;


/** Shows all commands before sent them to the Secondo Server **/
public static boolean SHOW_COMMAND = false;
/** The maximum string length. 
  * Ensure to use the same value as in the Secondo kernel.
  **/
public static int MAX_STRING_LENGTH = 48;




  /** flasg for debugging server communication **/
  public static boolean TRACE_SERVER_COMMANDS=false;

  /** small function comuting the currently used memory **/
  public static long usedMemory(){
    return rt.totalMemory()-rt.freeMemory();
  }

 /** Formats md given in bytes to human readable format **/
  public static String formatMemory(long md){
     String mem ="";
     if(Math.abs(md)>=1048576){
       mem = Double.toString(((int)(md/1048.576))/1000.0) + "MB";
     } else if(Math.abs(md)>1024){
       mem = Double.toString( ((int)(md/1.024))/1000.0 )+" kB";
     } else{
       mem = Long.toString(md)+" B";
     }
     return mem;
 }

/** prints out the current memory state **/
public static void printMemoryUsage(){
    Reporter.writeInfo("total Memory :"+" "+formatMemory(rt.totalMemory()));
    Reporter.writeInfo("free Memory  :"+" "+formatMemory(rt.freeMemory()));
}

  private static Runtime rt = Runtime.getRuntime();

}
