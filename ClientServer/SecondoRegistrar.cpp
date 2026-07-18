/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics &
Computer Science, Database Systems for New Applications.

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

M. Spiekermann: Trace messages and error messages added. Some critical
C++ code re-implemented. Bug fix for command UNREGISTER.

The command loop is a poll() multiplexer rather than an accept-one-then-serve
loop. Each connection carries its own read/dispatch/write state and no client
can hold up another. The command handlers are pure: each parses the argument
tail and returns the reply text, touching no socket, so the wire protocol is
unchanged (one command per connection, one or more reply lines).

*/

#include <string>
#include <algorithm>
#include <map>
#include <queue>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstring>

#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Application.h"
#include "Processes.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "CharTransform.h"
#include "LogMsg.h"
#include "Trace.h"

// MSG_NOSIGNAL keeps a write to a vanished peer from raising SIGPIPE on Linux;
// where it is absent (macOS) the accepted socket gets SO_NOSIGPIPE instead.
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

using namespace std;

// Caps for the connection multiplexer. kMaxConns bounds the fd table (poll's
// O(n) scan stays cheap); kMaxLineLen rejects a client that never sends a
// newline; kIdleTimeout reaps a connection that stops making progress.
static const size_t kMaxConns    = 256;
static const size_t kMaxLineLen  = 8192;
static const time_t kIdleTimeout = 30;

// Per-connection state. One command per connection: read into inbuf until a
// newline, dispatch, then drain outbuf and close.
struct RegConn
{
  string inbuf;
  string outbuf;
  size_t outSent;
  time_t lastIo;
  bool   closing;
  RegConn() : outSent( 0 ), lastIo( 0 ), closing( false ) {}
};


class SecondoRegistrar : public Application
{
 public:
  SecondoRegistrar( const int argc, const char** argv ) :
    Application( argc, argv ),
    trace("REGISTRAR"),
    EXIT_REGISTRAR_OK( 0 ),
    EXIT_REGISTRAR_NOQUEUE( 1 ),
    EXIT_REGISTRAR_ABORT( 2 )
  { };
  virtual ~SecondoRegistrar() {};
  int Execute();
 private:
  int  ProcessCommands();

  // Connection multiplexer helpers.
  void AcceptNewConns( int listenFd, time_t now );
  void ReadConn( int fd, time_t now );
  void FlushConn( int fd );
  void CloseConn( int fd );
  void ReapIdle( time_t now );

  // Command handlers: pure, socket-free, each returns the reply text.
  string DispatchLine( const string& line );
  string ExecRegister( istringstream& args );
  string ExecUnregister( istringstream& args );
  string ExecLock( istringstream& args );
  string ExecUnlock( istringstream& args );
  string ExecLogin( istringstream& args );
  string ExecLogout( istringstream& args );
  string ExecLogMsg( istringstream& args );
  string ExecShowMsgs();
  string ExecShowUsers();
  string ExecShowLocks();
  string ExecShowDatabases();

  string parmFile;
  string  port;

  Socket* msgSocket;
  map<int,RegConn>           conns;
  multimap<string,ProcessId> dbUsers;
  multimap<string,string>    dbRegister;
  map<string,string>         dbLocks;
  std::queue<string>         logMsgs;

  Trace trace;

  const int EXIT_REGISTRAR_OK;
  const int EXIT_REGISTRAR_NOQUEUE;
  const int EXIT_REGISTRAR_ABORT;

  typedef enum {  REGISTER, UNREGISTER,
    LOCK, UNLOCK,
    LOGIN, LOGOUT, LOGMSG,
    SHOWMSGS, SHOWUSERS, SHOWLOCKS, SHOWDATABASES } cmdTok;
  map<string,cmdTok> commandTable;
};

string
SecondoRegistrar::ExecRegister( istringstream& args )
{
  string database;
  string user;
  args >> database >> user;
  ostringstream reply;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock == dbLocks.end() )
  {
    dbRegister.insert( make_pair( database, user ) );
    reply << "0 Registrar: Ok" << "\n";
  }
  else
  {
    reply << "-1 Registrar: Database '" << database
          << "' is locked by " << posLock->second << "\n";
  }
  return reply.str();
}

