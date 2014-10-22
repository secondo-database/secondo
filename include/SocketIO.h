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

//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[<] [\lt ]
//[>] [\gt ]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Socket I/O

February 2002 Ulrich Telle

1.1 Overview

TCP (Transaction Control Protocol) is one of the most common protocols
on the Internet. It has two basic communicators: clients and servers.
Servers offer services to the network; clients connect to these services
through ports. Sockets are the basis of network communication. Client
software uses sockets to connect to servers. Servers use sockets to
process network connections. TCP sockets provide very reliable connections
using a special open communication flow similar to low-level stream I/O.

Implementing clients and servers are fairly straightforward. Servers build
a socket for listening on a port for incoming client connect requests.
Clients also build a socket, connect to the server socket and then exchange
messages.

The set of classes defined here supports a subset of the TCP/IP protocol
suite. Currently only IP version 4 is supported, but support for IP version
6 could be incorporated.

Implementations of sockets are mostly based on socket libraries provided
by the operating system. Local domain sockets are directly supported only
in Unix. For 32-bit-Windows this socket module provides a very efficient
implementation of local sockets using shared memory and semaphore objects.
This allows very fast inter-process communication on one computer.

*NOTE*: Different operating systems require different initialization and
deinitialization of the socket interface. This is done transparently by
instantiating a static instance of an internal class hidden in the
implementation. The constructor is called before the main function is
entered, thereby ensuring that the operating systems's socket interfaces
is properly installed. The destructor is called on normal termination of
the program.

1.3 Interface methods

The class ~Socket~ is the heart of this module. Network communication
is done by using methods of this class. Additionally to creating and
destroying sockets, reading and writing on these sockets there are
methods to identify the client and the server thus allowing a basic
form of authentication and access control through the classes
~SocketRule~ and ~SocketRuleSet~. It is possible to specify a list of
rules where each rule consists of an IP address, an IP address mask
and an access policy allowing or denying access. When an IP address
is checked against a set of rules, for each rule a bitwise *and*
operation is performed on the given IP address and the IP address
mask of a rule. The result is compared with the IP address of the rule.
If the result matches, the decision whether access should be allowed
or denied is based on the access policy of the rule and the default
access policy of the rule set. Finally, the class ~SocketAddress~
hides details of the internet addresses from the user.

The class ~Socket~ provides the following methods:

[23]    Creation/Removal & Input/Output    & Information          \\
        [--------]
        Socket           & Read            & IsLibraryInitialized \\
        [tilde]Socket    & Write           & IsOk                 \\
        CreateLocal      & GetSocketStream & GetErrorText         \\
        CreateGlobal     &                 & GetSocketAddress     \\
        Accept           & CancelAccept    & GetPeerAddress       \\
        CreateClient     & Close           & GetHostname          \\
        Connect          & ShutDown        & GetIP                \\
        GetDescriptor    &                 &                      \\

The class ~SocketRule~ provides the following methods:

[23]    Creation/Removal  & Checking & I/O settings \\
        [--------]
        SocketRule        & Match    & SetDelimiter \\
        [tilde]SocketRule & Allowed  & GetDelimiter \\
                          & Denied   &              \\

The class ~SocketRuleSet~ provides the following methods:

[23]    Creation/Removal     & Checking & Persistence \\
        [--------]
        SocketRuleSet        & AddRule  & LoadFromFile \\
        [tilde]SocketRuleSet & Ok       & StoreToFile \\

The class ~SocketAddress~ provides the following methods:

[22]    Creation/Removal     & Address handling \\
        [--------]
        SocketAddress        & SetAddress       \\
        [tilde]SocketAddress & GetSocketString  \\
        operator=            & GetIPAddress     \\
                             & GetPort          \\
                             & operator==       \\

The class ~SocketBuffer~ implements the *streambuf* interface of the
C++ standard library for socket streams. This greatly simplifies
reading and writing to and from sockets. Since this class is used
internally only, the class interface is not described here.

*/

#ifndef SOCKET_IO_H
#define SOCKET_IO_H

