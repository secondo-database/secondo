import java.io.*;
import java.net.*;
import java.nio.channels.IllegalBlockingModeException;

class SendUDP
{

  private DatagramSocket serverSocket;
  private InetAddress address;
  private int localPort;
  private int remotePort;
  private File file;
  private long pause;


/**
Sends msg to the server. If sending was successful, the return value is
true. Otherwise, an error message is printed out to cerr and 


**/ 
  private void sendMessage(String msg)throws IOException, SecurityException, IllegalBlockingModeException{
     byte sendData[];
     sendData = msg.getBytes();
     DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, address, remotePort);
     serverSocket.send(sendPacket);
  }

/**
Returns a constant string describing the usuage of this class as an application.

**/
  private String usage(){
     return  "usage :  java SendUDP -lp <localPort> -rp <remotePort> -a <remote host> -f <file>  -p <pause between lines in ms >";
  }

/**
Evaluates an option with value.

**/
  boolean readArg(String option , String value){
    if(option.equals("-lp")){
       try{
         localPort = Integer.parseInt(value);
         return true;
       } catch(Exception e){
         return false;
       }
    }
    if(option.equals("-p")){
       try{
          pause= Long.parseLong(value);
         return true;
       } catch(Exception e){
         return false;
       }
    }
    if(option.equals("-rp")){
       try{
         remotePort = Integer.parseInt(value);
        return true;
       } catch(Exception e){
         return false;
       }
    }
    if(option.equals("-a")){
      try{
        address = InetAddress.getByName(value);
        return true;
      } catch(Exception e){
         return false;
      }
    }
    if(option.equals("-f")){
      try{
        file = new File(value);
        return true;
      } catch(Exception e){
         return false;
      }
    }
   
    return true;
  }

/**
Evaluates a set of arguments. When a required argument is missing or an unknown 
option is found, the function returns false.

*/
  boolean readArgs(String[] args){
    localPort = -1;
    remotePort = -1;
    if((args.length % 2 ) !=0 ){ // always use -option <value> pairs
       return false;
    }
    for(int i=0;i<args.length;i = i+2){
       if(!readArg(args[i], args[i+1])){
          return false;
       }
    }
    if( (file==null) || (remotePort<0) || (address==null)){
      return false;
    }
    try{
      if(localPort >0){
         serverSocket = new DatagramSocket(localPort);
      } else {
         serverSocket = new DatagramSocket();
      }
    } catch(Exception e){
      return false; 
    }
    return true;
  }


/**
Sends a file per UDP to another host.


**/
  private boolean sendFile(){
    try{
       BufferedReader in = new BufferedReader(new FileReader(file));
       boolean ok = true;
       while(in.ready()){
         String line = in.readLine();
         try{
           sendMessage(line);
         } catch(PortUnreachableException e){
           System.err.println(" remote port " + remotePort + " not reachable, stop.");
           ok = false;
         } catch (IOException e){
            System.err.println("Problem in sending " + line);
         }
         if(pause>0){
           Thread.sleep(pause);
         }
       }
       return true;
    } catch(Exception e){
       return false;
    }

  }

/**
Main function.

**/

  public static void main(String args[]) throws Exception
  {
      SendUDP sendUDP = new SendUDP();     
      if(!sendUDP.readArgs(args)){
        System.err.println("invalid or non complete options ");
        System.err.println(sendUDP.usage());
        System.exit(1);
      }      
      sendUDP.sendFile();
   }
}

