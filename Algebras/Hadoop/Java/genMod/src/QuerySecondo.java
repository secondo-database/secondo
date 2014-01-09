



import java.io.IOException;
import java.net.InetAddress;
import sj.lang.*;

/*

The QuerySecondo class is used to call the local Secondo System to execute
all the Secondo queries that map or reduce functions need through TCP/IP.
It only contains three functions: open, query and close.   

*/

public class QuerySecondo {
  
  private sj.lang.ESInterface SecondoInterface = new ESInterface();
  private boolean isOpened = false;
  private IntByReference errorCode = new IntByReference(0);
  private IntByReference errorPos = new IntByReference(0);
  private StringBuffer errorMessage = new StringBuffer();
  private String hostName = "NULL";

  public void open(String hostName, String database) 
    throws Exception
  {
    open(hostName, database, 1234, false);
  }

  public void open(String servHostName, String database, int port, 
		  boolean createNew)
      throws IOException, InterruptedException
  {
    if (servHostName == "localhost")
      hostName = InetAddress.getLocalHost().getHostName();
    else
      hostName = servHostName;
    
    SecondoInterface.setPort(port);
    SecondoInterface.setHostname(servHostName);
    SecondoInterface.useBinaryLists(true);
    
    boolean ok = false;
    int connCounter = 0;
    while(!ok){
      ok = SecondoInterface.connect();
      if( !ok && (connCounter++ < 10) )
        Thread.sleep(500);
      else
        break;
    }
    
    if (!ok) {
      throw new IOException("Error in connect to Secondo Server in" 
          + " [" + hostName + ":" + port + "]"
          + " please check whether the Monitor is opened.");
    } else {
      System.out.println("Success in connect Secondo in" 
          + " [" + hostName + ":" + port + "]");
      isOpened = true;
      
      if (createNew){
    	  SecondoInterface.secondo("create database " + database,
    		new ListExpr(), errorCode, errorPos, errorMessage);
    	  if (errorCode.value != 0){
    		  System.out.println("The database may exist already.");
    		  SecondoInterface.secondo("delete database " + database,
    		    new ListExpr(), errorCode, errorPos, errorMessage);
    		  System.out.println("Delete the exist database.");
    		  SecondoInterface.secondo("create database " + database,
    		    new ListExpr(), errorCode, errorPos, errorMessage);
    	  }
      }
      SecondoInterface.secondo("open database " + database, 
          new ListExpr(), errorCode, errorPos, errorMessage);
      if (errorCode.value != 0) {
        throw new IOException("Error in open database :" + database 
            + " in [" + hostName + ":" + port + "]" + "\n\n" + errorMessage);
      } else {
        System.out.println("Success in open Database :" + database 
            + " in [" + hostName + ":" + port + "]");
      }
    }
  }

  public void close() throws IOException
  {
    if (!isOpened) {
      System.err.println("---The database in [" + hostName + "] "
          + "is closed already!---");
    } else {
      SecondoInterface.secondo("close database", new ListExpr(), 
          errorCode, errorPos, errorMessage);
      if (errorCode.value != 0) {
        throw new IOException("\n--\nError in close database in " 
            + " [" + hostName + "]\n"
            + "\nErrorMessage: " + errorMessage
            + "\nErrorCode: " + errorCode.value 
            + "\nErrorPos:  " + errorPos.value 
            + "\n--");
      } else {
        System.out.println("Success in close database in " 
            + " [" + hostName + "]");
        SecondoInterface.terminate();
        System.out.println("SHUT DOWN the connection to Secondo in " 
            + " [" + hostName + "]");
        isOpened = false;
      }
    }
  }
  
  public void query(String queryStr, ListExpr resultList) throws IOException
  {
	  query(queryStr, resultList, false);
  }

  public void query(String queryStr, ListExpr resultList, boolean ignoreError) throws IOException
  {
    long sT = System.currentTimeMillis();
    
    SecondoInterface.secondo(queryStr, resultList, 
        errorCode, errorPos, errorMessage);

    System.out.println("[" + hostName + "] start execute: " 
        + queryStr.toString());
    
    if (!ignoreError && errorCode.value != 0) {
      System.err.println(
          "\n--\nError in [" + hostName + "] executing :\n" 
          + queryStr + "\nErrorMessage: " + errorMessage
          + "\nErrorCode: " + errorCode.value 
          + "\nErrorPos:  " + errorPos.value 
          + "\n--");
      throw new IOException("Secondo Error");
    }
    
    long eT = System.currentTimeMillis();
    
    //print the query string after success
    System.out.println("[" + hostName + "] Success execute the query" 
    	+  "  | used " + ((eT - sT)/1000) + "s");
  }
  
  public String toMultiLines(String str, int len)
  {
	  char[] chs = str.toCharArray();
	  StringBuffer sb = new StringBuffer();
	  for(int i = 0, sum = 0; i < chs.length; i++)
	  {
		  sum++;
		  sb.append(chs[i]);
		  if ( sum >= len)
		  {
			  sum = 0;
			  sb.append("\n");
		  }
	  }
	  return sb.toString();
  }
}
