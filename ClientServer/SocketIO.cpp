/*
1 SocketIO

February 2002 Ulrich Telle

June 2002 Ulrich Telle Corrected error in ~SocketBuffer::underflow~

1.1 Overview

This module provides several classes for portable network communication
through sockets.

*/

using namespace std;

#include "SecondoConfig.h"
#include "SocketIO.h"

#ifdef SECONDO_WIN32
#include "Win32Socket.cpp"
#else
#include "UnixSocket.cpp"
#endif

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

int
SocketBuffer::xsputn( const char* s, int n )
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


int
SocketBuffer::xsgetn( char* s, int n )
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

// --- End of source ---

