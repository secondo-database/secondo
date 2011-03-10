

package tools;


import sj.lang.*;



/**
  Example client for Secondo !!
**/



public class ExClient{

private static String hostName="localhost";
private static int port = 1234;
private static ESInterface si = null;
private static IntByReference errorCode= new IntByReference();
private static IntByReference errorPos = new IntByReference();
private static ListExpr resultList= new ListExpr();
private static StringBuffer errorMessage = new StringBuffer();
private static boolean haltOnError = true;

/** sends command to the server, ignores the result. */
private static boolean sendCommand(String Command){
  si.secondo(Command,resultList,errorCode,errorPos,errorMessage);
  if(errorCode.value != 0){
     System.err.println("error in command " + Command);
     System.err.println(errorMessage.toString());
     if(haltOnError){
         si.terminate();
         System.exit(2);
     }
     return false;
  }
  return true;
}


public static void main(String[] args){
  if(args.length>0){
    hostName = args[0];
  }
  if(args.length>1){
    try{
       port = Integer.parseInt(args[1]);

    } catch (Exception e){
        System.err.println("Portnumber invalid");
        System.exit(1);
    }
  }


  // initialize the interface to secondo

  si = new ESInterface();

  si.setHostname(hostName);
  si.setPort(port);

  si.setUserName("");
  si.setPassWd("");

  si.useBinaryLists(true); 

  tools.Environment.MEASURE_TIME=false; // supress some messages


  if(!si.connect()){
     System.err.println("problem in connecting with a secondo server");
     System.exit(0);
  }


  haltOnError=false;
  sendCommand("create database ctest");
  sendCommand("open database ctest");
  sendCommand("delete testrel");
  haltOnError=true;

  for(int i=0; i<10 ; i++){
    sendCommand("let i"+i+" = " + i);
  }


 
  si.terminate();

  System.out.println("successful");
}



}
