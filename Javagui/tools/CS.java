package tools;

import java.io.*;
import java.net.*;
import java.util.*;


import sj.lang.*;

import java.util.*;


public class CS{

private class Connection extends Thread{
  InputStream in;
  OutputStream out;
  CS cs;

 public Connection(InputStream in, OutputStream out, CS cs){
   this.in = in;
   this.out = out;
   this.cs = cs;
 }

 public void run(){
   try{
      int next;
      while((next=in.read())>=0){
        byte b = (byte)next;
        out.write(b);
      }
      out.flush();
      System.out.println("Connection closed");
      cs.disconnectClient();

   }catch(Exception e){
     e.printStackTrace();
   }
 }
}


private InputStream in1;    // input stream from the SecondoServer
private OutputStream out1;  // output stream to the SecondoServer

private InputStream in2;    // input stream from the Secondo-Client
private OutputStream out2;  // output stream to the Secondo-Client


private static Vector set;  // set of free SecondoServer connections

/** Creates a new SecondoServerWrapper.
*/
public CS() {
  in1=null;
  in2 = null;
  out1=null;
  out2=null;
}


/** reads the next line from the inputstream 
  **/
private synchronized String nextLine(InputStream in){
   try{
     int next;
     StringBuffer line = new StringBuffer();
     next = in.read();
     while(next>=0 && (char)next!='\n'){
       line.append((char)next);
       next = in.read();
     }
     return line.toString();
   } catch(IOException e){
      e.printStackTrace();
      return null;
   }
}

/** Writes s to out.
  **/
private synchronized void sendString(String s,OutputStream out) throws IOException{
   int size = s.length();
   for(int i=0;i<size;i++){
        out.write((int)s.charAt(i));
   }
}


/** Callback function, called if the connection to a client is finished.
  **/
private void disconnectClient(){
   set.add(this);
   try{
     in2.close();
     out2.close();
   }catch(Exception e){
    System.err.println("Error in closing streams to the client");
   }
   in2=null;
   out2=null;
}

/** Connects this instance with an SecondoServer.
  * Performs the protocol up to the place where the 
  * server is ready to receive secondo queries.
  **/
public boolean connectWithServer(Socket server) throws IOException{
    in1 = server.getInputStream();
    out1 = server.getOutputStream();
    // process the intro of the secondoProtokoll
    // as client of the Secondo database system
    
    String line  = nextLine(in1);
    if(line==null){
       System.out.println("Connection error , line is null");
       return false;
    }
    System.out.println("Received : " + line);

    if(!line.equals("<SecondoOk/>")){
       System.out.println("<SecondoOK/> not received");
       return false;
    }

    System.out.println("Send authorization");
    
    sendString("<Connect>\n",out1);
    sendString("((user)(passwd))\n",out1);
    sendString("</Connect>\n",out1);
    out1.flush();

    System.out.println("end of authorization");


    System.out.println("read line");
    line = nextLine(in1);
    if(line==null){
      System.err.println("Connection error, expecting <SecondoIntro>"); 
      return false;
    }
    if(!line.equals("<SecondoIntro>")){
       System.out.println("No secondo intro received");
      return true;
    }

    System.out.println("Get the Secondo-Intro");
    do{
       line = nextLine(in1);
       System.out.println(line);
    } while(!line.equals("</SecondoIntro>"));
    System.out.println("SecondoIntro finished");
    return true;
}

/** Connects this instance to a SecondoClient.
  * The protocol intro is performed.
  * After that, the streams between client and server are
  * connected.
  */
public boolean connectClient(Socket client)throws IOException{
    in2 = client.getInputStream();
    out2 = client.getOutputStream();

    sendString("<SecondoOk/>\n",out2);
    out2.flush();

    System.out.println("Process the begin of the protocol"); 
    String line = nextLine(in2);
    if(line==null){
      System.out.println("Problem in connection, line is null");
      return false;
    }

    if(!line.equals("<Connect>")){
      System.out.println("Protocol error, <Connect> not received");
      return false;
    }
    System.out.println("Connect send");
    do{
      line = nextLine(in2);
      System.out.println("line:"+line);
    }while(!line.equals("</Connect>"));

    System.out.println("autgorization ok ");

    System.out.println("Send secondointro to the client" );

    sendString("<SecondoIntro>\n",out2);
    sendString("You are connected with a SecondoServerWrapper\n",out2);
    sendString("</SecondoIntro>\n",out2);
    out2.flush();

    System.out.println("SecondoIntro finished, connect the real server with the client"); 

    Connection c1 = new Connection(in1, out2,this);
    Connection c2 = new Connection(in2, out1,this);
    c1.start();
    c2.start();
    return true;
}

/** If a client tries to connect whith this instance but no connection
  * to a secondo server is avaiable, a new connection can be 
  * created using this function.
  **/
private static void addNewServer() throws IOException, UnknownHostException{
   System.out.println("Start"); 
   Socket client = new Socket("127.0.0.1",1234);
   CS cs = new CS(); 
   System.out.println("start connection with the secondo server");
   if(!cs.connectWithServer(client)){
     System.out.println("Connection with server failed" );
     System.exit(1);
   }
   System.out.println("connection with server ok");
   set.add(cs); 
}

/** Main fucntion **/
public static void main(String[] args) throws IOException, UnknownHostException{

   set = new Vector();  // empty connections
   addNewServer();
   ServerSocket ss = new ServerSocket(1236);
   while(true){
      Socket server = ss.accept(); // wait for new clients
      System.out.println("a new client is connected" );
      if(set.isEmpty()){ // create a new connection
         addNewServer();
      }
      CS cs = (CS) set.get(0);
      set.remove(cs);
      cs.connectClient(server);
   }
}

}