string
SecondoRegistrar::ExecUnregister( istringstream& args )
{
  string database;
  string user;
  args >> database >> user;
  bool found = false;
  multimap<string,string>::iterator pos;
  for ( pos  = dbRegister.lower_bound( database );
        pos != dbRegister.upper_bound( database ); pos++ )
  {
    if ( pos->second == user )
    {
      dbRegister.erase( pos );
      found = true;
      break;
    }
  }
  ostringstream reply;
  if ( found )
  {
    reply << "0 Registrar: Ok" << "\n";
  }
  else
  {
    reply << "-1 Registrar: Database '" << database
          << "' not registered by " << user << "\n";
  }
  return reply.str();
}

string
SecondoRegistrar::ExecLock( istringstream& args )
{
  string database;
  string user;
  args >> database >> user;
  ostringstream reply;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock == dbLocks.end() )
  {
    multimap<string,string>::iterator pos = dbRegister.find( database );
    if ( pos == dbRegister.end() )
    {
      dbLocks.insert( make_pair( database, user ) );
      reply << "0 Registrar: Ok" << "\n";
    }
    else
    {
      reply << "-1 Registrar: Database '" << database << "' is in use" << "\n";
    }
  }
  else
  {
    reply << "-1 Registrar: Database '" << database
          << "' is locked by " << posLock->second << "\n";
  }
  return reply.str();
}

string
SecondoRegistrar::ExecUnlock( istringstream& args )
{
  string database;
  string user;
  args >> database >> user;
  ostringstream reply;
  map<string,string>::iterator posLock = dbLocks.find( database );
  if ( posLock != dbLocks.end() )
  {
    dbLocks.erase( posLock );
    reply << "0 Registrar: Ok" << "\n";
  }
  else
  {
    reply << "-1 Registrar: Database '" << database
          << "' was not locked by " << user << "\n";
  }
  return reply.str();
}

string
SecondoRegistrar::ExecLogin( istringstream& args )
{
  string user;
  ProcessId pid;
  args >> user >> pid;
  dbUsers.insert( make_pair( user, pid ) );
  return "0 Registrar: Login ok\n";
}

string
SecondoRegistrar::ExecLogout( istringstream& args )
{
  string user;
  ProcessId pid;
  args >> user >> pid;
  bool found = false;
  multimap<string,ProcessId>::iterator pos;
  for ( pos  = dbUsers.lower_bound( user );
        pos != dbUsers.upper_bound( user ); pos++ )
  {
    if ( pos->second == pid )
    {
      dbUsers.erase( pos );
      found = true;
      break;
    }
  }
  ostringstream reply;
  if ( found )
  {
    reply << "0 Registrar: Ok" << "\n";
  }
  else
  {
    reply << "-1 Registrar: User '" << user << "' not logged in" << "\n";
  }
  return reply.str();
}

string
SecondoRegistrar::ExecLogMsg( istringstream& args )
{
  string msg;
  getline( args, msg );
  logMsgs.push( msg );
  return "0 Registrar: Ok\n";
}

string
SecondoRegistrar::ExecShowMsgs()
{
  ostringstream reply;
  reply << "1 Registrar: " << logMsgs.size() << " messages" << "\n";
  while (!logMsgs.empty())
  {
    reply << "* " << logMsgs.front() << "\n";
    logMsgs.pop();
  }
  reply << "0 Registrar: Ok" << "\n";
  return reply.str();
}

string
SecondoRegistrar::ExecShowUsers()
{
  ostringstream reply;
  reply << "1 Registrar: " << dbUsers.size() << " users" << "\n";
  multimap<string,ProcessId>::iterator posUser;
  for ( posUser  = dbUsers.begin();
        posUser != dbUsers.end(); posUser++ )
  {
    reply << "* " << posUser->first << " " << posUser->second << "\n";
  }
  reply << "0 Registrar: Ok" << "\n";
  return reply.str();
}

