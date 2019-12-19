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

1 Header File: Win32Socket

February 2002 Ulrich Telle

1.1 Overview

This module is an implementation of the abstract base class ~Socket~ for
the Microsoft Windows plattform.

For a description of the public interface see the ~SocketIO~ header file.

*/

#ifndef WIN32_SOCKET_H
#define WIN32_SOCKET_H

#include "SocketIO.h"
#include <winsock2.h>

class Win32Socket : public Socket
{
 public:
  Win32Socket( const std::string& address, const std::string& port,
	       std::ostream* traceInStream, std::ostream* traceOutStream,
	       bool destroyTrace );
  Win32Socket( SOCKET newSock,
	       std::ostream* traceInStream, std::ostream* traceOutStream,
	       bool destroyTrace );
  ~Win32Socket();
  SocketDescriptor GetDescriptor();
  bool    Open( const int listenQueueSize,
                const int socketType,
                const int flags = 0 );
  bool    Connect( const int maxAttempts, const time_t timeout );
  int     Read( void* buf, size_t minSize, size_t maxSize, time_t timeout );
  bool    Read( void* buf, size_t size );
  bool    Write( void const* buf, size_t size );
  bool    IsOk();
  std::string  GetErrorText();
  std::string  GetSocketAddress() const;
  std::string  GetPeerAddress() const;
  Socket* Accept( std::ostream* traceInStream,
		  std::ostream* traceOutStream,
		  bool destryTrace);
  bool    CancelAccept();
  bool    Close();
  bool    ShutDown();


  void setTraceStreams( std::ostream* traceInStream,
		        std::ostream* traceOutStream,
			bool destroyTrace) {
     
  }

 protected:
  void    SetStreamState( std::ios::iostate newState );

  SOCKET  s;
  int     lastError;  // error code of last failed operation
  std::string  hostAddress;
  std::string  hostPort;
  std::ostream* traceInStream;
  std::ostream* traceOutStream;
  bool destroyTrace;

  void removeTraces(){
     if(traceInStream==traceOutStream){
       traceOutStream = 0;
     }
     if(destroyTrace){
        if(traceInStream) delete traceInStream;
        if(traceOutStream) delete traceOutStream;
     }
     traceInStream = 0;
     traceOutStream=0;
  }


  enum ErrorCodes
  {
    EC_OK                  =  0,
    EC_NOT_OPENED          = -1,
    EC_BAD_ADDRESS         = -2,
    EC_CONNECTION_FAILED   = -3,
    EC_BROKEN_PIPE         = -4,
    EC_INVALID_ACCESS_MODE = -5,
    EC_MESSAGE_TRUNCATED   = -6
  };
};

#define SOCKET_BUF_SIZE (8*1024)
#define ACCEPT_TIMEOUT  (30*1000)

class LocalWin32Socket : public Socket
{
 public:
  LocalWin32Socket( const std::string& address,
		    std::ostream* traceInStream,
		    std::ostream* traceOutStream,
		    bool destroyStream );
  LocalWin32Socket( std::ostream* traceInStream, 
                    std::ostream* traceOutStream, 
                    bool destroyStream );
  ~LocalWin32Socket();
  SocketDescriptor GetDescriptor();
  bool    Open( const int listenQueueSize );
  bool    Connect( int maxAttempts, const time_t timeout );
  int     Read( void* buf, size_t minSize, size_t maxSize,time_t timeout );
  bool    Read( void* buf, size_t size );
  bool    Write( void const* buf, size_t size );
  bool    IsOk();
  std::string  GetErrorText();
  std::string  GetSocketAddress() const;
  std::string  GetPeerAddress() const;
  Socket* Accept( std::ostream* traceInStream,
		  std::ostream* traceOutStream,
		  bool destroyTrace);
  bool    CancelAccept();
  bool    Close();
  bool    ShutDown();

  void setTraceStreams( std::ostream* traceInStream,
		        std::ostream* traceOutStream,
			bool destroyTrace) {
     
  }

 protected:
  void    SetStreamState( std::ios::iostate newState );

  enum ErrorCodes
  {
    EC_OK                  =  0,
    EC_NOT_OPENED          = -1,
    EC_BROKEN_PIPE         = -2,
    EC_TIMEOUT_EXPIRED     = -3,
    EC_INVALID_ACCESS_MODE = -5,
    EC_MESSAGE_TRUNCATED   = -6
  };

  enum SocketSignals
  {
    RD,  // receive data
    RTR, // ready to receive
    TD,  // transfer data
    RTT  // ready to transfer
  };
  //------------------------------------------------------
  // Mapping between signals at opposite ends of socket:
  // TD  ---> RD
  // RTR ---> RTT
  //------------------------------------------------------

  struct LocalSocketBuffer
  {
    volatile int  recvWaitFlag;
    volatile int  sendWaitFlag;
    volatile int  dataEnd;
    volatile int  dataBeg;
             char dataBuf[SOCKET_BUF_SIZE - 4*sizeof(int)];
  };

  struct AcceptData
  {
    HANDLE signalHandle[4];
    HANDLE bufferHandle;
  };

  struct ConnectData
  {
    HANDLE mutexHandle;
    int    processId;
  };

  LocalSocketBuffer* recvBuffer;
  LocalSocketBuffer* sendBuffer;
  HANDLE      signalHandle[4];
  HANDLE      mutexHandle;
  HANDLE      bufferHandle;
  int         lastError;
  std::string      localName;

  std::ostream* traceInStream;
  std::ostream* traceOutStream;
  bool destroyTrace;
  
  void removeTraces(){
     if(traceInStream==traceOutStream){
       traceOutStream = 0;
     }
     if(destroyTrace){
        if(traceInStream) delete traceInStream;
        if(traceOutStream) delete traceOutStream;
     }
     traceInStream = 0;
     traceOutStream=0;
  }

};

#endif

