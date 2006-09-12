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

1 Implementation of the Checkpoint utility for the Berkeley DB

May 2002 Ulrich Telle

For client/server operation of the Berkeley DB environment checkpoints need to
be generated regularly. This utility program writes checkpoints at configurable
time intervals.

*/

using namespace std;

#include <db_cxx.h>
#define DB_FTYPE_SET  -1  // Call pgin/pgout functions
#if defined(__cplusplus)
extern "C" {
#endif
int __db_pgin(DB_ENV *, db_pgno_t, void *, DBT *);
int __db_pgout(DB_ENV *, db_pgno_t, void *, DBT *);
#if defined(__cplusplus)
}
#endif

#include "Application.h"
#include "Processes.h"
#include "SocketIO.h"
#include "Profiles.h"

const int EXIT_CHECKPOINT_OK    = 0;
const int EXIT_CHECKPOINT_NOENV = 1;
const int EXIT_CHECKPOINT_NOPGF = 2;
const int EXIT_CHECKPOINT_FAIL  = 3;

class SecondoCheckpoint : public Application
{
 public:
  SecondoCheckpoint( const int argc, const char** argv ) : 
    Application( argc, argv ) 
  {};
  virtual ~SecondoCheckpoint() {};
  int  Execute();
 private:
};

int
SecondoCheckpoint::Execute()
{
  string parmFile;
  if ( GetArgCount() > 1 )
  {
    parmFile = GetArgValues()[1];
  }
  else
  {
    parmFile = "SecondoConfig.ini";
  }
  string bdbHome = SmiProfile::GetParameter( "Environment", 
                                                "SecondoHome", "", parmFile );
  
  u_int32_t minutes = SmiProfile::GetParameter( "BerkeleyDB", 
                                                "CheckpointTime", 
                                                5, parmFile       );
  u_int32_t seconds = minutes * 60;
  int rc;

  // --- Setup of Berkeley DB environment
  DbEnv* bdbEnv = new DbEnv( DB_CXX_NO_EXCEPTIONS );
  bdbEnv->set_error_stream( &cerr );
  bdbEnv->set_errpfx( "SecondoCheckpoint" );
  rc = bdbEnv->open( bdbHome.c_str(), DB_JOINENV | DB_USE_ENVIRON, 0 );
  if ( rc != 0 )
  {
    //rc = bdbEnv->close( 0 );
    delete bdbEnv;
    return (EXIT_CHECKPOINT_NOENV);
  }

  // --- Register the standard pgin/pgout functions, in case we do I/O
  rc = bdbEnv->memp_register( DB_FTYPE_SET, __db_pgin, __db_pgout );
  if ( rc != 0 )
  {
    rc = bdbEnv->close( 0 );
    delete bdbEnv;
    return (EXIT_CHECKPOINT_NOPGF);
  }

  // --- Create checkpoints
  while (!ShouldAbort())
  {
    rc = bdbEnv->txn_checkpoint( 0, minutes, 0 );
    if ( rc != 0 ) break;
    for ( u_int32_t sec = 0; !ShouldAbort() && sec < seconds; sec += 5 )
    {
      Application::Sleep( 5 );
    }
  }

  // --- Clean up the environment
  rc = bdbEnv->close( 0 );
  delete bdbEnv;
  return ((rc == 0) ? EXIT_CHECKPOINT_OK : EXIT_CHECKPOINT_FAIL);
}

int main( const int argc, const char* argv[] )
{
  SecondoCheckpoint* appPointer = new SecondoCheckpoint( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

