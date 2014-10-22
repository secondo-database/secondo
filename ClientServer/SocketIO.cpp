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

1 SocketIO

February 2002 Ulrich Telle

June 2002 Ulrich Telle Corrected error in ~SocketBuffer::underflow~

1.1 Overview

This module provides several classes for portable network communication
through sockets.

*/


#include "SecondoConfig.h"
#include "SocketIO.h"
#include <string>
#include <sstream>

#ifdef SECONDO_WIN32
#include "Win32Socket.cpp"
#else
#include "UnixSocket.cpp"
#endif

using namespace std;

iostream&
Socket::GetSocketStream()
{
  return *ioSocketStream;
}

// --- SocketBuffer ---

#ifndef BUFSIZ
#  define BUFSIZ 1024
#endif

SocketBuffer::SocketBuffer( Socket& socket )
  : socketHandle( &socket ), bufferSize( BUFSIZ )
{
  inBuffer  = new char[BUFSIZ];
  outBuffer = new char[BUFSIZ];
  setg( inBuffer, inBuffer + BUFSIZ, inBuffer + BUFSIZ );
  setp( outBuffer, outBuffer + BUFSIZ );
}

SocketBuffer::~SocketBuffer()
{
  overflow( EOF ); // flush write buffer
  if ( inBuffer != 0 )
    delete [] inBuffer;
  if ( outBuffer )
    delete [] outBuffer;
}

int
SocketBuffer::sync()
{
  if ( pptr() && pbase() < pptr() && pptr() <= epptr() )
  {
    if ( !socketHandle->Write( pbase(), pptr() - pbase() ) )
    {
      return (EOF);
    }
    setp( outBuffer, outBuffer + bufferSize );
  }
  return (0);
}

int
SocketBuffer::overflow( int ch )
{
  if ( pbase() == 0 )
  {
    return (EOF);
  }
  if ( ch == EOF )
  {
    return (sync());
  }
  if ( pptr() == epptr() )
  {
    sync();
  }
  *pptr() = (char) ch;
  pbump( 1 );
  return (ch);
}

streamsize
SocketBuffer::xsputn( const char* s, streamsize n )
{
  int wval = epptr() - pptr();
  if ( n <= wval )
  {
    memcpy( pptr(), s, n * sizeof(char) );
    pbump(n);
    return (n);
  }
  memcpy( pptr(), s, wval * sizeof(char) );
  pbump( wval );
  if ( overflow() != EOF )
  {
    return (wval + xsputn ( s + wval, n - wval ));
  }
  return (wval);
}


int
SocketBuffer::underflow()
{
  if ( gptr () == 0 )
  {
    return (EOF); // input stream has been disabled
  }
  if ( gptr () < egptr () )
  {
    // make return value unsigned to be different from EOF
    return ((unsigned char) *gptr());
  }
  int rlen = socketHandle->Read( inBuffer, 1, bufferSize );
  if ( rlen <= 0 )
  {
    return (EOF);
  }
  setg( eback(), eback(), eback() + rlen );
  return ((unsigned char) *gptr());
}

int
SocketBuffer::uflow()
{
  int ret = underflow();
  if ( ret == EOF )
  {
    return (EOF);
  }
  gbump( 1 );
  return (ret);
}


streamsize
SocketBuffer::xsgetn( char* s, streamsize n )
{
  int rval = (gptr() && gptr() < egptr()) ? egptr() - gptr() : 0;
  if (rval >= n)
  {
    memcpy( s, gptr(), n * sizeof(char) );
    gbump( n );
    return (n);
  }
  memcpy( s, gptr(), rval * sizeof(char) );
  gbump( rval );
  if ( underflow() != EOF )
  {
    return (rval + xsgetn( s + rval, n - rval ));
  }
  return (rval);
}

int
SocketBuffer::pbackfail( int ch )
{
  return (EOF);
}

/*
1.2 UDP sockets

1.2.1 Class ~UDPaddress~

*/

// retrieve IP address, port number and IP-version of a sender
// returns false and empty strings on unknown protocol type or myAddrInfo NULL
bool UDPaddress::updateMemberVariables(const struct addrinfo *argAddrInfo)
{__UDP_ENTER__
  ostringstream tmp;

  if(!(argAddrInfo) || !(argAddrInfo->ai_addr)){
    __UDP_MSG("(!(argAddrInfo) || !(argAddrInfo->ai_addr). FAILED!")
    myOk = false;
    myIP = "";
    myPort = "";
    myCanonname = "";
    myFamily = AF_UNSPEC;
    myAddr = 0;
    myAddrlen = 0;
    __UDP_EXIT__
    return false;
  }
  __UDP_MSG("Called with *argAddrInfo = " << *argAddrInfo)

  struct sockaddr_storage sa;
  struct sockaddr_storage *sas = &sa;
  memset(&sa, 0, sizeof sa);
  unsigned int mini = min(sizeof(struct sockaddr_storage), 
                         sizeof(struct sockaddr));
  memcpy(&sa, argAddrInfo->ai_addr, mini);

  __UDP_MSG(" sa = " << sa);

  myFamily = sa.ss_family;

  if(myFamily == AF_INET){ // IPv4
    __UDP_MSG("sa.ss_family == AF_INET.")
    myAddrlen = sizeof (struct sockaddr_in);
    unsigned short int u_port = ((sockaddr_in*)sas)->sin_port;
    short h_port = ntohs(u_port);
    tmp << h_port;
    myPort = tmp.str();
#ifdef SECONDO_WIN32
    myIP = string(inet_ntoa(((sockaddr_in*)sas)->sin_addr));
#else
    char c_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,
              &(((sockaddr_in*)sas)->sin_addr.s_addr),
              c_ip,
              INET_ADDRSTRLEN);
    myIP = string(c_ip);
#endif
    __UDP_EXIT__
    return true;
  } else if(myFamily == AF_INET6){ // IPv6
    __UDP_MSG("sa.ss_family == AF_INET6.")
    myAddrlen = sizeof (struct sockaddr_in6);
    uint16_t u_port = ((sockaddr_in6*)sas)->sin6_port;
    uint16_t h_port = ntohs(u_port);
    tmp << h_port;
    myPort = tmp.str();
#ifdef SECONDO_WIN32
    cerr << "Cannot translate IPv6 addresses on Win32 system." << endl;
    myIP = string("");
    // on Vista and Server2008, it should work
#else
    char c_ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6,
              &(((sockaddr_in6*)sas)->sin6_addr.s6_addr),
              c_ip,
              INET6_ADDRSTRLEN);
    myIP = string(c_ip);
