/******************************************************************************

//paragraph	[10]	title:		[\centerline{\Large \bf] [}]

[10] SecondoInterface.java

April 2002 Ulrich Telle Client/Server version of the SecondoInterface for version 2 of Secondo


\tableofcontents

******************************************************************************/

package sj.lang;

import java.net.*;
import java.io.*;
import java.util.StringTokenizer;

/*

3 The SecondoInterface class implementation.

 In this section we describe the implementation of the SecondoInterface class.

*/

public class SecondoInterface extends Object {

/*
3.1 Public fields.

The next public fields are defined in the SecondoInterface class and
hence they are accessible by the user code.

*/
     // Secondo executable command in nested list syntax (list).
  public final int EXEC_COMMAND_LISTEXPR_SYNTAX    = 0;
     // Secondo executable command in SOS syntax (text).
  public final int EXEC_COMMAND_SOS_SYNTAX         = 1;
     // Secondo descriptive command after, or without, algebraic optimization (list).
  public final int DESCRIP_COMMAND_AFTER_OPT_LIST  = 2;
     // Secondo descriptive command after, or without, algebraic optimization (text).
  public final int DESCRIP_COMMAND_AFTER_OPT_TEXT  = 3;
     // Secondo descriptive command before algebraic optimization (list).
  public final int DESCRIP_COMMAND_BEFORE_OPT_LIST = 4;
     // Secondo descriptive command before algebraic optimization (text).
  public final int DESCRIP_COMMAND_BEFORE_OPT_TEXT = 5;
     // Command in some specific query language (e.g. SQL, GraphDB, etc.).
  public final int COMMAND_SPECIFIC_QUERY_LANGUAGE = 7;

  public static final int DESCRIPTIVE_ALGEBRA = 1;
  public static final int EXECUTABLE_ALGEBRA  = 2;

/*
3.2 Private fields.

The next private fields are defined in the SecondoInterface class.
Hence they are used into the SecondoInterface class code, but they are
not accessible by the user code.

*/

  /* FIELDS DEFINITION*/
  // If this field is set to true the debug messages will be printed in the
  // standard error output stream (System.err). Otherwise the debug messages
  // will not be shown.
  private static final boolean DEBUG_MODE = true;

  // This field sets the name of the file where Secondo() method must store the
  // result when the commandAsText parameter is set to true. It is predefined
  // to be "SecondoResult".
  private static final String SECONDO_RESULT_FILE = "SecondoResult";

  // Flag indicating if a connection with a Secondo server exists.
  protected boolean initialized;
  // Socket connected to the Secondo Server
  protected Socket serverSocket;
  // Connection's input reader (from secondoServerSocket).
  private BufferedReader inSocketStream;
  // Connection's output writer (to secondoServerSocket).
  private BufferedWriter outSocketStream;

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

