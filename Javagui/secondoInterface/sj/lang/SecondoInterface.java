/******************************************************************************
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//paragraph    [10]    title:        [\centerline{\Large \bf] [}]

[10] SecondoInterface.java

April 2002 Ulrich Telle Client/Server version of the SecondoInterface for version 2 of Secondo

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. 

\tableofcontents

******************************************************************************/

package sj.lang;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.StringTokenizer;
import java.util.Vector;

import tools.Reporter;
import tools.Environment;

/*

3 The SecondoInterface class implementation.

 In this section we describe the implementation of the SecondoInterface class.

*/
 
public class SecondoInterface{

/*
3.1 Public fields.

The next public fields are defined in the SecondoInterface class and
hence they are accessible by the user code.

*/
     // Secondo executable command in nested list syntax (list).
  public static final int EXEC_COMMAND_LISTEXPR_SYNTAX    = 0;
     // Secondo executable command in SOS syntax (text).
  public static final int EXEC_COMMAND_SOS_SYNTAX         = 1;
     // Secondo descriptive command after, or without, algebraic optimization (list).
  public static final int DESCRIP_COMMAND_AFTER_OPT_LIST  = 2;
     // Secondo descriptive command after, or without, algebraic optimization (text).
  public static final int DESCRIP_COMMAND_AFTER_OPT_TEXT  = 3;
     // Secondo descriptive command before algebraic optimization (list).
  public static final int DESCRIP_COMMAND_BEFORE_OPT_LIST = 4;
     // Secondo descriptive command before algebraic optimization (text).
  public static final int DESCRIP_COMMAND_BEFORE_OPT_TEXT = 5;
     // Command in some specific query language (e.g. SQL, GraphDB, etc.).
  public static final int COMMAND_SPECIFIC_QUERY_LANGUAGE = 7;

  public static final int DESCRIPTIVE_ALGEBRA = 1;
  public static final int EXECUTABLE_ALGEBRA  = 2;



/*
3.2 Private fields.

The next private fields are defined in the SecondoInterface class.
Hence they are used into the SecondoInterface class code, but they are
not accessible by the user code.

*/


  // This field sets the name of the file where Secondo() method must store the
  // result when the commandAsText parameter is set to true. It is predefined
  // to be "SecondoResult".
  private static final String SECONDO_RESULT_FILE = "SecondoResult";

  // Flag indicating if a connection with a Secondo server exists.
  protected boolean initialized;
  // Socket connected to the Secondo Server
  protected Socket serverSocket;
  // flag for binary reading lists from Server
  private boolean binaryLists = false;
  // Connection's input reader (from secondoServerSocket).
  private MyDataInputStream inSocketStream;
  // Connection's output writer (to secondoServerSocket).
  private MyBufferedOutputStream outSocketStream;

  // a vector containing all messageListeners
  private Vector<MessageListener> messageListener = new Vector<>();

  public SecondoInterface()
  {
    initialized = false;
    serverSocket = null;
  }

  public void destroy()
  {
    if ( initialized )
    {
      terminate();
    }
  }


  /* switch for using binary or textual list format */
  public void useBinaryLists(boolean ubl){
     binaryLists=ubl;
     if(ubl)
        Reporter.writeInfo("use binary lists");
     else
        Reporter.writeInfo("use textual lists");
  }