#endif
    __UDP_EXIT__
    return true;
  } else if(myFamily == AF_UNSPEC){
    __UDP_MSG("sa.ss_family == AF_UNSPEC.")
  } else {
    __UDP_MSG("sa.ss_family == ERROR.")
  }
  myOk = false;
  myErrorMsg = "Unknown protocol family.";
  __UDP_EXIT__
  return false;
}

bool UDPaddress::updateMemberVariables()
{__UDP_ENTER__
  ostringstream tmp;

  if(!(myOk && myAddr)){
    __UDP_MSG("Called with !(myOk && myAddr). FAILED!")
    __UDP_EXIT__
    return false;
  }
  __UDP_MSG("Called with myAddr = " << *myAddr)

  myFamily = myAddr->sa_family;

  if(myFamily == AF_INET){ // IPv4
    __UDP_MSG("myAddr->sa_famil == AF_INET.")
    myAddrlen = sizeof (struct sockaddr_in);
    unsigned short int u_port = ((sockaddr_in*)myAddr)->sin_port;
    short h_port = ntohs(u_port);
    tmp << h_port;
    myPort = tmp.str();
#ifdef SECONDO_WIN32
    myIP = string(inet_ntoa(((sockaddr_in*)myAddr)->sin_addr));
#else
    char c_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,
              &(((sockaddr_in*)myAddr)->sin_addr.s_addr),
              c_ip,
              INET_ADDRSTRLEN);
    myIP = string(c_ip);
#endif
    myCanonname = myIP;
    __UDP_EXIT__
    return true;
  } else if(myFamily == AF_INET6){ // IPv6
    __UDP_MSG("myAddr->sa_famil == AF_INET6.")
    myAddrlen = sizeof (struct sockaddr_in6);
    uint16_t u_port = ((sockaddr_in6*)myAddr)->sin6_port;
    uint16_t h_port = ntohs(u_port);
    tmp << h_port;
    myPort = tmp.str();
#ifdef SECONDO_WIN32
    cerr << "Cannot translate IPv6 addresses on Win32 system." << endl;
    myIP = string("");
    // on Vista and Server2008, it should work
#else
    char c_ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6,
              &(((sockaddr_in6*)myAddr)->sin6_addr.s6_addr),
              c_ip,
              INET6_ADDRSTRLEN);
    myIP = string(c_ip);
#endif
    myCanonname = myIP;
    __UDP_EXIT__
    return true;
  }else if(myFamily == AF_UNSPEC){
    __UDP_MSG("sa.ss_family == AF_UNSPEC.")
  } else {
    __UDP_MSG("sa.ss_family == ERROR.")
  }
  myOk = false;
  myErrorMsg = "Unknown protocol family.";
  myCanonname = "";
  myIP = "";
  myPort = "";
  myAddrlen = 0;
  __UDP_EXIT__
  return false;
}

UDPaddress::UDPaddress()
{__UDP_ENTER__
  myOk        = false;
  myErrorMsg  = "Uninitialized UDPaddress.";
  myFamily    = AF_UNSPEC;
  myAddrlen   = 0;
  myAddr      = 0;
  myIP        = "";
  myPort      = "";
  myCanonname = "";
  __UDP_EXIT__
}

UDPaddress::UDPaddress(const struct addrinfo *addr)
{__UDP_ENTER__
  if(!addr){
    __UDP_MSG("Calling with addr = NULL.")
    myOk        = false;
    myErrorMsg  = "Uninitialized UDPaddress.";
    myFamily    = AF_UNSPEC;
    myAddrlen   = 0;
    myAddr      = 0;
    myIP        = "";
    myPort      = "";
    myCanonname = "";
    __UDP_EXIT__
    return;
  }
  else{
    __UDP_MSG("Calling with addr = " << *addr)
  }
  struct addrinfo *myAddrInfo = 0;
  int resGetaddrinfo = getaddrinfo(NULL,
                                   NULL,
                                   addr,
                                   &myAddrInfo);
  if (resGetaddrinfo < 0) {
    myOk = false;
    myErrorMsg = getErrorCodeStr(resGetaddrinfo);
  } else {
    myOk = true;
    myErrorMsg = "";
    myFamily = myAddrInfo->ai_family;
    if(myFamily == AF_INET){
      __UDP_MSG("myAddrInfo->ai_family == AF_INET.")
      myAddr = (sockaddr*)(new struct sockaddr_in);
      memcpy(myAddr,myAddrInfo->ai_addr,sizeof(sockaddr_in));
    } else if(myFamily == AF_INET6){
      __UDP_MSG("myAddrInfo->ai_family == AF_INET6.")
      myAddr = (sockaddr*)(new struct sockaddr_in6);
      memcpy(myAddr,myAddrInfo->ai_addr,sizeof(sockaddr_in6));
    } else if(myFamily == AF_UNSPEC){
      __UDP_MSG("myAddrInfo->ai_family == AF_UNSPEC.")
          myOk = false;
      myErrorMsg = "Uninitialized UDPaddress.";
    } else { // Error
      __UDP_MSG("myAddrInfo->ai_family == ERROR.")
      myOk = false;
      myErrorMsg = "Illegal value for ai_family.";
    }
  }
  myOk = updateMemberVariables(myAddrInfo);
  if(myAddrInfo->ai_canonname){
    myCanonname = string(myAddrInfo->ai_canonname);
  } else {
    myCanonname = myIP;
  }
  if(myAddrInfo){
    freeaddrinfo(myAddrInfo);
  }
  __UDP_EXIT__
}

