/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] DServerCmdCallBackCommunication

\begin{center}
March 2012 Thomas Achmann
\end{center}

[TOC]

0 Description

The class ~DServerCmdCallBackCommunication~ is a helper class for
the ~DSeverCmd~ class. It derives from the class ~DServerCmdCommunication~ 
and implements the communication functinonality between a server and a client
SECONDO system. While class ~DServerCmdWorkerCommunication~ connects
directly to a running SECONDO instance (e.g. it is possible to send commands
to a remote SECONDO system) this class handles the interprocess communication.
It allows a running command on a SECONDO system to have a direct socket
commnication with another command running on a remote SecondoSystem.

The class DServerCmdCallBackCommunication provides methods to
initiate a socket communictation as well as to connect to an existing socket
(Server and client functionality).
It automatically opens an iostream socket communication or can reuse an
existing communication channel. There are also mehtods to close a socket
communication channel.

There exists several methods to transfer and receive data. Data can be 
of string, int or tuple type. 

The class ~DServerCmdCallBackCommunication~ also provides methods to
store error information.


*/

/*

1 Preliminaries

1.1 Defines

*/

#ifndef H_DSERVERCMDCALLBACKCOMM_H
#define H_DSERVERCMDCALLBACKCOMM_H

/*

1.2 Debug output

uncomment the following line, if debug output should
be written to stdout

*/
//#define DS_CMD_CB_DEBUG 1
/*

1.3 Includes

*/

#include "DServerCmdCommunication.h"
#include "SocketIO.h"
#include "StandardTypes.h"

/*

1.4 Forward declarations

*/

class GenericRelation;

/*

2 Class~ DServerCmdCallBackCommunication~

  * derives from class ~DServerCmdCommunication~

*/


