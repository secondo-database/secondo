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

1 UnixSocket -- Socket implementation for Unix-like platforms

February 2002 Ulrich Telle

1.1 Overview

This module is an implementation of the abstract base class ~Socket~ for
Unix-like platforms.

For a description of the public interface see the ~SocketIO~ header file.

*/

#if defined(__svr4__)
#define mutex system_mutex
#define socklen_t int
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif
#endif 

#ifdef SECONDO_LINUX
#include <sys/ioctl.h>
#else
#include <stropts.h>
#endif
#include <fcntl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
extern "C" {
#include <netdb.h>
}
#undef mutex

#include "UnixSocket.h"
#include <cstdio>
#include <signal.h>
#include <string>
#include <iostream>
#include <cassert>

#include "LogMsg.h"

using namespace std;

#define MAX_HOST_NAME   256

const string unixSocketDir = "/tmp/";

/*
1.1 Initialization of ~Socket~ library

*/
class UnixSocketLibrary
{
 public:
  UnixSocketLibrary()
  {
    static struct sigaction sigpipeIgnore;
    sigpipeIgnore.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &sigpipeIgnore, NULL );
  }
};

static UnixSocketLibrary unisockLib;

bool
Socket::IsLibraryInitialized()
{
  return (true);
}

/*
1.1 Global and Local Unix Sockets

*/
UnixSocket::UnixSocket( const string& addr, const string& port, const SocketDomain domain )
{ 
  hostAddress    = addr;
  hostPort       = port;
  if ( domain == SockAnyDomain &&
      (port.length() == 0 || addr == "localhost" ) )
  {
    this->domain = SockLocalDomain;
  }
  else
  {
    this->domain = domain;
  }
  createFile     = false;
  lastError      = EC_OK;
  ioSocketBuffer = 0;
  ioSocketStream = 0;
}

UnixSocket::UnixSocket( int newFd ) 
{ 
  fd = newFd; 
  hostAddress = "";
  hostPort = "";
  createFile = false;
  state = SS_OPEN; 
  lastError = EC_OK;
  ioSocketBuffer = new SocketBuffer( *this );
  ioSocketStream = new iostream( ioSocketBuffer );
  ioSocketStream->clear();
}

UnixSocket::~UnixSocket()
{
  Close();
  if ( createFile )
  {
    char name[MAX_HOST_NAME];
    sprintf( name, "%s%s", unixSocketDir.c_str(), hostAddress.c_str() );
    unlink( name );
  }
  if ( ioSocketStream != 0 )
  {
    delete ioSocketStream;
  }
  if ( ioSocketBuffer != 0 )
  {
    delete ioSocketBuffer;
  }
}

bool
UnixSocket::Open( const int listenQueueSize, const int sockType, const int flags )
{
  union
  {
    sockaddr    sock;
    sockaddr_in sock_inet;
    char        name[MAX_HOST_NAME];
  } u;
  int sa_len;

  createFile = false; 
  
  if ( hostAddress.length() > 0 )
  {
    char hostname[MAX_HOST_NAME];
    unsigned short port;
    if ( hostAddress.length() >= sizeof(hostname) ||
         sscanf( hostPort.c_str(), "%hd", &port ) != 1 )
    {
      lastError = EC_BAD_ADDRESS;
      return (false);
    }
    
    if ( domain == SockLocalDomain )
    {
      u.sock.sa_family = AF_UNIX;
      sa_len = offsetof( sockaddr, sa_data ) + 
               sprintf( u.sock.sa_data, "%s%s", 
               unixSocketDir.c_str(), hostAddress.c_str() );
      
      unlink( u.sock.sa_data ); // remove file if existed
      createFile = true; 
    }
    else
    {
      u.sock_inet.sin_family = AF_INET;
      u.sock_inet.sin_addr.s_addr = htonl( INADDR_ANY );
      u.sock_inet.sin_port = htons( port );
      sa_len = sizeof(sockaddr_in);
    } 
  }
  else
  {
    u.sock.sa_family = AF_INET;
    sa_len = 0;
  } 
  if ( (fd = socket( u.sock.sa_family, sockType, 0 )) < 0)
  {
    lastError = errno;
    return (false);
  }
  if ( hostAddress.length() > 0 )
  {
    int on = 1;
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on) );

    if ( bind( fd, &u.sock, sa_len ) < 0 )
    {
      lastError = errno;
      ::close(fd);
      return (false);
    }
  }
  if ( sockType == SOCK_STREAM )
  {
    if ( listen( fd, listenQueueSize ) < 0 )
    {
      lastError = errno;
      ::close( fd );
      return (false);
    }
  }
  else if ( flags & ENABLE_BROADCAST )
  {
    int enabled = 1;
    setsockopt( fd, SOL_SOCKET, SO_BROADCAST, (char*) &enabled, sizeof(enabled) );
  }    
  lastError = EC_OK;
  state = SS_OPEN;
  return (true);
}