UDPaddress::UDPaddress(const struct sockaddr_storage *addr)
{__UDP_ENTER__
  if(!addr){
    __UDP_MSG("Calling with addr = NULL.")
    myOk        = false;
    myErrorMsg  = "Uninitialized UDPaddress.";
    myFamily    = AF_UNSPEC;
    myAddrlen   = 0;
    myAddr      = 0;
    myIP        = "";
    myPort      = "";
    myCanonname = "";
    __UDP_EXIT__
    return;
  }
  __UDP_MSG("Calling with addr =." << *addr)
  myOk = true;
  myFamily = addr->ss_family;
  myCanonname = "";
  myIP = "";
  myPort = "";
  myErrorMsg = "";
  myAddrlen = 0;
  myAddr = 0;
  if(myFamily == AF_INET){
    __UDP_MSG("addr->ss_family == AF_INET.")
    myAddrlen = sizeof (struct sockaddr_in);
    myAddr = (sockaddr*)(new struct sockaddr_in);
    memcpy(myAddr,addr,sizeof(sockaddr_in));
  } else if(myFamily == AF_INET6){
    __UDP_MSG("addr->ss_family == AF_INET6.")
    myAddrlen = sizeof (struct sockaddr_in6);
    myAddr = (sockaddr*)(new struct sockaddr_in6);
    memcpy(myAddr,addr,sizeof(sockaddr_in6));
  } else if(myFamily == AF_UNSPEC){
    __UDP_MSG("myAddrInfo->ai_family == AF_UNSPEC.")
    myOk = false;
    myErrorMsg = "Uninitialized UDPaddress.";
  } else { // Error
    __UDP_MSG("addr->ss_family == ERROR.")
    myOk = false;
    myErrorMsg = "Illegal value for ss_family.";
  }
  myOk = updateMemberVariables();
  myCanonname = myIP;
  __UDP_EXIT__
}

UDPaddress::UDPaddress(const struct sockaddr_in *addr)
{__UDP_ENTER__
  if(!addr){
    __UDP_MSG("Calling with addr = NULL.")
    myOk        = false;
    myErrorMsg  = "Uninitialized UDPaddress.";
    myFamily    = AF_UNSPEC;
    myAddrlen   = 0;
    myAddr      = 0;
    myIP        = "";
    myPort      = "";
    myCanonname = "";
    __UDP_EXIT__
    return;
  }
  __UDP_MSG("Calling with addr =." << *addr)
  myOk = true;
  myCanonname = "";
  myIP = "";
  myPort = "";
  myCanonname  = "";
  myErrorMsg = "";
  myAddrlen = 0;
  myAddr = 0;
  myFamily = addr->sin_family;
  if(myFamily == AF_INET){
    __UDP_MSG("addr->sin_family == AF_INET.")
    myAddrlen = sizeof (struct sockaddr_in);
    myAddr = (sockaddr*)(new struct sockaddr_in);
    memcpy(myAddr,addr,sizeof(sockaddr_in));
    myOk = updateMemberVariables();
    myCanonname = myIP;
  } else {
    __UDP_MSG("addr->sin_family == ERROR.")
    myOk = false;
    myErrorMsg = "Illegal value for sin_family.";
  }
  __UDP_EXIT__
}

UDPaddress::UDPaddress(const struct sockaddr_in6 *addr)
{__UDP_ENTER__
  if(!addr){
    __UDP_MSG("Calling with addr = NULL.")
    myOk        = false;
    myErrorMsg  = "Uninitialized UDPaddress.";
    myFamily    = AF_UNSPEC;
    myAddrlen   = 0;
    myAddr      = 0;
    myIP        = "";
    myPort      = "";
    myCanonname = "";
    return;
    __UDP_EXIT__
  }
  __UDP_MSG("Calling with addr =." << *addr)
  myOk = true;
  myCanonname = "";
  myIP = "";
  myPort = "";
  myCanonname  = "";
  myErrorMsg = "";
  myAddrlen = 0;
  myAddr = 0;
  myFamily = addr->sin6_family;
  if(myFamily == AF_INET6){
    __UDP_MSG("addr->sin6_family == AF_INET6.")
    myAddrlen = sizeof (struct sockaddr_in6);
    myAddr = (sockaddr*)(new struct sockaddr_in6);
    memcpy(myAddr,addr,sizeof(sockaddr_in6));
    myOk = updateMemberVariables();
    myCanonname = myIP;
  } else {
    __UDP_MSG("addr->sin6_family == ERROR.")
    myOk = false;
    myErrorMsg = "Illegal value for sin6_family.";
  }
  __UDP_EXIT__
}


