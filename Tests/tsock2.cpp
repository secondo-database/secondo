using namespace std;
#include "SocketIO.h"
#include <time.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

int main( int argc, char* argv[] )
{
  string errbuf;
  string hostaddr;
  Socket* sock;
  bool server = false;

  if (argc < 4)
  {
    cout << "Socket test program 2 (Socket stream test)" << endl
         << "Usage: tsock2 [(local-server|global-server)|client] host port" << endl;
    return EXIT_FAILURE;
  }

  if ( *argv[1] == 'l' || *argv[1] == 'L' ) // local server
  {
    Socket* gate = Socket::CreateLocal( argv[2] );
    if ( !gate->IsOk() )
    {
      errbuf = gate->GetErrorText();
      cout << "Failed to create local socket: " << errbuf << endl;
    }
    sock = gate->Accept();
    if ( sock == 0 )
    {
      errbuf = gate->GetErrorText();
      cout << "Failed to accept socket: " << errbuf << endl;
    }
    delete gate;
    server = true;
  }
  else if ( *argv[1] == 'g' || *argv[1] == 'G' )  // global server
  {
    Socket* gate = Socket::CreateGlobal( argv[2], argv[3] );
    if ( !gate->IsOk() )
    { 
      errbuf = gate->GetErrorText();
      delete gate;
      cout << "Failed to create global socket: " << errbuf << endl;
    }
    hostaddr = gate->GetSocketAddress();
    cout << "Global Socket Address=" << hostaddr << endl;
    cout << "Name=" << Socket::GetHostname( hostaddr ) << endl;
    sock = gate->Accept();
    if ( sock == 0 )
    {
      errbuf = gate->GetErrorText();
      delete gate;
      cout << "Failed to accept socket: " << errbuf << endl;
    }
    delete gate;
    hostaddr = sock->GetSocketAddress();
    cout << "Accept Socket: Socket Address=" << hostaddr << endl;
    cout << "Name=" << Socket::GetHostname( hostaddr ) << endl;
    hostaddr = sock->GetPeerAddress();
    cout << "Accept Socket: Peer Address=" << hostaddr << endl;
    cout << "Name=" << Socket::GetHostname( hostaddr ) << endl;
    server = true;
  }
  else   // client
  {
    sock = Socket::Connect( argv[2], argv[3] );
    if ( sock == 0 )
    {
      cout << "Failed to connect to server" << endl;
    }
    else if ( !sock->IsOk() )
    {
      errbuf = sock->GetErrorText();
      cout << "Connection to server failed: " << errbuf << endl;
    }
    hostaddr = sock->GetSocketAddress();
    cout << "Connect Socket: Socket Address=" << hostaddr << endl;
    cout << "Name=" << Socket::GetHostname( hostaddr ) << endl;
    hostaddr = sock->GetPeerAddress();
    cout << "Connect Socket: Peer Address=" << hostaddr << endl;
    cout << "Name=" << Socket::GetHostname( hostaddr ) << endl;
    server = false;
  }

  if (server)
  {
    iostream& io = sock->GetSocketStream();
    char ioBuf[81];
    bool cont = true;
    if ( io )
    {
      cout << "Received text is echoed to the sender." << endl;
      do
      {
        io.getline( ioBuf, 81 );
        if ( !io.good() || io.eof() || io.fail() || io.bad() )
        {
          cont = false;
          cout << "Socket stream flags:" << endl;
          cout << "good: " << io.good() << endl;
          cout << "eof : " << io.eof()  << endl;
          cout << "fail: " << io.fail() << endl;
          cout << "bad : " << io.bad()  << endl;
        }
        else
        {
          cont = strncmp(ioBuf, "quit", 4) != 0;
          cout << "Received: " << ioBuf << endl;
          io << "Echo: <" << ioBuf << ">" << endl;
        }
      }
      while (cont);
    }
    cout << "Server stopped." << endl;
  }
  else
  {
    iostream& io = sock->GetSocketStream();
    char ioBuf[81];
    bool cont = true;
    if (io)
    {
      do
      {
        cout << "Sending text to server" << endl
             << " - enter 'quit' to end the session regularly," << endl
             << " - enter 'exit' to interrupt immediately" << endl;
        cout << "Enter text: ";
        cin.getline( ioBuf, 81 );
        cont = strncmp( ioBuf, "quit", 4 ) != 0;
        if ( strncmp( ioBuf, "exit", 4 ) == 0 ) break;
        io << ioBuf << endl;
        io.getline( ioBuf, 81 );
        cout << "Received from server:" <<endl;
        cout << ioBuf << endl << endl;
      }
      while (cont);
    }
    cout << "Client stopped." << endl;
  } 
  delete sock;
  return EXIT_SUCCESS;
}