string
SecondoRegistrar::ExecShowLocks()
{
  ostringstream reply;
  reply << "1 Registrar: " << dbLocks.size() << " locks" << "\n";
  map<string,string>::iterator pos;
  for ( pos  = dbLocks.begin();
        pos != dbLocks.end(); pos++ )
  {
    reply << "* " << pos->first << " " << pos->second << "\n";
  }
  reply << "0 Registrar: Ok" << "\n";
  return reply.str();
}

string
SecondoRegistrar::ExecShowDatabases()
{
  ostringstream reply;
  reply << "1 Registrar: " << dbRegister.size() << " databases" << "\n";
  multimap<string,string>::iterator pos;
  for ( pos  = dbRegister.begin();
        pos != dbRegister.end(); pos++ )
  {
    reply << "* " << pos->first << " " << pos->second << "\n";
  }
  reply << "0 Registrar: Ok" << "\n";
  return reply.str();
}

string
SecondoRegistrar::DispatchLine( const string& line )
{
  istringstream args( line );
  string cmd;
  args >> cmd;
  trace.out( cmd );
  transform( cmd.begin(), cmd.end(), cmd.begin(), ::toupper );
  map<string,cmdTok>::iterator cmdPos = commandTable.find( cmd );
  if ( cmdPos != commandTable.end() )
  {
    switch (cmdPos->second) {
      case REGISTER:      return ExecRegister( args );
      case UNREGISTER:    return ExecUnregister( args );
      case LOCK:          return ExecLock( args );
      case UNLOCK:        return ExecUnlock( args );
      case LOGIN:         return ExecLogin( args );
      case LOGOUT:        return ExecLogout( args );
      case LOGMSG:        return ExecLogMsg( args );
      case SHOWMSGS:      return ExecShowMsgs();
      case SHOWUSERS:     return ExecShowUsers();
      case SHOWLOCKS:     return ExecShowLocks();
      case SHOWDATABASES: return ExecShowDatabases();
    }
  }
  trace.out( "Invalid Command" );
  return "-2 Registrar: Invalid Command " + cmd + "\n";
}

void
SecondoRegistrar::CloseConn( int fd )
{
  ::close( fd );
  conns.erase( fd );
}

void
SecondoRegistrar::AcceptNewConns( int listenFd, time_t now )
{
  // Accept up to the cap, then stop and let the listen backlog hold the rest.
  // Accepting and immediately closing would look like success to a client that
  // is mid-write, so we leave the surplus queued instead.
  while ( conns.size() < kMaxConns )
  {
    int cfd = ::accept( listenFd, NULL, NULL );
    if ( cfd < 0 )
    {
      break;   // EAGAIN once the backlog is drained, or a transient error
    }
    // The event loop must never block on one connection, so the accepted
    // socket has to be non-blocking. If we cannot make it so, drop this
    // connection rather than let a blocking recv()/send() stall every other
    // client.
    int fl = fcntl( cfd, F_GETFL, 0 );
    if ( fl == -1 || fcntl( cfd, F_SETFL, fl | O_NONBLOCK ) == -1 )
    {
      cerr << "REGISTRAR: cannot set an accepted socket non-blocking ("
           << strerror( errno ) << "); dropping the connection." << endl;
      ::close( cfd );
      continue;
    }
    fcntl( cfd, F_SETFD, FD_CLOEXEC );
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt( cfd, SOL_SOCKET, SO_NOSIGPIPE, (char*) &one, sizeof(one) );
#endif
    RegConn c;
    c.lastIo = now;
    conns[cfd] = c;
  }
}