  public boolean initialize( String user, String pswd,
                             String host, int port )
  {
    String secHost = host;
    int secPort = port;
    String line;

    if ( !initialized )
    {
      Reporter.writeInfo( "Initializing the Secondo system ..." );
      // Connect with server, needed host and port
      if ( secHost.length() > 0 && secPort > 0 )
      {
        Reporter.writeInfo( "SecondoInterface: Connecting with Secondo server '" +
                            secHost + "' on port " + secPort + " ..." );
        try
        {
          serverSocket = new Socket( secHost, secPort );
          try {
            // Creates the input/output streams.
            inSocketStream = new MyDataInputStream( new BufferedInputStream( serverSocket.getInputStream() ) );
            outSocketStream = new MyBufferedOutputStream( ( serverSocket.getOutputStream() ) );

          } catch (IOException e) {
            closeSocket( serverSocket );
            serverSocket = null; // To help detecting internal errors.
            Reporter.writeError( "SecondoInterface: IOError creating input/output streams for serverSocket.." );
          }
        } catch (UnknownHostException e) {
          Reporter.writeError( "SecondoInterface: Network error: Unknown host '" + secHost + "'." );
        } catch (IOException e) {
          Reporter.writeError( "SecondoInterface: Network error: Unable to connect with the Secondo Server." );
        }

        if ( serverSocket != null ) {
          try {
            line = inSocketStream.readLine();
        if(line==null){
							Reporter.writeError("Could not read data from server.");
           initialized = false;
           return false;
        }
            if ( line.equals( "<SecondoOk/>" ) ) {
              outSocketStream.write( "<Connect>\n" 
                                    + user +"\n"
                                    + pswd +"\n"
                                    + "</Connect>\n" );
              outSocketStream.flush();
              line = inSocketStream.readLine();

              if(line==null){
                 initialized = false;
								Reporter.writeError("Could not read data from server.");
                return false;
              }
              if ( line.equals( "<SecondoIntro>" ) ) {
                do {
                  line = inSocketStream.readLine();
                  if(line==null){
                     initialized = false;
										Reporter.writeError("Could not read data from server.");
                     return false;
                  } 
                  if ( !line.equals( "</SecondoIntro>" ) ) {
                     Reporter.writeInfo( line );
                  }
                } while ( !line.equals( "</SecondoIntro>" ) );
                   initialized = true;
              } else if ( line.equals("<SecondoError>") ) {
                initialized = false;
                do{
                   line = inSocketStream.readLine();
                   if(line==null){
                      initialized = false;
                      Reporter.writeError("SecondoError, connection lost");
                   }
                   Reporter.writeError( "Server-Error: " + line );
                } while(line!=null && !line.equals("</SecondoError>"));
              } else {
                Reporter.writeError( "Unidentifiable response from server: " + line );
              }
            }
            else
            {
              Reporter.writeError( "Unidentifiable response from server: " + line );
            }
          } catch (IOException e) {
             Reporter.debug(e);
             initialized = false;
          }
        } else {
					Reporter.writeError("Connection to SecondoServer could not be established.");
          initialized = false;
        }
        if ( !initialized && serverSocket != null ) {
          closeSocket( serverSocket );
          serverSocket = null;
        }
      } else {
        Reporter.writeError( "*** SecondoInterface: Invalid or missing host (" + secHost +
                            ") and/or port (" + secPort + ")." );
      }
    }
    return (initialized);
  }

  public void terminate()
  {
    if ( serverSocket != null )
    {
      try {
        outSocketStream.write( "<Disconnect/>\n" );
        outSocketStream.flush();
      } catch (IOException e) {
        Reporter.writeError( "SecondoInterface: Terminate: Error sending <Disconnect/>." );
      }
      try {
        inSocketStream.close();
      } catch (IOException e) {
        Reporter.writeError( "SecondoInterface: Terminate: Error closing input stream." );
      }
      try {
        outSocketStream.close();
      } catch (IOException e) {
        Reporter.writeError( "SecondoInterface: Terminate: Error closing output stream." );
      }
      try {
        serverSocket.close();
      } catch (IOException e) {
        Reporter.writeError( "SecondoInterface: Terminate: Error closing server socket." );
      }
      serverSocket = null;
    }
  }

  /** reads a List from the server.
    * If there is an error, null is returned.
     */
  private ListExpr receiveList(String finalToken, IntByReference errorCode) throws IOException{
     String line;
     ListExpr answerList = new ListExpr();
     if(!binaryLists){
         StringBuffer result = new StringBuffer();
         long t1 = System.currentTimeMillis();
         do {
            line = inSocketStream.readLine();
            if(line!=null){
               if (line.compareTo( finalToken ) != 0 ) {
                    result.append(line);
                    result.append("\n");
                }
             } else{
                errorCode.value = 81;
                return null; 
             }
         } while (!line.equals(finalToken));
         long t2 = System.currentTimeMillis();
         answerList.readFromString( result.toString() );
         long t3 = System.currentTimeMillis();
         long parsetime = t3-t2;
         long receivetime = t2-t1;
         long alltime = t3-t1;
         if(Environment.MEASURE_TIME){
            Reporter.writeInfo("receive a nested list (textual) : "+receivetime+" milliseconds");
            Reporter.writeInfo("parsing                         : "+parsetime+" milliseconds");
            Reporter.writeInfo("sum                             : "+alltime+" milliseconds"); 
         }
     } else { // handle binary lists
       long t1 = System.currentTimeMillis();
        answerList = ListExpr.readBinaryFrom(inSocketStream);
        if(answerList==null){
             errorCode.value=81;
             return null;
         }
         line = inSocketStream.readLine();
         if(!line.equals(finalToken)){
             Reporter.writeError("SecondoInterface: Missing"+finalToken);
             Reporter.writeError("received :" +line);
             errorCode.value=81;
             return null;
          }
          long t = System.currentTimeMillis()-t1;
          if(Environment.MEASURE_TIME){
               Reporter.writeInfo("receive and building a nested list (binary) :"+t+" milliseconds");
          }
     } // handle binary lists
     return answerList;
  }