/*
1.2 Imports and definitions

*/
#include "SecondoConfig.h"
#include <time.h>
#include <iostream>
#include <vector>
#include <cstdio>
#ifdef SECONDO_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>          // for getprotobynumber()
#endif


// Debugging and tracing UDPsockets - begin

// (Uncomment the following line for detailed tracing)
//#define __TRACE_UDP__

#ifdef __TRACE_UDP__

#define __UDP_POS__ __FILE__ << ".." << __PRETTY_FUNCTION__ << "@" << __LINE__
#define __UDP_ENTER__ cerr << "ENTER " <<  __UDP_POS__ << endl;
#define __UDP_EXIT__ cerr << "EXIT " <<  __UDP_POS__ << endl;
#define __UDP_MSG(X) cerr << __UDP_POS__ << ": " << X << endl;

#else

#undef __POS__
#define __POS__ __FILE__
#define __UDP_ENTER__
#define __UDP_EXIT__
#define __UDP_MSG(X)

#endif

// Debugging and tracing UDPsockets - end

using namespace std;

#define DEFAULT_CONNECT_MAX_ATTEMPTS 100
/*
Defines the default for how many attempts are made at most to connect
a client socket to a server socket.

*/
#define DEFAULT_RECONNECT_TIMEOUT    1  // seconds
/*
Defines the default time interval between two connect attempts.

*/
#define DEFAULT_LISTEN_QUEUE_SIZE    5
/*
Defines the default capacity of the listener queue.

*/
#define LINGER_TIME                  10 // seconds
/*
Defines how long the kernel should try to send data still waiting in
the socket buffer after the socket was closed.

*/
#define WAIT_FOREVER                 ((time_t)-1)
/*
Specifies an indefinite time period. Specifying this value for a time out
period is identical to blocking I/O mode.

*/
#define ENABLE_BROADCAST             1
/*
Defines the flag for enabling broadcast messages on UDP sockets.

*/

#ifdef SECONDO_WIN32
#include <winsock2.h>
typedef SOCKET SocketDescriptor;
#else
typedef int    SocketDescriptor;
#define INVALID_SOCKET (-1)
#endif
/*
Is a type definition for a socket handle.
If a socket is not valid, the handle value equals "INVALID\_SOCKET"[4].

*/