void
SecondoRegistrar::ReadConn( int fd, time_t now )
{
  map<int,RegConn>::iterator it = conns.find( fd );
  if ( it == conns.end() )
  {
    return;
  }
  RegConn& c = it->second;
  char buf[4096];
  for (;;)
  {
    ssize_t n = ::recv( fd, buf, sizeof(buf), 0 );
    if ( n > 0 )
    {
      c.lastIo = now;
      c.inbuf.append( buf, (size_t) n );
      string::size_type nl = c.inbuf.find( '\n' );
      if ( nl != string::npos )
      {
        // One command per connection: dispatch the first complete line and
        // stop reading. Any trailing bytes are ignored, as before.
        c.outbuf  = DispatchLine( c.inbuf.substr( 0, nl ) );
        c.outSent = 0;
        c.closing = true;
        FlushConn( fd );
        return;
      }
      if ( c.inbuf.size() > kMaxLineLen )
      {
        c.outbuf  = "-2 Registrar: Invalid Command\n";
        c.outSent = 0;
        c.closing = true;
        FlushConn( fd );
        return;
      }
      continue;   // partial line: keep draining until EAGAIN
    }
    if ( n == 0 )
    {
      CloseConn( fd );   // peer closed
      return;
    }
    if ( errno == EINTR )
    {
      continue;
    }
    if ( errno == EAGAIN || errno == EWOULDBLOCK )
    {
      return;            // nothing more to read for now
    }
    CloseConn( fd );     // hard error
    return;
  }
}

void
SecondoRegistrar::FlushConn( int fd )
{
  map<int,RegConn>::iterator it = conns.find( fd );
  if ( it == conns.end() )
  {
    return;
  }
  RegConn& c = it->second;
  while ( c.outSent < c.outbuf.size() )
  {
    ssize_t n = ::send( fd, c.outbuf.data() + c.outSent,
                        c.outbuf.size() - c.outSent, MSG_NOSIGNAL );
    if ( n > 0 )
    {
      c.outSent += (size_t) n;
    }
    else if ( n < 0 && errno == EINTR )
    {
      continue;
    }
    else if ( n < 0 && ( errno == EAGAIN || errno == EWOULDBLOCK ) )
    {
      return;            // socket buffer full: finish on the next POLLOUT
    }
    else
    {
      CloseConn( fd );   // EPIPE or another write error
      return;
    }
  }
  if ( c.closing )
  {
    CloseConn( fd );
  }
}

void
SecondoRegistrar::ReapIdle( time_t now )
{
  for ( map<int,RegConn>::iterator it = conns.begin(); it != conns.end(); )
  {
    if ( now - it->second.lastIo > kIdleTimeout )
    {
      int fd = it->first;
      ++it;              // advance before the erase inside CloseConn
      CloseConn( fd );
    }
    else
    {
      ++it;
    }
  }
}

