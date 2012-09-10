/*

[newpage]

4 Multi-thread Secondo execution Class SecExRunnable

This class will run several Secondo queries in a dependent thread.
If the query command start with *SYNC*, that's means 
this query can't be run by multiple threads simultaneously, 
need use the ~lock~ to synchronize the threads. 

Besides, we also use a string array to keep the query commands, 
so that we can run multiple queries in one thread continuously. 

*/

package ParallelSecondo;

import sj.lang.ListExpr;

public class SecExRunnable implements Runnable {

  private static byte[] lock = new byte[0];
  String[] results;
  String hostName = "", dbName = "", relName = "";
  String[] queries;
  boolean isOpened = false;
  
  
  QuerySecondo secEntity;
  
  public SecExRunnable(String hn, String dbn, int port, 
                       String[] qrs){
    hostName = hn;
    dbName = dbn;
    queries = qrs.clone();
    results = new String[qrs.length];
    try {
      secEntity = new QuerySecondo();
      secEntity.open(hostName, dbName, port);
      isOpened = true;
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  public void run()
  {
    ListExpr resultList;
    
    try{
      for (int i=0; i < queries.length; i++){
        String queryStr = queries[i];
        resultList = new ListExpr();
        
        if(queryStr.startsWith("SYNC")){
          queryStr = queryStr.substring(5).trim();
          synchronized (lock) {
            secEntity.query(queryStr, resultList);
          }
        }else{
          secEntity.query(queryStr, resultList);
        }
        results[i] = resultList.toString();
      }
    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      try {
        secEntity.close();
        isOpened = false;
      } catch (Exception e) {
        e.printStackTrace();
      }
      printResults();
    }
  }
  
  public String getResults()
  {
    String res = "";
    if(results.length > 0){
      for(int i=0; i<results.length; i++){
        res += results[i] + " | ";
      }
    }
    return res;
  }
  
  private void printResults()
  {
    if(results.length > 0){
      for(int i=0; i<results.length; i++){
        System.out.println(results[i]);
      }
    }
  }
  
  public String getResult(int i)
  {
    if (i > 0 && i < results.length)
      return results[i];
    else
      return "ERROR";
  }
  
  public boolean isFinished()
  {
    return !isOpened;
  }
  
  public boolean isInitialized(){
    return isOpened;
  }
}
