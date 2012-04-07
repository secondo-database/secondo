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

*/

/*
[1] DServerCmdCommunication

March 2012 Thomas Achmann

The class DServerCmdCommunication is a helper class for communication
between master and workers. It implements basic communication functinonality
with another host via an iostream. The socket connection itself
is performed elsewhere.


*/


#ifndef H_DSERVERCMDCOMMUNICATION_H
#define H_DSERVERCMDCOMMUNICATION_H



/*

1 Preliminaries

1.1 Includes

*/

#include <iostream>
#include <vector>

/*

1.2 debug output

uncomment the following line, if debug output should
be written to stdout

*/
// #define DS_CMD_COMM_DEBUG 1

/*

2 Class ~DServerCmdCommunication~

*/
class DServerCmdCommunication   
{
public:

/*

2.1 Default constructor

*/
  DServerCmdCommunication()
    : m_iostr(NULL)
    , m_debugMSG ("") {}
/*

2.2 Constructor 

  * string BBGMSG - adds a string infront of any debug msg

*/
  DServerCmdCommunication(const std::string& DBGMSG)
    : m_iostr(NULL)
    , m_debugMSG (DBGMSG) {}

/*

2.3 Default destructor

*/
  virtual ~DServerCmdCommunication() {}

/*

2.4 Method ~bool setStream~

  * iostream inStream  - the stream used for communication 

  * returns true, if stream is usable

*/

  bool setStream(std::iostream &inStream)
  {
    m_iostr = &inStream;
    
    if (m_iostr == NULL ||
        !(m_iostr -> good()))
      return false;
    return true;
  }

/*

2.5 Sending

methods for sending data via iostream to remote secondo system

2.5.1 Method ~bool sendSecondoCmd~

sends a secondo command to the worker.

  * int inCmdTyp         - command type (0 - NetstedList; 1 - cmd line text)

  * const string[&] inCmd  - command for secondo

  * returns true: success; false: error


*/
  bool sendSecondoCmd(int inCmdType,
                      const std::string& inCmd)

  // secondo does not acknowledge tokens!
  {
    
    if (! sendIOS("<Secondo>", false))
      return false;
    
    if (!sendIOS(inCmdType, false) )
      return false;
        
    if (!sendIOS(inCmd, false) )
      return false;

    if (! sendIOS("</Secondo>", false))
      return false;
    
    return true;
  }

/*

2.5.2 Method ~bool sendIOS~

sends a tagged line to secondo

  * const string[&] inOpenTag - opening tag (e.g. '[<]CMD[>]'

  * const string[&] inLine  - line for secondo

  * const string[&] inCloseTag  - closing tag (e.g. '[<]/CMD[>]'

  * bool reqAck  - expect '[<]ACK/[>]' token after sending a line

  * returns true: success; false: error


*/


  bool sendIOS(const std::string& inOpenTag,
               const std::string& inLine,
               const std::string& inCloseTag,
               bool reqAck = true)
  {
    if (! sendIOS(inOpenTag, reqAck))
      return false;
    if (! sendIOS(inLine, reqAck))
      return false;
    if (! sendIOS(inCloseTag, reqAck))
      return false;
    return true;
  }

/*

2.5.3 Method ~bool sendIOS~

sends a tagged line to secondo

  * const string[&] inOpenTag - opening tag (e.g. '[<]CMD[>]'

  * int inLine  - line for secondo

  * const string[&] inCloseTag  - closing tag (e.g. '[<]/CMD[>]'

  * bool reqAck  - expect '[<]ACK/[>]' token after sending a line

  * returns true: success; false: error

*/

  bool sendIOS(const std::string& inOpenTag,
               int inLine,
               const std::string& inCloseTag,
               bool reqAck = true)
  {
    if (! sendIOS(inOpenTag, reqAck))
      return false;
    if (! sendIOS(inLine, reqAck))
      return false;
    if (! sendIOS(inCloseTag, reqAck))
      return false;
    return true;
  }

/*

2.5.4 Method ~bool sendIOS~

sends a single line to secondo

  * const string[&] inLine  - line for secondo

  * bool checkRet         - expect '[<]ACK/[>]' token after sending a line

  * returns true: success; false: error


*/

