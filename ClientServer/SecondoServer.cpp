using namespace std;

#include <string>
#include "Application.h"
#include "SocketIO.h"

class SecondoServer : public Application
{
 public:
  SecondoServer( const int argc, const char** argv ) : Application( argc, argv ) {};
  virtual ~SecondoServer() {};
  int Execute();
 private:
  Socket* client;
};

int
SecondoServer::Execute()
{
  client = GetSocket();
  string line;
  if ( client != 0 )
  {
    iostream& iosock = client->GetSocketStream();
    do
    {
      getline( iosock, line );
      cout << "Read: " << line << endl;
      iosock << "<ECHO> " << line << endl;
    }
    while (!iosock.fail() && line != "quit");
    client->Close();
    delete client;
  }
  return (0);
}

int main( const int argc, const char* argv[] )
{
  SecondoServer* appPointer = new SecondoServer( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