bool
UnixSocket::IsOk()
{
  return (lastError == EC_OK);
}

string
UnixSocket::GetErrorText()
{
  string msg; 
  switch (lastError)
  {
    case EC_OK:
      msg = "Ok";
      break;
    case EC_NOT_OPENED:
      msg = "Socket not opened";
      break;
    case EC_BAD_ADDRESS:
      msg = "Bad address";
      break;
    case EC_CONNECTION_FAILED:
      msg = "Connection failed";
      break;
    case EC_BROKEN_PIPE:
      msg = "Connection is broken";
      break; 
    case EC_INVALID_ACCESS_MODE:
      msg = "Invalid access mode";
      break;
    case EC_MESSAGE_TRUNCATED:
      msg = "Sent message was truncated";
      break;
    default: 
      msg = strerror( lastError );
      break;
  }
  return (msg);
}

string
UnixSocket::GetSocketAddress() const
{
  assert( this );
  union
  {
    sockaddr    addr;
    sockaddr_in addr_inet;
    char        name[MAX_HOST_NAME];
  } u;
  socklen_t bytes = sizeof( u.addr );
  if ( ::getsockname( fd, &u.addr, &bytes )  == 0 )
  {
    if ( u.addr.sa_family == AF_INET )
    {
      return (inet_ntoa( u.addr_inet.sin_addr ));
    }
    else
    {
      return ("127.0.0.1");
    }
  }
  else
  {
    return ("<unknown>");
  }
}

string
UnixSocket::GetPeerAddress() const
{
  assert( this );
  union
  {
    sockaddr    addr;
    sockaddr_in addr_inet;
    char        name[MAX_HOST_NAME];
  } u;
  socklen_t bytes = sizeof( u.addr );
  if ( ::getpeername( fd, &u.addr, &bytes ) == 0 )
  {
    if ( u.addr.sa_family == AF_INET )
    {
      return (inet_ntoa( u.addr_inet.sin_addr ));
    }
    else
    {
      return ("127.0.0.1");
    }
  }
  else
  {
    return ("<unknown>");
  }
}

Socket*
UnixSocket::Accept()
{
  int s;

  if ( state != SS_OPEN )
  {
    lastError = EC_NOT_OPENED;
    return (NULL);
  }

  while ( (s = ::accept( fd, NULL, NULL )) < 0 && errno == EINTR );

  if ( s < 0 )
  {
    lastError = errno;
    return (NULL);
  }
  else if ( state != SS_OPEN )
  {
    lastError = EC_NOT_OPENED;
    return (NULL);
  }
  else
  {
    static struct linger l = { 1, LINGER_TIME };
    if ( domain == SockGlobalDomain )
    {
      int enabled = 1;
      if ( setsockopt( s, IPPROTO_TCP, TCP_NODELAY,
                       (char*) &enabled, sizeof(enabled) ) != 0 )
      {
        lastError = errno;
        ::close( s );    
        return (NULL);
      }
    }
    if ( setsockopt( s, SOL_SOCKET, SO_LINGER, (char*) &l, sizeof(l) ) != 0 )
    {
      lastError = EC_INVALID_ACCESS_MODE;
      ::close( s );
      return (NULL); 
    }
    lastError = EC_OK;
    return (new UnixSocket( s ));
  }
}

SocketDescriptor
UnixSocket::GetDescriptor()
{
  if ( state != SS_OPEN )
  {
    lastError = EC_NOT_OPENED;
    return (-1);
  }
  return (fd);
}

bool
UnixSocket::CancelAccept() 
{
  // Wakeup listener
  state = SS_SHUTDOWN;
  delete Socket::Connect( hostAddress, hostPort, domain, 1, 0 );
  return (true);
}  