/*
1.3 Class "Socket"[1]

The class "Socket"[1] defines an abstract socket interface. The concrete
operating system dependent socket classes are implemented as derived
classes.

Since ~Socket~ is an abstract base class, instances of ~Socket~ are
created through the use of special static "factory" methods.

*NOTE*: Since signals present portability problems, this code does not
support signals (like SIGIO, etc.), which is why there is no interface
to the ~fcntl~ function.

*/
class SocketBuffer;
/*
Is a forward declaration of the stream buffer class for socket stream support.

*/
class SDB_EXPORT Socket {
 public:
  enum SocketDomain
  {
    SockAnyDomain,
    SockLocalDomain,
    SockGlobalDomain
  };
/*
Is an enumeration of domain types:

  * ~SockAnyDomain~ -- the domain type is chosen automatically

  * ~SockLocalDomain~ -- the domain type is local (i.e. Unix domain sockets)

  * ~SockGlobalDomain~ -- the domain type is global (i.e. INET sockets)

*/
  static bool IsLibraryInitialized();
/*
Checks whether the operating socket interface was successfully initialized.

*/
  Socket() { state = SS_CLOSE; }
/*
Initializes a ~Socket~ instance as an invalid socket in closed state.

*/
  virtual ~Socket() {}
/*
Destroys a socket.

*/
  static Socket*  Connect( const string& address,
                           const string& port,
                           const SocketDomain domain =
                             SockAnyDomain,
                           const int maxAttempts =
                             DEFAULT_CONNECT_MAX_ATTEMPTS,
                           const time_t timeout =
                             DEFAULT_RECONNECT_TIMEOUT );

/*
Establishes a connection to a server. This method will do at most
~maxAttempts~ attempts to connect to the server, with an interval of
~timeout~ seconds between the attempts. The address of the server is
specified by ~address~ and ~port~, both as strings. The type of the
connection is specified by ~domain~. The following values of this
parameter are recognized:

  * ~SockAnyDomain~ -- the domain is chosen automatically

  * ~SockLocalDomain~ -- local domain (connection with a server on the same host)

  * ~SockGlobalDomain~ -- internet domain

If ~SockAnyDomain~ is specified, a local connection is chosen when either
the port was omitted in the specification of the address or hostname is
*localhost*; a global connection is used in all other cases.

This method always creates a new socket object and returns a pointer to it.
If a connection to the server was not established, this socket contains an
error code describing reason of failure. So the returned socket should be
first checked by its ~IsOk~ method.

*/
  static Socket*  CreateLocal( const string& address,
                               const int listenQueueSize =
                                 DEFAULT_LISTEN_QUEUE_SIZE);
/*
Creates and opens a socket in the local domain at the server side.
The parameter ~address~ specifies the name to be assigned to the socket.
The parameter ~listenQueueSize~ specifies the size of the listen queue.

This method always creates a new socket object and returns a pointer to it.
If a connection to the server was not established, this socket contains an
error code describing reason of failure. So the returned socket should be
first checked by its ~IsOk~ method.

*/
  static Socket*  CreateGlobal( const string& address,
                                const string& port,
                                const int listenQueueSize =
                                  DEFAULT_LISTEN_QUEUE_SIZE );
/*
Creates and opens a socket in the global (internet) domain at the server side.
The parameter ~address~ specifies the name to be assigned to the socket.
The parameter ~listenQueueSize~ specifies the size of the listen queue.

This method always creates a new socket object and returns a pointer to it.
If a connection to the server was not established, this socket contains an
error code describing reason of failure. So the returned socket should be
first checked by its ~IsOk~ method.

*/
  virtual SocketDescriptor GetDescriptor() = 0;
/*
Returns the socket descriptor of the socket. This socket descriptor may be
inherited by a child process later on.

*/
  static Socket*  CreateClient( const SocketDescriptor sd );
/*
(Re)creates a socket instance for the socket descriptor ~sd~, created by
the ~Accept~ method.

This method is provided to allow passing socket descriptors to child
processes.

*NOTE*: While in Unix-like systems socket handles are inherited by child
processes automatically, this is not the case in Windows systems. Usually
you have to take special measure to make a socket handle inheritable and
to make it accessible by a child process. The parent process must not close
the socket before the child process has finished using the socket.

*/
  virtual int     Read( void* buf,
                        size_t minSize, size_t maxSize,
                        time_t timeout = WAIT_FOREVER ) = 0;
/*
Receives incoming data, transferring it from the socket into the buffer
~buf~. The maximal size of the buffer is given by ~maxSize~.
The function returns the number of bytes actually read from the socket,
which ranges from ~minSize~ to ~maxSize~. The function does not return
to the caller before at least ~minSize~ bytes were received or a time out
occurred. When the return value is less than ~minSize~, a time out has occurred.
When the return value is less than zero, an error has occurred. Usually it
means that the socket has disconnected.

*/
  virtual bool    Read( void* buf, size_t size ) = 0;
/*
Receives incoming data, transferring it from the socket into the buffer
~buf~. The function does not return to the caller before exactly ~size~
bytes were received or an error occurred. The function returns "true"[4] when
the transfer was successful.

*/
  virtual bool    Write( void const* buf, size_t size ) = 0;
/*
Sends the data contained in buffer ~buf~ over the socket. The buffer is
assumed to contain ~size~ bytes. The function returns "true"[4] when all
bytes could be transferred successfully. In case of an error "false"[4] is
returned.

*/
  virtual bool    IsOk() = 0;
/*
Checks whether the socket is correctly initialized and ready for operation.

*/
  virtual string  GetErrorText() = 0;
/*
Returns an error message text for the last error occurred.

*/
  virtual string  GetSocketAddress() const = 0;
/*
Returns the IP address of the socket in string representation.

*/
  virtual string  GetPeerAddress() const = 0;
/*
Returns the IP address of the socket to which this socket is connected
in string representation.

*/
  virtual Socket* Accept() = 0;
/*
Is called by a server to establish a pending client connection.
When the client executes the ~Connect~ method and accesses the server's
accept port, this method will create a new socket, which can be used for
communication with the client.

The function returns a pointer to a new socket that controls the
communication between client and server. The new socket must be released
by the server once it has finished using it. If the operation failed a
"NULL"[4] pointer is returned.

The function ~Accept~ blocks until a connection will be established and
therefore cannot be used to detect activity on multiple sockets.

*/
  virtual bool    CancelAccept() = 0;
/*
Cancels an accept operation and closes the socket.

*/
  virtual bool    Close() = 0;
/*
Closes the socket.

*NOTE*: The operating system decrements the associated reference counter
of the socket by one. The TCP/IP connection is closed when the reference
counter reaches zero.

*/
  virtual bool    ShutDown() = 0;
/*
Shuts down the socket. Thereafter read and write operations on the
socket are prohibited. All future attempts to read or write data
from/to the socket will be refused. But all previously initiated
operations are guaranteed to be completed. The function returns
"true"[4] if the operation was successfully completed, "false"[4] otherwise.

*/
  static string GetHostname( const string& ipAddress );
/*
Tries to get the fully qualified host name corresponding to the IP address
~ipAddress~ which is given in string representation. If the method fails
the string "<unknown>"[2] will be returned.

*/
  static int GetIP( const string& address );
/*
Gets the IP address of the host. "address" parameter should contain either
symbolic host name (for example "robinson"), or a string with IP address
(for example "195.239.208.225")

*/
  iostream& GetSocketStream();
/*
Returns a reference to the I/O stream associated with the socket.

An I/O stream is available only for sockets created by the methods
~Connect~ and ~Accept~.

*/
 protected:
  enum { SS_OPEN, SS_SHUTDOWN, SS_CLOSE } state;
/*
Defines the socket state.

*/
   SocketBuffer* ioSocketBuffer; // Socket stream buffer
   iostream*     ioSocketStream; // Socket I/O stream
};

