/*
//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Messenger

May 2002 Ulrich Telle

1.1 Overview

The "Secondo"[3] server consists of several processes which need to communicate
with each other. The communication is based on local sockets. Quite often
only short messages need to be exchanged between two processes. Examples
are registering access to a "Secondo"[3] database, locking a database or
writing log messages. In these cases the class ~Messenger~ provides an easy to
use interface to communicate with a message queue server.

1.1 Class "Messenger"[1]

This class implements a simple mechanism for interprocess communication with a
message queue server. The method ~Send~ connects to the message queue server,
sends a simple string message and waits for an answer. After receiving the
answer string, it disconnects from the message queue server.

*/

#ifndef MESSENGER_H
#define MESSENGER_H

#include <string>

class Messenger
{
 public:
  Messenger( const string& queueName )
    : msgQueue( queueName ) {};
/*
Creates a messenger instance for communication with the message queue ~queueName~.

*/
  virtual ~Messenger() {};
/*
Destroys a messenger instance.

*/
  bool Send( const string& message, string& answer );
/*
Send the ~message~ string to the message queue and wait for the ~answer~ string.
If the communication was successful, the method returns "true"[4].

*/
 protected:
 private:
  string msgQueue; // Name of the message queue
};

#endif