  /** informs all currently messageListener. **/
  private void informListener(ListExpr message){
      for(int i=0;i<messageListener.size();i++){
           ((MessageListener)messageListener.get(i)).processMessage(message);
      }
  }

  /** removes ml */
  public void removeMessageListener(MessageListener ml){
     messageListener.remove(ml);
  }
  
  /** adds ml  */
  public void addMessageListener(MessageListener ml){
     if(!messageListener.contains(ml)){
        messageListener.add(ml);
     }
  }


/** receives a response from the secondo server 
  * when send a command to it.
  **/
  private void receiveResponse(ListExpr resultList,
                             IntByReference errorCode,
                             IntByReference errorPos,
                             StringBuffer errorMessage) throws IOException{
     String line = inSocketStream.readLine();
     if(line==null){
        errorCode.value=81; // network error
        return;
     }
     while(line.equals("<Message>")){
        ListExpr messageList = receiveList("</Message>",errorCode);
        if(messageList==null){ // error
           return;
        }
        informListener(messageList);
        line = inSocketStream.readLine();
        if(line==null){
             errorCode.value =81; 
             return;
        }
     }
    

     // handle error
     if( line.equals("<SecondoError>" )) {
        errorCode.value = 80;
        errorMessage.setLength( 0 );
          line = inSocketStream.readLine();
        if(line !=null) {
             errorMessage.append( line );
             line = inSocketStream.readLine();
           } else{
             Reporter.writeError( "SecondoInterface: Network Error in method secondo reading SecondoError." );
             errorCode.value = 81;
             return;
         }
         return; // end of handling the <SecondoError> Message
      }

     if(!line.equals("<SecondoResponse>")){ 
        // protocol error
        errorCode.value=81;
        return;
     } 
     // read the answerlist
     ListExpr answerList = receiveList("</SecondoResponse>",errorCode);
     if(answerList==null){
         return;
     }   
     errorCode.value = answerList.first().intValue();
     errorPos.value  = answerList.second().intValue();
     errorMessage.setLength( 0 );
     errorMessage.append( answerList.third().textValue() );
     resultList.setValueTo( answerList.fourth() );
}



/** performs a restore (object or database) command supporting the
  * secondo method. 
  **/
  private void callRestoreCommand(String command,
                                  ListExpr resultList,
                                  IntByReference errorCode,
                                  IntByReference errorPos,
                                  StringBuffer errorMessage) throws IOException{

     resultList.setValueTo(ListExpr.theEmptyList());
     if(command.startsWith("restore")){
          command = "("+command+")";
     }
     ListExpr cmdList = new ListExpr();
     if(cmdList.readFromString(command)!=0){
	errorMessage.append("cannot parse restore command");     
        errorCode.value=9; // syntax error in command
        return;
     }
     int len = cmdList.listLength();
     if(len !=4 && len !=5){ // not a correct restore command
	errorMessage.append("invalid number of arguments fo restore command ");     
        errorCode.value=9; // syntax error in command
        return;
     }

     // a restore command has to be in format
     // restore [database] name from <filename>
     

     boolean dbrestore = false; // switch for object or db restore
     String name="";       // name of the object or the database
     String filename="";   // name of the file to restore
     ListExpr filelist=null; // list representing the filename
     
     if(len==5){ // possible a database restore
        // the first four elements has to be symbols
        ListExpr le = cmdList;
        for(int i=0;i<4;i++){
           if(le.first().atomType()!=ListExpr.SYMBOL_ATOM){
	      errorMessage.append("non- symbol in as first arguments of "+
		                  "restore command");	      
              errorCode.value=9;
              return; 
           }
         }   
         if(!cmdList.first().symbolValue().equals("restore") ||
            !cmdList.second().symbolValue().equals("database") ||
            !cmdList.fourth().symbolValue().equals("from")){
	    errorMessage.append("invalid keywords in restore command");	 
            errorCode.value=9;
            return;
         }
         name = cmdList.third().symbolValue();
         filelist = cmdList.fifth(); // filename
         dbrestore=true;
      } else if(len==4){
        ListExpr le = cmdList;
        for(int i=0;i<3;i++){
           if(le.first().atomType()!=ListExpr.SYMBOL_ATOM){
	      errorMessage.append("invalid argument in object restore command");	   
              errorCode.value=9;
              return; 
           }
         }   
         if(!cmdList.first().symbolValue().equals("restore") ||
            !cmdList.third().symbolValue().equals("from")){
            errorMessage.append("invalid keyword in object restore command found");
            errorCode.value=9;
            return;
         }
         name = cmdList.second().symbolValue();
         filelist = cmdList.fourth();
         dbrestore=false; 
      }

      switch(filelist.atomType()){
          case ListExpr.SYMBOL_ATOM: filename=filelist.symbolValue();break;
          case ListExpr.STRING_ATOM: filename=filelist.stringValue();break;
          case ListExpr.TEXT_ATOM: filename=filelist.textValue();break;
          default: errorCode.value=9;
                   return;
      }
      filename = tools.CommonUtils.expandVar(filename);
      // check whether the file exists
      File file = new File(filename);
      if(!file.exists() || !file.canRead()){
         Reporter.writeWarning("try to restore an object from an non-existing" +
		                            " or non-readable file");	
         Reporter.writeWarning("file is " + file.getAbsolutePath()); 
         errorMessage.append("error in reading file, file '" + file.getAbsolutePath() +"'  not available");
         errorCode.value= 28;
         return;
      } 
        BufferedInputStream restoreFile = null;
            try {
                     restoreFile = new BufferedInputStream( new FileInputStream( filename ) );
            } catch (FileNotFoundException e) {
          errorCode.value=28;
          return;
      }

      // command seems to be correct and the file seems to be readable
      // => begin server communication

      String tag = dbrestore?"DbRestore":"ObjectRestore"; 
      outSocketStream.write("<"+tag+">\n");
      outSocketStream.write(name+"\n");
      outSocketStream.write("<FileData>\n");
      long filelength = file.length();
      outSocketStream.write(""+filelength+"\n");
      // write the content of the file
      outSocketStream.flush();
      long sentData = 0; // number of data sent 
      int next;
      while ((next=restoreFile.read())>=0 ){
              outSocketStream.write((byte)next);
              sentData++;
       }
       if(sentData!=filelength){ // should never occur
          Reporter.writeError("sent data("+sentData+") unequals filesize ("+filelength+") ");
       }
       outSocketStream.write( "</FileData>\n" );
       outSocketStream.write("</"+tag+">\n");
       outSocketStream.flush();
       restoreFile.close();
       receiveResponse(resultList, errorCode, errorPos, errorMessage);
  }

/** performs a save (object or database) command supporting the 
  * secondo method. It has to be ensured that the command starts
  * with "save" or "(  save" .
  **/
  private void callSaveCommand(String command,
                               ListExpr resultList,
                               IntByReference errorCode,
                               IntByReference errorPos, 
                                StringBuffer errorMessage) throws IOException{

     resultList.setValueTo(ListExpr.theEmptyList());
     if(command.startsWith("save"))
          command = "("+command+")";
     ListExpr cmdList = new ListExpr();
     if(cmdList.readFromString(command)!=0){
        errorCode.value=9; // syntax error in command
        return;
     }
     int len = cmdList.listLength();
     if(len!=4){
        errorCode.value=9;
        return;
     }
     // check for correct List structure
     if( cmdList.first().atomType()!=ListExpr.SYMBOL_ATOM ||
         cmdList.second().atomType()!=ListExpr.SYMBOL_ATOM ||
         cmdList.third().atomType()!=ListExpr.SYMBOL_ATOM ){
            errorCode.value=9;
            return;
      }
      if(!cmdList.first().symbolValue().equals("save") ||
         !cmdList.third().symbolValue().equals("to")){
          errorCode.value=9;
          return;
      }

      ListExpr fileList = cmdList.fourth();
      String filename="";
      switch( fileList.atomType() ){
        case ListExpr.SYMBOL_ATOM: filename=fileList.symbolValue(); break;
        case ListExpr.STRING_ATOM: filename=fileList.stringValue(); break;
        case ListExpr.TEXT_ATOM: filename = fileList.textValue(); break;
        default: errorCode.value=9;
                 return;
      }
      String objectName = cmdList.second().symbolValue();

      //  all thems to be correct, begin communication
      if(objectName.equals("database")){
          outSocketStream.write("<DbSave/>\n");
      }else{
          outSocketStream.write("<ObjectSave>\n");
          outSocketStream.write(objectName+"\n");
          outSocketStream.write("</ObjectSave>\n");
      }
      // wait for answer
      outSocketStream.flush();
      receiveResponse(resultList, errorCode, errorPos, errorMessage);
      if(errorCode.value==0){ // no error save List
          errorCode.value=resultList.writeToFile(filename);
      }
       // suppress a result
       resultList.setValueTo(new ListExpr());
  }

/**************************************************************************
3.1 The Secondo Procedure

*/

protected void secondo(String command,
                    ListExpr resultList,
                    IntByReference errorCode,
                    IntByReference errorPos,
                    StringBuffer errorMessage){
   // write command to console if desired

 if(Environment.SHOW_COMMAND){
    Reporter.write(command);
  }  

  // clean the errormessage
  errorMessage.setLength(0); 
  errorCode.value    = 0;
  resultList.setValueTo( ListExpr.theEmptyList() );


  // check for existing connection
  if ( serverSocket == null ) { 
     errorCode.value = 80;
     return;
  }

  // check for restore command
  command=command.trim();
  try{
     if(command.startsWith("restore ")){
         callRestoreCommand(command, resultList, errorCode, errorPos, errorMessage);
         return;
     }
     if(command.startsWith("(") && command.substring(1,command.length()).trim().startsWith("restore ")){
         callRestoreCommand(command, resultList, errorCode, errorPos, errorMessage);
         return;
     }
     // check for save command
     if(command.startsWith("save")){
        callSaveCommand(command,resultList,errorCode,errorPos,errorMessage);
        return;
     }
     if(command.startsWith("(") && command.substring(1,command.length()).trim().startsWith("save ")){
        callSaveCommand(command,resultList,errorCode,errorPos,errorMessage);
        return;
     }
 
      // not a special command 
      int commandLevel = command.startsWith("(")?0:1; 
      outSocketStream.write( "<Secondo>\n");
      outSocketStream.write( commandLevel + "\n" );
      outSocketStream.write(command);
      outSocketStream.write("\n</Secondo>\n" );
      outSocketStream.flush();
      receiveResponse(resultList, errorCode, errorPos, errorMessage);
  } catch(IOException e){
     errorCode.value=81;
     return;
  }
}

public boolean setHeartbeat(int heart1, int heart2){
   try{
     outSocketStream.write("<HEARTBEAT>\n");
     outSocketStream.write("" + heart1 + "\n");
     outSocketStream.write("" + heart2 + "\n");
     outSocketStream.write("</HEARTBEAT>\n");
     outSocketStream.flush();
     String line = inSocketStream.readLine();
     return "<YES>".equals(line); 
  } catch(IOException e){
     return false;
  }
}


/*
1.3 Procedure ~NumericTypeExpr~

*/
  public ListExpr numericTypeExpr( ListExpr type )
  {
    ListExpr list = new ListExpr();
    ListExpr internalType = ListExpr.oneElemList( type );
    if ( serverSocket != null ) {
      String line;
      StringBuffer listBuffer = new StringBuffer();
      if ( internalType.writeToString( listBuffer ) == 0 ) {
        try {
          outSocketStream.write( "<NumericType>\n" +
                                 listBuffer.toString() + "\n" +
                                 "</NumericType>\n" );
          outSocketStream.flush();
          line = inSocketStream.readLine();
      if (line==null) throw new IOException();
          if ( line.equals( "<NumericTypeResponse>" ) ) {
            line = inSocketStream.readLine();
            list.readFromString( line );
            line = inSocketStream.readLine(); // Skip end tag '</NumericTypeResponse>'
          } else {
            // Ignore error response
            do {
              line = inSocketStream.readLine();
          if(line==null) throw new IOException();
            } while ( line.compareTo( "</SecondoError>" ) != 0 );
          }
        } catch (IOException e) {
          Reporter.writeError( "SecondoInterface: Network Error in method numericTypeExpr." );
        }
      }
    }
    return (list);
  }

