/*
---- 
This file is part of SECONDO.

Copyright (C) 2007, University in Hagen, 
Faculty of Mathematics and Computer Science, 
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


//characters  [1]  verbatim:  [\verb@] [@]


Feb. 2007, M. Spiekermann: Documentation of the Client-Server-Protocol. New
protocol tag __<Message>__ introduced. 


1 Protocol Overview

The client and the server interchange messages as byte sequences which will be
sent over a TCP-IP socket. A message is a simple tag of the pattern "<tag/>"[1]
or a pair of start and end tags "<tag>...</tag>". After each tag a newline
symbol "\n" must be sent. Between a paired tag some message dependent data
will be delivered.


2 Connecting

After the connection is established the server confirms it.

Server:

----
    <SecondoOK/>\n
----

Then the client needs to send an authorization. Currently username and
password are send unencrypted. If authorization is decativated
(see SecondoConfig.ini) the received information is ignored.

Client:

----
    <Connect>\n
    user\n
    password\n
    Key=Value\n
    ...
    </Connect>\n
----

The block is "lines until </Connect>": the first is the user, the second the
password, and every further line is a ~Key=Value~ datum. Exactly one key is
defined, and it is not optional:

----
    ProtocolVersion=2\n
----

Again, the server will confirm this.


Server:

----
    <SecondoIntro>\n
    You are connected with a Secondo server.\n
    ProtocolVersion=2\n
    BinaryTransfer=YES|NO\n
    </SecondoIntro>\n
----

The ~BinaryTransfer~ line states whether this server transfers lists in binary
form or as text (see ~csp::BINARY\_TRANSFER\_TAG~ below); the client adopts it
instead of consulting its own configuration, and refuses a server that omits
it. Any other line is informational.

The ~ProtocolVersion~ line states which client/server protocol this server
speaks (~csp::PROTOCOL\_VERSION~). It is checked in *both* directions -- the
client sends its version in ~<Connect>~ and the server sends its version here
-- because neither check alone fails fast. A client older than the version line
just prints it: ~applyIntroLine~ returns false for lines it does not recognise
and callers print the rest, so such a client would sail past the announcement
and desynchronise on the first result instead. Since the version the server
reads from ~<Connect>~ is the one it can act on before writing anything, that is
where a mismatch is refused; the intro line is what lets a *new* client refuse
an old server.

There is no compatibility mode. Version 2 changed the result framing (section 3),
so a mismatched pair would not merely misread one answer, it would lose the
framing for the rest of the connection. Both ends therefore refuse to proceed.

or returns an error, which will be sent in a single line. In the future there
could be send more specific information about a server. The error message does
not contain the error of a specific secondo command instead it indicates a
protocol error.

Server:

----
    <SecondoError>\n
    message\n
    ...
    </SecondoError>\n
----

The message runs until the end tag and may be several lines -- the version
mismatch above is.

Afterwards ther server waits for client requests.

3 Running Commands

Afterwards the Client can send one of the requests explained below.
The most interesting one is to send a Secondo command to the server.

----
    <Secondo>\n
    cmdLevel[ flag]...\n
    line1\n
    ...
    lineN\n
    </Secondo>\n
----

"cmdLevel" is an integer: 0 (list syntax), 1 (SOS syntax) or 2 (SQL, which the
server hands to its embedded optimizer). The command itself can be wrapped into
several lines. Every command which is known by the SecondoInterface (see
"SecondoInterface.h") can be used. Note: The command cannot contain the line
"</Secondo>".

The rest of the level line carries per-command protocol flags, a part the
server has always discarded, so a flag is invisible to a peer that does not
know it. Two are defined (see "SQLLanguage.h"): "planonly" stops an SQL command
after optimizing, and "optimizer" says the user addressed the optimizer
explicitly.

A client that does not want to decide which of the three languages a typed
command is written in sends the level -1 instead and lets the server classify
it. The server then answers with the level it resolved to, ahead of everything
else:

----
    <CommandLevel>level</CommandLevel>\n
----

where "level" is 0, 1, 2 or 3 (an optimizer control goal, whose result is the
text it printed). This line is written only for a client that asked, so the
protocol is unchanged for one sending an explicit level.

3.1 The result frame

The answer is a *frame*: a sequence of length-delimited records carrying the
result list, closed by a record that carries the status.

----
    <SecondoResult>\n
    record*
    terminator
    </SecondoResult>\n
----

A record is one ASCII tag byte, space-separated decimal fields and a "\n",
followed by the body. The body is **not** newline-terminated -- the next
record's tag byte follows its last payload byte immediately.

----
    tag  header                            body

    D    D <n>\n                           n bytes of the result
    M    M <n>\n                           n bytes: one complete nested list
    E    E <errorCode> <errorPos> <m>\n    m bytes of message text
    A    A <errorCode> <errorPos> <m>\n    m bytes of message text
----

  * "D" -- *data*. A piece of the result. The result is the concatenation of
    all "D" bodies and nothing else, encoded in the connection's transfer mode,
    textual or binary (see ~BinaryTransfer~ above). Unlike protocol version 1
    the status is no longer the first three elements of the transmitted list. A
    command with no result still writes a payload: the encoding of the empty
    list, so the reader has one rule and no special case. The server emits at
    most ~csp::RESULT\_CHUNK\_SIZE~ bytes per record; a reader must accept any
    "n", including 0.

  * "M" -- *message*. A Secondo runtime message (the ~<Message>~ block of
    section 3.2) that was raised while the result was already being written,
    and so could not be sent as a block. Its body is **not** part of the
    result: it is a nested list of its own, complete in this one record, in the
    same transfer mode as the result. The reader hands it to the message
    handler and carries on reassembling the result from the "D" records around
    it, as if the "M" had not been there. See 3.2 for why it exists.

  * "E" and "A" -- the *terminator*. Exactly one of them ends every frame, and
    it is the last record. See below.

A whole frame, in textual transfer mode so that it can be read here -- a result
of 90000 bytes interrupted once by a message, then ending successfully:

----
    <SecondoResult>\n
    D 65536\n  <65536 bytes>          first chunk of the result
    D 111\n    <111 bytes>            flushed early to make room for ...
    M 23\n     (simple ("100 tuples"))    ... this message
    D 24353\n  <24353 bytes>          the result continues where it left off
    E 0 0 0\n                         success, no message text
    </SecondoResult>\n
----

The result is 65536 + 111 + 24353 = 90000 bytes: the "M" body is not part of it,
and neither are the five record headers. Reading it back is
"take the length from the header, then take that many bytes" over and over --
no scanning for delimiters, and nothing in a body that has to be escaped.

"E" and "A" are the point of the whole frame. "E" says the "D" bytes are a
complete encoding and ~errorCode~ is the command's outcome. "A" says the server
stopped early: the bytes are an incomplete encoding, the client must discard
them rather than parse them, and ~errorCode~ says why. In version 1 the status
came *before* the result, so a server that streamed a result had to promise
success before it knew, and the binary encoding's length prefix had already
promised exactly N elements with no way to retract; the only remaining move was
to drop the connection. The frame retracts on the encoding's behalf.

Because the framing is length-delimited, resynchronisation is decoupled from
parsing the payload: whatever the list reader makes of the bytes, the frame
reader can always drain records to the terminator and consume
"</SecondoResult>". The connection therefore survives an error that version 1
could only report by closing the socket.

3.2 Messages

After a "<Secondo>" request the server may send some information to the client.
These contain a list of any structure. The client (in particular the
  implementation of Class ~MessageHandler~) has to care about it.

----
    <Message>
    list\n
    </Message>
----

Such blocks appear *before* the "<SecondoResult>" frame, and that is the only
place they can appear. Once the frame is open every byte on the socket belongs
to a record, so a block written into it would be read as result data and the
connection would lose the framing from there on.

A message can nonetheless be raised after the frame is open -- an attribute's
~Out~ calling ~cmsg.send()~ while its tuple is being written, for instance. That
is what the "M" record is for: the server flushes whatever payload has
accumulated as a "D" record, writes the message as one "M" record, and resumes
filling "D" records where it left off. Chunk boundaries are arbitrary, so the
early flush costs nothing, and the reader sees the same byte sequence for the
result either way.

Version 1 had no such record. Its only option was to suppress messages for the
whole command -- which is what the server did, and which is why the sink that
streamed a result had to turn them off before writing the first tuple.


4 Retrieving operator information

It is possible to ask Secondo for some unique identificators of an 
operator value mapping. The input is the operator's name and a list 
containing the arguments.

----
   <startGetOperatorIndexes>
     opname\n
     list\n
   <endGetOperatorIndexes>
----

The respose to this query are three numbers identifying the value map. and a 
nested list represening the result type.

----
   <startGetOperatorIndexesResponse>
     int\n
     int\n
     int\n
     list\n
   <endGetOperatorIndexesResponse>
----


5 CostEstimations

5.1 GetCosts

The client may ask for estimating costs for an operator.

----
  <GETCOSTS>\n
    nostreams\n
    algId\n
    opId\n\
    funId\n
   ( noTuples\n
    sizeOfTuple\n ) ^ nostreams
    memory\n
  </GETCOSTS>\n
----

All values are integer values. The first integer (nostreams) determines
how many noTuples/sizeOfTuple pairs are sent to the kernel. 

The server will answer by:

----
  <COSTRESPONSE>\n
     success\n
     cost\n
  </COSTRESPONSE>\n
----

success and cost are returned as integer values. 


6 Get Linear Cost Function

Client's request

----   
  <GETLINEARCOSTFUN>\n
     nostreams\n
     algId\n
     opId\n
     funId\n
    ( notuples\n
      sizeofTuple\n ) ^ nostreams
  </GETLINEARCOSTFUN>\n
----

Server's answer:

----
   <LINEARCOSTFUNRESPONSE>\n
       success\n
       sufficientMemory\n
       timeAtSuffMemory\n
       timeAt16MB\n  
   </LINEARCOSTFUNRESPONSE>\n
----

All returned values are doubles.



7 Get detailed cost functions

Client's request

----   
  <GETCOSTFUN>\n
     nostreams\n
     algId\n
     opId\n
     funId\n
    ( notuples\n
      sizeofTuple\n ) ^ nostreams
  </GETCOSTFUN>\n
----

Server's answer:

----
   <COSTFUNRESPONSE>\n
       success\n
       funType\n
       sufficientMemory\n
       timeAtSuffMemory\n
       timeAt16MB\n  
       a\n
       b\n
       c\n
       d\n
   </COSTFUNRESPONSE>\n
----

The funType is an integer values, all other values are doubles.



4 Catalog Information

Deprecated! Will be removed in future versions.

These kinds of client requests are very special. Sometimes it may
be necessary to get internal information about a specific type, 
if so the messages below are needed.

Please refer to the implementation for further details.  

----
<NumericType>\n 
  type (string)\n
</NumericType>\n

<NumericTypeResponse>\n 
  outlist (textual list)
</NumericTypeResponse>\n 
----

----
<GetTypeId>\n 
  type (string)\n
</GetTypeId>\n

<GetTypeIdResponse>\n 
  algebraId TypeId (two int values separated by a space)\n
</GetTypeIdResponse>\n 
----

----
<LookUpType>\n 
  typeExpression (textual list)\n
</LookUpType>\n

<LookUpTypeResponse>\n
  ((name) algebraId typeId) (textual list)\n
</LookUpTypeResponse>\n
----

5 Save and Restore

It is possible to interchange objects or databases between the client and the
server site. Hence you can use the client to restore objects or databases or to
save object or databases. Some special messages are needed since the usual
Secondo-commands for this purpose assume that the files are on the server's
site. All of the requests below will return a "<SecondoResult>" frame.


The save requests will return a Secondo result list which is the list
representation of an object or database.

----
<ObjectSave>\n
  name (string)\n
</ObjectSave>\n

<DbSave/>
----

The restore requests will return the ~normal~ result lists of the corresponding
Secondo commands. These requests need to transmit a file to the server. 

----
    <DbRestore>
    dbName\n
    filename\n
    <FileData>\n
    N\n
    byte1 ... byteN
    </FileData>\n
    </DbRestore>\n
----

----
    <ObjectRestore>
    objName\n
    filename\n
    <FileData>\n
    N\n
    byte1 ... byteN
    </FileData>\n
    </ObjectRestore>\n
----

The values of "dbName", "objName" and "fileName" are strings. The value
 "N" indicates the file size in bytes followed by the bytes of the file.

6 File Transfer 

For transfer a file from client to the server (for example for importing it),
 the client sends

----
   <FileTransfer>\n
   filename\n
----

Depending wether overwriting of files is allowed or not, the next line 
sent to the server is.

----
   <ALLOW_OVERWRITING>\n
----

or
  
----
  <DISALLOW_OVERWRITING>\n
----

The answer of the server in case of an error is.

----
  <SecondoError>\n
   ErrorMessage \n
  </SecondoError>\n
----

If there are no problems up to now, the server answers:

----
  <SecondoOK>
----

In this case, the client sends the file to the server:

----
   <FileData>\n
   N\n
   byte1..byteN
   </FileData>\n
   </FileTransfer>
----

The filename is the name of the file created on server side. N is 
the size of the file.


The reverse way, i.e. requesting a file from the server, works as follows:

The client sends to the server:

----
   <RequestFile>\n
   filename\n
   </RequestFile>
----

The answer of the server is:

----
   <FileData>\n
   N\n
   byte1...byteN
   </FileData>
----

in case of successful access to the file or in case of an error

----
    <SendFileError>\n
----


7 Disconnecting

The client can close the connection by sending

----
<Disconnect/>/n
----

*/


