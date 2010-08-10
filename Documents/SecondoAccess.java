//  Accessing Secondo through a Java application is of course possible. And it needs only three steps:
//
//  1. Start up a Secondo Monitor.
//  2. Build a lib directory in your Java program, and import all the classes and jars in        
//     $SECONDO_BUILD_DIR/Java/*.
//  3. Start to write the Java application. See an example below. 


import java.io.*;
import sj.lang.*;

public class SecondoAccess {

private static boolean query(ListExpr resultList)
  throws IOException
{
  ESInterface SecondoInterface = new ESInterface();
  SecondoInterface.setPort(1234);
  SecondoInterface.setHostname("localhost");
  SecondoInterface.useBinaryLists(true);
  
  boolean ok = SecondoInterface.connect();
  if (!ok)
    return false;
  
  IntByReference errorCode = new IntByReference(0);
  IntByReference errorPos = new IntByReference(0);
  StringBuffer errorMessage = new StringBuffer();
  
  SecondoInterface.secondo("open database opt", new ListExpr(), errorCode, errorPos, errorMessage);
  String query = "ten";
  //SecondoInterface.secondo("query " + query, resultList, errorCode, errorPos, errorMessage);
  SecondoInterface.secondo("query ten", 
      resultList, errorCode, errorPos, errorMessage);
  System.out.println(resultList);
  SecondoInterface.secondo("let two = ten feed filter[.no < 3]", 
      resultList, errorCode, errorPos, errorMessage);
  System.out.println(resultList);
  SecondoInterface.secondo("query two", 
      resultList, errorCode, errorPos, errorMessage);
  System.out.println(resultList);
  if (errorCode.value != 0)
  {
    System.err.println("Error in executing query " + query + "\n\n" + errorMessage);
  }
  
  SecondoInterface.secondo("close database", resultList, errorCode, errorPos, errorMessage);
  
  //very important
  SecondoInterface.terminate();
  
  if (errorCode.value != 0)
  {
    System.err.println("Error in executing query " + query + "\n\n" + errorMessage);
  }
  
  return errorCode.value == 0;
}

public static void main(String args[])
  throws IOException
{
  ListExpr resultList = new ListExpr();
  if (!query(resultList))
  {
    System.err.println("error!");
  }
  else
  {
    System.err.println("success!");
    System.out.println(resultList.toString());			
  }
}


}