  public boolean getTypeId( String name,
                            IntByReference algebraId, IntByReference typeId )
  {
    boolean ok = false;
    algebraId.value = 0;
    typeId.value = 0;
    if ( serverSocket != null ) {
      String line;
      try {
        outSocketStream.write( "<GetTypeId>\n" +
                               name + "\n" +
                               "</GetTypeId>\n" );
        outSocketStream.flush();
        line = inSocketStream.readLine();
    if(line==null) throw new IOException();
        if ( line.compareTo( "<GetTypeIdResponse>" ) == 0 ) {
          line = inSocketStream.readLine();
          if(line==null) throw new IOException();
          StringTokenizer tokenizer = new StringTokenizer( line );
          if ( tokenizer.countTokens() >= 2 ) {
            algebraId.value = Integer.parseInt( tokenizer.nextToken() );
            typeId.value =  Integer.parseInt( tokenizer.nextToken() );
          }
          line = inSocketStream.readLine(); // Skip end tag '</GetTypeIdResponse>'
          ok = !(algebraId.value == 0 && typeId.value == 0);
        } else {
          // Ignore error response
          do {
            line = inSocketStream.readLine();
        if(line==null) throw new IOException();
          } while ( line.compareTo( "</SecondoError>" ) != 0 );
          ok = false;
        }
      } catch (IOException e) {
        Reporter.writeError( "SecondoInterface: Network Error in method getTypeId." );
      }
    }
    return (ok);
  }