#ifndef SEC_CSProtocol_H
#define SEC_CSProtocol_H

#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <cstdlib>

//#define TRACE_ON 1
#undef TRACE_ON
#include "LogMsg.h"
#include "ErrorCodes.h"
#include "NestedList.h"
#include "Messages.h"
#include "TraceMacros.h"
#include "limits.h"
#include "DebugWriter.h"


extern DebugWriter dwriter;


/*
Utility functions
   
*/   

namespace csp {

/*
Write ~list~ onto ~os~ the way this connection transfers lists: as text when
~binary~ is false, in the binary encoding otherwise.

~binary~ is a parameter and not read from the ~Server:BinaryTransfer~ runtime
flag, which is what this used to do. That flag is process-global, while the
transfer mode is a property of one connection (see ~CSProtocol~): a client
holding connections to two servers that disagree needs both of them right, and
on a client the flag says nothing about what the peer does anyway.

*/
void
sendList(std::ostream& os, NestedList* nl, ListExpr list, bool binary);

/*
How the server announces its list transfer mode in the ~<SecondoIntro>~ block:
the line is ~BinaryTransfer=YES~ or ~BinaryTransfer=NO~. The server writes it
(SecondoServer.cpp), the client reads it and adopts it in place of its own
Server:BinaryTransfer setting (SecondoInterfaceCS.cpp). Both sides used to take
that setting from their own configuration file with nothing on the wire to
agree on it, and a disagreement did not fail -- it deadlocked, each side
waiting for the other. The client requires the line, so a server that does not
send it is refused rather than hung on.

*/
const std::string BINARY_TRANSFER_TAG = "BinaryTransfer=";

/*
The client/server protocol this build speaks. Sent by the client as a datum of
the ~<Connect>~ block and by the server as a line of the ~<SecondoIntro>~
block, and refused by either end on a mismatch -- see section 2 of the protocol
description above for why both directions are checked.

Mirrored by ~PROTOCOL\_VERSION~ in
"Javagui/secondoInterface/sj/lang/SecondoInterface.java"; the two must agree.
Nothing enforces that at build time on purpose: because a mismatch is a hard
refusal rather than a degraded mode, a drifted Java constant makes ~tools.CSTest~
(run by "ClientServer/TestClientServer") fail to connect at all. That is a
stronger guard than a grep over the two files, which could only compare the
literals and not the frame readers behind them.

*/
const std::string PROTOCOL_VERSION_TAG = "ProtocolVersion=";
const int         PROTOCOL_VERSION     = 2;

/*
Largest payload the server puts in one ~D~ record. Nothing depends on the value:
a reader takes the length from the record. At 165 MB it costs about 2524 records
of some ten bytes of header each, around 0.015 per cent.

*/
const size_t      RESULT_CHUNK_SIZE    = 65536;

/*
Where an ~M~ record found inside a result frame is delivered. Implemented by
~CSProtocol~, which knows the connection's transfer mode and the message
handler; ~ChunkedInBuf~ only knows it read some bytes.

*/
class FrameMessageSink
{
 public:
  virtual ~FrameMessageSink() {}
  virtual void frameMessage( const std::string& body ) = 0;
};

/*
The writing half of the result frame: an ~std::streambuf~ that turns whatever is
written to it into ~D~ records on the underlying stream.

A producer therefore needs to know nothing about the framing -- ~Relation::OutStreamed~
writes tuples to an ~std::ostream~ exactly as it did when that stream was the
socket itself. The buffer is what makes chunk boundaries arbitrary, which in
turn is what makes ~emitMessage~ possible.

*/
class ChunkedOutBuf : public std::streambuf
{
 public:
  explicit ChunkedOutBuf( std::ostream& sink,
                          size_t chunkSize = RESULT_CHUNK_SIZE )
    : out( sink ), buffer( chunkSize > 0 ? chunkSize : 1 )
  {
    setp( &buffer[0], &buffer[0] + buffer.size() );
  }

