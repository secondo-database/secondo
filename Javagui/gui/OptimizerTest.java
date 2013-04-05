


package gui;

import java.io.*;
import java.util.*;
import sj.lang.*;


public class OptimizerTest{


private String testOptimizerFile= "testSQL";
private Vector<String> requiredDatabases;
private CommandPanel cmdp;
private MainWindow mw;


public OptimizerTest(CommandPanel cmdp, MainWindow mw){
   this.cmdp = cmdp;
   this.mw = mw;
}


public  int testOptimizer(){
   if(!cmdp.isConnected()){
      tools.Reporter.showError("no connection");
      return 1;
   }
   try{
      readConfiguration(); 
      ensureDatabases();
      return runTestFile();
   } catch(Exception e){
      tools.Reporter.debug(e);
      tools.Reporter.showError("problem in testOptimizeri\n"+e);
       return 1;
   }
}

private  void readConfiguration(){
   Properties config = new Properties();
   requiredDatabases = new Vector<String>();
   String rD;
   try{
       config.load(new FileInputStream(new File(Environment.testOptimizerConfigFile)));
       testOptimizerFile = config.getProperty("SCRIPTFILE","testSQL");
       rD = config.getProperty("DATABASES","opt optext berlintest");

   } catch(Exception e){
      tools.Reporter.showError("Problem in reading file + " + Environment.testOptimizerConfigFile
                           +"\n use default values");
      testOptimizerFile="testSQL";
      rD = "opt optext berlintest";  
   }
   StringTokenizer st = new StringTokenizer(rD);
   while(st.hasMoreTokens()){
      requiredDatabases.add(st.nextToken()); 
   } 
}


private void ensureDatabases() throws Exception{
   // first retrieve existing databases
   IntByReference errorCode = new IntByReference();
   ListExpr resultList = new ListExpr();
   StringBuffer errorMsg = new StringBuffer();
   cmdp.execCommand("list databases",errorCode, resultList, errorMsg);

   if(errorCode.value != 0){
      throw new Exception("Problem during list databases");
   }

   if(resultList.listLength()!=2){
      throw new Exception("malformed list databases result");
   }
   resultList = resultList.second();
   if(resultList.listLength()!=2){
      throw new Exception("malformed list databases result");
   }
   resultList = resultList.second();
   TreeSet<String> availDB = new TreeSet<String>();
   while(!resultList.isEmpty()){
        availDB.add(resultList.first().toString().trim());
        resultList = resultList.rest();
   }


   for(int i=0;i<requiredDatabases.size();i++){
     String db = requiredDatabases.get(i);
     String dbUC = db.toUpperCase().trim();
     if(!availDB.contains(dbUC)){
        restoreDB(db);
     } 
   }
}

private void restoreDB(String db) throws Exception{
   // first retrieve existing databases
   IntByReference errorCode = new IntByReference();
   ListExpr resultList = new ListExpr();
   StringBuffer errorMsg = new StringBuffer();
   String cmd = "{close database | restore database "+db+" from '../bin/"+db+"'}";
   cmdp.execCommand(cmd, errorCode, resultList, errorMsg);
   if(errorCode.value!=0){
     if(db.equals("optext")){ // special treatment for standard database
        restoreOptExt();
        return;
     }
     String err = ServerErrorCodes.getErrorMessageText(errorCode.value);
     throw new Exception("Error during restoring " + db + "\n " + err);
   }
}

private int runTestFile(){
  int tm = tools.Environment.TESTMODE;
  mw.setTestMode(tools.Environment.SIMPLE_TESTMODE);
  int res =   mw.executeFile(testOptimizerFile,true, false);
  mw.setTestMode(tm);
  return res;
}

private void restoreOptExt() throws Exception{
   // first retrieve existing databases
   IntByReference errorCode = new IntByReference();
   ListExpr resultList = new ListExpr();
   StringBuffer errorMsg = new StringBuffer();
   String cmd = "close database";
   cmdp.execCommand(cmd, errorCode, resultList, errorMsg);
   // ignore errors
   cmd = "restore database optext from '../bin/opt'";
   cmdp.execCommand(cmd, errorCode, resultList, errorMsg);
   if(errorCode.value!=0){
      throw new Exception("cannot create optext from opt");
   }
   cmd = "open database optext";
   cmdp.execCommand(cmd, errorCode, resultList, errorMsg);

   Vector<String> deriveCmds = new Vector<String>();

   deriveCmds.add(   "derive OrteH = Orte feed extend[BevTH: .BevT div 100 ] " 
                   + "sortby [BevTH] nest [BevTH ; SubRel] consume");
   deriveCmds.add(   "derive OrteM = Orte feed extend[BevTH: .BevT div 100] "
                   + "sortby[BevTH] nest [BevTH;SubH] extend[BevM: .BevTH div 10] "
                   + "sortby[BevM] nest [BevM;SubM] consume");
   deriveCmds.add(   "derive OrteM2 = Orte feed extend[BevTH: .BevT div 100] " 
                   + "sortby[BevTH] nest [BevTH;SubH] extend[BevM: .BevTH div 10] "
                   + "sortby[BevM] nest [BevM;SubM] extend[SubMH: fun(alias: TUPLE) "
                   + "Orte feed     extend[Temp: .BevT div 1000] filter[.Temp=attr(alias, BevM)] "
                   + "remove[Temp] aconsume] consume");
   deriveCmds.add(   "derive StaedteNested = Staedte feed sortby[SName, Bev] "
                   + "nest[SName, Bev; SubRel] consume");

   for(int i=0;i<deriveCmds.size();i++){
      cmd = deriveCmds.get(i);
      StringTokenizer st = new StringTokenizer(cmd);
      st.nextToken();
      String objName = st.nextToken();
      cmdp.execCommand(cmd, errorCode, resultList, errorMsg);
      if(errorCode.value!=0){
        throw new Exception("Cannot restore optExt database; problem with object " + objName);
      }
      tools.Reporter.writeInfo("Object " + objName + " successful");
   }

}



}






