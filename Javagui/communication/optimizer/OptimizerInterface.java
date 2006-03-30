//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package communication.optimizer;

import java.io.*;
import java.net.*;
import tools.Reporter;


/* this class provides a interface for communication with a
 * optimizer server.
 */
public class OptimizerInterface{

/** sets the Hostname of the optimizer server */
public void setHost(String HostName){
   if(HostName!=null){
      if(!HostName.equals(this.HostName))
         ServerChanged=true;
      this.HostName = HostName;
   }
}

/** sets the port no of the optimizer server */
public void setPort(int Port){
   if(Port>=0){
     if(Port!=PortNr)
        ServerChanged=true;
      PortNr = Port;
   }
}

/** return the host-name of the optimizer server */
public String getHost(){
  return HostName;
}

/** returns the port no of the optimizer server */
public int getPort(){
  return PortNr;
}

/** connect this interface with the optimizerServer
  * returns true if successful, false otherwise
  */
public boolean connect(){
  if(isConnected() & !ServerChanged) // connection exists
     return true;
  if(isConnected() & ServerChanged) // close existing connection
     disconnect();
  try{
     ClientSocket = new Socket(HostName,PortNr);
     in = new BufferedReader(new InputStreamReader(ClientSocket.getInputStream()));
     out = new BufferedWriter(new OutputStreamWriter(ClientSocket.getOutputStream()));
     sendLine("<who>");
     out.flush();
     String answer = in.readLine();
     if(answer==null){ // Server is down
        disconnect();
	LastError = ErrorCodes.CONNECTION_BROKEN;
	return false;
     }

     if(answer.equals("<optimizer>")){ // found an optimizer
        LastError = ErrorCodes.NO_ERROR;
	return true;
     }
     else{
        disconnect();
	LastError=ErrorCodes.NO_OPTIMIZER;
	return false;
     }

  }
  catch(UnknownHostException UHE){
     LastError = ErrorCodes.UNKNOW_HOST;
     ClientSocket = null;
     return false;
  }
  catch(IOException IOE){
     LastError = ErrorCodes.IO_ERROR;
     ClientSocket = null;
     return false;
  }
  catch(SecurityException SE){
     LastError = ErrorCodes.SECURITY_ERROR;
     ClientSocket = null;
     return false;
  }

}

public boolean isConnected(){
   if(ClientSocket==null)
      return false;
   return ClientSocket.isConnected();
}


public int getLastError(){
  return LastError;
}




public void disconnect(){
  if(ClientSocket!=null){
     try{
        sendLine("<end connection>");
	out.flush();
     }catch(Exception e){
        Reporter.writeError("error in sending end-message to optimizer-server");
     }
     finally{
        try{
	  ClientSocket.close();
	}
	catch(Exception e2){
	  Reporter.writeError("error in closing connection to optimizer-server");
	}
     }
  }
  ClientSocket = null;
  in  = null;
  out = null;
  LastError = ErrorCodes.NO_ERROR;
}


public String optimize_execute(String query, String Database, IntObj ErrorCode, boolean executeFlag){
   query = query.trim();
   String QUERY = query.toUpperCase();
   if(!QUERY.startsWith("SQL") && !QUERY.startsWith("SELECT") && ! executeFlag){
       ErrorCode.value = ErrorCodes.NO_OPTIMIZATION_POSSIBLE;
       return query;
   }

   try{
     if(executeFlag)
        sendLine("<execute>");
     else
        sendLine("<optimize>");
     sendLine("<database>");
     sendLine(Database);
     sendLine("</database>");
     sendLine("<query>");
     sendLine(query);
     sendLine("</query>");
     if(executeFlag)
        sendLine("</execute>");
     else
        sendLine("</optimize>");
     out.flush();
     String answer = in.readLine();
     if(answer==null){ // Server is down
         disconnect();
	 LastError = ErrorCodes.CONNECTION_BROKEN;
	 return "";
     }
     if(!answer.equals("<answer>")){
         ErrorCode.value=ErrorCodes.PROTOCOL_ERROR;
         disconnect();
	 return "";
     }
     String Line = in.readLine();
     if(Line==null){ // Server is down
         disconnect();
	 LastError = ErrorCodes.CONNECTION_BROKEN;
	 return "";
     }
     StringBuffer result = new StringBuffer();
     while(!Line.equals("</answer>")){
       if(executeFlag)
          result.append(Line+"\n");
       else
          result.append(Line);
       Line = in.readLine();
       if(Line==null){ // Server is down
         LastError = ErrorCodes.CONNECTION_BROKEN;
         disconnect();
	 return "";
       }
     }
     ErrorCode.value = ErrorCodes.NO_ERROR;
     String Opt = result.toString();
     if(Opt.equals("")){
        ErrorCode.value = ErrorCodes.OPTIMIZATION_FAILED;
	return "";
     }
     return Opt;
   }catch(IOException IOE){
      ErrorCode.value = ErrorCodes.IO_ERROR;
      disconnect();
      return "";
   }
}


private void sendLine(String Line) throws IOException{
  Line +="\n";
  int len = Line.length();
  out.write(Line,0,len);
}


private String HostName="localhost";
private int PortNr=1235;
private boolean ServerChanged=true;
private Socket ClientSocket;
private BufferedReader in;
private BufferedWriter out;
private int LastError = ErrorCodes.NO_ERROR;

}