  ~ChunkedOutBuf() { sync(); }

/*
Flush whatever payload has accumulated as a ~D~ record, then write ~body~ as an
~M~ record. Called when a message is raised while the result is being written;
the partial ~D~ costs nothing because a reader takes every chunk's length from
its header.

*/
  bool emitMessage( const std::string& body )
  {
    if ( sync() != 0 ) {
      return false;
    }
    out << 'M' << ' ' << body.size() << '\n';
    out.write( body.data(), body.size() );
    return out.good();
  }

/*
Close the frame: flush the last ~D~ record and write the terminator. ~kind~ is
'E' when the payload is a complete encoding and 'A' when the writer stopped
early -- see section 3.1 above.

*/
  bool writeTerminator( char kind, int errorCode, int errorPos,
                        const std::string& message )
  {
    sync();
    out << kind << ' ' << errorCode << ' ' << errorPos << ' '
        << message.size() << '\n';
    out.write( message.data(), message.size() );
    return out.good();
  }

 protected:
  int_type overflow( int_type c ) override
  {
    if ( sync() != 0 ) {
      return traits_type::eof();
    }
    if ( !traits_type::eq_int_type( c, traits_type::eof() ) ) {
      *pptr() = traits_type::to_char_type( c );
      pbump( 1 );
    }
    return traits_type::not_eof( c );
  }