extern string GetProcessName();
/*
Returns the current host name combined with an identifier of the current process.

*/

/*
1.1 Class "SocketAddress"[1]

Class "SocketAddress"[1] represents a socket address. Socket addresses
combine an IP address with a port number. The IP address identifies a
host, while the port number identifies a service available on the host.
Typically used by TCP/IP clients to indicate a machine and service they
are connecting to.

*/
class SDB_EXPORT SocketAddress
{
 public:
  SocketAddress();
/*
Initializes a socket address, which consists of
an IP address and a port number.

The IP address is set to the wildcard address ("INADDR\_ANY"[4]).
The port number is set to zero.

*/
  SocketAddress( const SocketAddress& sockAddr );
/*
Creates a socket address that is an identical copy of ~sockAddr~.

*/
  SocketAddress( const string& ipAddr, uint16_t portNo = 0 );
/*
Initializes a socket address converting the string representation of
an IP address ~ipAddr~ into the internal binary representation. The
port number is set to ~portNo~.

*/
  virtual ~SocketAddress();
/*
Destroys a socket address.

*/
  SocketAddress& operator=( const SocketAddress& sockAddr );
/*
Changes ~self~ into an identical copy of the socket address referenced
by ~sockAddr~.

*/
  bool operator==( const SocketAddress& sockAddr ) const;
/*
Compares ~self~ with socket address ~sockAddr~.
The method returns "true"[4] if both objects are equal, otherwise it
returns "false"[4].

*/
  void SetAddress( const string& ipAddr,
                   uint16_t portNo = 0 );
/*
Sets the socket address converting the string representation of
an IP address ~ipAddr~ into the internal binary representation. The
port number is set to ~portNo~.

*/
  void SetAddress( const string& ipAddr,
                   const string& portNo );
/*
Sets the socket address converting the string representation of
an IP address ~ipAddr~ into the internal binary representation. The
string representation ~portNo~ of the port number is also converted.

*/
  string GetSocketString() const;
/*
Returns the IP address of the socket including the port number as
a string. The port number is appended to the IP address after
inserting a colon as a delimiter, i.e. ~132.176.69.10:1234~.

*/
  string GetIPAddress() const;
/*
Returns the IP address portion of the socket address in string format.

*/
  uint16_t GetPort() const;
/*
Returns the port number portion of the socket address in host byte order.

*/
 protected:
  int sa_len;
  union
  {
    struct sockaddr    sock;
    struct sockaddr_in sock_inet;
  } u;
};

