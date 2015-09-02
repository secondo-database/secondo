

package tools;


import sj.lang.*;
import java.io.*;
import java.util.StringTokenizer;



/**
  Example client for Secondo !!
**/



public class Ex2Client{

private static String hostName="localhost";
private static int port = 1234;
private static ESInterface si = null;
private static IntByReference errorCode= new IntByReference();
private static IntByReference errorPos = new IntByReference();
private static ListExpr resultList= new ListExpr();
private static StringBuffer errorMessage = new StringBuffer();
private static boolean haltOnError = true;
private static BufferedReader in;


static String readCommand(){

  String command = "";
  System.out.print("> ");
  while(true){
      String line="";
      try{
         line = in.readLine();
      } catch(Exception e){
        System.err.println("keyboard problem");
        System.exit(1);
      }
      if(line.length()==0){
          return command.trim();
      }
      command += "\n"+line;
      if(line.endsWith(";") || command.trim().equals("quit")){
        return command.trim();
      }
  }
}

static void processCommand(String cmd){

  if(cmd.equals("quit")){
     return;
  }
  if(cmd.startsWith("special ")){
     processSpecial(cmd.substring(8).trim());
  } else {
     sendCommand(cmd);
  }
}

static void sendCommand(String cmd){
  si.secondo(cmd,resultList,errorCode,errorPos,errorMessage);
  if(errorCode.value != 0){ 
     System.err.println("error in command '" + cmd+ "'");
     System.err.println("error code is " + errorCode.value);
     System.err.println(ServerErrorCodes.getErrorMessageText(errorCode.value));
     System.err.println(errorMessage.toString());
  } else {
     System.out.println("Command successful, result is ");
     System.out.println(""+resultList);
  }
}

static void processSpecial( String cmd){

  if(cmd.equals("getPID")){
     System.out.println("ServerPID = " + si.getPid());
     return;
  }
  if(cmd.equals("getSendFilePath")){
     System.out.println("getSendFilePath = " + si.getSendFilePath());
     return;
  }
  if(cmd.equals("getSendFileFolder")){
     System.out.println("getSendFileFolder = " + si.getSendFileFolder());
     return;
  }
  if(cmd.equals("getRequestFilePath")){
     System.out.println("getRequestFilePath = " + si.getRequestFilePath());
     return;
  }
  if(cmd.equals("getRequestFileFolder")){
     System.out.println("getRequestFileFolder = " + si.getRequestFileFolder());
     return;
  }
  if(cmd.startsWith("requestFile")){
     requestFile(cmd);
     return;
  }
  if(cmd.startsWith("sendFile")){
     sendFile(cmd);
     return;
  }
  if(cmd.equals("help")){
     showCommands();
     return;
  } 
  System.out.println("Unknown command");

}


static void requestFile(String cmd){

  StringTokenizer st = new StringTokenizer(cmd);
  if(st.countTokens()!=4){
     System.out.println("Usage: special requestFile <remoteFile> <localFile> <allow overwrite>");
     return;
  }
  st.nextToken();
  String rf = st.nextToken();
  String lf = st.nextToken();
  boolean ao;
  try{
    ao = Boolean.parseBoolean(st.nextToken());
  } catch(Exception e){
     System.out.println("Usage: special requestFile <remoteFile> <localFile> <allow overwrite>");
     System.out.println("allow overwrite is not a boolean");
     return;
  }
  int rc = si.requestFile(rf, lf, ao);
  if(rc==0){
     System.out.println("request file successful");
  } else {
     System.out.println("requestFile failed with error code " + rc);
     System.out.println(ServerErrorCodes.getErrorMessageText(rc));
  }
}

static void sendFile(String cmd){
  StringTokenizer st = new StringTokenizer(cmd);
  if(st.countTokens()!=4){
     System.out.println("Usage: special sendFile <localFile> <remote file> <allow overwrite>");
     return;
  }
  st.nextToken();
  String rf = st.nextToken();
  String lf = st.nextToken();
  boolean ao;
  try{
    ao = Boolean.parseBoolean(st.nextToken());
  } catch(Exception e){
     System.out.println("Usage: special sendFile <remoteFile> <localFile> <allow overwrite>");
     System.out.println("allow overwrite is not a boolean");
     return;
  }
  int rc = si.sendFile(rf, lf, ao);
  if(rc==0){
     System.out.println("send file successful");
  } else {
     System.out.println("sendFile failed with error code " + rc);
     System.out.println(ServerErrorCodes.getErrorMessageText(rc));
  }
}

static void showCommands(){

   System.out.println("getPID");
   System.out.println("getSendFilePath");
   System.out.println("getSendFileFolder");
   System.out.println("getRequestFilePath");
   System.out.println("getRequestFileFolder");
   System.out.println("requestFile <remoteFile> <localFile> <allowOverwreite>");
   System.out.println("sendFile <localFile> <remoteFile> <allowOverwrite>");
   System.out.println("help");
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

  try{
     in = new BufferedReader(new InputStreamReader((System.in)));
  } catch(Exception e){
      System.err.println("Cannot read from keyboard");
      System.exit(0);
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
    System.out.println("error in connecting with server" );
    System.exit(0);
  }

  System.out.println("connected"); 

  String userInput = "";

  while(!userInput.equals("quit")){
     userInput = readCommand();
     processCommand(userInput);
  }

 
  si.terminate();

}



}