  int sync() override
  {
    const std::streamsize n = pptr() - pbase();
    if ( n > 0 ) {
      out << 'D' << ' ' << n << '\n';
      out.write( pbase(), n );
      setp( &buffer[0], &buffer[0] + buffer.size() );
    }
    return out.good() ? 0 : -1;
  }

 private:
  std::ostream&     out;
  std::vector<char> buffer;
};

/*
The reading half: an ~std::streambuf~ that pulls records off the socket and
serves the ~D~ bodies as one continuous byte stream, so a list reader sees the
payload and never the framing.

This adapter is not a convenience. Without it a client would have to collect the
whole payload before it could parse any of it -- 165 MB for "query roads" -- and
that is a straight regression against reading the list incrementally off the
socket, which is what version 1 did. ~M~ records are dispatched as they are met;
the terminator ends the stream and is left available through ~terminatorKind~
and friends.

*/
class ChunkedInBuf : public std::streambuf
{
 public:
  ChunkedInBuf( std::istream& src, FrameMessageSink* sink,
                const std::string& endTag = "</SecondoResult>" )
    : in( src ), msgSink( sink ), endResultTag( endTag ),
      buffer( RESULT_CHUNK_SIZE ), finished( false ), broken( false ),
      kind( 0 ), errorCode( 0 ), errorPos( 0 )
  {
    setg( 0, 0, 0 );
  }

/*
Read records, discarding payload, until the terminator has been consumed. This
is what makes the connection survive a payload the list reader could not make
sense of: framing is length-delimited, so draining never has to understand the
bytes it is skipping.

*/
  bool drainToTerminator()
  {
    setg( 0, 0, 0 );
    while ( !finished && !broken ) {
      readRecord();
      setg( 0, 0, 0 );
    }
    return !broken;
  }

  bool ok() const { return !broken; }
  bool atEnd() const { return finished; }

/*
The terminator, valid once the stream has ended: 'E' if the payload is complete,
'A' if the server aborted mid-result, 0 if the frame itself broke.

*/
  char terminatorKind() const { return kind; }
  int  terminatorErrorCode() const { return errorCode; }
  int  terminatorErrorPos() const { return errorPos; }
  const std::string& terminatorMessage() const { return message; }

 protected:
  int_type underflow() override
  {
    if ( gptr() < egptr() ) {
      return traits_type::to_int_type( *gptr() );
    }
    while ( !finished && !broken ) {
      readRecord();
      if ( gptr() < egptr() ) {
        return traits_type::to_int_type( *gptr() );
      }
    }
    return traits_type::eof();
  }

 private:
/*
Read exactly ~n~ bytes. Goes through the stream buffer rather than
~std::istream::read~ because a socket hands over what has arrived: a short read
is normal and must be looped on, and ~read~ would raise failbit on the way.

*/
  bool readFully( char* dst, size_t n )
  {
    size_t got = 0;
    while ( got < n ) {
      const std::streamsize r = in.rdbuf()->sgetn( dst + got, n - got );
      if ( r <= 0 ) {
        return false;
      }
      got += (size_t) r;
    }
    return true;
  }

/*
Consume one record. On ~D~ the get area is left pointing at its body; on ~M~ the
body is dispatched and the get area stays empty; on ~E~/~A~ the status is kept,
"</SecondoResult>" is consumed and the stream is finished. Anything else is a
protocol error, which finishes the stream too -- there is no way to find the
next record boundary once the framing is lost.

*/
  void readRecord()
  {
    char tag = 0;
    if ( !in.get( tag ) ) {
      fail();
      return;
    }
    if ( tag == 'D' || tag == 'M' ) {
      size_t n = 0;
      if ( !readHeader( 0, n ) ) {
        return;
      }
      if ( n > buffer.size() ) {
        buffer.resize( n );
      }
      if ( n > 0 && !readFully( &buffer[0], n ) ) {
        fail();
        return;
      }
      if ( tag == 'D' ) {
        setg( &buffer[0], &buffer[0], &buffer[0] + n );
      } else if ( msgSink != 0 ) {
        msgSink->frameMessage( std::string( &buffer[0], n ) );
      }
      return;
    }
    if ( tag == 'E' || tag == 'A' ) {
      size_t n = 0;
      if ( !readHeader( 2, n ) ) {
        return;
      }
      message.assign( n, '\0' );
      if ( n > 0 && !readFully( &message[0], n ) ) {
        fail();
        return;
      }
      kind = tag;
      errorCode = headerFields[0];
      errorPos = headerFields[1];
      finished = true;
      // The frame is over; the tag closing it is an ordinary line again.
      std::string line;
      std::getline( in, line );
      if ( line != endResultTag ) {
        broken = true;
      }
      return;
    }
    fail();
  }

/*
Read the ~count~ leading decimal fields into ~headerFields~ and the trailing
length into ~n~, then the newline that ends the header.

*/
  bool readHeader( int count, size_t& n )
  {
    for ( int i = 0; i < count; i++ ) {
      if ( !(in >> headerFields[i]) ) {
        fail();
        return false;
      }
    }
    long long len = -1;
    if ( !(in >> len) || len < 0 ) {
      fail();
      return false;
    }
    if ( in.get() != '\n' ) {
      fail();
      return false;
    }
    n = (size_t) len;
    return true;
  }

  void fail()
  {
    broken = true;
    finished = true;
    kind = 0;
    setg( 0, 0, 0 );
  }

  std::istream&      in;
  FrameMessageSink*  msgSink;
  const std::string  endResultTag;
  std::vector<char>  buffer;
  bool               finished;
  bool               broken;
  char               kind;
  int                errorCode;
  int                errorPos;
  int                headerFields[3];
  std::string        message;
};

/*
Write a complete result frame for a list that already exists -- the answer to
every command whose result was not streamed. The framing is the same
~ChunkedOutBuf~ the streaming path uses, so there is one implementation of it
and a client cannot tell the two apart.

*/
inline void
sendResultFrame( std::ostream& out, NestedList* nl, ListExpr list, bool binary,
                 int errorCode, int errorPos, const std::string& errorMessage )
{
  out << "<SecondoResult>" << std::endl;
  {
    ChunkedOutBuf chunks( out );
    std::ostream chunkStream( &chunks );
    sendList( chunkStream, nl, list, binary );
    chunkStream.flush();
    // A list that already exists was written whole, so the terminator is
    // always 'E' and errorCode is simply the command's outcome.
    chunks.writeTerminator( 'E', errorCode, errorPos, errorMessage );
  }
  out << "</SecondoResult>" << std::endl;
}

} // end of namespace