UDPaddress::UDPaddress(const string ip,
                       const string port,
                       const short int IPver)
{__UDP_ENTER__
  __UDP_MSG("Calling  with IP = "<<ip<<", port = "<<port<<", IPver = "<<IPver);
  struct addrinfo lhints;
  struct addrinfo *myAddrInfo = 0;
  memset(&lhints, 0, sizeof lhints);   // make sure the struct is empty
  /*
  // Workaround for MacOS (memset() doesn't work there properly):
  unsigned char *ptr = static_cast<unsigned char*>(&lhints);
  for(int i = 0; i < sizeof lhints; i++) {
    ptr[i] = 0;
  }
  */

  lhints.ai_family   = IPver;
  lhints.ai_socktype = SOCK_DGRAM;     // UDP datagram type socket
  int resGetaddrinfo = 0;
  if(ip == ""){
    lhints.ai_flags  = AI_PASSIVE;     // fill in my IP for me
    resGetaddrinfo=getaddrinfo(NULL,
                               port.c_str(),
                               &lhints,
                               &myAddrInfo);
  }else{
    resGetaddrinfo=getaddrinfo(ip.c_str(),
                               port.c_str(),
                               &lhints,
                               &myAddrInfo);
  }
  if (resGetaddrinfo < 0) {
    myOk = false;
    myErrorMsg = getErrorCodeStr(resGetaddrinfo);
    __UDP_MSG("getaddrinfo() FAILED!")
  } else {
      __UDP_MSG("getaddrinfo() succeded. *myAddrInfo = " << *myAddrInfo)
    myOk = true;
    myErrorMsg = "";
    myFamily = myAddrInfo->ai_family;
    if(myFamily == AF_INET){
      __UDP_MSG("myAddrInfo->ai_family == AF_INET.")
      myAddr = (sockaddr*) (new struct sockaddr_in);
      memcpy(myAddr,myAddrInfo->ai_addr,sizeof(sockaddr_in));
    } else if(myFamily == AF_INET6){
      __UDP_MSG("myAddrInfo->ai_family == AF_INET6.")
      myAddr = (sockaddr*) (new struct sockaddr_in6);
      memcpy(myAddr,myAddrInfo->ai_addr,sizeof(sockaddr_in6));
    } else { // Error
      __UDP_MSG("myAddrInfo->ai_family == ERROR.")
      myOk = false;
      myErrorMsg = "Illegal value for ai_family.";
    }
    if(myAddrInfo->ai_canonname){
      __UDP_MSG("myAddrInfo->ai_canonname != NULL.")
      myCanonname = string(myAddrInfo->ai_canonname);
    } else {
      __UDP_MSG("myAddrInfo->ai_canonname == NULL.")
      myCanonname = "";
    }
    updateMemberVariables(myAddrInfo);
  }
  if(myAddrInfo){
     freeaddrinfo(myAddrInfo);
  }
  __UDP_EXIT__
}

UDPaddress::UDPaddress(const UDPaddress &addr)
{__UDP_ENTER__
  //copy constructor
  __UDP_MSG("Calling with addr = " << addr)
  myOk        = addr.myOk;
  myErrorMsg  = addr.myErrorMsg;
  myIP        = addr.myIP;
  myPort      = addr.myPort;
  myCanonname = addr.myCanonname;
  myFamily    = addr.myFamily;
  myAddrlen   = addr.myAddrlen;
  if(myFamily == AF_INET){
    __UDP_MSG("addr.myFamily == AF_INET.")
    myAddr = (sockaddr*)(new struct sockaddr_in);
    memcpy(myAddr,addr.myAddr,sizeof(sockaddr_in));
  } else if(myFamily == AF_INET6){
    __UDP_MSG("addr.myFamily == AF_INET6.")
    myAddr = (sockaddr*)(new struct sockaddr_in6);
    memcpy(myAddr,addr.myAddr,sizeof(sockaddr_in6));
  } else { // Error
    __UDP_MSG("addr.myFamily == ERROR.")
    myOk = false;
    myErrorMsg = "Illegal value for ai.family.";
  }
  __UDP_EXIT__
}

UDPaddress::~UDPaddress()
{__UDP_ENTER__
  if(myAddr) {
    if(myFamily == AF_INET){
      __UDP_MSG("myFamily == AF_INET.")
      delete (sockaddr_in*)(myAddr);
    } else if(myFamily == AF_INET6){
      __UDP_MSG("myFamily == AF_INET6.")
      delete (sockaddr_in6*)(myAddr);
    } else { // Error
      __UDP_MSG("myFamily == ERROR.")
      myOk = false;
      myErrorMsg = "Illegal value for myFamily.";
    }
    myAddr = 0;
  }
  __UDP_EXIT__
}

UDPaddress& UDPaddress::operator=(const UDPaddress& addr)
{__UDP_ENTER__
  if( this != &addr ){
    if(myAddr){
      __UDP_MSG("Called with *this = " << myAddr << ".")
      if(myFamily == AF_INET){
        delete (sockaddr_in*)(myAddr);
      } else if(myFamily == AF_INET6){
        delete (sockaddr_in6*)(myAddr);
      } else {
        cerr << __PRETTY_FUNCTION__
             << ": Unknown protocol family type: myFamily = "
             << myFamily << ". Cannot delete myAddr!" << endl;
      }
      myAddr = 0;
    } else {
      __UDP_MSG("Called with myAddr = NULL.")
      ;
    }
    __UDP_MSG("Called with addr = " << addr << ".")
    myOk = addr.myOk;
    myErrorMsg  = addr.myErrorMsg;
    myFamily    = addr.myFamily;
    myAddrlen   = addr.myAddrlen;
    myIP        = addr.myIP;
    myPort      = addr.myPort;
    myCanonname = addr.myCanonname;
    if(myFamily == AF_INET){
      __UDP_MSG("addr.myFamily == AF_INET (" << myFamily << ").")
      myAddr    = (sockaddr*)(new struct sockaddr_in);
      memcpy(myAddr,addr.myAddr,sizeof(sockaddr_in));
    } else if(myFamily == AF_INET6){
      __UDP_MSG("addr.myFamily == AF_INET6 (" << myFamily << ").")
      myAddr    = (sockaddr*)(new struct sockaddr_in6);
      memcpy(myAddr,addr.myAddr,sizeof(sockaddr_in6));
    } else if(myFamily == AF_UNSPEC){
      __UDP_MSG("addr.myFamily == AF_UNSPEC (" << myFamily << ").")
      myOk = false;
      myErrorMsg = "Uninitialized UDPaddress.";
      myAddr = 0;
    } else { // Error
      __UDP_MSG("addr.myFamily == ERROR (" << myFamily << ").")
      myOk = false;
      myErrorMsg = "Illegal value for myFamily.";
      myAddr = 0;
    }
  }
  __UDP_MSG("this = " << *this << ".")
  __UDP_EXIT__
  return (*this);
}

string UDPaddress::getErrorCodeStr(const int resGetaddrinfo)
{
#ifdef SECONDO_WIN32
  stringstream s;
  s << WSAGetLastError();
  return s.str();
#else
  return string(gai_strerror(resGetaddrinfo));
#endif
}