bool
UnixSocket::Connect( int maxAttempts, time_t timeout )
{
  int   rc;
  unsigned short port;

  if ( domain != SockLocalDomain)
  {
    if ( hostAddress.length() >= MAX_HOST_NAME ||
         sscanf( hostPort.c_str(), "%hd", &port ) != 1 )
    {
      lastError = EC_BAD_ADDRESS;
      return (false);
    }
  }
  createFile = false; 

  union
  { 
    sockaddr    sock;
    sockaddr_in sock_inet;
    char        name[MAX_HOST_NAME];
  } u;

  int sa_len;

  if ( domain == SockLocalDomain ||
      (domain == SockAnyDomain   &&
       (hostPort.length() == 0 || hostAddress == "localhost")) )
  {
    // connect UNIX socket
    u.sock.sa_family = AF_UNIX;
    sa_len = offsetof( sockaddr, sa_data ) +
             sprintf( u.sock.sa_data, "%s%s",
             unixSocketDir.c_str(), hostAddress.c_str() );
  }
  else
  {
    u.sock_inet.sin_family = AF_INET;  
    u.sock_inet.sin_addr.s_addr = inet_addr( hostAddress.c_str() );
  
    if ( (int)(u.sock_inet.sin_addr.s_addr) == -1 )
    {
      struct hostent* hp;  // entry in hosts table
      if ( (hp = gethostbyname( hostAddress.c_str() )) == NULL || 
            hp->h_addrtype != AF_INET )
      {
        lastError = EC_BAD_ADDRESS;
        return (false);
      }
      memcpy( &u.sock_inet.sin_addr, hp->h_addr, sizeof(u.sock_inet.sin_addr) );
    }
    u.sock_inet.sin_port = htons( port );
    sa_len = sizeof(u.sock_inet);
  }
  while (true)
  {
    if ( (fd = socket( u.sock.sa_family, SOCK_STREAM, 0 )) < 0 )
    {
      lastError = errno;
      return (false);
    }
    do
    {
      rc = ::connect( fd, &u.sock, sa_len );
    }
    while (rc < 0 && errno == EINTR);

    if ( rc < 0 )
    {
      lastError = errno;
      ::close( fd );
      if ( lastError == ENOENT || lastError == ECONNREFUSED )
      {
        if ( --maxAttempts > 0 )
        {
          sleep( timeout );
        }
        else
        {
          break;
        }
      }
      else
      {
        return (false);
      }
    }
    else
    {
      if ( u.sock_inet.sin_family == AF_INET )
      {
        int enabled = 1;
        if ( setsockopt( fd, IPPROTO_TCP, TCP_NODELAY,
                         (char*) &enabled, sizeof(enabled) ) != 0 )
        {
          lastError = errno;
          ::close( fd );    
          return (false);
        }
      }
      ioSocketBuffer = new SocketBuffer( *this );
      ioSocketStream = new iostream( ioSocketBuffer );
      ioSocketStream->clear();
      lastError = EC_OK;
      state = SS_OPEN;
      return (true);
    }
  }
  lastError = EC_CONNECTION_FAILED;
  return (false);
}

int
UnixSocket::Read( void* buf, size_t minSize, size_t maxSize, time_t timeout )
{ 
  size_t size = 0;
  time_t start = 0;
  if ( state != SS_OPEN )
  {
    SetStreamState( ios::failbit );
    lastError = EC_NOT_OPENED;
    return (-1);
  }
  if ( timeout != WAIT_FOREVER )
  {
    start = time( NULL ); 
  }
  do
  {
    ssize_t rc; 
    if ( timeout != WAIT_FOREVER)
    {
      fd_set events;
      struct timeval tm;
      FD_ZERO( &events );
      FD_SET( fd, &events );
      tm.tv_sec = timeout;
      tm.tv_usec = 0;
      while ( (rc = select( fd+1, &events, NULL, NULL, &tm )) < 0 &&
              errno == EINTR );
      if ( rc < 0 )
      {
        SetStreamState( ios::failbit );
        lastError = errno;
        return (-1);
      }
      if ( rc == 0 )
      {
        return (size);
      }
      time_t now = time( NULL );
      timeout = start + timeout >= now ? timeout + start - now : 0;  
    }
    while ( (rc = ::read( fd, (char*) buf + size, maxSize - size )) < 0 &&
            errno == EINTR );
    if ( rc < 0 )
    {
      SetStreamState( ios::failbit );
      lastError = errno;
      return (-1);
    }
    else if ( rc == 0 )
    {
      SetStreamState( ios::failbit | ios::eofbit );
      lastError = EC_BROKEN_PIPE;
      return (-1); 
    }
    else
    {
      size += rc; 
    }
  }
  while (size < minSize); 

  return ((int) size);
}

bool
UnixSocket::Read( void* buf, size_t size )
{ 
  if ( state != SS_OPEN )
  {
    SetStreamState( ios::failbit );
    lastError = EC_NOT_OPENED;
    return (false);
  }

  do
  {
    ssize_t rc; 
    while ( (rc = ::read( fd, buf, size )) < 0 && errno == EINTR );
    if ( rc < 0 )
    {
      SetStreamState( ios::failbit );
      lastError = errno;
      return (false);
    }
    else if ( rc == 0 )
    {
      SetStreamState( ios::failbit | ios::eofbit );
      lastError = EC_BROKEN_PIPE;
      return (false);
    }
    else
    {
      buf = (char*) buf + rc; 
      size -= rc; 
    }
  }
  while ( size != 0 ); 

  return (true);
}
    