class ServerMessage : public MessageHandler {

  std::iostream& iosock;
  const std::string startMessage;
  const std::string endMessage;
  bool ignore;
  bool binary;
  // Non-zero while a result is being streamed: a message raised then cannot be
  // a <Message> block, because the block would land in the middle of the
  // payload and be read as list data. It becomes an M record in the frame
  // instead. Version 1 had no such record and had to drop the message.
  csp::ChunkedOutBuf* chunked;

  public:
  virtual bool handleMsg(NestedList* nl, ListExpr msg,
                         int source __attribute__((unused))) {

   if (chunked != 0) {
     std::ostringstream body;
     csp::sendList(body, nl, msg, binary);
     return chunked->emitMessage(body.str());
   }

   if (ignore) {
     std::cerr << "Warning: Last request was not <Secondo>! "
          << "Message will not be sent to the client." << std::endl
          << startMessage << std::endl
          << nl->ToString(msg)
          << endMessage << std::endl;
     return false;
   }

   //std::cerr << "Sending message ..." << std::endl;
   iosock << startMessage << std::endl;
   csp::sendList(iosock, nl, msg, binary);
   iosock << endMessage << std::endl;

   return true;
  }

  void Flush(){
    iosock.flush();
  }

  ServerMessage(std::iostream& ios, bool binaryTransfer = false) :
   iosock(ios),
   startMessage("<Message>"),
   endMessage("</Message>"),
   ignore(true),
   binary(binaryTransfer),
   chunked(0)
  {}

  ~ServerMessage() {}

  void ignoreMsg(bool value) { ignore = value; }

  void setBinaryTransfer(bool value) { binary = value; }

/*
Send messages as ~M~ records into ~buf~ until told otherwise; 0 restores the
ordinary ~<Message>~ blocks.

*/
  void redirect(csp::ChunkedOutBuf* buf) { chunked = buf; }

};



/*
4 struct CSProtocol

*/

struct CSProtocol : public csp::FrameMessageSink {

 private:
 std::iostream& iosock;
 const std::string err;
 bool ignoreMsg;
 NestedList* nl;
 ServerMessage* msgHandler;
 MessageCenter* msg;
 // The protocol version the peer announced, and whether it said anything at
 // all. Both ends refuse a peer whose version differs from
 // csp::PROTOCOL_VERSION or that never named one; see section 2 above.
 bool peerVersionKnown;
 int  peerVersion;
 // Set for the duration of one ReadFrame, so an M record met inside the frame
 // reaches the same handler a <Message> block before it would have.
 MessageHandler* frameMsgHandler;
 int             frameMsgSource;
 // How lists arrive on *this* connection (see BINARY_TRANSFER_TAG above). The
 // mode is a property of the connection, not of the process: one client may
 // hold connections to two servers that disagree, and keeping it here rather
 // than in the global Server:BinaryTransfer flag keeps both of them right --
 // and stops one connect from rewriting state another connection is reading.
 //
 // Where the value comes from depends on which end this is. The *server*
 // transfers lists the way its own configuration says and announces that in
 // the intro block, so it knows the mode from the moment it is built. A
 // *client* has to be told by the server it connected to, through
 // setBinaryTransfer. Reading a list before that has happened is a protocol
 // error and is refused -- guessing from the process-wide flag would silently
 // decode the stream the wrong way, which is the failure this negotiation was
 // introduced to remove.
 bool binaryTransferKnown;
 bool binaryTransfer;
 // Where a streamed result goes, or 0 for the usual whole-list reading. See
 // setResultSink.
 NestedList::BinaryListSink* resultSink;


 public:
 const std::string startFileData;
 const std::string endFileData;
 const std::string startObjectRestore;
 const std::string endObjectRestore;
 const std::string startDbRestore;
 const std::string endDbRestore;
 const std::string startResult;
 const std::string endResult;
 const std::string startMessage;
 const std::string endMessage;
 const std::string startError;
 const std::string endError;
 const std::string sendFileError;
 const std::string startRequestOperatorIndexes;
 const std::string endRequestOperatorIndexes;
 const std::string startOperatorIndexesResponse;
 const std::string endOperatorIndexesResponse;
 const std::string startFileTransfer;
 const std::string endFileTransfer;
 const std::string startRequestFile;
 const std::string endRequestFile;
 
 CSProtocol(NestedList* instance, std::iostream& ios, bool server = false) : 
   iosock(ios), 
   err("Protocol-Error: "),
   startFileData("<FileData>"),  
   endFileData("</FileData>"),
   startObjectRestore("<ObjectRestore>"),
   endObjectRestore("</ObjectRestore>"),
   startDbRestore("<DbRestore>"),
   endDbRestore("</DbRestore>"),
   // Deliberately not the version-1 tag <SecondoResponse>: the contents differ,
   // and Algebras/Distributed scans response text for that literal by hand. A
   // new tag turns "silently misparses" into "unrecognised tag on line 1",
   // which is the right failure when there is no compatibility mode.
   startResult("<SecondoResult>"),
   endResult("</SecondoResult>"),
   startMessage("<Message>"),
   endMessage("</Message>"),
   // <SecondoError> and not <Error>: the server has never written the latter,
   // so this branch was dead and an error arriving where a response was
   // expected lost its text and left the end tag in the stream.
   startError("<SecondoError>"),
   endError("</SecondoError>"),
   sendFileError("<SendFileError/>"),
   startRequestOperatorIndexes("<REQUESTOPERATORINDEXES>"),
   endRequestOperatorIndexes("</REQUESTOPERATORINDEXES>"),
   startOperatorIndexesResponse("<OPERATORINDEXESRESPONSE>"),
   endOperatorIndexesResponse("</OPERATORINDEXESRESPONSE>"),
   startFileTransfer("<FileTransfer>"),
   endFileTransfer("</FileTransfer>"),
   startRequestFile("<RequestFile>"),
   endRequestFile("</RequestFile>")
 {
   ignoreMsg = true;
   nl = instance;
   // The server goes by its own configuration -- that is what it announces to
   // the client. A client starts out not knowing and must be told.
   binaryTransferKnown = server;
   binaryTransfer = server && RTFlag::isActive("Server:BinaryTransfer");
   peerVersionKnown = false;
   peerVersion = 0;
   frameMsgHandler = 0;
   frameMsgSource = -1;
   resultSink = 0;
   msg = MessageCenter::GetInstance();

   // The message handler will send Secondo runtime messages
   // to the server.
   msgHandler = new ServerMessage(ios, binaryTransfer);
   if (server)
     msg->AddHandler(msgHandler);
   msgHandler->ignoreMsg(true);
 }