ostream& UDPaddress::Print(ostream &o) const
{
  o << "UDPaddress["
      << "\n\tmyOk=" << myOk
      << ",\n\tmyErrorMsg = " << myErrorMsg
      << ",\n\tmyFamily = " << myFamily
      << ",\n\tmyAddrLen = " << myAddrlen
      << ",\n\t*myAddr = ";
  if(myAddr){
    o << *myAddr;
  } else {
      o << "VOID";
  }
  o   << ",\n\tmyIP = " << myIP
      << ",\n\tmyPort = " << myPort
      << ",\n\tmyCanonname = " << myCanonname
    << "\n]";
  return o;
};

ostream& operator<<(ostream &o, const UDPaddress &u)
{
  return u.Print(o);
};

ostream& operator<<(ostream &o, const struct sockaddr &a)
{
  o << "addrinfo =~= ";
  if(a.sa_family == AF_INET){
    o << *(reinterpret_cast<const struct sockaddr_in*>(&a));
  } else if(a.sa_family == AF_INET6){
    o << *(reinterpret_cast<const struct sockaddr_in6*>(&a));
  } else {
    o << a.sa_family << " (unknown type " << a.sa_family << ")"
        << ", sa_data = ";
    for(int i=0; i<14; i++){
      o << ((unsigned char) a.sa_data[i]) << " ";
    }
  }
  o << " ]";
  return o;
}

ostream& operator<<(ostream &o, const struct sockaddr_in &a)
{
  o << "sockaddr_in["
    << "\n\tsin_family = " << a.sin_family;
  if(a.sin_family == AF_INET){
    o << " (IPv4)";
  } else if(a.sin_family == AF_INET6){
    o << " (IPv6 !)";
  } else {
    o << " (unknown type " << a.sin_family << ")";
  }
  o << ",\n\tsin_port = " << ntohs(a.sin_port);
#ifndef SECONDO_WIN32
  char tmpcstr[INET_ADDRSTRLEN];
  inet_ntop(a.sin_family, &a.sin_addr, tmpcstr, INET_ADDRSTRLEN);
  o << ",\n\tsin_addr = " << string(tmpcstr);
#endif
  o << "\n]";
  return o;
}

ostream& operator<<(ostream &o, const struct sockaddr_in6 &a)
{
  o << "sockaddr_in6[ "
    << "\n\tsin6_family = " << a.sin6_family;
  if(a.sin6_family == AF_INET){
    o << " (IPv4 !)";
  } else if(a.sin6_family == AF_INET6){
    o << " (IPv6)";
  } else {
    o << " (unknown type " << a.sin6_family << ")";
  }
  o << ",\n\tsin6_port = " << ntohs(a.sin6_port)
    << ",\n\tsin6_flowinfo = " << ntohl(a.sin6_flowinfo);
#ifndef SECONDO_WIN32
  char tmpcstr[INET6_ADDRSTRLEN];
  inet_ntop(a.sin6_family, &a.sin6_addr, tmpcstr, INET6_ADDRSTRLEN);
  o << ",\n\tsin6_addr = " << string(tmpcstr);
#endif
  o << ",\n\tsin6_scope_id = " << a.sin6_scope_id
    << " ]";
  return o;
}

ostream& operator<<(ostream &o, const struct sockaddr_storage &a)
{
  o << "sockaddr_storage =~= ";
  if(a.ss_family == AF_INET){
    o << *(reinterpret_cast<const struct sockaddr_in*>(&a));
  } else if(a.ss_family == AF_INET6){
    o << *(reinterpret_cast<const struct sockaddr_in6*>(&a));
  } else {
    o << a.ss_family << " (unknown type " << a.ss_family << ")";
  }
  return o;
}

ostream& operator<<(ostream &o, const struct addrinfo &a)
{
  o << "addrinfo["
      << "\n\tai_flags = " << a.ai_flags
      << ",\n\tai_family = ";
  switch(a.ai_family){
    case AF_INET: { o << "AF_INET"; break; }
    case AF_INET6: { o << "AF_INET6"; break; }
    case AF_UNSPEC: { o << "AF_UNSPEC"; break; }
    default: { o << "UNKNOWN (" << a.ai_family << ")"; }
  }
  o << ",\n\tai_socktype = ";
  switch(a.ai_socktype){
    case SOCK_STREAM: { o << "SOCK_STREAM"; break; }
    case SOCK_DGRAM: { o << "SOCK_DGRAM"; break; }
    default: { o << "UNKNOWN (" << a.ai_socktype << ")"; }
  }
  o << ",\n\t*ai_protocol = ";
  struct protoent* p = getprotobynumber(a.ai_protocol);
  if(p){
    o << *p;
  } else {
    o << "NULL !";
  }
  o << ",\n\tai_addrlen = " << a.ai_addrlen
    << ",\n\t*ai_addr = ";
  if(a.ai_addr){
    switch(a.ai_family){
      case AF_INET: {
        o << *(reinterpret_cast<const struct sockaddr_in*>(a.ai_addr));
        break;
      }
      case AF_INET6: {
        o << *(reinterpret_cast<const struct sockaddr_in6*>(a.ai_addr));
        break;
      }
      case AF_UNSPEC: { o << "AF_UNSPEC"; break; }
      default: { o << "UNKNOWN (" << a.ai_family << ")"; }
    }
  } else {
    o << " VOID !";
  }
  o << ",\n\tai_canonname = ";
  if(a.ai_canonname){
    o << string(a.ai_canonname);
  } else {
    o << "VOID !";
  }
  o << ",\n\t*ai_next = ";
  if(a.ai_next){
    o << *(a.ai_next);
  } else {
    o << "VOID !";
  }
  o << "\n]";
  return o;
}

ostream& operator<<(ostream &o, const struct protoent     &p)
{
  o << "protoent[ p_name = ";
  if(p.p_name){o << string(p.p_name);}else{o << "NULL";}
  o << ", p_proto = " << p.p_proto
    << " ]";
  return o;
}

/*
1.2.2 Class ~UDPsocket~

*/