  public boolean initialize( String user, String pswd,
                             String host, int port )
  {
    String secHost = host;
    int secPort = port;
    String line;
    if ( !initialized )
    {
      System.out.println( "Initializing the Secondo system ..." );
      // Connect with server, needed host and port
      if ( secHost.length() > 0 && secPort > 0 )
      {
        System.out.println( "SecondoInterface: Connecting with Secondo server '" +
                            secHost + "' on port " + secPort + " ..." );
        try
        {
          serverSocket = new Socket( secHost, secPort );
          try {
            // Creates the input/output streams.
            inSocketStream = new BufferedReader( new InputStreamReader( serverSocket.getInputStream() ) );
            outSocketStream = new BufferedWriter( new OutputStreamWriter( serverSocket.getOutputStream() ) );
          } catch (IOException e) {
            closeSocket( serverSocket );
            serverSocket = null; // To help detecting internal errors.
            System.out.println( "SecondoInterface: IOError creating input/output streams for serverSocket.." );
          }
        } catch (UnknownHostException e) {
          System.out.println( "SecondoInterface: Network error: Unknown host '" + secHost + "'." );
        } catch (IOException e) {
          System.out.println( "SecondoInterface: Network error: Unable to connect with the Secondo Server." );
        }

        if ( serverSocket != null ) {
          try {
            line = inSocketStream.readLine();
            if ( line.equals( "<SecondoOk/>" ) ) {
              outSocketStream.write( "<Connect>\n" +
                                    "((" + user + ") (" + pswd + "))\n" +
                                    "</Connect>\n" );
              outSocketStream.flush();
              line = inSocketStream.readLine();
              if ( line.equals( "<SecondoIntro>" ) )
              {
                do
                {
                  line = inSocketStream.readLine();
                  if ( !line.equals( "</SecondoIntro>" ) )
                  {
                    System.out.println( line );
                  }
                }
                while ( !line.equals( "</SecondoIntro>" ) );
                initialized = true;
              }
              else if ( line.equals( "<SecondoError>" ) )
              {
                line = inSocketStream.readLine();
                System.out.println( "Server-Error: " + line );
                line = inSocketStream.readLine();
              }
              else
              {
                System.out.println( "Unidentifiable response from server: " + line );
              }
            }
            else if ( line.equals( "<SecondoError>" ) )
            {
              line = inSocketStream.readLine();
              System.out.println( "Server-Error: " + line );
              line = inSocketStream.readLine();
            }
            else
            {
              System.out.println( "Unidentifiable response from server: " + line );
            }
          } catch (IOException e) {
          }
        } else {
          System.out.println( "failed." );
        }
        if ( !initialized && serverSocket != null ) {
          closeSocket( serverSocket );
          serverSocket = null;
        }
      } else {
        System.out.println( "*** SecondoInterface: Invalid or missing host (" + secHost +
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
        System.out.println( "SecondoInterface: Terminate: Error sending <Disconnect/>." );
      }
      try {
        inSocketStream.close();
      } catch (IOException e) {
        System.out.println( "SecondoInterface: Terminate: Error closing input stream." );
      }
      try {
        outSocketStream.close();
      } catch (IOException e) {
        System.out.println( "SecondoInterface: Terminate: Error closing output stream." );
      }
      try {
        serverSocket.close();
      } catch (IOException e) {
        System.out.println( "SecondoInterface: Terminate: Error closing server socket." );
      }
      serverSocket = null;
    }
  }

/**************************************************************************
3.1 The Secondo Procedure

*/

  public void secondo( String commandText,
                       ListExpr commandLE,
                       int commandLevel,
                       boolean commandAsText,
                       boolean resultAsText,
                       ListExpr resultList,
                       IntByReference errorCode,
                       IntByReference errorPos,
                       StringBuffer errorMessage )
  {
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands.

Error Codes: see definition module.

If value 0 is returned, the command was executed without error.

*/

    String cmdText = "";
    StringBuffer cmdTextBuffer = new StringBuffer();
    ListExpr list = new ListExpr();
    ListExpr answerList = new ListExpr();
    ListExpr errorList;
    ListExpr errorInfo;
    String filename, dbName, objName, typeName;
    String listCommand;         /* buffer for command in list form */
    boolean readResponse = false;

    errorMessage.setLength( 0 );
    errorCode.value    = 0;
    errorList    = ListExpr.oneElemList( ListExpr.symbolAtom( "ERRORS" ) );
    errorInfo    = errorList;
    resultList.setValueTo( ListExpr.theEmptyList() );

    if ( serverSocket == null )
    {
      errorCode.value = 80;
    } else {
      switch (commandLevel)
      {
        case 0:  // executable, list form
        case 2:  // descriptive, list form
        {
          if ( commandAsText ) {
            cmdText = commandText;
          } else {
            if ( commandLE.writeToString( cmdTextBuffer ) != 0 )
            {
              cmdText = cmdTextBuffer.toString();
            } else {
              errorCode.value = 9;  // syntax error in command/expression
            }
          }
          break;
        }
        case 1:  // executable, text form
        case 3:  // descriptive, text form
        {
          cmdText = commandText;
          break;
        }
        default:
        {
          errorCode.value = 31;  // Command level not implemented
        }
      } // switch
    }
    String line = "";
    if ( errorCode.value != 0 )
    {
      return;
    }

    int posDatabase = cmdText.indexOf( "database" );
    int posSave     = cmdText.indexOf( "save" );
    int posRestore  = cmdText.indexOf( "restore" );
    int posTo       = cmdText.indexOf( "to" );
    int posFrom     = cmdText.indexOf( "from" );

    if ( posDatabase >= 0 &&
         posSave     >= 0 &&
         posTo       >= 0 &&
         posSave < posDatabase && posDatabase < posTo )
    {
      if ( commandLevel == 1 || commandLevel == 3 )
      {
        cmdText = "(" + commandText + ")";
      }
      if ( list.readFromString( cmdText ) == 0 )
      {
        if ( list.listLength() == 4 &&
             list.first().symbolValue().equals( "save" ) &&
             list.second().symbolValue().equals( "database" ) &&
             list.third().symbolValue().equals( "to" ) &&
             list.fourth().isAtom() &&
            (list.fourth().atomType() == ListExpr.SYMBOL_ATOM) )
        {
          filename = list.fourth().symbolValue();
          try {
            outSocketStream.write( "<DbSave>\n" + filename + "\n</DbSave>\n" );
            outSocketStream.flush();
            line = inSocketStream.readLine();
            if ( line.compareTo( "<ReceiveFile>" ) == 0 )
            {
              filename = inSocketStream.readLine();
              line = inSocketStream.readLine();  // Hope it is '</ReceiveFile>'
              BufferedWriter restoreFile = null;
              try {
                restoreFile = new BufferedWriter( new FileWriter( filename ) );
              } catch (IOException e) {
              }
              if ( restoreFile != null )
              {
                outSocketStream.write( "<ReceiveFileReady/>\n" );
                outSocketStream.flush();
                line = inSocketStream.readLine();
                if ( line.compareTo( "<ReceiveFileData>" ) == 0 )
                {
                  while ( line.compareTo( "</ReceiveFileData>" ) != 0 )
                  {
                    line = inSocketStream.readLine();
                    if ( line.compareTo( "</ReceiveFileData>" ) != 0 )
                    {
                      restoreFile.write( line );
                      restoreFile.newLine();
                    }
                  }
                }
                restoreFile.close();
              } else {
                outSocketStream.write( "<ReceiveFileError/>\n" );
                outSocketStream.flush();
              }
            }
            line = inSocketStream.readLine();
            if(line!=null)
	       readResponse = true;
	    else{
	       System.out.println( "SecondoInterface: Network Error in method secondo while receiving file." );
               errorCode.value = 81;
	    }
          } catch (IOException e) {
            System.out.println( "SecondoInterface: Network Error in method secondo while receiving file." );
            errorCode.value = 81;
          }
        } else {
          // Not a valid 'save database' command
          errorCode.value = 1;
        }
      } else {
        // Syntax error in list
        errorCode.value = 9;
      }
    } else if ( posDatabase >= 0 &&
                posRestore  >= 0 &&
                posFrom     >= 0 &&
                posRestore < posDatabase && posDatabase < posFrom )
    {
      if ( commandLevel == 1 || commandLevel == 3 ) {
        cmdText = "(" + commandText + ")";
      }
      if ( list.readFromString( cmdText ) == 0 )
      {
        if ( list.listLength() == 5 &&
             list.first().symbolValue().equals( "restore" ) &&
             list.second().symbolValue().equals( "database" ) &&
             list.third().isAtom() &&
            (list.fourth().atomType() == ListExpr.SYMBOL_ATOM) &&
             list.fourth().symbolValue().equals( "from" ) &&
             list.fifth().isAtom() &&
            (list.fifth().atomType() == ListExpr.SYMBOL_ATOM) )
        {
          filename = list.fifth().symbolValue();
          try {
            outSocketStream.write( "<DbRestore>\n" +
                                   list.third().symbolValue() + " " + filename +
                                   "\n</DbRestore>\n" );
            outSocketStream.flush();
            line = inSocketStream.readLine();
            if ( line.compareTo( "<SendFile>" ) == 0 )
            {
              filename = inSocketStream.readLine();
              line = inSocketStream.readLine();  // Hope it is '</SendFile>'
              BufferedReader restoreFile = null;
              try {
                restoreFile = new BufferedReader( new FileReader( filename ) );
              } catch (FileNotFoundException e) {
              }
              if ( restoreFile != null )
              {
                outSocketStream.write( "<SendFileData>\n" );
                while (restoreFile.ready())
                {
                  line = restoreFile.readLine();
                  outSocketStream.write( line );
                  outSocketStream.write( "\n" );
                }
                outSocketStream.write( "</SendFileData>\n" );
                outSocketStream.flush();
                restoreFile.close();
              } else {
                outSocketStream.write( "<SendFileError/>\n" );
                outSocketStream.flush();
              }
            }
            line = inSocketStream.readLine();
	    if(line!=null){
               readResponse = true;
	    } else{
	       System.out.println( "SecondoInterface: Network Error in method secondo while sending file." );
               errorCode.value = 81;
	    }
          } catch (IOException e) {
            System.out.println( "SecondoInterface: Network Error in method secondo while sending file." );
            errorCode.value = 81;
          }
        }
        else
        {
          // Not a valid 'restore database' command
          errorCode.value = 1;
        }
      }
      else
      {
        // Syntax error in list
        errorCode.value = 9;
      }
    } else {
      // Send Secondo command
      try {
        outSocketStream.write( "<Secondo>\n" +
                               commandLevel + "\n" + cmdText +
                               "\n</Secondo>\n" );
        outSocketStream.flush();
        // Receive result
        line = inSocketStream.readLine();
	if(line!=null)
           readResponse = true;
	else{
	   System.out.println( "SecondoInterface: Network Error in method secondo receiving results ." );
           errorCode.value = 81;
	}
      } catch (IOException e) {
        System.out.println( "SecondoInterface: Network Error in method secondo receiving results ." );
        errorCode.value = 81;
      }
    }
    if ( readResponse ) {
      if ( line.compareTo( "<SecondoResponse>" ) == 0 ) {
        String result = "";
	boolean ok = true;
        try {
          do {
            line = inSocketStream.readLine();
	    if(line!=null){
              if ( line.compareTo( "</SecondoResponse>" ) != 0 ) {
                 result += line;
              }
	    }  
	    else{
	      ok = false;
              System.out.println( "SecondoInterface: Network Error in method secondo reading SecondoResponse." );
              errorCode.value = 81;
            }
          } while (ok && line.compareTo( "</SecondoResponse>" ) != 0);
          answerList.readFromString( result );
          errorCode.value = answerList.first().intValue();
          errorPos.value  = answerList.second().intValue();
          errorMessage.setLength( 0 );
          errorMessage.append( answerList.third().textValue() );
          resultList.setValueTo( answerList.fourth() );
        } catch (IOException e) {
          System.out.println( "SecondoInterface: Network Error in method secondo reading SecondoResponse." );
          errorCode.value = 81;
        }
      } else if ( line.compareTo( "<SecondoError>" ) == 0 ) {
        errorCode.value = 80;
        errorMessage.setLength( 0 );
        try {
	  line = inSocketStream.readLine();
	  if(line !=null) {
             errorMessage.append( line );
             line = inSocketStream.readLine();
	   } else{
              System.out.println( "SecondoInterface: Network Error in method secondo reading SecondoError." );
              errorCode.value = 81;
	   }
        } catch (IOException e) {
          System.out.println( "SecondoInterface: Network Error in method secondo reading SecondoError." );
          errorCode.value = 81;
        }
      } else {
        errorCode.value = 80;
      }
    }
    if ( resultAsText ) {
      answerList.writeToFile( "SecondoResult" );
    }
  }

/*
1.3 Procedure ~NumericTypeExpr~

*/
  public ListExpr numericTypeExpr( int level, ListExpr type )
  {
    ListExpr list = new ListExpr();
    ListExpr internalType = ListExpr.oneElemList( type );
    if ( serverSocket != null ) {
      String line;
      StringBuffer listBuffer = new StringBuffer();
      if ( internalType.writeToString( listBuffer ) == 0 ) {
        try {
          outSocketStream.write( "<NumericType>\n" +
                                 "(" + level + " " + listBuffer.toString() + ")\n" +
                                 "</NumericType>\n" );
          outSocketStream.flush();
          line = inSocketStream.readLine();
          if ( line.equals( "<NumericTypeResponse>" ) ) {
            line = inSocketStream.readLine();
            list.readFromString( line );
            line = inSocketStream.readLine(); // Skip end tag '</NumericTypeResponse>'
          } else {
            // Ignore error response
            do {
              line = inSocketStream.readLine();
            } while ( line.compareTo( "</SecondoError>" ) != 0 );
          }
        } catch (IOException e) {
          System.out.println( "SecondoInterface: Network Error in method numericTypeExpr." );
        }
      }
    }
    return (list);
  }

  public boolean getTypeId( int level,
                            String name,
                            IntByReference algebraId, IntByReference typeId )
  {
    boolean ok = false;
    algebraId.value = 0;
    typeId.value = 0;
    if ( serverSocket != null ) {
      String line;
      try {
        outSocketStream.write( "<GetTypeId>\n" +
                               level + " " + name + "\n" +
                               "</GetTypeId>\n" );
        outSocketStream.flush();
        line = inSocketStream.readLine();
        if ( line.compareTo( "<GetTypeIdResponse>" ) == 0 ) {
          line = inSocketStream.readLine();
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
          } while ( line.compareTo( "</SecondoError>" ) != 0 );
          ok = false;
        }
      } catch (IOException e) {
        System.out.println( "SecondoInterface: Network Error in method getTypeId." );
      }
    }
    return (ok);
  }