 ~CSProtocol(){
     msg->RemoveHandler(msgHandler);
     delete msgHandler;
  }

 /*
 Read the next result frame's payload into ~sink~ instead of into one list, so
 that a consumer who walks it once never has it all at the same time. 0 puts
 the usual whole-list reading back.

 Only a *binary* result can be streamed, and only one of the shape
 ~(type value)~ (see ~NestedList::ReadBinaryStreamed~). Everything else is
 read whole with the sink left untouched, so a caller has to be ready for
 both -- which is why the sink is asked afterwards whether it was used, rather
 than told beforehand that it will be.

 The sink is *not* cleared after a frame: it stays until it is replaced. A
 caller that installs one owns clearing it, and must not let it outlive the
 object it points at.

 */
 void setResultSink( NestedList::BinaryListSink* sink )
 {
   resultSink = sink;
 }

 /*
 Adopt the transfer mode the server announced for this connection. Called by
 the client once, while reading the intro block, before any list is read.

 */
 void setBinaryTransfer(const bool binary)
 {
   binaryTransfer = binary;
   binaryTransferKnown = true;
   msgHandler->setBinaryTransfer(binary);
 }

 bool usesBinaryTransfer() const
 {
   return binaryTransfer;
 }

/*
Whether the transfer mode is settled for this connection. False on a client
that has not yet read the server's announcement; reading a list in that state
is refused rather than guessed at.

*/
 bool transferModeKnown() const
 {
   return binaryTransferKnown;
 }

/*
Consume one line of the server's ~<SecondoIntro>~ block. Returns true when the
line was the transfer-mode announcement, which is then adopted for this
connection; false for anything else, which is informational and the caller may
print it.

Every client has to do this, so it lives here rather than being copied into
each of them -- which is how ~SecondoInterfaceREPLAY~ came to miss it.

*/
 bool applyIntroLine(const std::string& line)
 {
   if ( applyVersionLine(line) )
   {
     return true;
   }
   if ( line.compare(0, csp::BINARY_TRANSFER_TAG.size(),
                     csp::BINARY_TRANSFER_TAG) != 0 )
   {
     return false;
   }
   setBinaryTransfer(line.substr(csp::BINARY_TRANSFER_TAG.size()) == "YES");
   return true;
 }

/*
Consume one datum line of a client's ~<Connect>~ block -- everything after the
user and the password. Returns true when the line was recognised; an unknown
key is not an error, so a future client may add one without breaking this
server's framing, which reads to ~</Connect>~ either way.

*/
 bool applyConnectLine(const std::string& line)
 {
   return applyVersionLine(line);
 }

/*
The ~ProtocolVersion=~ line, which appears in both blocks. 0 when the peer never
sent one, which is how a peer older than this protocol presents itself.

*/
 bool applyVersionLine(const std::string& line)
 {
   if ( line.compare(0, csp::PROTOCOL_VERSION_TAG.size(),
                     csp::PROTOCOL_VERSION_TAG) != 0 )
   {
     return false;
   }
   peerVersion = atoi(line.substr(csp::PROTOCOL_VERSION_TAG.size()).c_str());
   peerVersionKnown = true;
   return true;
 }

 int peerProtocolVersion() const { return peerVersion; }

 bool versionKnown() const { return peerVersionKnown; }

/*
Whether the peer speaks the protocol this build speaks. False both for a
mismatch and for a peer that never named a version -- there is nothing to
distinguish "version 1" from "silent", and both are refused the same way.

*/
 bool versionAgreed() const
 {
   return peerVersionKnown && peerVersion == csp::PROTOCOL_VERSION;
 }

/*
The refusal text, written by whichever end noticed. ~peerDescription~ names the
other end ("The Secondo server at host:port" / "The client").

*/
 std::string versionMismatchMessage(const std::string& peerDescription) const
 {
   std::ostringstream os;
   os << peerDescription << " speaks client/server protocol version ";
   if ( peerVersionKnown ) {
     os << peerVersion;
   } else {
     os << "1";
   }
   os << "; this end speaks version " << csp::PROTOCOL_VERSION << ".\n"
      << "There is no compatibility mode: the result framing differs and the "
         "first command would desynchronise the connection. Rebuild and "
         "restart both ends from the same source revision.";
   return os.str();
 }

/*
The line naming this build's version, for either block.

*/
 static std::string versionLine()
 {
   std::ostringstream os;
   os << csp::PROTOCOL_VERSION_TAG << csp::PROTOCOL_VERSION;
   return os.str();
 }


 void skipRestOfLine()
 {
   iosock.ignore( INT_MAX, '\n' );
 }

 void IgnoreMsg(bool value)
 {
    ignoreMsg = value;
    msgHandler->ignoreMsg(value);
    SHOW(ignoreMsg)
 }

/*
Server side: while a result is being streamed, messages become ~M~ records in
the frame rather than ~<Message>~ blocks, which would land in the middle of the
payload. 0 restores the ordinary blocks.

*/
 void RedirectMessages(csp::ChunkedOutBuf* buf)
 {
    msgHandler->redirect(buf);
 }