UDPsocket::UDPsocket()
{__UDP_ENTER__
  ok = true;
  errorMsg  = "";
  status = UDPVOID;
  connected = false;
  mySocket = -1;
  myAddress      = UDPaddress();
  partnerAddress = UDPaddress();
__UDP_EXIT__
}

UDPsocket::UDPsocket(const UDPaddress &address)
{__UDP_ENTER__
  ok        = true;
  errorMsg  = "";
  status    = UDPVOID;
  connected = false;
  mySocket  = -1;
  myAddress = address;
  partnerAddress = UDPaddress();
  mySocket = socket(myAddress.getFamilyI(),
                    myAddress.getSockTypeI(),
                    0);                         // Auto-choose correct protocol
  if(mySocket < 0){ // Error
    ok = false;
    errorMsg = string(strerror(errno));
    __UDP_MSG("socket() failed.")
  } else {          // OK
    status    = UDPFRESH;
    __UDP_MSG("socket() succeeded.")
  }
  __UDP_EXIT__
}

UDPsocket::UDPsocket(const UDPsocket &sock)
{
  status = sock.status;
  connected = sock.connected;
  ok = sock.ok;
  mySocket = sock.mySocket;
  partnerAddress = sock.partnerAddress;
  myAddress = sock.myAddress;
  errorMsg = sock.errorMsg;
}

UDPsocket::~UDPsocket()
{__UDP_ENTER__
  if( (status != UDPFRESH) && (status != UDPVOID) ){
    ok = (close() == 0);
    if(!ok){
      __UDP_MSG("close() failed: " << string(strerror(errno)));
    }
  }
  __UDP_EXIT__
}

bool UDPsocket::bind()
{__UDP_ENTER__
  ok = true;
  errorMsg  = "";
  if(!myAddress.isOk()){
    ok = false;
    __UDP_MSG("Invalid local address: myAddress = " << myAddress)
    __UDP_EXIT__
    return false;
  }
  if(status == UDPVOID){
    ok = false;
    errorMsg = "Void socket!";
    __UDP_MSG("Void socket!")
  }
  if(status != UDPFRESH){
    __UDP_MSG("Already bound!")
  } else {
    int ibound = ::bind(mySocket,
                        myAddress.getAddrI(),
                        myAddress.getAddrLenI());
    if(ibound < 0){
      ok = false;
      errorMsg = string(strerror(errno));
      __UDP_MSG("bind() failed - Error!")
      __UDP_EXIT__
      return false;
    }
    status = UDPALL;
    __UDP_MSG("bind() succeeded!")
  }
  __UDP_EXIT__
  return (ok);
}

int UDPsocket::writeTo(const UDPaddress &receiver, const string &message)
{__UDP_ENTER__
    __UDP_MSG("Called with receiver = " << receiver
               << ", message = " << message << ".")
  ok = true;
  errorMsg = "";
  if(    (status == UDPVOID)
      || (status == UDPNONE)
      || (status == UDPNOSEND) ){
    ok = false;
    errorMsg = "Socket void or shut down for sending.";
    __UDP_MSG("Socket void or shut down for sending.")
    __UDP_EXIT__
    return -1;
  }
  if(!receiver.isOk()){
    ok = false;
    errorMsg = "Invalid recipient address.";
    __UDP_MSG("Invalid recipient address.")
    __UDP_EXIT__
    return -1;
  }
  if(status == UDPFRESH){
    __UDP_MSG("bind() required.")
    ok = bind();
  }
  if(!ok){
    __UDP_MSG("bind() failed.")
    __UDP_EXIT__
    errorMsg = string(strerror(errno));
    return -1;
  }
  if(connected){
    ok = false;
    errorMsg = "Socket already connected.";
    __UDP_MSG("Socket already connected to " << partnerAddress << "!")
    __UDP_EXIT__
    return -1;
  }
  unsigned int flags = 0;
  int msg_len  = message.length();
  int sent_len = sendto( mySocket,
                         message.c_str(),
                         msg_len,
                         flags,
                         receiver.getAddrI(),
                         receiver.getAddrLenI());
  if(sent_len < 0){ // Error
    ok = false;
    errorMsg = string(strerror(errno));
    __UDP_MSG("sendto() failed.")
  } else if(sent_len < msg_len){ // Truncated
    ostringstream status;
    status << "Message truncated (" << sent_len << "/" << msg_len
           << " bytes).";
    errorMsg = status.str();
    ok        = false;
  } else { // OK - entire message sent
    __UDP_MSG("sendto() succeeded.")
  }
  __UDP_EXIT__
  return sent_len;
}

int UDPsocket::write(const string &message)
{__UDP_ENTER__
    ok = true;
    errorMsg = "";
    if(    (status == UDPVOID)
            || (status == UDPFRESH)
            || (status == UDPNONE)
            || (status == UDPNOSEND) ){
      ok = false;
      errorMsg = "Socket void, fresh or shut down for sending.";
      __UDP_MSG("Socket void, fresh  or shut down for sending.")
      __UDP_EXIT__
      return -1;
    }
    if(!connected){
      ok = false;
      errorMsg = "Socket unconnected.";
      __UDP_MSG("Socket unconnected.")
          __UDP_EXIT__
          return -1;
    }
    if(!myAddress.isOk()){
      ok = false;
      errorMsg = "Invalid sender address.";
      __UDP_MSG("Invalid sender address " << myAddress)
          __UDP_EXIT__
          return -1;
    }
    if(!partnerAddress.isOk()){
      ok = false;
      errorMsg = "Invalid recipient address.";
      __UDP_MSG("Invalid recipient address" << partnerAddress)
      __UDP_EXIT__
      return -1;
    }
    unsigned int flags = 0;
    int msg_len  = message.length();
    int sent_len = send( mySocket,
                         message.c_str(),
                         msg_len,
                         flags);
    if(sent_len < 0){ // Error
      ok = false;
      errorMsg = string(strerror(errno));
      __UDP_MSG("send() failed.")
    } else if(sent_len < msg_len){ // Truncated
      ostringstream status;
    status << "Message truncated (" << sent_len << "/" << msg_len
           << " bytes).";
    errorMsg = status.str();
    ok        = false;
  } else { // OK - entire message sent
    __UDP_MSG("sendto() succeeded.")
  }
  __UDP_EXIT__
  return sent_len;
}