/*
1.1 Class "SocketRule"[1]

*/

class SDB_EXPORT SocketRule
{
 public:
  enum Policy { DENY, ALLOW };
/*
Is an enumeration of access policies:

  *  ~ALLOW~ -- specifies that access should be granted when a host address
matches the rule.

  *  ~DENY~ -- specifies that access should be denied when a host address
matches the rule.

*/
  SocketRule();
/*
Creates an empty rule.

*/
  SocketRule( const string& strIpAddr,
              const string& strIpMask,
              const Policy setAllowDeny = ALLOW );
/*
Creates a rule initialized by the given IP address ~strIpAddr~ and IP
address mask ~strIpMask~ and the access policy ~setAllowDeny~.

*/
  virtual ~SocketRule() {};
/*
Destroys a rule.

*/
  bool Match( const SocketAddress& host );
/*
Checks whether the IP address ~host~ matches the rule. The access policy
is ignored.

*/
  bool Allowed( const SocketAddress& host );
/*
Checks whether access should be allowed for the IP address ~host~.

*/
  bool Denied( const SocketAddress& host );
/*
Checks whether access should be denied for the IP address ~host~.

*/
  static void SetDelimiter( const char newDelimiter = '/' );
/*
Sets the delimiter used between IP address, IP address mask and access
policy when a rule is sent onto an output stream.

*/
  static char GetDelimiter();
/*
Returns the current delimiter used between IP address, IP address mask
and access policy when a rule is sent onto an output stream.

*/
  friend ostream& operator <<( ostream& os, SocketRule& r );
/*
Allows to send a rule to an output stream.

*/
 protected:
  Policy  allowDeny;     // Access policy
  in_addr ipAddr;        // IP address of the rule
  in_addr ipMask;        // IP address mask
  static char delimiter; // Output delimiter
};

/*
1.1 Class "SocketRuleSet"[1]

*/

class SDB_EXPORT SocketRuleSet
{
 public:
  SocketRuleSet( SocketRule::Policy initDefaultPolicy =
                   SocketRule::DENY );
/*
Creates an empty rule set with a default access policy ~initDefaultPolicy~.

*/
  virtual ~SocketRuleSet() {};
/*
Destroys a rule set.

*/
  void AddRule( const string& strIpAddr,
                const string& strIpMask,
                SocketRule::Policy allowDeny =
                  SocketRule::ALLOW );
/*
Adds a rule consisting of the IP address ~strIpAddr~, the IP address mask
~strIpMask~ and the access policy ~allowDeny~ to the rule set.

*/
  bool Ok( const SocketAddress& host );
/*
Checks whether access should be granted for the IP address ~host~.

*/
  bool LoadFromFile( const string& ruleFileName );
/*
Loads a set of rules from the file with name ~ruleFileName~.
The method returns "true"[4] when the file could be read successfully.

*/
  bool StoreToFile( const string& ruleFileName );
/*
Stores a set of rules into the file with name ~ruleFileName~.
The method returns "true"[4] when the file could be written successfully.

*/
  friend ostream& operator <<(ostream& os, SocketRuleSet& r);
/*
Allows to send a set of rules to an output stream.

*/
 protected:
   vector<SocketRule> rules;         // Set of rules
   SocketRule::Policy defaultPolicy; // Access policy for set
};

/*
1.1 Class "SocketBuffer"[1]

The class "SocketBuffer"[1] implements the standard *streambuf* protocol
for sockets. Separate buffers for reading and writing are implemented.

*/