 bool nextLine(const std::string& exp, std::string& errMsg)
 { 
   std::string line="";
   getline( iosock, line );
  
   if ( line != exp ) {
     errMsg = err + exp + " expected! But got \"" + line + "\"\n";
     std::cerr << errMsg << std::endl;
     return false;
   }
   //std::cerr << "line: \"" << line << "\"" << std::endl; 
   return true;  
 }

  
 bool SendFile(const std::string& filename) {

  std::string line = "";  
  
  //cout << "Begin SendFile()" << std::endl;
  //cout << "Transmitting file: " << filename;

  std::ifstream restoreFile( filename.c_str(), std::ios::binary );
  if ( ! restoreFile ){
     iosock <<  sendFileError << std::endl;
     return false;
  }
  
  // send begin file sequence
  iosock << startFileData << std::endl;
  
  try {

    {
      const unsigned int bufSize =512;
      char buf[bufSize];       
      restoreFile.seekg (0, restoreFile.end);
      uint64_t length = restoreFile.tellg();
      restoreFile.seekg (0, restoreFile.beg);

      // send file size
      iosock << length << std::endl;         
      // cout << "SendFile: file size: " << length <<  " bytes." << std::endl;
      
      // send file data
      //uint64_t read2 = 0;
      while (!restoreFile.eof() && !iosock.fail())
      {
        restoreFile.read(buf, bufSize);
        unsigned int read = restoreFile.gcount();
        //read2 += read;
        iosock.write(buf, read);
      }
      //cout << "SendFile: transmitted " 
      //     << read2 <<  " bytes to the server." << std::endl;

      restoreFile.close();
      iosock.flush();
    }
    // send end sequence => empty file;
    iosock << endFileData << std::endl;
    iosock.flush();

  } catch (std::ios_base::failure&) {
     std::cerr << std::endl 
          << "Caught exception: I/O error on socket stream object!"
          << std::endl;
     return false;
  }   

  //cout << "End SendFile()" << std::endl;
  return true;
}       

bool ReceiveFile( const std::string& localFileName )
{
  
  std::string errMsg = "";
  std::string line="";
  getline(iosock, line);
  if(line == sendFileError ){ 
     return false;
  }
  if(line != startFileData){
    // protocol error
    return false;
  }

  // read file size
  uint64_t size = 0;
  iosock >> size;    
  skipRestOfLine();
  // cout << "Size: " << size << std::endl;
  
  std::ofstream localFile;
  localFile.open( localFileName.c_str(), std::ios::binary );
  
  unsigned int bufSize=512;
  char buf[bufSize];
  [[maybe_unused]] size_t calls=0;  // used by the (disabled) debug output below

  // get data and write them to local file

  while (!iosock.fail() && size)
  {
    if (size < bufSize)
      bufSize = size;       
    iosock.read(buf, bufSize);
    calls++;
    size_t read=iosock.gcount();
    localFile.write(buf, read);
    size -= read; 
  }
  //cout << "Average read bytes per iosock.read(): " 
  //     << (1.0*size2)/calls << std::endl;
  localFile.close();

  // check protool end sequence
  if ( !nextLine(endFileData, errMsg) ) {
    return false;    
  }
    
  return true;
}



/*
Read one nested list that is delimited by ~endTag~ on a line of its own. This is
the version-1 shape and is still how a ~<Message>~ block is transmitted; the
result of a command is not -- see ~ReadFrame~.

*/
bool
ReadList(const std::string& endTag, ListExpr& resultList,
         int& errorCode, bool debug, void* caller,
         int callerID) {

  dwriter.write(debug, std::cout, caller, callerID, "start ReadList");
  std::string line = "";
  std::string result = "";
  bool success = false;
  if ( !transferModeKnown() ) {
    // Nobody said how this connection transfers lists. Falling back to the
    // process-wide flag would decode the stream by guesswork -- and get it
    // wrong exactly when the server disagrees with this side's configuration,
    // which is the case the announcement exists for. Fail instead.
    dwriter.write(true, std::cerr, caller, callerID,
                  "list transfer mode was never agreed for this connection");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    resultList = nl->TheEmptyList();
    return false;
  }
  if ( !usesBinaryTransfer() ) {
    dwriter.write(debug, std::cout, caller, callerID, "textual list transfer");
    // textual data transfer
    do {
      getline( iosock, line );
      if ( line != endTag )
      {
        result += line + "\n";
      }
    } while (line != endTag && !iosock.fail()); 
    dwriter.write(debug, std::cout, caller, callerID,
                  "text received, parse it");
    nl->ReadFromString( result, resultList );
    dwriter.write(debug, std::cout, caller, callerID, "list parsing finished");
    success = true;
    
  } else { // binary data transfer
    dwriter.write(debug, std::cout, caller, callerID, "binary list transfer");
    nl->ReadBinaryFrom(iosock, resultList);
    dwriter.write(debug, std::cout, caller, callerID, "list transfer finished");
    
    //std::ofstream outFile("TTYCS.bnl");
    //nl->WriteBinaryTo(resultList, outFile);
    
    getline( iosock, line );
    dwriter.write(debug, std::cout, caller, callerID, "end tag read");
    
    if (line != endTag ) 
    {
      dwriter.write(true, std::cerr, caller, callerID, "end tag invalid");
      errorCode = ERR_IN_SECONDO_PROTOCOL;
      resultList = nl->TheEmptyList();
    } 
    else 
    {
      success = true;
    }  
  }
  dwriter.write(debug, std::cout, caller, callerID,
                "finished methjod ReadList");
  return success;
}


/*
Deliver an ~M~ record met inside a result frame. Called by ~ChunkedInBuf~, which
has the bytes but neither the transfer mode nor the handler; parsing and routing
are identical to a ~<Message>~ block met before the frame.

*/
virtual void frameMessage( const std::string& body )
{
  ListExpr messageList = nl->TheEmptyList();
  if ( usesBinaryTransfer() ) {
    std::istringstream in( body );
    if ( !nl->ReadBinaryFrom( in, messageList ) ) {
      return;
    }
  } else if ( !nl->ReadFromString( body, messageList ) ) {
    return;
  }
  msg->Send( nl, messageList, frameMsgSource );
  if ( frameMsgHandler ) {
    frameMsgHandler->handleMsg( nl, messageList, frameMsgSource );
  }
}

/*
Read one ~<SecondoResult>~ frame, the start tag already consumed. Fills
~resultList~ with the result and the three status fields from the terminator.

The status arrives *after* the payload, which is the whole reason the frame
exists: the server no longer has to promise an outcome before it knows one. Two
consequences show up here. An ~A~ terminator means the payload is a partial
encoding, so whatever was parsed is discarded rather than handed back. And
whichever way that goes, records are drained to the terminator before returning,
so the connection is left positioned at the next response -- version 1 could
only recover from a confused reader by dropping the socket.

*/
bool
ReadFrame( ListExpr& resultList,
           int& errorCode,
           int& errorPos,
           std::string& errorMessage,
           MessageHandler* extHandler = 0,
           int source = -1,
           bool debug = false,
           void* caller = 0,
           int callerID = 1 )
{
  resultList = nl->TheEmptyList();
  if ( !transferModeKnown() ) {
    // Same reason as in ReadList: decoding by guesswork gets it wrong exactly
    // when the two ends disagree, which is the case the announcement is for.
    dwriter.write(true, std::cerr, caller, callerID,
                  "list transfer mode was never agreed for this connection");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    return false;
  }

  frameMsgHandler = extHandler;
  frameMsgSource = source;

  csp::ChunkedInBuf frame( iosock, this );
  {
    std::istream payload( &frame );
    if ( usesBinaryTransfer() ) {
      dwriter.write(debug, std::cout, caller, callerID, "binary list transfer");
      // A truncated encoding terminates rather than hangs: ReadBinaryRec's
      // default case returns false as soon as it reads past the end.
      if ( resultSink != 0 ) {
        // Whether it really streamed is the sink's to find out; an answer
        // whose shape does not allow it comes back in resultList as usual.
        //
        // An M record met part way through the payload still reaches the
        // handler from here, and its list is built in the same rolled-back
        // region -- so it lives until the element being read is done with,
        // rather than until the end of the frame. That is within what
        // ~handleMsg~ has always promised its callers, which is the list for
        // the duration of the call, but it is a shorter life than before.
        bool streamed = false;
        nl->ReadBinaryStreamed( payload, resultList, *resultSink, streamed );
      } else {
        nl->ReadBinaryFrom( payload, resultList );
      }
    } else {
      dwriter.write(debug, std::cout, caller, callerID,
                    "textual list transfer");
      std::ostringstream text;
      text << payload.rdbuf();
      nl->ReadFromString( text.str(), resultList );
    }
  }
  dwriter.write(debug, std::cout, caller, callerID, "payload read");

  frame.drainToTerminator();
  frameMsgHandler = 0;
  frameMsgSource = -1;

  if ( !frame.ok() ) {
    dwriter.write(true, std::cerr, caller, callerID, "result frame is broken");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    resultList = nl->TheEmptyList();
    return false;
  }

  errorCode = frame.terminatorErrorCode();
  errorPos = frame.terminatorErrorPos();
  errorMessage = frame.terminatorMessage();

  if ( frame.terminatorKind() == 'A' ) {
    dwriter.write(debug, std::cout, caller, callerID, "result was aborted");
    resultList = nl->TheEmptyList();
    if ( errorCode == 0 ) {
      // The server aborted without a code of its own -- say what happened
      // rather than report success over a partial answer.
      errorCode = ERR_RESULT_TRUNCATED;
    }
    return false;
  }
  return true;
}


int
ReadResponse( ListExpr& resultList,
              int& errorCode,
              int& errorPos,
              std::string& errorMessage,
              MessageHandler* msgHandler = 0,
              int source  = -1,
              bool debug = false,
              void* caller = 0,
              int callerID = 1)
{

  dwriter.write(debug, std::cout, caller, callerID, "called ReadResponse");

  // read next line
  std::string line="";
  try{
      getline( iosock, line );
  } catch(std::ios_base::failure& ex){
    
    dwriter.write(debug, std::cerr, caller, callerID, "exception occured");
    std::cerr << "Exception occurred during reading response from server" 
              << std::endl; 
    std::cerr << "Exceptionm is " << ex.what() << std::endl;
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    try{
       iosock.clear();
    } catch(...){
       std::cerr << "clear failed" << std::endl;
    }
    return errorCode;
  }
  dwriter.write(debug, std::cout, caller, callerID, "read first line");
  
  bool badbit = iosock.bad();
  bool success = false;

  // read messages if present
  ListExpr messageList = nl->Empty();
  while ( !badbit && line == startMessage ) 
  {
    dwriter.write(debug, std::cout, caller, callerID, "receive message");
    success = ReadList(endMessage, messageList, errorCode, debug, 
                       caller, callerID);
    dwriter.write(debug, std::cout, caller, callerID,
                  "message received", success);
    if (success) {
      dwriter.write(debug, std::cout, caller, callerID,
                    "send message to center");
      msg->Send(nl,messageList, source);
      if(msgHandler){
          msgHandler->handleMsg(nl, messageList, source);
      }
      getline( iosock, line );
      badbit = iosock.bad();
      dwriter.write(debug, std::cout, caller, callerID,
                    "sending message finished");
    }  
  } 

  // network error 
  if (badbit) {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    dwriter.write(debug, std::cout, caller, callerID, "network error");
    return errorCode;
  }   
    
  
  if ( line == startResult )
  {
    dwriter.write(debug, std::cout, caller, callerID, "read result frame");

    // The status comes out of the frame's terminator; there is no longer a
    // four-element wrapper around the result to unpack it from.
    success = ReadFrame(resultList, errorCode, errorPos, errorMessage,
                        msgHandler, source, debug, caller, callerID);
    dwriter.write(debug, std::cout, caller, callerID, "reading frame", success);
  }
  else if ( line == startError )
  {
    dwriter.write(debug, std::cout, caller, callerID, "receive error");
    // Read to the end tag rather than taking one line: the message may be
    // several, and stopping after the first left the rest of it in the stream
    // to be misread as the next response.
    errorMessage = "";
    while ( getline( iosock, line ) && line != endError )
    {
      if ( !errorMessage.empty() ) {
        errorMessage += "\n";
      }
      errorMessage += line;
    }
    errorCode = ERR_IN_SECONDO_PROTOCOL;
    dwriter.write(debug, std::cout, caller, callerID, "protocol error");
  }
  else
  {
    dwriter.write(debug, std::cout, caller, callerID, "protocol error");
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  dwriter.write(debug, std::cout, caller, callerID, "readResponse finsihed");
  return errorCode;
}

};

#endif