string UDPsocket::readFrom(UDPaddress &sender, const double timeoutSecs)
{__UDP_ENTER__
  ok = true;
  string result("");
  if(    (status == UDPVOID)
      || (status   == UDPNONE)
      || (status   == UDPNORECV) ){
    ok = false;
    errorMsg = "Socket void or shut down for receiving.";
    __UDP_MSG("Socket void or shut down for receiving.")
    __UDP_EXIT__
    return "";
  }
  if( status == UDPFRESH ){ // we first need to bind the socket
    __UDP_MSG("bind() required!")
    if ( !bind() ){
      // ok and errorMsg already set by ::bind()
      __UDP_MSG("bind() failed!")
      __UDP_EXIT__
      return result;
    }
  }
  if(connected){
    ok = false;
    errorMsg = "Socket already connected.";
    __UDP_MSG("Socket already connected to " << partnerAddress << "!")
    __UDP_EXIT__
    return "";
  }
  struct timeval vtTimeout;
  bool hasTimeout = (timeoutSecs>0.0);
  if(hasTimeout){
  // prepare timeval as timeout for select()
    int secs = static_cast<int>(timeoutSecs);                   // seconds
    int ms   = static_cast<int>((timeoutSecs - secs) * 1000000);// microsecs
    vtTimeout.tv_sec  = secs;
    vtTimeout.tv_usec = ms;
    __UDP_MSG("Set timeout to " << secs << "sec " << ms << " µs")
  }
  fd_set fdsread;     // Set of file descriptors
  int intFdMax = -1;
  if(ok){ // wait for a message
  // prepare filedescriptor list for select()
    FD_ZERO(&fdsread);            // clear the FD set
    FD_SET(mySocket, &fdsread);   // add socket to FD set
    intFdMax = mySocket;          // number of the highest FD in set
  // prepare message buffer and sender address info
    char messageBuf[UDP_MAXBUF];
    memset(messageBuf,0,UDP_MAXBUF);
    struct sockaddr_storage addr_Sender; // info for sender address
    int addr_Sender_Length = sizeof(addr_Sender);
    // wait for message
    int selected = -1;
    if(hasTimeout){ // with timeout (waiting)
      selected = ::select(intFdMax+1, &fdsread, NULL, NULL, &vtTimeout);
    } else{         // without timeout (blocking)
      selected = ::select(intFdMax+1, &fdsread, NULL, NULL, NULL);
    }
    if(selected == -1){ // ERROR!
      ok = false;
      errorMsg = string(strerror(errno));
      __UDP_MSG("select(): Failed.")
    } else if(selected == 0){ // TIMEOUT!
      ok = false;
      errorMsg = "Timeout.";
      __UDP_MSG("select(): Timeout!")
    } else{
      __UDP_MSG("select(): Succeeded!")
    // Receive the datagram
      int res_len = ::recvfrom(mySocket,
                               messageBuf, UDP_MAXBUF,
                               0,
                               (struct sockaddr*)&addr_Sender,
                               (socklen_t*) &addr_Sender_Length);
      if(res_len < 0){ // Receive failed
        ok = false;
        errorMsg = string(strerror(errno));
        __UDP_MSG("recvfrom() failed - Error!")
      } else if(res_len+1 >= UDP_MAXBUF){ // Message too long
        ok = false;
        ostringstream oss_status;
        oss_status << "Message buffer overflow: "
            << res_len << "/" << UDP_MAXBUF << " bytes.";
        errorMsg = oss_status.str();
        __UDP_MSG("recvfrom() failed - Buffer overflow!")
      } else {
        result = string(messageBuf); // Get the message
        __UDP_MSG("recvfrom() succeeded!")
      }
      sender = UDPaddress(&addr_Sender);
    }
  }
  __UDP_EXIT__
  return result;
}

string UDPsocket::read(const double timeoutSecs)
{__UDP_ENTER__
  ok = true;
  string result("");
  if(    (status == UDPVOID)
      || (status == UDPFRESH)
      || (status == UDPNONE)
      || (status == UDPNORECV) ){
    ok = false;
    errorMsg = "Socket void, fresh or shut down for receiving.";
    __UDP_MSG("Socket void, fresh or shut down for receiving.")
    __UDP_EXIT__
    return "";
  }
  if(!connected){
    ok = false;
    errorMsg = "Socket unconnected.";
    __UDP_MSG("Socket unconnected.")
    __UDP_EXIT__
    return "";
  }
  if(!myAddress.isOk()){
    ok = false;
    errorMsg = "Invalid sender address.";
    __UDP_MSG("Invalid sender address " << myAddress)
    __UDP_EXIT__
    return "";
  }
  if(!partnerAddress.isOk()){
    ok = false;
    errorMsg = "Invalid recipient address.";
    __UDP_MSG("Invalid recipient address" << partnerAddress)
    __UDP_EXIT__
    return "";
  }
  struct timeval vtTimeout;
  bool hasTimeout = (timeoutSecs>0.0);
  if(hasTimeout){
  // prepare timeval as timeout for select()
    int secs = static_cast<int>(timeoutSecs);                   // seconds
    int ms   = static_cast<int>((timeoutSecs - secs) * 1000000);// microsecs
    vtTimeout.tv_sec  = secs;
    vtTimeout.tv_usec = ms;
    __UDP_MSG("Set timeout to " << secs << "sec " << ms << " µs")
  }
  fd_set fdsread;     // Set of file descriptors
  int intFdMax = -1;
  if(ok){ // wait for a message
  // prepare filedescriptor list for select()
    FD_ZERO(&fdsread);            // clear the FD set
    FD_SET(mySocket, &fdsread);   // add socket to FD set
    intFdMax = mySocket;          // number of the highest FD in set
  // prepare message buffer and sender address info
    char messageBuf[UDP_MAXBUF];
    memset(messageBuf,0,UDP_MAXBUF);
  // wait for message
    int selected = -1;
    if(hasTimeout){ // with timeout (waiting)
      selected = ::select(intFdMax+1, &fdsread, NULL, NULL, &vtTimeout);
    } else{         // without timeout (blocking)
      selected = ::select(intFdMax+1, &fdsread, NULL, NULL, NULL);
    }
    if(selected == -1){ // ERROR!
      ok = false;
      errorMsg = string(strerror(errno));
      __UDP_MSG("select(): Failed.")
    } else if(selected == 0){ // TIMEOUT!
      ok = false;
      errorMsg = "Timeout";
      __UDP_MSG("select(): Timeout!")
    } else{
      __UDP_MSG("select(): Succeeded!")
  // Receive the datagram
      int res_len = ::recv(mySocket,
                           messageBuf,
                           UDP_MAXBUF,
                           0);
      if(res_len < 0){ // Receive failed
        ok = false;
        errorMsg = string(strerror(errno));
        __UDP_MSG("recv() failed - Error!")
      } else if(res_len+1 >= UDP_MAXBUF){ // Message too long
        ok = false;
        ostringstream oss_status;
        oss_status << "Message buffer overflow: "
            << res_len << "/" << UDP_MAXBUF << " bytes.";
        errorMsg = oss_status.str();
        __UDP_MSG("recv() failed - Buffer overflow!")
      } else {
        result = string(messageBuf); // Get the message
        __UDP_MSG("recv() succeeded!")
      }
    }
  }
  __UDP_EXIT__
  return result;
}