int
SecondoRegistrar::ProcessCommands()
{
  trace.enter(__FUNCTION__);

  commandTable["REGISTER"]      = REGISTER;
  commandTable["UNREGISTER"]    = UNREGISTER;
  commandTable["LOCK"]          = LOCK;
  commandTable["UNLOCK"]        = UNLOCK;
  commandTable["LOGIN"]         = LOGIN;
  commandTable["LOGOUT"]        = LOGOUT;
  commandTable["LOGMSG"]        = LOGMSG;
  commandTable["SHOWMSGS"]      = SHOWMSGS;
  commandTable["SHOWUSERS"]     = SHOWUSERS;
  commandTable["SHOWLOCKS"]     = SHOWLOCKS;
  commandTable["SHOWDATABASES"] = SHOWDATABASES;

  int listenFd = msgSocket->GetDescriptor();
  int lfl = fcntl( listenFd, F_GETFL, 0 );
  if ( lfl != -1 )
  {
    fcntl( listenFd, F_SETFL, lfl | O_NONBLOCK );
  }

  int rc = 0;
  while (rc == 0)
  {
    if ( Application::Instance()->ShouldAbort() )
    {
      rc = EXIT_REGISTRAR_ABORT;
      break;
    }

    // One slot for the listener plus one per live connection. The listener is
    // masked when the table is full, so poll does not spin on a backlog we are
    // deliberately not draining yet.
    vector<struct pollfd> pfds;
    struct pollfd lp;
    lp.fd      = listenFd;
    lp.events  = ( conns.size() < kMaxConns ) ? POLLIN : 0;
    lp.revents = 0;
    pfds.push_back( lp );
    for ( map<int,RegConn>::iterator it = conns.begin();
          it != conns.end(); ++it )
    {
      struct pollfd cp;
      cp.fd      = it->first;
      cp.events  = 0;
      if ( !it->second.closing )
      {
        cp.events |= POLLIN;      // still reading this connection's command
      }
      if ( it->second.outSent < it->second.outbuf.size() )
      {
        cp.events |= POLLOUT;     // a reply is waiting for the socket buffer
      }
      cp.revents = 0;
      pfds.push_back( cp );
    }

    // poll() is not restarted after a signal, so a SIGTERM breaks the wait and
    // the ShouldAbort check above ends the loop on the next turn.
    int ready = poll( pfds.data(), pfds.size(), 1000 );
    if ( Application::Instance()->ShouldAbort() )
    {
      rc = EXIT_REGISTRAR_ABORT;
      break;
    }
    time_t now = time( 0 );
    if ( ready > 0 )
    {
      if ( pfds[0].revents & POLLIN )
      {
        AcceptNewConns( listenFd, now );
      }
      for ( size_t i = 1; i < pfds.size(); i++ )
      {
        int   cfd = pfds[i].fd;
        short re  = pfds[i].revents;
        if ( re == 0 || conns.find( cfd ) == conns.end() )
        {
          continue;
        }
        // POLLIN before POLLHUP: on AF_UNIX a hangup can be reported with data
        // still buffered, and that data may be the command.
        if ( re & POLLIN )
        {
          ReadConn( cfd, now );
          if ( conns.find( cfd ) == conns.end() )
          {
            continue;
          }
        }
        if ( re & POLLOUT )
        {
          FlushConn( cfd );
          if ( conns.find( cfd ) == conns.end() )
          {
            continue;
          }
        }
        if ( re & ( POLLHUP | POLLERR | POLLNVAL ) )
        {
          CloseConn( cfd );
        }
      }
    }
    ReapIdle( now );
  }

  // Shutdown: drop every live connection. The listen socket belongs to
  // msgSocket, whose destructor closes and unlinks it.
  for ( map<int,RegConn>::iterator it = conns.begin();
        it != conns.end(); ++it )
  {
    ::close( it->first );
  }
  conns.clear();

  if ( rc == EXIT_REGISTRAR_ABORT )
  {
    rc = EXIT_REGISTRAR_OK;
  }
  return (rc);
}

int
SecondoRegistrar::Execute()
{
  SetAbortMode( true );

  // Shut down on a terminating signal instead of dying in the poll loop. A
  // clean exit runs the delete msgSocket below, whose destructor unlinks the
  // /tmp socket file; killed where it stands, the registrar leaks that file.
  SetGracefulTermination( true );

  if (RTFlag::isActive("Registar:trace")) {
    trace.on();
  }
  int rc = EXIT_REGISTRAR_NOQUEUE;
  if ( GetArgCount() > 1 )
  {
    parmFile = GetArgValues()[1];
  }
  else
  {
    parmFile = "SecondoConfig.ini";
  }
  port = GetArgCount() > 2 ? GetArgValues()[2] : "";

  trace.show( VAL(parmFile) );
  string msgQueue = SmiProfile::GetUniqueSocketName( parmFile, port );
  trace.show( VAL(msgQueue) );
  msgSocket = Socket::CreateLocal( msgQueue );
  trace.out("local socket created!");
  if ( msgSocket->IsOk() )
  {
    trace.out("local socket is ok!");
    rc = ProcessCommands();
  }
  else
  {
    cerr << "REGISTRAR: Error! local socket is *not* ok." << endl;
  }
  delete msgSocket;
  cerr << "REGISTRAR is going down with rc = " << rc << endl;
  return (rc);
}

int main( const int argc, const char* argv[] )
{
  SecondoRegistrar* appPointer = new SecondoRegistrar( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}
