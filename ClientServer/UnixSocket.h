/*

1 Header File: UnixSocket

February 2002 Ulrich Telle

1.1 Overview

This module is an implementation of the abstract base class ~Socket~ for
the Unix-like platforms.

For a description of the public interface see the ~SocketIO~ header file.

*/

#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include "SocketIO.h"

class UnixSocket : public Socket
{
 public:
  UnixSocket( const string& address, const string& port, const SocketDomain domain );
  UnixSocket( int newSocketDescriptor );
  ~UnixSocket();
  SocketDescriptor GetDescriptor();
  bool    Open( const int listenQueueSize, const int sockType, const int flags = 0);
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

  SocketDescriptor fd;
  int              lastError;   // error code of last failed operation 
  string           hostAddress;
  string           hostPort;
  SocketDomain     domain;    // Unix domain or INET socket
  bool             createFile; // Unix domain sockets use files for connection

  enum error_codes { 
    EC_OK                  =  0,
    EC_NOT_OPENED          = -1,
    EC_BAD_ADDRESS         = -2,
    EC_CONNECTION_FAILED   = -3,
    EC_BROKEN_PIPE         = -4, 
    EC_INVALID_ACCESS_MODE = -5,
    EC_MESSAGE_TRUNCATED   = -6
  };
};

#endif

