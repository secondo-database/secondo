using namespace std;

#include "SecondoConfig.h"
#ifdef SECONDO_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "SocketIO.h"
#include <cstdio>

// --- SocketAddress ---

SocketAddress::SocketAddress()
{
  u.sock_inet.sin_family      = AF_INET;
  u.sock_inet.sin_port        = 0;
  u.sock_inet.sin_addr.s_addr = INADDR_ANY;
}

SocketAddress::SocketAddress( const SocketAddress& sockAddr )
{
  SocketAddress::operator=( sockAddr );
}

SocketAddress::SocketAddress( const string& ipAddr, uint16_t portNo = 0 )
{
  SetAddress( ipAddr, portNo );
}

SocketAddress::~SocketAddress()
{
  assert( this );
}

SocketAddress& SocketAddress::operator=( const SocketAddress& sockAddr )
{
  assert( this );
  assert( &sockAddr );
  if ( this != &sockAddr )
  {
    u.sock_inet = sockAddr.u.sock_inet;
  }
  return (*this);
}

bool SocketAddress::operator==( const SocketAddress& sockAddr ) const
{
  assert( this );
  assert( &sockAddr );
  bool result = true;
  if ( this != &sockAddr )
  {
    result = ((u.sock_inet.sin_family      == sockAddr.u.sock_inet.sin_family) &&
              (u.sock_inet.sin_port        == sockAddr.u.sock_inet.sin_port) &&
              (u.sock_inet.sin_addr.s_addr == sockAddr.u.sock_inet.sin_addr.s_addr));
  }
  return (result);
}

void SocketAddress::SetAddress( const string& ipAddr,
                                uint16_t portNo /* = 0 */ )
{
  assert( this );
  assert( &ipAddr );
  u.sock_inet.sin_family = AF_INET;
  u.sock_inet.sin_port   = htons( portNo );
  u.sock_inet.sin_addr.s_addr = inet_addr( ipAddr.c_str() );
//  inet_aton( ipAddr.c_str(), &u.sock_inet.sin_addr.s_addr );
  // Zero the sin_zero array.
  for ( unsigned int i = 0; i < nelems( u.sock_inet.sin_zero ); i++ )
    u.sock_inet.sin_zero[i] = 0;
}

void SocketAddress::SetAddress( const string& ipAddr, const string& portNo )
{
  assert( this );
  assert( &ipAddr );
  u.sock_inet.sin_family = AF_INET;
  unsigned short port;
  sscanf( portNo.c_str(), "%hd", &port );
  u.sock_inet.sin_port   = htons( port );
  u.sock_inet.sin_addr.s_addr = inet_addr( ipAddr.c_str() );
//  inet_aton( ipAddr.c_str(), &u.sock_inet.sin_addr.s_addr );
  // Zero the sin_zero array.
  for ( unsigned int i = 0; i < nelems( u.sock_inet.sin_zero ); i++ )
    u.sock_inet.sin_zero[i] = 0;
}

string SocketAddress::GetSocketString() const
{
  assert( this );
  assert( u.sock_inet.sin_family == AF_INET );
  char buffer[20];
  sprintf( buffer, ":%d", GetPort() );
  string SocketStr = GetIPAddress() + ":" + buffer;
  return (SocketStr);
}

string SocketAddress::GetIPAddress() const
{
  assert( this );
  return (inet_ntoa( u.sock_inet.sin_addr ));
}

uint16_t SocketAddress::GetPort() const
{
  assert( this );
  return (ntohs( u.sock_inet.sin_port ));
}