class DServerCmdCallBackCommunication :
  public DServerCmdCommunication
{
/*

2.1 Private enumeration

  * enum MyConnection - describing the type of connection

*/
  enum MyConnection { DSC_CB_NONE, 
                      // this is a server connection, gate exists
                      DSC_CB_CLIENT, 
                      // this is a server connection, NO gate exists
                      DSC_CB_SERVER,
                      //using an existing connection
                      DSC_CB_RESTORE, 
  } ;

/*

2.2 Private default constructor

  * may not be used!

*/
  DServerCmdCallBackCommunication() {}

/*

2.3 Constructor

  * const string[&] inHost - name of the host

  * const string[&] inPort - port number

  * const string[&] DBGMSG - debug message to appear during debugging (optional)

*/
public:
  DServerCmdCallBackCommunication(const std::string& inHost,
                                  const std::string& inPort,
                                  const std::string& DBGMSG = "")
    :DServerCmdCommunication(DBGMSG)
    , m_cbHost(inHost)
    , m_cbPort(inPort)
    , m_cbConnection(DSC_CB_NONE)
    , m_cbGate(NULL)
    , m_cbSock(NULL)
    , m_callbackIoStrOpen(false) {} 
/*

2.4 Destructor

*/
  virtual ~DServerCmdCallBackCommunication()
  {
    closeCallBackCommunication();
  }

/*

2.5 Error handling

2.5.1 Method ~bool hasError~

  * returns true, if an error was detected

*/

  bool hasError() { return m_error; }

/*

2.5.2 Method ~const string[&] getErrorText~
  
  * returns error message in case of error

*/

 
  const std::string& getErrorText() const
  {
    return m_errorText;
  }

/*

2.6 Initiating socket communication

2.6.1 Method ~bool createGlobalSocket~

  Server: opens a socket to a client process

  * returns true - success

*/
 bool createGlobalSocket()
  {
    if (m_cbHost.empty())
      {
        setErrorText("Server - No host for callback communication!");
        return false;
      }
    
    if (m_cbPort.empty())
      {
        setErrorText("Server - No port for callback communication!");
        return false;
      }

    if (m_callbackIoStrOpen)
      {
        setErrorText("Server - Callback communication to worker on " + 
                     m_cbHost +
                     ":" + m_cbPort + " already opened!");
        return false;
      }
#ifdef DS_CMD_CB_DEBUG
    cout << "CONNECTING CALLBACK to " << m_cbPort << "@" << m_cbHost << endl;
#endif
    m_cbSock = Socket::Connect(m_cbHost, m_cbPort, Socket::SockGlobalDomain); 
    
    if (!(m_cbSock -> IsOk()))
      {
        std::string err_txt = "Server - Callback could not be initiated.\n";
        err_txt += "Reason:";
        err_txt += m_cbSock -> GetErrorText();
        setErrorText(err_txt);
        closeCallBackCommunication();
        return false;
      }
    
    setServerConnection();
    return setSocketStream();
  }

/*

2.6.2 Method ~bool startSocket~

  Client: opens a socket to a server process

  * returns true - success

*/

  bool startSocket()
  {
    if (m_cbHost.empty())
      {
        setErrorText("Client - No host for callback communication!");
        return false;
      }
    
    if (m_cbPort.empty())
      {
        setErrorText("Client - No port for callback communication!");
        return false;
      }

    if (m_callbackIoStrOpen)
      {
        setErrorText("Client - Callback communication to worker on " + 
                     m_cbHost +
                     ":" + m_cbPort + " already opened!");
        return false;
      }

#ifdef DS_CMD_CB_DEBUG
    cout << "OPENING CALLBACK on " << m_cbPort << "@" << m_cbHost << endl;
#endif
    m_cbGate =  Socket::CreateGlobal( m_cbHost, 
                                      m_cbPort );
    
    if (!(m_cbGate -> IsOk()))
      {
        std::string err_txt = "Client - Callback could not be initiated.\n";
        err_txt += "Reason:";
        err_txt += m_cbGate -> GetErrorText();
        setErrorText(err_txt);
        closeCallBackCommunication();
        return false;
      }
    return true;
  }

/*

2.6.3 Method ~bool startSocketCommunication~

  Client: accepts a communication to a server

  * returns true - success

*/
  bool startSocketCommunication()
  {
    if (m_cbGate == NULL)
      {
        std::string err_txt = "Client - Callback could not be initiated.\n";
        err_txt += "Reason: Socket connection not available";
        setErrorText(err_txt);
        return false;
      }

    if (!(m_cbGate -> IsOk()))
      {
        std::string err_txt = "Client - Callback could not be initiated.\n";
        err_txt += "Reason:";
        err_txt += m_cbGate -> GetErrorText();
        setErrorText(err_txt);
        closeCallBackCommunication();
        return false;
      }
    
#ifdef DS_CMD_CB_DEBUG
    cout << "ACCEPTING SOCKET  on " << m_cbPort << "@" << m_cbHost << endl;
#endif
    m_cbSock = m_cbGate->Accept();
    
    // m_cbSock is checked here:
    setClientConnection();
    return setSocketStream();
  }

/*

2.6.4 Method ~bool restoreStreamCommunication~

  Client: restore an existing socket communication

  * returns true - success

*/

  bool restoreStreamCommunication(Socket *inWorkerCBConnection)
  {
    if (m_cbSock != NULL)
      {
        std::string err_txt = "Server - Callback could not be initiated.\n";
        err_txt += "Reason: Socket connection not available";
        setErrorText(err_txt);
        return false;
      }

    m_cbSock = inWorkerCBConnection;

    if (!(m_cbSock -> IsOk()))
      {
        std::string err_txt = "Server - Callback could not be initiated.\n";
        err_txt += "Reason:";
        err_txt += m_cbSock -> GetErrorText();
        setErrorText(err_txt);
        closeCallBackCommunication();
        return false;
      }
    
    // m_cbSock is checked here:
    setRestoredConnection();
    return setSocketStream();
  }

/*

2.7 Closing socket communication

2.7.1 Method ~closeCallBackCommunication~

closes a communication channel completly

*/

  void closeCallBackCommunication()
  {
#ifdef DS_CMD_CB_DEBUG
    std::cout << "CLOSING ALL " 
              << m_cbHost << ":" << m_cbPort << std::endl;
    printConnType();
#endif
    closeSocketCommunication();
    closeSocket();
  }
/*

2.7.2 Method ~closeSocket~

Client: close a communication channel completly

*/
  void closeSocket()
  {
#ifdef DS_CMD_CB_DEBUG
    printConnType();
#endif
    // only used for callbacks on clients
    if (getConnectionType() == DSC_CB_CLIENT)
      {
        if (m_cbGate != NULL)
          {
#ifdef DS_CMD_CB_DEBUG
            std::cout << "CLOSING cbGate " 
                      << m_cbHost << ":" << m_cbPort << std::endl;
#endif
            m_cbGate -> Close();
            delete m_cbGate;
          }
        else
          {
            setErrorText( "ERROR CLOSING cbGate " +
                          m_cbHost + ":" + m_cbPort +
                          " : no cbGate available!");
          }
        m_cbGate = NULL;
      }
    else
      {
#ifdef DS_CMD_CB_DEBUG
        std::cout << "Warning - CLOSING cbGate "
                  << m_cbHost << ":" << m_cbPort 
                  << " : not a client communication!" << std::endl;
#endif
      }

    noConnection();
  }


/*

2.7.3 Method ~forceCloseSavedCommunication~

Client: close a communication channel completly

*/
  void forceCloseSavedCommunication()
  {
    std::string line;
    m_cbConnIsSaved = false;
    if (isRestoredConnection())
      setServerConnection();
    receiveLineFromCallBack(line);

#ifdef DS_CMD_CB_DEBUG
     std::cout << "FORCE CLOSE COMMUNICATION " 
             << m_cbHost << ":" << m_cbPort << endl;
#endif

  }

/*

2.7.4 Method ~closeSocketCommunication~

Server: close a communication channel completly

*/
  void closeSocketCommunication()
  {
#ifdef DS_CMD_CB_DEBUG
    printConnType();
#endif
    if (m_cbSock != NULL && !m_cbConnIsSaved && !isRestoredConnection())
      {
#ifdef DS_CMD_CB_DEBUG
        std::cout << "CLOSING cbSock " 
                  << m_cbHost << ":" << m_cbPort << std::endl;
#endif
        m_cbSock -> Close();
        delete m_cbSock;
      }
    else
      {
#ifdef DS_CMD_CB_DEBUG
        std::cerr << "ERROR: CLOSING cbSock " 
                  << m_cbHost << ":" << m_cbPort;
        if (m_cbSock == NULL)
          cerr << " : no cbSock!" << std::endl;
        else if (m_cbConnIsSaved)
          cerr << " : is saved connection!" << std::endl;
        else if (isRestoredConnection())
          cerr << " : is restored conn!" << std::endl;
        else
          cerr << " : unknown reason!" << std::endl;
#endif
      }
    m_cbSock = NULL; 
    if (getConnectionType() != DSC_CB_CLIENT)
      noConnection();
    // else we need to close the gate!
  }

/*

2.8 Accessing socket communication

2.8.1  Method ~getSocketCommunicationForSaving~

(sets flag, that the communication is not destroyed
 upon deletion of the ~DServerCmdCallBackCommunication~ object)

  * returns the socket communicaiton object

*/
  Socket* getSocketCommunicationForSaving()
  {
    m_cbConnIsSaved = true;
    return m_cbSock;
  }

/*

2.7 Sending

2.7.1  Method ~bool sendTagToCallBack~

sends a single tag (e.g. [<]NOERROR/[>])

(expects always an acknowledge token)

  * const string[&] inTag - the tag (e.g. ``NOERROR'')

  * returns true - success

*/
  bool sendTagToCallBack(const std::string& inTag)
  {
#ifdef DS_CMD_CB_DEBUG
    cout << "sendTagToCallBack1:" << inTag << endl;
#endif
    return sendIOS("<" + inTag + "/>", true);
  }
/*

2.7.2  Method ~bool sendTextToCallBack~

sends text enclosed in open/closing tag

  * const string[&] inTag - the tag (e.g. ``NAME'')

  * const string[&] inText - the message 

  * bool reqAck - true: expecting acknowledge token

  * returns true - success

*/
  bool sendTextToCallBack(const std::string& inTag,
                          const std::string& inText,
                          bool reqAck = true)
  {
#ifdef DS_CMD_CB_DEBUG
    cout << "sendTagToCallBack2:" << inTag << ":" << inText << endl;
#endif
    return sendIOS("<" + inTag + ">",
                   inText,
                   "</" + inTag + ">",
                   reqAck);
  }
/*

2.7.3  Method ~bool sendTextToCallBack~

sends text enclosed in open/closing tag

  * const string[&] inTag - the tag (e.g. ``SIZE'')

  * int inText - the message 

  * bool reqAck - true: expecting acknowledge token

  * returns true - success

*/
  bool sendTextToCallBack(const std::string& inTag,
                          int text1,
                          bool reqAck = true)
  {
    return sendIOS("<" + inTag + ">",
                   text1,
                   "</" + inTag + ">",
                   reqAck);
  }
/*

2.7.4  Method ~bool writeTupleToCallBack~

sends a tuple (as binary stream) 

(this method is threadsave w/ regards to DB access)

  * Tuple[ast] inTupe - the tuple to be sent

  * returns true - success

*/
  bool writeTupleToCallBack(Tuple *inTuple);

/*

2.8 Receiving

2.8.1 Method ~bool getTagFromCallBack~

receives a tag (e.g. [<]NOERROR/[>])

(always acknowledges receiving of tag)

  * const string[&] inTag - the tag (e.g. ``NOERROR'')

  * returns true - success

*/
  bool getTagFromCallBack(const std::string& inTag)
  {
    std::string line;
    return receiveIOS(inTag, line);
  }  

/*

2.8.2 Method ~bool getTagFromCallBack~

receives a tag with content

  * const string[&] inTag - the tag (e.g. ``NAME'')

  * string[&] outLine - the received message

  * bool reqAck - true: expecting acknowledge token

  * returns true - success

*/
  bool getTagFromCallBack(const std::string& inTag,
                          std::string &outLine,
                          bool reqAck  = true)
  {
    return receiveIOS(inTag, 
                      outLine,
                      reqAck);
  } 

/*

2.8.3 Method ~bool receiveLineFromCallBack~

receives a line w/o tag

(always acknowledges receiving of tag)

  * string[&] outLine - the received message

  * returns true - success

*/
  bool receiveLineFromCallBack(std::string& outLine)
  {
    return receiveIOS(outLine);
  }

/*

2.8.4 Method ~bool getTagFromCallBackTF~

receives a one of two tags (e.g. [<]DONE/[>] or [<]ERROR/[>])

If first tag was received, method will return true

If second tag was received, method will retrun false;

If an error occurred the ~outErrorFlag~ is set to false;

(always acknowledges receiving of tag)

  * const string[&] inRetTrueTag - tag, if method should return true 
  (e.g. ``DONE'')

  * const string[&] inRetFalseTag - tag, if method should return false 
  (e.g. ``ERROR'')

  * bool[&] outErrorFlag - false: an error occurred

  * returns true - ~inRetTrueTag~ was received

  * returns false - ~inRetFalseTag~ was received

*/
  bool getTagFromCallBackTF(const std::string &inRetTrueTag,
                            const std::string &inRetFalseTag,
                            bool &outErrorFlag)
  {
    std::string line;
    bool retVal = false;
    bool receive = true;
    outErrorFlag = receiveIOS(line);
    if (line.find("<" + inRetTrueTag + "/>") != std::string::npos)
      retVal = true;
    else if (line.find("<" + inRetFalseTag + "/>") != std::string::npos)
      retVal = false;
    else
      receive = false;
    
    if (receive)
      sendIOS("<ACK/>", false);
    else
      sendIOS("<ERROR/>", false);

    return retVal;
  }
/*

2.8.5 Method ~bool receiveTupleFromCallBack~

receives a tuple (binary stream) and inserts it into a relation

(this method is threadsave w/ regards to DB access)

  * ListExpr inTupleType - type of the expected tuple

  * GenericRelation[ast] inRel - pointer to the relation, where the 
tuple will be stored

  * returns true - success

*/
  bool receiveTupleFromCallBack(ListExpr inTupleType,
                                GenericRelation *rel);


/*

2.9 Set connection type

2.9.1 Method ~noConnection~

sets the type of this object to DSC[_]CB[_]NONE

*/
void noConnection() 
  {
    m_cbConnection = DSC_CB_NONE; 
  } 
/*

2.9.2 Method ~setServerConnection~

sets the type of this object to DSC[_]CB[_]SERVER

*/
  void setServerConnection() 
  {
    m_cbConnection = DSC_CB_SERVER; 
  } 
 
/*

2.9.3 Method ~setRestoredConnection~

sets the type of this object to DSC[_]CB[_]RESTORE

*/
  void setRestoredConnection() 
  {
    m_cbConnection = DSC_CB_RESTORE; 
  } 

/*

2.9.4 Method ~setClientConnection~

sets the type of this object to DSC[_]CB[_]CLIENT

*/
  void setClientConnection() 
  { 
    m_cbConnection = DSC_CB_CLIENT; 
  }
/*

2.9 Check connection type

2.9.1 Method ~isRestoredConnection~

checks, if the type of this object is DSC[_]CB[_]RESTORE

*/
  bool isRestoredConnection() const
  {
    return m_cbConnection == DSC_CB_RESTORE;
  }

/*

2.10 Access connection type

2.10.1 Method ~getConnectionType~

returns connection type of this object

*/
  MyConnection getConnectionType() const
  {
    return m_cbConnection;
  }

/*

2.11 Print connection type

2.11.1 Method ~printConnType~

*/
  void printConnType() const
  {
    std::cout << "ConnType:";
    switch(m_cbConnection)
      {

      case DSC_CB_NONE:
        std::cout << "CB_NONE";
        break;
      case DSC_CB_CLIENT:
        std::cout << "CB_CLIENT";
        break;
      case DSC_CB_SERVER:
        std::cout << "CB_SERVER";
        break;
      case DSC_CB_RESTORE:
        std::cout << "CB_RESTORE";
        break;
      default:
        std::cout << "ERROR ConnType!";
        break;
      }
    std::cout << endl;
  }
  
/*

2.12 Private section

*/

private:

/*

2.12.1 Private methods

*/
  void setErrorText(const std::string& inErrTxt)
  {
    m_error = true;
    m_errorText = inErrTxt;
  }

  bool setSocketStream()
  {
    if (m_cbSock == NULL)
      {
        setErrorText("Could not open call back communication!");
        closeCallBackCommunication();
        return false;
      }

    if (!(m_cbSock -> IsOk()))
      {
        std::string err_txt = "Callback could not be established.\n";
        err_txt += "Reason:";
        err_txt += m_cbSock -> GetErrorText();
        setErrorText(err_txt);
        closeCallBackCommunication();
        return false;
      }

    if (!setStream(m_cbSock -> GetSocketStream()))
      {
         setErrorText("Could not initiate communication to worker!");
         closeCallBackCommunication();
         return false;
      }
#ifdef DS_CMD_CB_DEBUG
    cout << "STREAM TO " << m_cbHost << ":" << m_cbPort << "IS OPEN!" << endl;
#endif
    m_cbConnIsSaved = false;
    m_callbackIoStrOpen = true;
    return true;
  }

  bool    Write( void const* buf, size_t size )
  {
    if (!(m_cbSock -> IsOk()))
      {
        std::cerr << "ERROR WRITING binary data to call back" << std::endl; 
        return false;
      }
    bool ret = m_cbSock -> Write(buf, size); 
    return ret;
  }

  bool Read( void* buf, size_t size )
  {
    if (!(m_cbSock -> IsOk()))
      {
        std::cerr << "ERROR READING binary data from call back" << std::endl; 
        return false;
      }

    bool ret = m_cbSock -> Read(buf, size);

    return ret;
  }

/*

2.12.2 Private members

*/
  std::string m_cbHost;
  std::string m_cbPort;
  MyConnection m_cbConnection;
  Socket* m_cbGate;
  Socket* m_cbSock;

  bool m_callbackIoStrOpen;
  bool m_cbConnIsSaved;

  bool m_error;
  std::string m_errorText;
/*

2.13 End of class 

*/
};
#endif //H_DSERVERCMDCALLBACKCOMM_H
