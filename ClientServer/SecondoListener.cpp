using namespace std;

#include <string>
#include <algorithm>

#include "Application.h"
#include "Processes.h"
#include "SocketIO.h"
#include "Profiles.h"

const int EXIT_LISTENER_OK       = 0;
const int EXIT_LISTENER_NOPF     = 1;
const int EXIT_LISTENER_NOHOST   = 2;
const int EXIT_LISTENER_NOSERVER = 3;
const int EXIT_LISTENER_NOSOCKET = 4;
const int EXIT_LISTENER_FAIL     = 5;

class SecondoListener : public Application
{
 public:
  SecondoListener( const int argc, const char** argv ) : Application( argc, argv ) {};
  virtual ~SecondoListener() {};
  int  Execute();
  bool ClientAllowed();
  bool AbortOnSignal( int sig );
  void LogMessage( const string msg );
 private:
  string parmFile;
  Socket* gate;
  Socket* client;
};

bool
SecondoListener::AbortOnSignal( int sig )
{
  if ( gate != 0 )
  {
    gate->CancelAccept();
  }
  return (false);
}

void
SecondoListener::LogMessage( const string msg )
{
}

int
SecondoListener::Execute()
{
  int rc = EXIT_LISTENER_OK;
  SetAbortMode( true );

  // --- Load ruleSet
  string rulePolicy = SmiProfile::GetParameter( "Environment", "RulePolicy", "ALLOW", parmFile );
  transform( rulePolicy.begin(), rulePolicy.end(), rulePolicy.begin(), toupper );
  SocketRule::Policy policy = (rulePolicy == "ALLOW") ? SocketRule::ALLOW : SocketRule::DENY;
  SocketRuleSet ipRules( policy );
  string ruleSetFile = SmiProfile::GetParameter( "Environment", "RuleSetFile", "", parmFile );
  if ( ruleSetFile != "" )
  {
    ipRules.LoadFromFile( ruleSetFile );
  }

  // --- Get configuration file
  parmFile = (GetArgCount() > 1) ? GetArgValues()[1] : "SecondoConfig.ini";

  // --- Get host and port of Secondo server
  string host = SmiProfile::GetParameter( "Environment", "SecondoHost", "", parmFile );
  string port = SmiProfile::GetParameter( "Environment", "SecondoPort", "", parmFile );

  if ( host.length() == 0 || port.length() == 0 )
  {
    return (EXIT_LISTENER_NOHOST);
  }

  // --- Get name of client server program
  string section = (GetArgCount() > 2) ? GetArgValues()[2] : "BerkeleyDB";
  string server  = SmiProfile::GetParameter( section, "ServerProgram", "", parmFile );
  if ( server.length() == 0 )
  {
    return (EXIT_LISTENER_NOSERVER);
  }

  // --- Start up process factory
  if ( !ProcessFactory::StartUp() )
  {
    return (EXIT_LISTENER_NOPF);
  }

  // --- Create listener socket

  gate = Socket::CreateGlobal( host, port );
  if ( gate && gate->IsOk() )
  {
    while (!ShouldAbort())
    {
      client = gate->Accept();
      if ( client && client->IsOk() )
      {
        if ( ipRules.Ok( SocketAddress( client->GetPeerAddress() ) ) )
        {
          // --- Spawn server for client
          int pidServer;
          string pgmArgs = string( "\"" ) + parmFile + "\""; 
          if ( !ProcessFactory::SpawnProcess( server, pgmArgs,
                                              pidServer, false, client ) )
          {
            // --- Start of server failed
            iostream& ss = client->GetSocketStream();
            ss << "<ERROR> -1 Server not available. Try again later." << endl;
            client->Close();
            delete client;
            LogMessage( "Start of client server failed" );
          }
          Application::Sleep( 0 );
        }
        else
        {
          // --- Reject client
          iostream& ss = client->GetSocketStream();
          ss << "<ERROR> -2 Connection rejected." << endl;
          client->Close();
          delete client;
          string errmsg = string( "Client '" ) + client->GetPeerAddress() + "' not allowed.";
          LogMessage( errmsg );
        }
      }
    }
    ProcessFactory::WaitForAll();
  }
  else
  {
    string errbuf = gate->GetErrorText();
    delete gate;
    LogMessage( "Failed to create global socket: " + errbuf );
    rc = EXIT_LISTENER_NOSOCKET;
  }
  gate->Close();
  delete gate;
  ProcessFactory::ShutDown();
  return (rc);
}

int main( const int argc, const char* argv[] )
{
  SecondoListener* appPointer = new SecondoListener( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

