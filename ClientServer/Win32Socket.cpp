/*

1 Win32Socket -- Socket implementation for Microsoft Windows

February 2002 Ulrich Telle

April 2002 Ulrich Telle Corrected an error in ~Accept~

1.1 Overview

This module is an implementation of the abstract base class ~Socket~ for
the Microsoft Windows plattform.

For a description of the public interface see the ~SocketIO~ header file.

*/

#include "Win32Socket.h"
#include <cstdio>
#include <string>

#define MAX_HOST_NAME         256
#define MILLISECOND           1000

static HANDLE watchDogMutex;

/*
1.1 Initialization of ~Winsock~ library

*/
class Win32SocketLibrary {
 public:
  Win32SocketLibrary()
  {
    WSADATA wsa;
    initialized = (WSAStartup( MAKEWORD(2,1), &wsa ) == 0);
    if ( !initialized )
    {
      cerr << "Failed to initialize windows sockets: " <<
              WSAGetLastError() << endl;
    }
    watchDogMutex = CreateMutex(NULL, TRUE, NULL);
  }
  ~Win32SocketLibrary()
  {
    WSACleanup();
  }
  bool initialized;
};

static Win32SocketLibrary ws32Lib;

bool
Socket::IsLibraryInitialized()
{
  return (ws32Lib.initialized);
}

/*
1.1 Global Windows Sockets

*/
Win32Socket::Win32Socket( const string& addr, const string& port )
{ 
  hostAddress = addr;
  hostPort = port; 
  lastError = EC_OK;
  s = INVALID_SOCKET;
  ioSocketBuffer = 0;
  ioSocketStream = 0;
}

Win32Socket::Win32Socket( SOCKET newSock )
{ 
  s = newSock; 
  hostAddress = "";
  hostPort    = ""; 
  state = SS_OPEN;
  lastError = EC_OK;
  ioSocketBuffer = new SocketBuffer( *this );
  ioSocketStream = new iostream( ioSocketBuffer );
  ioSocketStream->clear();
}

Win32Socket::~Win32Socket()
{
  Close();
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
Win32Socket::Open( const int listenQueueSize, const int sockType, const int flags )
{
  unsigned short port = 0;

  if ( hostAddress.length() > 0 )
  {
    if ( sscanf( hostPort.c_str(), "%hd", &port ) != 1 )
    {
      lastError = EC_BAD_ADDRESS;
      return (false);
    }
  }
  if ( (s = socket( AF_INET, sockType, 0 )) == INVALID_SOCKET )
  {
    lastError = WSAGetLastError();
    return (false);
  }
  if ( hostAddress.length() > 0 )
  {
    struct sockaddr_in insock;
    insock.sin_family = AF_INET;
    insock.sin_addr.s_addr = htonl( INADDR_ANY );
    insock.sin_port = htons( port );

    int on = 1;
    setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on) );

    if ( bind( s, (sockaddr*) &insock, sizeof(insock) ) != 0 )
    {
      lastError = WSAGetLastError();
      closesocket( s );
      return (false);
    }
  }
  if ( sockType == SOCK_STREAM )
  {
    if ( listen( s, listenQueueSize ) != 0 )
    {
      lastError = WSAGetLastError();
      closesocket( s );
      return (false);
    } 
  }
  else if ( flags & ENABLE_BROADCAST )
  {
    int enabled = 1;
    setsockopt( s, SOL_SOCKET, SO_BROADCAST, (char*) &enabled, sizeof(enabled) );
  }	
  lastError = EC_OK;
  state = SS_OPEN;
  return (true);
}

