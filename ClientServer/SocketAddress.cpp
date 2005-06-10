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

*/
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
#include <cassert>

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

SocketAddress::SocketAddress( const string& ipAddr, uint16_t portNo )
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