  boolean lookUpTypeExpr( ListExpr type, StringBuffer name,
                          IntByReference algebraId, IntByReference typeId )
  {
    boolean ok = false;
    if ( serverSocket != null ) {
      String line;
      StringBuffer listBuffer = new StringBuffer();
      if ( type.writeToString( listBuffer ) == 0 ) {
        try {
          outSocketStream.write( "<LookUpType>\n" +
                                 listBuffer.toString() + "\n" +
                                 "</LookUpType>\n" );
          outSocketStream.flush();
          line = inSocketStream.readLine();
      if(line==null) throw new IOException();
          if ( line.equals("<LookUpTypeResponse>") ) {
            ListExpr list = new ListExpr();
            line = inSocketStream.readLine();
        if(line==null) throw new IOException();
            list.readFromString( line );
            name.setLength( 0 );
            if ( !list.first().isEmpty() ) {
              name.append( list.first().first().symbolValue() );
            }
            algebraId.value = list.second().intValue();
            typeId.value    = list.third().intValue();
            line = inSocketStream.readLine();
            list = null;
            ok = true;
          } else {
            // Ignore error response
            do {
              line = inSocketStream.readLine();
          if(line==null) throw new IOException();
            } while ( line.compareTo( "</SecondoError>" ) != 0 );
          }
        } catch (IOException e) {
          Reporter.writeError( "SecondoInterface: Network Error in method lookUpTypeExpr." );
        }
      }
    }
    if ( !ok ) {
      name.setLength( 0 );
      algebraId.value = 0;
      typeId.value    = 0;
    }
    return (ok);
  }

/*
3.5.7 The closeSocket method.

This method takes as argument a socker ~socket~ and closes it.
It returns doing nothing if the received socket is null. No error
message is returned to the user if an error happen while trying to
close the socket.

*/