  boolean lookUpTypeExpr( int level,
                          ListExpr type, StringBuffer name,
                          IntByReference algebraId, IntByReference typeId )
  {
    boolean ok = false;
    if ( serverSocket != null ) {
      String line;
      StringBuffer listBuffer = new StringBuffer();
      if ( type.writeToString( listBuffer ) == 0 ) {
        try {
          outSocketStream.write( "<LookUpType>\n" +
                                 "(" + level + " " + listBuffer.toString() + ")\n" +
                                 "</LookUpType>\n" );
          outSocketStream.flush();
          line = inSocketStream.readLine();
          if ( line == "<LookUpTypeResponse>" ) {
            ListExpr list = new ListExpr();
            line = inSocketStream.readLine();
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
            } while ( line.compareTo( "</SecondoError>" ) != 0 );
          }
        } catch (IOException e) {
          System.out.println( "SecondoInterface: Network Error in method lookUpTypeExpr." );
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
        System.err.println("SecondoInterface: IOException while closing server socket.");
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
	  System.out.println( "SecondoInterface: Error closing stream, object is neither a BufferedReader nor a BufferedWriter." );
	}
      } catch (ClassNotFoundException e) {
        System.out.println( "SecondoInterface: ClassNotFoundException while closing stream.");
      } catch (IOException except) {
        System.out.println( "SecondoInterface: IOException while closing stream.");
      }
    }
  }

}
