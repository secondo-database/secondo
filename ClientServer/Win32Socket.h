/*

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
  Win32Socket( const string& address, const string& port );
  Win32Socket( SOCKET newSock );
  ~Win32Socket();
  SocketDescriptor GetDescriptor();
  bool    Open( const int listenQueueSize, const int socketType, const int flags = 0 );
  bool    Connect( const int maxAttempts, const time_t timeout );
  int     Read( void* buf, size_t minSize, size_t maxSize, time_t timeout );
  bool    Read( void* buf, size_t size );
  bool    Write( void const* buf, size_t size );
  bool    IsOk();
  string  GetErrorText();
  string  GetSocketAddress() const;
  string  GetPeerAddress() const;
  Socket* Accept();
  bool    CancelAccept();
  bool    Close();
  bool    ShutDown();
 protected:
  void    SetStreamState( ios::iostate newState );

  SOCKET  s; 
  int     lastError;  // error code of last failed operation 
  string  hostAddress;
  string  hostPort;

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
  LocalWin32Socket( const string& address );
  LocalWin32Socket(); 
  ~LocalWin32Socket();
  SocketDescriptor GetDescriptor();
  bool    Open( const int listenQueueSize );
  bool    Connect( int maxAttempts, const time_t timeout );
  int     Read( void* buf, size_t minSize, size_t maxSize,time_t timeout );
  bool    Read( void* buf, size_t size );
  bool    Write( void const* buf, size_t size );
  bool    IsOk(); 
  string  GetErrorText();
  string  GetSocketAddress() const;
  string  GetPeerAddress() const;
  Socket* Accept();
  bool    CancelAccept();
  bool    Close();
  bool    ShutDown();
 protected: 
  void    SetStreamState( ios::iostate newState );

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
  string      localName;
};
	   
#endif