  private final void closeSocket( Socket socket ) {
    // If the socket is alredy closed, return doing nothing.
    if ( socket != null ) {
      try {
        socket.close();
      } catch (IOException e) {
        Reporter.writeError("SecondoInterface: IOException while closing server socket.");
      }
    }
  }

/*
3.5.8 The closeStream method.

This method takes as argument an object ~stream~ and if it is a
BufferedReader or a BufferedWrited it closes it. It returns doing
nothing if the received object is null or is not an stream. No error
message is returned to the user if an error happen while trying to
close the stream.

*/

  private final void closeStream( Object stream ) {
    // If the stream is alredy closed, return doing nothing.
    if ( stream != null ) {
      try {
        if ( stream.getClass() == Class.forName( "java.io.BufferedReader" ) ) {
          ((BufferedReader)stream).close();
        } else if ( stream.getClass() == Class.forName( "java.io.BufferedWriter" ) ) {
          ((BufferedWriter)stream).close();
        } else {
           Reporter.writeError( "SecondoInterface: Error closing stream, object is neither a BufferedReader nor a BufferedWriter." );
    }
      } catch (ClassNotFoundException e) {
        Reporter.writeError( "SecondoInterface: ClassNotFoundException while closing stream.");
      } catch (IOException except) {
        Reporter.writeError( "SecondoInterface: IOException while closing stream.");
      }
    }
  }

private class MyBufferedOutputStream extends BufferedOutputStream{
     public MyBufferedOutputStream(OutputStream out){
          super(out);
     }
     public void write(String s) throws IOException{
            if(Environment.TRACE_SERVER_COMMANDS){
                Reporter.write("Send to Server: \""+s+"\"");
            }
            if(Environment.ENCODING!=null) {
                try{
                  super.write(s.getBytes(Environment.ENCODING));
                } catch(Exception e){
                   System.err.println("invalid encoding found, check environment");
                   System.exit(0);
                }
            } else { 
                super.write(s.getBytes());
            }
            flush();
     }