class SDB_EXPORT SocketBuffer : public streambuf
{
 public:
  SocketBuffer( Socket& socket );
/*
Creates a socket buffer associated with the socket ~socket~.

*/
  ~SocketBuffer();
/*
Destroys a socket buffer.

*/
  bool is_open() const { return socketHandle != 0; }
/*
Checks whether the stream buffer is ready for operation.

*/
  SocketBuffer* close();
/*
Closes the stream buffer.

*/
  streampos seekoff( streamoff, ios::seekdir, unsigned int )
    { return EOF; }
/*
Disallows seeking in the stream buffer since a TCP stream is strictly
sequential.

*/
  streamsize xsputn( const char* s, const streamsize n );
/*
Allows faster writing onto the socket of a string consisting on ~n~ characters.

*/
  streamsize xsgetn( char* s, const streamsize n );
/*
Allows faster reading from the socket of a string consisting on ~n~ characters.

*/
 protected:
  virtual int overflow( int ch = EOF );
/*
Writes the data in the output buffer to the associated socket.

*/
  virtual int uflow();
  virtual int underflow();
/*
Tries to read data from the associated socket, when the input buffer is empty.

*/
  virtual int sync();
/*
Flushes all output data from the buffer to the associated socket.

*/
  virtual int pbackfail( int ch = EOF );


  streamsize showmanyc() { cerr << "showmanyc called!" << endl; return 0; };

  streampos seekpos ( streampos sp,
                      ios_base::openmode which = ios_base::in | ios_base::out )
  {
    cerr << "streampos called!" << endl;
    return EOF;
  };

  streambuf * setbuf ( char * s, streamsize n )
  {
      cerr << "setbuf called!" << endl; return this;
  };
  void imbue ( const locale & loc ) { cerr << "imbue called!" << endl; };


/*
Disallows to unget a character.

*/
 private:
  SocketBuffer( const SocketBuffer& );
  SocketBuffer& operator=( const SocketBuffer& );

  Socket* socketHandle; // Handle of associated socket
  int     bufferSize;   // Size of the I/O buffer
  char*   inBuffer;     // Input buffer
  char*   outBuffer;    // Output buffer
};

/*

1.2 UDP-Sockets

UDP (User Datagram Protocol) is a connectionless transmission network protocol.
Its advantage is, that it is faster than TCP. Its disadvantage is, that there is
no guarantee of correctness, corect ordering or reliabilty of deliverance of
data sent.

Instead of sending data streams between two connected sockets, UDP allows to
send ~datagrams~ to remote sockets and receive such data.

Initialization and finalization is already done by the TCP/IP part in this file.

*/

#ifndef UDP_MAXBUF
#define UDP_MAXBUF 1048576 // set bufferlength to 1MB
#endif

struct addrinfo* CloneAddrInfo(const struct addrinfo *orig);
/*
A function to create a clone of a ~struct addrinfo~

*/

/*
1.2.1 Class ~UDPaddress~

*/

class SDB_EXPORT UDPaddress
{
  public:
    UDPaddress();
/*
Creates a new ~UDPaddress~ automatically determining the own IP-address and
some standard port number.

*/
    UDPaddress(const struct addrinfo *addr);
/*
Creates a ~UDPaddress~ from a given ~struct addrinfo~, that may provide
information on IP-version, IP address, port number, flags, etc.

*/
    UDPaddress(const struct sockaddr_in      *addr);
    UDPaddress(const struct sockaddr_in6     *addr);
    UDPaddress(const struct sockaddr_storage *addr);
/*
Creates a ~UDPaddress~ from a given valid ~struct sockaddr\_in~/~sockaddr\_in6~/~sockaddr\_storage~

*/
    UDPaddress(const UDPaddress &addr);
/*
Copy constructor.
Creates a ~UDPaddress~ from another given UDPaddress ~addr~.

*/
    UDPaddress(const string ip,
               const string port,
               const short int IPver = AF_UNSPEC);
/*
Creates a ~UDPaddress~ using the given IP address, port number, and ip-version.
IP-version is determined automatically. Valid values for IPver are AF\_INET,
AF\_INET6, or AF\_UNSPEC (default). When AF\_UNSPEC is used, the IP-version is
determined automatically from ~ip~ which may be an IP-address or a fully
qualified cononic host name.

*/
    ~UDPaddress();