bool
Win32Socket::Connect( int maxAttempts, const time_t timeout )
{
  unsigned short port;

  if ( hostAddress.length() >= MAX_HOST_NAME    ||
       sscanf( hostPort.c_str(), "%hd", &port ) != 1 )
  {
    lastError = EC_BAD_ADDRESS;
    return (false);
  }

  struct sockaddr_in insock;  // inet socket address
  struct hostent*    hp;      // entry in hosts table
	
  if ( (hp = gethostbyname( hostAddress.c_str() )) == NULL ||
        hp->h_addrtype != AF_INET )
  {
    lastError = EC_BAD_ADDRESS;
    return (false);
  }
  insock.sin_family = AF_INET;
  insock.sin_port = htons( port );
  memcpy( &insock.sin_addr, hp->h_addr, sizeof(insock.sin_addr) );
    
  while (true)
  {
    if ( (s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
    {
      lastError = WSAGetLastError();
      return (false);
    }
    if ( ::connect( s, (sockaddr*) &insock, sizeof(insock) ) != 0 )
    {
      lastError = WSAGetLastError();
      closesocket( s );
      if ( lastError == WSAECONNREFUSED )
      {
        if ( --maxAttempts > 0 )
        {
          Sleep( timeout*MILLISECOND );
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
      int enabled = 1;
      if ( setsockopt( s, IPPROTO_TCP, TCP_NODELAY,
                       (char*) &enabled, sizeof(enabled) ) != 0 )
      {
        lastError = WSAGetLastError();
        closesocket( s );	
        return (false);
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
Win32Socket::Read( void* buf, size_t minSize, size_t maxSize, time_t timeout )
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
    int rc;
    if ( timeout != WAIT_FOREVER )
    {
      fd_set events;
      struct timeval tm;
      FD_ZERO( &events );
      FD_SET( s, &events );
      tm.tv_sec = timeout;
      tm.tv_usec = 0;
      rc = select( s+1, &events, NULL, NULL, &tm );
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
    rc = recv( s, (char*) buf + size, maxSize - size, 0 );
    if ( rc < 0 )
    { 
      SetStreamState( ios::failbit );
      lastError = WSAGetLastError();
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
Win32Socket::Read( void* buf, size_t size )
{ 
  if ( state != SS_OPEN )
  {
    SetStreamState( ios::failbit );
    lastError = EC_NOT_OPENED;
    return (false);
  }

  do
  {
    int rc = recv( s, (char*) buf, size, 0 );
    if ( rc < 0 )
    {
      SetStreamState( ios::failbit );
      lastError = WSAGetLastError();
      return (false);
    }
    else if (rc == 0)
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
  while (size != 0); 

  return (true);
}
    
bool
Win32Socket::Write( void const* buf, size_t size )
{ 
  if ( state != SS_OPEN )
  {
    SetStreamState( ios::failbit );
    lastError = EC_NOT_OPENED;
    return (false);
  }
  
  do
  {
    int rc = send( s, (char*) buf, size, 0 );
    if ( rc < 0 )
    {
      SetStreamState( ios::failbit );
      lastError = WSAGetLastError();
      return (false);
    }
    else if (rc == 0)
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
  while (size != 0); 

  return (true);
}
    
bool
Win32Socket::IsOk()
{
  return (lastError == EC_OK);
}

string
Win32Socket::GetErrorText()
{
  string msg; 
  int    len;
  char   msgbuf[256];

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
      msg = "Connection broken";
      break; 
    case EC_INVALID_ACCESS_MODE:
      msg = "Invalid access mode";
      break;
    case EC_MESSAGE_TRUNCATED:
      msg = "Sent message truncated";
      break;
    default: 
      len = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                           lastError, 0, msgbuf, 256, NULL);
      if ( len == 0 ) 
      {
        sprintf( msgbuf, "Unknown error code %u", lastError );
      }
      msg = msgbuf;
      break;
  }
  return (msg);
}

string
Win32Socket::GetSocketAddress() const
{
  assert( this );
  struct sockaddr_in addr;
  int bytes = sizeof( sockaddr );
  if ( ::getsockname( s, (sockaddr*) &addr, &bytes )  == 0 )
  {
    return (inet_ntoa( addr.sin_addr ));
  }
  else
  {
    return ("<unknown>");
  }
}

string
Win32Socket::GetPeerAddress() const
{
  assert( this );
  struct sockaddr_in addr;
  int bytes = sizeof( sockaddr );
  if ( ::getpeername( s, (sockaddr*) &addr, &bytes ) == 0 )
  {
    return (inet_ntoa( addr.sin_addr ));
  }
  else
  {
    return ("<unknown>");
  }
}

Socket*
Win32Socket::Accept()
{
  if ( state != SS_OPEN )
  {
    lastError = EC_NOT_OPENED;
    return (NULL);
  }

  SOCKET newSock = ::accept( s, NULL, NULL );

  if ( newSock == INVALID_SOCKET )
  {
    lastError = WSAGetLastError();
    return (NULL);
  }
  else
  {
    static struct linger l = { 1, LINGER_TIME };
    if ( setsockopt( newSock, SOL_SOCKET, SO_LINGER, (char*) &l, sizeof(l) ) != 0 )
    {
      lastError = EC_INVALID_ACCESS_MODE;
      closesocket( newSock );
      return (NULL); 
    }
    int enabled = 1;
    if ( setsockopt( newSock, IPPROTO_TCP, TCP_NODELAY,
                     (char*) &enabled, sizeof(enabled) ) != 0 )
    {
      lastError = WSAGetLastError();
      closesocket( newSock );
      return (NULL);
    }
    lastError = EC_OK;
    return (new Win32Socket( newSock ));
  }
}

bool
Win32Socket::CancelAccept() 
{
  bool result = Close();
  // Wakeup listener
  delete Socket::Connect( hostAddress, hostPort, SockGlobalDomain, 1, 0 );
  return (result);
}

bool
Win32Socket::Close()
{
  if ( state != SS_CLOSE )
  {
    state = SS_CLOSE;
    if ( closesocket(s) == 0 )
    {
      SetStreamState( ios::eofbit );
      lastError = EC_OK;
      return (true);
    }
    else
    {
      SetStreamState( ios::failbit );
      lastError = WSAGetLastError();
      return (false);
    }
  }
  return (true);
}

bool
Win32Socket::ShutDown()
{
  if ( state == SS_OPEN )
  {
    state = SS_SHUTDOWN;
    int rc = ::shutdown( s, 2 );
    if ( rc != 0 )
    {
      SetStreamState( ios::failbit );
      lastError = WSAGetLastError();
      return (false);
    } 
  } 
  SetStreamState( ios::eofbit );
  lastError = EC_OK;
  return (true);
}

void
Win32Socket::SetStreamState( ios::iostate newState )
{
  if ( ioSocketStream != 0 )
  {
    ioSocketStream->setstate( newState );
  }
}

SocketDescriptor
Win32Socket::GetDescriptor()
{
  if ( state != SS_OPEN )
  {
    lastError = EC_NOT_OPENED;
    return (INVALID_SOCKET);
  }
  return (s);
}

Socket*
Socket::CreateClient( const SocketDescriptor sd )
{
  Win32Socket* sock = new Win32Socket( sd );
  return (sock);
}

int
Socket::GetIP( const string& address )
{
  int ip = inet_addr( address.c_str() );
  if ( (unsigned) ip == INADDR_NONE )
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
Socket::CreateLocal( const string& address, const int listenQueueSize )
{
  LocalWin32Socket* sock = new LocalWin32Socket( address );
  sock->Open( listenQueueSize );
  return (sock);
}

Socket*
Socket::CreateGlobal( const string& address, const string& port, const int listenQueueSize )
{
  Win32Socket* sock = new Win32Socket( address, port );
  sock->Open( listenQueueSize, SOCK_STREAM );
  return (sock);
}

Socket*
Socket::Connect( const string& address, const string& port,
                 const SocketDomain domain,
                 const int maxAttempts, const time_t timeout )
{
  if ( domain == SockLocalDomain ||
      (domain == SockAnyDomain   &&
       (port.length() == 0 || address == "localhost")) )
  {
    LocalWin32Socket* s = new LocalWin32Socket( address );
    s->Connect( maxAttempts, timeout );
    return (s);
  }
  else
  {
    Win32Socket* s = new Win32Socket( address, port );
    s->Connect( maxAttempts, timeout ); 
    return (s);
  }  
}

string
GetProcessName()
{ 
  static char name[MAX_HOST_NAME+8];
  gethostname( name, MAX_HOST_NAME );
  sprintf( name + strlen( name ), ":%x", (unsigned int) GetCurrentProcessId() );
  return (name);
}

/*
1.1 Local Windows Sockets

*/

void
LocalWin32Socket::SetStreamState( ios::iostate newState )
{
  if ( ioSocketStream != 0 )
  {
    ioSocketStream->setstate( newState );
  }
}

int
LocalWin32Socket::Read( void* buf, size_t minSize, size_t maxSize,
                        time_t timeout )
{
  time_t start = 0;
  char* dst = (char*) buf;
  size_t size = 0;
  lastError = EC_OK;
  if ( timeout != WAIT_FOREVER )
  {
    start = time( NULL );
    timeout *= 1000; // convert seconds to miliseconds
  }
  while ( size < minSize && state == SS_OPEN )
  {
    recvBuffer->recvWaitFlag = true;
    size_t begin = recvBuffer->dataBeg;
    size_t end = recvBuffer->dataEnd;
    size_t recvSize = (begin <= end) ? end - begin : sizeof(recvBuffer->dataBuf) - begin;
    if (recvSize > 0)
    {
      recvBuffer->recvWaitFlag = false;
      if ( recvSize >= maxSize )
      {
        memcpy( dst, &recvBuffer->dataBuf[begin], maxSize );
        begin += maxSize;
        size += maxSize;
      }
      else
      {
        memcpy( dst, &recvBuffer->dataBuf[begin], recvSize );
        begin += recvSize;
        dst += recvSize;
        size += recvSize;
        maxSize -= recvSize;
      } 
      recvBuffer->dataBeg = (begin == sizeof(recvBuffer->dataBuf)) ? 0 : begin;
      if (recvBuffer->sendWaitFlag)
      {
        SetEvent(signalHandle[RTR]);
      }        
    }
    else
    {
      HANDLE h[2];
      h[0] = signalHandle[RD];
      h[1] = mutexHandle;
      int rc = WaitForMultipleObjects( 2, h, false, timeout );
      recvBuffer->recvWaitFlag = false;
      if ( rc != WAIT_OBJECT_0 )
      {
        if ( rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1)
        {
          SetStreamState( ios::failbit | ios::eofbit );
          lastError = EC_BROKEN_PIPE;
          ReleaseMutex(mutexHandle);
        }
        else if ( rc == WAIT_TIMEOUT )
        {
          return (size);
        }
        else
        {
          SetStreamState( ios::failbit );
          lastError = GetLastError();
        }
        return (-1);
      }
      if ( timeout != WAIT_FOREVER )
      {
        time_t now = time( NULL );
        timeout = timeout >= (now - start)*1000 ? timeout - (now - start)*1000 : 0;
      }
    }
  }            
  return (size < minSize ? -1 : (int) size);
}

bool
LocalWin32Socket::Read( void* buf, size_t size )
{
  char* dst = (char*) buf;
  lastError = EC_OK;
  while (size > 0 && state == SS_OPEN)
  {
    recvBuffer->recvWaitFlag = true;
    size_t begin = recvBuffer->dataBeg;
    size_t end = recvBuffer->dataEnd;
    size_t recvSize = (begin <= end) ? end - begin : sizeof(recvBuffer->dataBuf) - begin;
    if (recvSize > 0)
    {
      recvBuffer->recvWaitFlag = false;
      if (recvSize >= size)
      {
        memcpy( dst, &recvBuffer->dataBuf[begin], size );
        begin += size;
        size = 0;
      }
      else
      {
        memcpy( dst, &recvBuffer->dataBuf[begin], recvSize );
        begin += recvSize;
        dst += recvSize;
        size -= recvSize;
      } 
      recvBuffer->dataBeg = (begin == sizeof(recvBuffer->dataBuf)) ? 0 : begin;
      if ( recvBuffer->sendWaitFlag )
      {
        SetEvent( signalHandle[RTR] );
      }
    }
    else
    {
      HANDLE h[2];
      h[0] = signalHandle[RD];
      h[1] = mutexHandle;
      int rc = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
      recvBuffer->recvWaitFlag = false;
      if ( rc != WAIT_OBJECT_0 )
      {
        if ( rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1 )
        {
          SetStreamState( ios::failbit | ios::eofbit );
          lastError = EC_BROKEN_PIPE;
          ReleaseMutex( mutexHandle );
        }
        else
        {
          SetStreamState( ios::failbit );
          lastError = GetLastError();
        }
        return (false);
      }
    }
  }            
  return (size == 0);
}

bool
LocalWin32Socket::Write( const void* buf, size_t size )
{
  char* src = (char*) buf;
  lastError = EC_OK;
  while (size > 0 && state == SS_OPEN)
  {
    sendBuffer->sendWaitFlag = true;
    size_t begin = sendBuffer->dataBeg;
    size_t end = sendBuffer->dataEnd;
    size_t snd_size = (begin <= end) ? 
                      sizeof(sendBuffer->dataBuf) - end - (begin == 0) : begin - end - 1;
    if ( snd_size > 0 )
    {
      sendBuffer->sendWaitFlag = false;
      if ( snd_size >= size )
      {
        memcpy( &sendBuffer->dataBuf[end], src, size );
        end += size;
        size = 0;
      }
      else
      {
        memcpy( &sendBuffer->dataBuf[end], src, snd_size );
        end += snd_size;
        src += snd_size;
        size -= snd_size;
      } 
      sendBuffer->dataEnd = (end == sizeof(sendBuffer->dataBuf)) ? 0 : end;
      if ( sendBuffer->recvWaitFlag )
      {
        SetEvent( signalHandle[TD] );
      }
    }
    else
    {
      HANDLE h[2];
      h[0] = signalHandle[RTT];
      h[1] = mutexHandle;
      int rc = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
      recvBuffer->sendWaitFlag = false;
      if ( rc != WAIT_OBJECT_0 )
      {
        if ( rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1 )
        {
          SetStreamState( ios::failbit | ios::eofbit );
          lastError = EC_BROKEN_PIPE;
          ReleaseMutex( mutexHandle );
        }
        else
        {
          SetStreamState( ios::failbit );
          lastError = GetLastError();
        }    
        return (false);
      }
    }
  }                
  return (size == 0);
}

#define MAX_ADDRESS_LEN 64

LocalWin32Socket::LocalWin32Socket( const string& address )
{
  localName = address;
  lastError = EC_NOT_OPENED;
  mutexHandle = NULL;
}
 
bool
LocalWin32Socket::Open( int )
{
  char buf[MAX_ADDRESS_LEN];    
  int  i;

  for ( i = RD; i <= RTT; i++ )
  {  
    sprintf( buf, "%s.%c", localName.c_str(), i + '0' );
    signalHandle[i] = CreateEvent( NULL, false, false, buf );
    if ( GetLastError() == ERROR_ALREADY_EXISTS )
    {
      WaitForSingleObject( signalHandle[i], 0 );
    }
    if ( !signalHandle[i] )
    {
      lastError = GetLastError();
      while (--i >= 0)
      {
        CloseHandle( signalHandle[i] );
      }
      return (false);
    }    
  }
  sprintf( buf, "%s.shr", localName.c_str() );
  bufferHandle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                              0, sizeof(LocalSocketBuffer)*2, buf );
  if ( !bufferHandle )
  {
    lastError = GetLastError();
    for ( i = RD; i <= RTT; i++ )
    {  
      CloseHandle( signalHandle[i] );
    }
    return (false);
  }
  recvBuffer = (LocalSocketBuffer*) MapViewOfFile( bufferHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
  if ( !recvBuffer )
  {
    lastError = GetLastError();
    CloseHandle( bufferHandle );
    for ( i = RD; i <= RTT; i++ )
    {
      CloseHandle( signalHandle[i] );
    }
    return (false);
  }    
  sendBuffer = recvBuffer+1;
  recvBuffer->dataBeg = recvBuffer->dataEnd = 0;
  sendBuffer->dataBeg = sendBuffer->dataEnd = 0;     
  lastError = EC_OK;
  state = SS_OPEN;
  return (true);
}

LocalWin32Socket::LocalWin32Socket()
{
  int i;
  bufferHandle = NULL;
  mutexHandle = NULL; 
  localName = "";

  for ( i = RD; i <= RTT; i++ )
  {
    signalHandle[i] = CreateEvent( NULL, false, false, NULL );
    if ( !signalHandle[i] )
    {
      lastError = GetLastError();
      while (--i >= 0)
      {
        CloseHandle( signalHandle[i] );
      }
      return;
    }    
  }
  // create anonymous shared memory section
  bufferHandle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                              0, sizeof(LocalSocketBuffer)*2, NULL );
  if (!bufferHandle)
  {
    lastError = GetLastError();
    for (i = RD; i <= RTT; i++)
    {
      CloseHandle( signalHandle[i] );
    }
    return;
  }
  recvBuffer = (LocalSocketBuffer*) MapViewOfFile( bufferHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
  if ( !recvBuffer )
  {
    lastError = GetLastError();
    CloseHandle( bufferHandle );
    for ( i = RD; i <= RTT; i++ )
    {
      CloseHandle( signalHandle[i] );
    }
    bufferHandle = NULL;
    return;
  }    
  sendBuffer = recvBuffer+1;
  recvBuffer->dataBeg = recvBuffer->dataEnd = 0;
  sendBuffer->dataBeg = sendBuffer->dataEnd = 0;     
  ioSocketBuffer = new SocketBuffer( *this );
  ioSocketStream = new iostream( ioSocketBuffer );
  ioSocketStream->clear();
  lastError = EC_OK;
  state = SS_OPEN;
}

LocalWin32Socket::~LocalWin32Socket()
{
  Close();
  if ( ioSocketStream != 0 )
  {
    delete ioSocketStream;
  }
  if ( ioSocketBuffer != 0 )
  {
    delete ioSocketBuffer;
  }
}    

Socket*
LocalWin32Socket::Accept()
{   
  HANDLE h[2];

  if ( state != SS_OPEN )
  {
    return (NULL);
  }
          
  ConnectData* cdp = (ConnectData*) sendBuffer->dataBuf;
  cdp->processId = GetCurrentProcessId();
  cdp->mutexHandle = watchDogMutex;
  while (true)
  {
    SetEvent( signalHandle[RTR] );
    int rc = WaitForSingleObject( signalHandle[RD], ACCEPT_TIMEOUT );
    if ( rc == WAIT_OBJECT_0 )
    {
      if ( state != SS_OPEN )
      {
        lastError = EC_NOT_OPENED;
        return (NULL);
      }
      lastError = EC_OK;
      break;
    }
    else if ( rc != WAIT_TIMEOUT )
    {
      lastError = GetLastError();
      return (NULL);
    }
  }
  LocalWin32Socket* sock = new LocalWin32Socket();
  sock->mutexHandle = ((ConnectData*) recvBuffer->dataBuf)->mutexHandle;
  AcceptData* adp = (AcceptData*) sendBuffer->dataBuf;
  adp->bufferHandle = sock->bufferHandle;
  for ( int i = RD; i <= RTT; i++ )
  {
    adp->signalHandle[(i + TD - RD) & RTT] = sock->signalHandle[i];
  }
  SetEvent( signalHandle[TD] );
  h[0] = signalHandle[RD];
  h[1] = sock->mutexHandle;
  int rc = WaitForMultipleObjects( 2, h, FALSE, INFINITE );
  if ( rc != WAIT_OBJECT_0 )
  {
    if ( rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1 )
    {
      lastError = EC_BROKEN_PIPE;
      ReleaseMutex( mutexHandle );
    }
    else
    {
      lastError = GetLastError();
    }    
    delete sock;
    return (NULL);
  }  
  return (sock);
}

SocketDescriptor
LocalWin32Socket::GetDescriptor()
{
  lastError = EC_INVALID_ACCESS_MODE;
  return (INVALID_SOCKET);
}

bool
LocalWin32Socket::CancelAccept()
{
  state = SS_SHUTDOWN;
  SetEvent( signalHandle[RD] );
  SetEvent( signalHandle[RTT] );
  return (true);
}  

bool
LocalWin32Socket::IsOk()
{
  return (!lastError);
}

bool
LocalWin32Socket::Close()
{
  if ( state != SS_CLOSE )
  {
    state = SS_CLOSE;
    if (mutexHandle)
    {
      CloseHandle( mutexHandle );
    }
    for ( int i = RD; i <= RTT; i++ )
    { 
      CloseHandle( signalHandle[i] );
    }
    UnmapViewOfFile( recvBuffer < sendBuffer ? recvBuffer : sendBuffer );
    CloseHandle( bufferHandle );
    SetStreamState( ios::eofbit );
    lastError = EC_NOT_OPENED;
  }
  return (true);
}

string
LocalWin32Socket::GetSocketAddress() const
{
  return (localName);
}

string
LocalWin32Socket::GetPeerAddress() const
{
  return (localName);
}

string
LocalWin32Socket::GetErrorText()
{
  string msg;
  int    len;
  char   msgbuf[256];

  switch (lastError)
  {
    case EC_OK:
      msg = "Ok";
      break;
    case EC_NOT_OPENED:
      msg = "Socket not opened";
      break;
    case EC_BROKEN_PIPE:
      msg = "Connection is broken";
      break;
    case EC_TIMEOUT_EXPIRED:
      msg = "Connection timeout expired";
      break;
    case EC_INVALID_ACCESS_MODE:
      msg = "Invalid access mode";
      break;
    default:     
      len = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                           lastError, 0, msgbuf, 256, NULL);
      if ( len == 0 ) 
      {
        sprintf( msgbuf, "Unknown error code %u", lastError );
      }
      msg = msgbuf;
      break;
  }
  return (msg);
}

bool
LocalWin32Socket::ShutDown()
{
  if ( state == SS_OPEN)
  {
    state = SS_SHUTDOWN;
    SetEvent( signalHandle[RD] );
    SetEvent( signalHandle[RTT] );
    SetStreamState( ios::eofbit );
  }
  return (true);
}

bool
LocalWin32Socket::Connect( int maxAttempts, time_t timeout )
{
  char buf[MAX_ADDRESS_LEN];
  int  rc, i, error_code;
  HANDLE h[2];

  for ( i = RD; i <= RTT; i++ )
  {
    sprintf( buf, "%s.%c", localName.c_str(), ((i + TD - RD) & RTT) + '0' );
    signalHandle[i] = CreateEvent( NULL, false, false, buf );
    if ( !signalHandle[i] )
    {
      lastError = GetLastError();
      while (--i >= 0)
      {
        CloseHandle( signalHandle[i] );
      }
      return (false);
    }    
  }
  sprintf( buf, "%s.shr", localName.c_str() );
  bufferHandle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                              0, sizeof(LocalSocketBuffer)*2, buf );
  if (!bufferHandle)
  {
    lastError = GetLastError();
    for (i = RD; i <= RTT; i++)
    {
      CloseHandle( signalHandle[i] );
    }
    return (false);
  }
  sendBuffer = (LocalSocketBuffer*) MapViewOfFile( bufferHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
  if ( !sendBuffer )
  {
    lastError = GetLastError();
    for (i = RD; i <= RTT; i++)
    {  
      CloseHandle( signalHandle[i] );
    }
    CloseHandle( bufferHandle );
    return (false);
  }
  recvBuffer = sendBuffer+1;
  state = SS_SHUTDOWN;
  mutexHandle = NULL;

  rc = WaitForSingleObject( signalHandle[RTT], timeout*maxAttempts*MILLISECOND );
  if ( rc != WAIT_OBJECT_0 )
  {
    error_code = rc == WAIT_TIMEOUT ? EC_TIMEOUT_EXPIRED : GetLastError();
    Close();
    lastError = error_code;
    return (false);
  }
  ConnectData* cdp = (ConnectData*) recvBuffer->dataBuf;
  HANDLE hServer = OpenProcess( STANDARD_RIGHTS_REQUIRED | PROCESS_DUP_HANDLE,
                                FALSE, cdp->processId );
  if ( !hServer )
  {
    error_code = GetLastError();
    Close();
    lastError = error_code;
    return (false);
  }
  HANDLE hSelf = GetCurrentProcess();
  if ( !DuplicateHandle( hServer, cdp->mutexHandle, hSelf, &mutexHandle, 
                         0, FALSE, DUPLICATE_SAME_ACCESS ) ||
       !DuplicateHandle( hSelf, watchDogMutex, hServer, 
                         &((ConnectData*) sendBuffer->dataBuf)->mutexHandle, 
                         0, FALSE, DUPLICATE_SAME_ACCESS ) )
  {
    error_code = GetLastError();
    CloseHandle( hServer );
    Close();
    lastError = error_code;
    return (false);
  }
  SetEvent( signalHandle[TD] );
  h[0] = signalHandle[RD];
  h[1] = mutexHandle;
  rc = WaitForMultipleObjects( 2, h, FALSE, INFINITE );

  if ( rc != WAIT_OBJECT_0 )
  {
    if ( rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1 )
    {
      error_code = EC_BROKEN_PIPE;
      ReleaseMutex( mutexHandle );
    }
    else
    {
      error_code = GetLastError();
    }
    CloseHandle( hServer );
    Close();
    lastError = error_code;
    return (false);
  }
  AcceptData ad = *(AcceptData*) recvBuffer->dataBuf;

  SetEvent( signalHandle[TD] );
  for ( i = RD; i <= RTT; i++ )
  {
    CloseHandle( signalHandle[i] );
  }
  UnmapViewOfFile( sendBuffer );
  CloseHandle( bufferHandle );
  bufferHandle = NULL;

  if ( !DuplicateHandle( hServer, ad.bufferHandle, hSelf, &bufferHandle, 
                         0, FALSE, DUPLICATE_SAME_ACCESS ) )
  {
    lastError = GetLastError();
    CloseHandle( hServer );
    CloseHandle( mutexHandle ); 
    return (false);
  }
  else
  {
    for (i = RD; i <= RTT; i++)
    {
      if (!DuplicateHandle( hServer, ad.signalHandle[i], 
                            hSelf, &signalHandle[i], 
                            0, FALSE, DUPLICATE_SAME_ACCESS ) )
      {
        lastError = GetLastError();
        CloseHandle( hServer );
        CloseHandle( bufferHandle );
        CloseHandle( mutexHandle );
        while (--i >= 0)
        {
          CloseHandle( signalHandle[1] );
        }
        return (false);
      }
    }
  }
  CloseHandle( hServer );

  sendBuffer = (LocalSocketBuffer*) MapViewOfFile( bufferHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
  if ( !sendBuffer )
  {
    lastError = GetLastError();
    CloseHandle( bufferHandle );
    CloseHandle( mutexHandle );
    for ( i = RD; i <= RTT; i++ )
    {
      CloseHandle( signalHandle[i] );
    }
    return (false);
  }
  recvBuffer = sendBuffer+1;
  ioSocketBuffer = new SocketBuffer( *this );
  ioSocketStream = new iostream( ioSocketBuffer );
  ioSocketStream->clear();
  lastError = EC_OK;
  state = SS_OPEN; 
  return (true);
}

// --- End of source ---