bool UDPsocket::connect(const UDPaddress &remote)
{__UDP_ENTER__
  ok = true;
  errorMsg = "";
  if( status == UDPVOID ){
    ok = false;
    errorMsg = "Void socket.";
    __UDP_MSG("Void socket!")
    __UDP_EXIT__
    return false; // remain last error code
  }
  if(connected){ // already connected
    __UDP_MSG("Already connected!")
    __UDP_EXIT__
    return true;
  }
  if( status == UDPFRESH ){ // we first need to bind the socket
    __UDP_MSG("bind() required!")
     if ( !bind() ){
      // ok and errorMsg already set by ::bind()
      __UDP_MSG("bind() failed!")
      __UDP_EXIT__
      return false;
    }
  }
  int conn = ::connect(mySocket, remote.getAddrI(), remote.getAddrLenI());
  if(conn < 0){
    ok = false;
    errorMsg = string(strerror(errno));
    __UDP_MSG("Connect() failed!")
  } else {
    connected = true;
    partnerAddress = UDPaddress(remote);
    __UDP_MSG("Connect() succeeded!")
  }
  __UDP_EXIT__
  return ok;
}

bool UDPsocket::shutdown(UDPSocketState how)
{__UDP_ENTER__
  ok =  true;
  errorMsg = "";
  if(     (how != UDPNORECV) && (how != UDPNOSEND)
       && (how != UDPNONE) && (how != UDPALL) ) {
    ok = false;
    errorMsg = "Illegal shutdown type.";
    __UDP_MSG("Illegal shutdown type!")
    __UDP_EXIT__
    return false;
  }
  if( status == UDPVOID ){
    ok = false;
    errorMsg = "Void socket.";
    __UDP_MSG("Void socket!")
        return false;
  }
  if( status == UDPFRESH ){
    ok = false;
    errorMsg = "Socket unbound.";
    __UDP_MSG("Socket unbound!")
    return false;
  }
  int res = ::shutdown(mySocket, how);
  if(res < 0){
    ok = false;
    errorMsg = string(strerror(errno));
    __UDP_MSG("shutdown() failed!")
  } else {
    status = how;
    __UDP_MSG("shutdown() succeeded!")
  }
  __UDP_EXIT__
  return ok;
}

bool UDPsocket::close()
{__UDP_ENTER__
  ok = true;
  errorMsg = "";
  if( (status == UDPVOID) || (status == UDPFRESH) ){
    errorMsg  = "Socket void or unbound.";
    __UDP_MSG("Socket void or unbound!")
    __UDP_EXIT__
    return true;
  }
#ifdef SECONDO_WIN32
  int cr = closesocket(mySocket);
  if(cr <0){
    ok = false;
    errorMsg = string(strerror(errno));
    __UDP_MSG("closesocket() failed:" << errorMsg);
  }
#else
  ::close(mySocket);
#endif
  connected = false;
  status    = UDPFRESH;
  __UDP_EXIT__
  return ok;
}

UDPsocket& UDPsocket::operator=(const UDPsocket& sock)
{
  if( this != &sock ){
    status    = sock.status;
    connected = sock.connected;
    ok        = sock.ok;
    mySocket  = sock.mySocket;
    partnerAddress = sock.partnerAddress;
    myAddress = sock.myAddress;
    errorMsg  = sock.errorMsg;
  }
  return (*this);
}

ostream& UDPsocket::Print(ostream &o) const
{
  o << "UDPsocket["
    << "status = ";
  switch(status){
    case UDPVOID:   { o << "VOID"   ; break;}
    case UDPFRESH:  { o << "FRESH"  ; break;}
    case UDPNORECV: { o << "NORECV" ; break;}
    case UDPNOSEND: { o << "NOSEND" ; break;}
    case UDPNONE:   { o << "NONE"   ; break;}
    case UDPALL:    { o << "ALL"    ; break;}
    default:        { o << "UNKNOWN!"; }
  }
  o << "(" << status << ")"
    << ",\n\tconnected = " << connected
    << ",\n\tok = " << ok
    << ",\n\tmySocket = " << mySocket
    << ",\n\tmyAddress = " << myAddress
    << ",\n\tpartnerAddress = " << partnerAddress
    << ",\n\terrorMsg = " << errorMsg
    << "\n]";
  return o;
}

ostream& operator<<(ostream &o, const UDPsocket &s)
{
  return s.Print(o);
}

// --- End of source ---