bool
UnixSocket::Write( void const* buf, size_t size )
{ 
  int sleepCtr = 0;
  int writeAttempts = 0;

  if ( state != SS_OPEN )
  {
    cerr << "EC_NOT_OPENED" << endl;
    SetStreamState( ios::failbit );
    lastError = EC_NOT_OPENED;
    return (false);
  }
  
  do
  {
    ssize_t rc; 
    while ( (rc = ::write( fd, buf, size )) < 0 && errno == EINTR ) { usleep(100); sleepCtr++; };
    if ( rc < 0 )
    {
      cerr << "Lasterror = " << errno << endl;
      SetStreamState( ios::failbit );
      lastError = errno;
      return (false);
    }
    else if ( rc == 0 )
    {
      cerr << "Broken Pipe!" << endl;
      SetStreamState( ios::failbit | ios::eofbit );
      lastError = EC_BROKEN_PIPE;
      return (false);
    }
    else
    { // the cast below is necessary to avoid a warning of 
      // comparison of signed and unsigned values. 
      if ( ((size_t) rc) < size ) { writeAttempts++; } 
      buf = (char*) buf + rc; 
      size -= rc; 
    }
  }
  while (size != 0); 

  LOGMSG( "Socket:SendStat",
    if ( writeAttempts || sleepCtr ) {
      cerr << "Write Attempts: " << writeAttempts << ", " << "Sleep calls (100ms): " << sleepCtr << endl;
    } 
  )
  return (true);
}
    
bool
UnixSocket::Close()
{
  if ( state != SS_CLOSE )
  {
    state = SS_CLOSE;
    if ( ::close( fd ) == 0 )
    {
      SetStreamState( ios::eofbit );
      lastError = EC_OK;
      return (true);
    }
    else
    {
      SetStreamState( ios::failbit | ios::eofbit );
      lastError = errno;
      return (false);
    }
  }
  lastError = EC_OK;
  return (true);
}

bool
UnixSocket::ShutDown()
{
  if ( state == SS_OPEN )
  {
    SetStreamState( ios::eofbit );
    state = SS_SHUTDOWN;
    int rc = ::shutdown( fd, 2 );
    if ( rc != 0 )
    {
      SetStreamState( ios::failbit );
      lastError = errno;
      return (false);
    } 
  } 
  return (true);
}

void
UnixSocket::SetStreamState( ios::iostate newState )
{
  if ( ioSocketStream != 0 )
  {
    ioSocketStream->setstate( newState );
  }
}

Socket*
Socket::CreateLocal( const string& address, const int listenQueueSize )
{
  UnixSocket* sock = new UnixSocket( address, "0", SockLocalDomain );
  sock->Open( listenQueueSize, SOCK_STREAM );
  return (sock);
}

Socket*
Socket::CreateGlobal( const string& address, const string& port, const int listenQueueSize )
{
  UnixSocket* sock = new UnixSocket( address, port, SockGlobalDomain );
  sock->Open( listenQueueSize, SOCK_STREAM );
  return (sock);
}

Socket*
Socket::CreateClient( const SocketDescriptor sd )
{
  UnixSocket* sock = new UnixSocket( sd );
  return (sock);
}

int
Socket::GetIP( const string& address )
{
  int ip = inet_addr( address.c_str() );
  if (ip < 0)
  {
    struct hostent* hp;  // entry in hosts table
    if ( (hp = gethostbyname( address.c_str() )) == NULL ||
          hp->h_addrtype != AF_INET )
    {
      return (-1);
    }
    ip = *(int*)hp->h_addr;
  }
  return (ntohl( ip ));
}

string
Socket::GetHostname( const string& ipAddress )
{
  int ip = inet_addr( ipAddress.c_str() );
  if ( (unsigned) ip != INADDR_NONE )
  {
    struct hostent* hp;  // entry in hosts table
    if ( (hp = gethostbyaddr( (char*) &ip, sizeof(ip), AF_INET )) != 0 )
    {
      return (hp->h_name);
    }
    else
    {
      return ("<unknown>");
    }
  }
  else
  {
    return ("<unknown>");
  }
}

Socket*
Socket::Connect( const string& address, const string& port,
                 const SocketDomain domain, 
                 const int maxAttempts, const time_t timeout )
{
  UnixSocket* sock = new UnixSocket( address, port, domain );
  sock->Connect( maxAttempts, timeout );
  return (sock);
}

  
string
GetProcessName() 
{ 
  static char name[MAX_HOST_NAME+8];
  struct utsname localHost;
  uname( &localHost );
  sprintf( name, "%s:%d", localHost.nodename, (int) getpid() );
  return (name);
}

// --- End of source ---