    inline string getFamily() const
    { if(myFamily == AF_INET){
        return "IPv4";
      } else if(myFamily == AF_INET6){
        return "IPv6";
      }
      return "unknown";
    };
/*
Return the protocol family (IPv4, IPv6) associated with this address as a string.

*/
    inline int getFamilyI() const                  { return myFamily;   };
    inline int getSockTypeI() const                { return SOCK_DGRAM; };
    inline size_t getAddrLenI() const              { return myAddrlen;  };
    inline const struct sockaddr *getAddrI() const { return myAddr;     };
/*
Get code for IP-family, socket type, protocol type, address type length, and
address. Used for calls of ~socket()~, connect(), etc.

*/
    inline string getIP() const { return myIP; };

/*
Return the associated IP address in string format.
Two versions: as integer (for socket()) and text.

*/
    inline string getHostName() const { return myCanonname; };
/*
Return the fully qualified host name as a string.

*/
    inline string getPort() const      { return myPort; };
/*
Return the associated port number in string format.

*/
    inline bool isOk() const           { return myOk; };
/*
Returns true, iff the instance has been created successfully.
Error descriptions can be accessed by means of ~getErrorText()~.

*/
    string getErrorText() const        { return myErrorMsg; };
/*
Returns the error description.

*/
    UDPaddress& operator=(const UDPaddress& addr);
/*
The assignment operator

*/
    ostream& Print(ostream& o) const;

  protected:
    bool   myOk;
    string myErrorMsg;
    int    myFamily;
    size_t myAddrlen;
    struct sockaddr *myAddr;
    string myIP;
    string myPort;
    string myCanonname;

    bool updateMemberVariables();
    bool updateMemberVariables(const struct addrinfo *myAddrInfo);

    static string getErrorCodeStr(const int resGetaddrinfo);
};

ostream& operator<<(ostream &o, const struct sockaddr_in  &a);
ostream& operator<<(ostream &o, const struct sockaddr_in6 &a);
ostream& operator<<(ostream &o, const struct sockaddr_storage &a);
ostream& operator<<(ostream &o, const struct sockaddr     &a);
ostream& operator<<(ostream &o, const UDPaddress          &a);
ostream& operator<<(ostream &o, const struct addrinfo     &a);
ostream& operator<<(ostream &o, const struct protoent     &p);


/*
1.2.1 Class ~UDPsocket~

*/

enum UDPSocketState {UDPVOID=0,   // Invalid socket (not yet created)
                     UDPFRESH=1,  // newly created but unbound socket
                     UDPNORECV=2, // bound socket: no receive - send only
                     UDPNOSEND=3, // bound socket: no send - receive only
                     UDPNONE=4,   // bound socket: may neither send, nor receive
                     UDPALL=5};   // bound socket: may send and receive

class SDB_EXPORT UDPsocket
{
  public:
    UDPsocket();
/*
Initially clears ~ok~ and ~errorMsg~.
Create a fresh ~UDPsocket~ in state UDPVOID.

*/
    UDPsocket(const UDPaddress &address);
/*
Initially clears ~ok~ and ~errorMsg~.
Create a ~UDPsocket~ using the data from UDPaddress. The socket will be in state
UDPVOID (if ~address~ in invalid) or UDPFRESH.

*/

  UDPsocket(const UDPsocket &sock);
/*
Copy constructor

*/

    ~UDPsocket();

    bool bind();
/*
Initially clears ~ok~ and ~errorMsg~.
Bind the socket to a local port as referenced by protected member ~myAddress~.
If successful, the socket state is changed to UDPALL. On failure, state will be
UDPFRESH. Status transitions:

----
UDPVOID --> UDPVOID
{UDPFRESH, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL} --> UDPALL
----

Returns ~true~ iff the socket is bound at the end of this function.

*/
    bool connect(const UDPaddress &remote);
/*
Initially clears ~ok~ and ~errorMsg~.
Connect to a ~remote~ address. UDPsocket do not need to be connected to work
with them, but they are allowed to do so. Returns true, if remote is
~status~ remains unchanged.

~status~ remains unchanged.

*/

    int writeTo(const UDPaddress &receiver, const string &message);
/*
Initially clears ~ok~ and ~errorMsg~.
Send datagram ~message~ to the address specified by ~receiver~.
Returns the amount of transmitted bytes. If <0, an error occured.

----
UDPFRESH --> UDPALL
UDPVOID, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL: unchanged
----

*/