     public void writeln(String s) throws IOException{
        write(s +"\n");
     }

}

public int sendFile( String localfilename,
                                  String serverFileName,
                                  boolean allowOverwrite ){

   if((localfilename.length()==0) || (serverFileName.length()==0)){
       return ServerErrorCodes.ERR_INVALID_FILE_NAME;
   }
   try{
      outSocketStream.writeln("<FileTransfer>");
      outSocketStream.writeln(serverFileName);   
      String allow = allowOverwrite?"<ALLOW_OVERWRITE>":"<DISALLOW_OVERWRITE>";
      outSocketStream.writeln(allow);
      outSocketStream.flush();
   } catch(Exception e){
      e.printStackTrace();
      return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
   }

   String line;
   try{
     line = inSocketStream.readLine();
     if(line.equals("<SecondoError>")){
        int errCode = ServerErrorCodes.ERR_FILE_EXISTS;
        line = inSocketStream.readLine();
        line = line.trim();
         try{
            errCode = Integer.parseInt(line);
         } catch(NumberFormatException e){
         }
         line = inSocketStream.readLine();
         if(!line.equals("</SecondoError>")){
          return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
         }    
         return errCode;
     }

     if(!line.equals("<SecondoOK>")){
       return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
     }

     sendFile(localfilename);
     outSocketStream.writeln("</FileTransfer>");
     ListExpr resultList = new ListExpr();
     IntByReference errorCode= new IntByReference(); 
     IntByReference errorPos = new IntByReference();
     StringBuffer errorMessage = new StringBuffer();
     receiveResponse( resultList, errorCode, errorPos,
                    errorMessage );   
     return errorCode.value;
   } catch(Exception e){
      e.printStackTrace();
      return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
   }
}



private boolean sendFile(String filename) {

  String line = "";  

  BufferedInputStream in;

  try{
     in = new BufferedInputStream(new FileInputStream(filename));
  } catch(Exception e){
     try{
         outSocketStream.writeln("<SendFileError/>");
     } catch(IOException e2){
        return false;
     }
     return false;
  }

  try{ 
     // send begin file sequence
     outSocketStream.writeln("<FileData>");
     File file = new File(filename);
  
     try {
         long length = file.length();
         int bufSize =512;
         byte[]  buf = new byte[bufSize];    

         // send file size
         outSocketStream.writeln(""+length);
         while(in.available()>0){
            int len = Math.min(bufSize,in.available());
            in.read(buf,0,len);
            outSocketStream.write(buf,0,len);
         }
         in.close(); 
         // send end sequence => empty file;
         outSocketStream.writeln("</FileData>");
         outSocketStream.flush();
     } catch (Exception e) {
        e.printStackTrace();
        return false;
     }   
     return true;
  } catch(Exception e2){
     e2.printStackTrace();
     return false;
  }
} 


public int requestFile(String serverFilename,
                String localFilename,
                boolean allowOverwrite ){

  File localFile = new File(localFilename);
  if(!allowOverwrite && localFile.exists()){
     return ServerErrorCodes.ERR_FILE_EXISTS;
  }
  try{
     outSocketStream.writeln("<RequestFile>");
     outSocketStream.writeln(serverFilename);
     outSocketStream.writeln("</RequestFile>");
     outSocketStream.flush();

     String line;
     line = inSocketStream.readLine();

     if(line.equals("<SecondoError>")){
         int code= -1;
         line = inSocketStream.readLine();
         line = line.trim();
         try{
           code = Integer.parseInt(line);
         } catch(Exception e) {}

         line = inSocketStream.readLine();
         if(!line.equals("</SecondoError>")){
            return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
         } else {
           return code;
         }
      }

      if(!line.equals("<SecondoOK>")){
        return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
      }

      // the answer may be SecondoError or SecondoResponse
      boolean  ok = receiveFile(localFilename);

      if(ok){
          return 0;
      } else {
          return ServerErrorCodes.ERR_IN_FILETRANSFER;
      }
   } catch(Exception e2){
       e2.printStackTrace();
       return ServerErrorCodes.ERR_IN_SECONDO_PROTOCOL;
   }
}


private boolean receiveFile( String localFileName ) throws IOException
{
  
  String errMsg = ""; 
  String line="";
  line = inSocketStream.readLine();
  if(line.equals("<SendFileError/>")){  
     return false;
  }
  if(!line.equals("<FileData>")){
    // protocol error
    return false;
  }
  // read file sizeA
  int size;
  line = inSocketStream.readLine();
  line = line.trim();
  try{
    size = Integer.parseInt(line);
  } catch(Exception e){
     return false;
  } 

  BufferedOutputStream out;  
  try{
     out = new BufferedOutputStream(new FileOutputStream(localFileName));
  } catch(Exception e){
     out = null;
  }
  
  int bufSize=512;
  byte[] buf = new byte[bufSize]; 
  int size2=size;

  // get data and write them to local file
  try{
      while(size>0) {
         if (size < bufSize){
            bufSize = size;  
         }  
         int read =  inSocketStream.read(buf,0,bufSize); 
         out.write(buf, 0, read);
         size -= read; 
      }
      out.close();
  } catch(Exception e){
     e.printStackTrace();
     return false;
  }
  line = inSocketStream.readLine();
  return line.equals("</FileData>");
}



public String getRequestFileFolder(){
   try{
      outSocketStream.writeln("<REQUEST_FILE_FOLDER>"); 
      outSocketStream.flush();
      String line = inSocketStream.readLine();
      return line;
   } catch(Exception e){
      return null;
   }
}

public String getRequestFilePath(){
   try{
      outSocketStream.writeln("<REQUEST_FILE_PATH>");
      outSocketStream.flush();
      return inSocketStream.readLine();
   } catch(Exception e){
      return null;
   }
}

public String getSendFileFolder(){
   try{
      outSocketStream.writeln("<SEND_FILE_FOLDER>");
      outSocketStream.flush();
      return inSocketStream.readLine();
   } catch(Exception e){
      return null;
   }
}

public String getSendFilePath(){
   try{
      outSocketStream.writeln("<SEND_FILE_PATH>");
      outSocketStream.flush();
      return inSocketStream.readLine();
   } catch(Exception e){
      return null;
   }
}

public int getPid(){
   try{
      outSocketStream.writeln("<SERVER_PID>");
      outSocketStream.flush();
      String line = inSocketStream.readLine();
      try{
         return Integer.parseInt(line);
      } catch(Exception e){
         e.printStackTrace();
         return 0;
      }
   } catch(Exception e2){
      return -1;
   }
}

public boolean cancelQuery(int pid) {
   try{
      outSocketStream.writeln("<CANCEL_QUERY>");
      outSocketStream.writeln(Integer.toString(pid));
      outSocketStream.writeln("</CANCEL_QUERY>");
      outSocketStream.flush();
      String line = inSocketStream.readLine();
      
      return line.startsWith("<OK/>");
   } catch(Exception e){
      e.printStackTrace();
      return false;
   }
}


}