  bool sendIOS(const std::string& inLine,
               bool checkRet)
  {
    if (m_iostr -> good())
      {
#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout << m_debugMSG << " SEND1: " 
                    << std::endl << inLine << std::endl;
          if (checkRet)
            std::cout << "w/ ACK" << std::endl;
          else
            std::cout << "w/o ACK" << std::endl;
          
#endif
        (*m_iostr) << inLine << std::endl;
      }

    if (!(m_iostr -> good()))
      {
#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout << m_debugMSG << " ERROR: stream is not available!"
                    << std::endl;
#endif
        return false;
      }
    
    if (checkRet)
      {
        std::string line;
        getline(*m_iostr, line);

        if (line.find("<ACK/>") != std::string::npos)
          return true;
        
        return false;
      }
    return true;
  } 
/*

2.5.5 Method ~bool sendIOS~

sends a single line to secondo

  * int inLine  - line for secondo

  * bool checkRet         - expect '[<]ACK/[>]' token after sending a line

  * returns true: success; false: error


*/
  bool sendIOS(int inLine,
               bool checkRet)
  {
    if (m_iostr -> good())
      {
#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout << m_debugMSG << " SEND: " 
                    << std::endl << inLine << std::endl;
          if (checkRet)
            std::cout << "w/ ACK" << std::endl;
          else
            std::cout << "w/o ACK" << std::endl;
#endif
        (*m_iostr) << inLine << std::endl;
      }

    if (!(m_iostr -> good()))
      {
#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout  << m_debugMSG << " ERROR: stream is not available!"
                     << std::endl;
#endif
        return false;
      }
    if (checkRet)
      {
        std::string line;
        getline(*m_iostr, line);
        if (line.find("<ACK/>") != std::string::npos)
          return true;

        return false;
      }
    return true;
  } 

/*

2.6 Receiving

receiving data from a remote secondo system via iostream


2.6.1 Method ~bool receiveIOS~

receives a single line from secondo

  * string[&] outLine - line, which will be received

  * returns true: success; false: error


*/
  bool receiveIOS(std::string& outLine)
  {
    if (m_iostr -> good())
      {
        getline(*m_iostr, outLine);

#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout  << m_debugMSG << " RECIOS: " << std::endl
                << outLine << std::endl;
#endif
        if (!(m_iostr -> good()))
          {
#ifdef DS_CMD_COMM_DEBUG
            if (!m_debugMSG.empty())
              std::cout << m_debugMSG << "ERROR: stream is not available!" 
                        << std::endl;
#endif
            return false;
          }
      }
    return true;
  } 

/*

2.6.2 Method ~bool receiveIOS~

expects a certain ~inTag~ from the iostream and stores the data
in ~outLine~

  * const string[&] inTag - expected Tag from the remote system

  * string[&] outLine - line, which will be received

  * bool reqAck       - require acknowledge; send '[<]ACK/[>]' token 
after receiving a line;  send '[<]ERROR/[>]' token in case of an error

  * returns true: success; false: error


*/
  bool receiveIOS(const std::string& inTag,
                  std::string& outLine,
                  bool reqAck = true)
  {
#ifdef DS_CMD_COMM_DEBUG
        if (!m_debugMSG.empty())
          std::cout  << m_debugMSG << " RECIOS: " << std::endl;
#endif
    const std::string openTag = "<" + inTag + ">";
    const std::string closeTag =  "</" + inTag + ">";
    const std::string finishTag =  "<" + inTag + "/>";

    if (m_iostr -> good())
      {
        std::string tagLine;
        if (!receiveIOS(tagLine))
          {
            if (reqAck) sendIOS("<ERROR/>", false);
            return false;
          }
        if (tagLine == finishTag)
          {
            if (reqAck) sendIOS("<ACK/>", false);
            return true; // receive only one tag:  <.../>
          }
        else if (tagLine != openTag)
          {
            if (reqAck) sendIOS("<ERROR/>", false);
            return false;
          }
        
        if (reqAck) sendIOS("<ACK/>", false);

        if (!receiveIOS(outLine))
          {
            if (reqAck) sendIOS("<ERROR/>", false);
            return false;
          }
        
        if (reqAck) sendIOS("<ACK/>", false);

        if (!receiveIOS(tagLine))
          {
            if (reqAck) sendIOS("<ERROR/>", false);
            return false;
          }
        if (tagLine != closeTag)
          {
            if (reqAck) sendIOS("<ERROR/>", false);
            return false;
          }

        if (!(m_iostr -> good()))
          {
#ifdef DS_CMD_COMM_DEBUG
            if (!m_debugMSG.empty())
              std::cout << "ERROR: stream is not available!"<< std::endl;
#endif
            return false;
          }
        
      }
    
    if (reqAck) sendIOS("<ACK/>", false);
    return true;
  }
/*

2.7 Method ~void SetDebugHeader()~

sets the output string of debug output

  * const string[&] inStr - new debug header string

*/
  void SetDebugHeader(const std::string& inStr) { m_debugMSG = inStr; }

/*

2.8 private methods

*/
private:
  // n/a
/*

2.9 private members

*/
private:
  // the socket stream 
  std::iostream* m_iostr;

  // debug message header
  std::string m_debugMSG;

};

#endif // H_DSERVERCMDCOMMUNICATION_H