    int write(const string &message);
/*
Initially clears ~ok~ and ~errorMsg~.
Send datagram ~message~ to the connected remote socket. Requires the socket to
be bound to a local port and connected to a remote socket.

Returns the amount of transmitted bytes. If <0, an error occured.

----
UDPVOID, UDPFRESH, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL: unchanged
----

*/

    string readFrom(UDPaddress &sender, const double timeoutSecs);
/*
Initially clears ~ok~ and ~errorMsg~.
Binds the UDPsocket to the specified port (if necessary) and waits for data
for up to ~timeoutSecs~ seconds. Negative ~timeoutSecs~ results in blocking.

~sender~ is changed to the address of the sender. If a timeout occurs, ~sender~
remains unmodified.

If the socket is not already bound to a local port, this is done.

The received message is returend as a string. If an error or timeout occurs,
the result is an empty string.

----
UDPVOID, UDPALL --> UDPVOID
UDPFRESH, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL: unchanged
----

*/
    string read(const double timeoutSecs);
/*
Initially clears ~ok~ and ~errorMsg~.
Receive a datagram message (return value) to the connected remote socket.
Requires the socket to be bound to a local port and connected to a remote socket.

Waits for data for up to ~timeoutSecs~ seconds. Negative ~timeoutSecs~ results
in blocking.

The received message is returend as a string. If an error or timeout occurs,
the result is an empty string and an according error message is passed.

----
UDPVOID, UDPALL --> UDPVOID
UDPFRESH, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL: unchanged
----

*/
    bool close();
/*
Initially clears ~ok~ and ~errorMsg~.
Close the socket. Returns true, iff the socket was closed and reaches status
UDPFRESH.

----
UDPVOID --> UDPVOID
UDPFRESH, UDPNORECV, UDPNOSEND, UDPNONE, UDPALL --> UDPFRESH
----

*/
    bool shutdown(UDPSocketState how);
/*
Initially clears ~ok~ and ~errorMsg~.
Shut down the socket. Returns the state of ~ok~.
~how~ must be from UDPNORECV, UDPNOSEND, UDPNONE, UDPALL.
Returns ~false~ for invalid argument or if status is in UDPVOID, UDPFRESH.

----
UDPVOID --> UDPVOID
UDPFRESH --> UDPFRESH
UDPNORECV, UDPNOSEND, UDPNONE, UDPALL -->
----

*/

    inline bool isOk() const { return ok; };
/*
Check whether some error has occured on this socket. If ~true~, call ~getErrorText()~
to get the last error message.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/
    inline string getErrorText() const { return errorMsg; };
/*
Return the last error message for this UDPsocket.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/
    inline UDPaddress getAddress() const { return myAddress; };

/*
Return this socket's address.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/

    inline UDPaddress getPartnerAddress() const { return partnerAddress; };
/*
Return this socket's partner's address. If the socket is not connected, for the
result address !isOk() will hold.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/

    inline bool isConnected() const { return connected; };
/*
Returns true, iff this socket is connected to a remote host.
Only if it is connected, methods ~write()~ and ~read()~ can be used.
Use methods ~writeTo()~ and ~readFrom()~, otherwise.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/

    inline UDPSocketState getStatus() const { return status; };
/*
Returns the sockets's state.

Does not change ~status~, ~ok~ or ~errorMsg~.

*/

    UDPsocket& operator=(const UDPsocket& sock);
/*
Assignment operator

*/

    ostream& Print(ostream& o) const;

  protected:
    UDPSocketState status;          // socket state
    bool connected;                 // true, iff in connected mode
    bool ok;                        // false, iff an error occured during the
                                    //  last member function call. Should be set
                                    //  set to true at the beginning of such
                                    //  functions.
    SocketDescriptor mySocket;      // socket handles
    UDPaddress partnerAddress;      // current communication partner
    UDPaddress myAddress;           // address info used to create
                                    //    the socketHandle
    string errorMsg;                // last error message
};

ostream& operator<<(ostream &o, const UDPsocket &s);

#endif

