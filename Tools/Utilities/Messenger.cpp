using namespace std;

#include <iostream>
#include <string>
#include "Messenger.h"
#include "SocketIO.h"

bool
Messenger::Send( const string& message, string& answer )
{
  bool ok = false;
  answer = "";
  Socket* msgServer = Socket::Connect( msgQueue, "", Socket::SockLocalDomain, 3, 1 );
  if ( msgServer && msgServer->IsOk() )
  {
    iostream& ss = msgServer->GetSocketStream();
    ss << message << endl;
    getline( ss, answer );
    ok = true;
  }
  else
  {
    answer = "Connect to registrar failed.";
  }
  delete msgServer;
  return (ok);
}

