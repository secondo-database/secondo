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


#include <db_cxx.h>
#include <fstream>
#include <sstream>


#include "Application.h"
#include "Processes.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "WinUnix.h"

const int EXIT_CHECKPOINT_OK    = 0;
const int EXIT_CHECKPOINT_NOENV = 1;
const int EXIT_CHECKPOINT_FAIL  = 2;

// Warn near the threshold, recover below a lower one (hysteresis) so a run
// hovering at the edge does not spam the log every tick.
const unsigned LOCK_WARN_PERCENT   = 80;
const unsigned LOCK_RECOVER_PERCENT = 60;

using namespace std;

/*
Watches the shared Berkeley DB lock region for approaching exhaustion.

Berkeley DB serves a fixed number of lockers, locks and objects from a region
shared by every process attached to the environment. Once a table fills,
transaction and lock requests fail. Make the user aware of the problem
that he can fix by increasing the configuration parameters in the ini file.

*/
static void
CheckLockPressure( DbEnv* env, ostream& log, bool warned[3] )
{
  // DB_STAT_CLEAR resets the "max so far" counters after reading, so each call
  // reports the *peak* usage since the previous wake-up. Sampling the current
  // count would miss short bursts: with a healthy table, lockers are released
  // almost immediately, so a snapshot reads near zero even mid-spike. The
  // high-water mark is what reveals how close a burst came to the limit.
  DB_LOCK_STAT* sp = 0;
  if ( env->lock_stat( &sp, DB_STAT_CLEAR ) != 0 || sp == 0 )
  {
    return;
  }
  const char* names[3]  = { "locker", "lock", "object" };
  const char* params[3] = { "MaxLockers", "MaxLocks", "MaxLockObjects" };
  u_int32_t   peak[3]   = { sp->st_maxnlockers, sp->st_maxnlocks,
                            sp->st_maxnobjects };
  u_int32_t   mx[3]     = { sp->st_maxlockers, sp->st_maxlocks,
                            sp->st_maxobjects };
  free( sp );

  for ( int i = 0; i < 3; ++i )
  {
    if ( mx[i] == 0 )
    {
      continue;
    }
    // 64 bit math so the percentage cannot overflow on large tables.
    unsigned pct = (unsigned) ( (uint64_t) peak[i] * 100 / mx[i] );
    bool high = pct >= LOCK_WARN_PERCENT;
    bool low  = pct <= LOCK_RECOVER_PERCENT;
    if ( high && !warned[i] )
    {
      warned[i] = true;
      ostringstream m;
      m << "Berkeley DB " << names[i] << " table peaked at " << peak[i]
        << "/" << mx[i] << " (>=" << LOCK_WARN_PERCENT << "% full); increase "
        << params[i] << " in the configuration to avoid exhaustion.";
      log  << "WARNING: " << m.str() << endl;
      cerr << "[SecondoCheckpoint] WARNING: " << m.str() << endl;
    }
    else if ( low && warned[i] )
    {
      warned[i] = false;
      log << "Berkeley DB " << names[i] << " table back below "
          << LOCK_RECOVER_PERCENT << "% (peak " << peak[i] << "/" << mx[i]
          << ")." << endl;
    }
  }
}

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
  if (GetArgCount() > 1) {
    parmFile = GetArgValues()[1];
    if(parmFile=="--help"){
      cout << "SecondoCheckpoint " << endl
           << "This program sets a checkpoint at " << endl
           << "some Berkeley DB Environment in regular time intervals."
           << endl << endl
           << "It accepts up to two arguments :" << endl
           << "1st arg : parameter file "  << endl
           << "2nd arg : SecondoHome " << endl
           << "If the first argument is not given, SecondoConfig.ini " 
           << "is used as a default value. " << endl
           << "If the second argument is omitted, the SecondoHome is "
           << "taken from the configuration file. " << endl;
      cout << "This process is started automatically by the monitor. " << endl
           << "Do not start it manually." << endl;
      return 0;
    }
  } else {
    parmFile = "SecondoConfig.ini";
  }
  string dbDir;
  if (GetArgCount() > 2) {
    dbDir = GetArgValues()[2];
  } else {
    dbDir = SmiProfile::GetParameter("Environment", "SecondoHome", "",parmFile);
  }
  
  u_int32_t minutes = SmiProfile::GetParameter( "BerkeleyDB",
                                                "CheckpointTime",
                                                5, parmFile       );
  u_int32_t seconds = minutes * 60;
  int rc;

  // Shut down on a terminating signal instead of dying where it lands. This
  // process spends its life inside BerkeleyDB, holding region locks that are
  // not robust; being killed mid-region would orphan a lock and wedge every
  // other process in the environment. The loop below polls ShouldAbort(), which
  // now returns true when a terminating signal arrives, and shuts down the
  // environment which releases the region cleanly.
  SetGracefulTermination( true );

  // --- Setup of Berkeley DB environment
  DbEnv* bdbEnv = new DbEnv( DB_CXX_NO_EXCEPTIONS );

  ofstream f;
  f.open("Checkpoint.msg");
  bdbEnv->set_error_stream( &f );
  bdbEnv->set_errpfx( "SecondoCheckpoint" );
  f << "using parmfile '" << parmFile << "'" << endl;
  f << "Opening environment '" << dbDir << "'" << endl;
  rc = bdbEnv->open(dbDir.c_str(), DB_JOINENV | DB_USE_ENVIRON, 0 );
  if ( rc != 0 )
  {
    bdbEnv->err(rc, "%s", "Environment open failed!");
    //rc = bdbEnv->close( 0 );
    delete bdbEnv;
    return (EXIT_CHECKPOINT_NOENV);
  }
  f << "Opening environment successful" << endl;
  f << "Set checkpoint every " << minutes << " minutes" << endl;

  // Report the configured table sizes once, and confirm lock_stat works, so
  // the periodic pressure warnings below have a visible baseline.
  bool lockWarned[3] = { false, false, false };
  {
    DB_LOCK_STAT* sp = 0;
    if ( bdbEnv->lock_stat( &sp, 0 ) == 0 && sp != 0 )
    {
      f << "Lock region: lockers max " << sp->st_maxlockers
        << ", locks max " << sp->st_maxlocks
        << ", objects max " << sp->st_maxobjects << endl;
      free( sp );
    }
  }

  // --- Create checkpoints
  while (!ShouldAbort())
  {
    rc = bdbEnv->txn_checkpoint( 0, minutes, 0 );
    if ( rc != 0 ) {
      bdbEnv->err(rc, "%s", "txn_checkpoint failed!");
      break;
    }
    for ( u_int32_t sec = 0; !ShouldAbort() && sec < seconds; sec += 5 )
    {
      // Watch the shared lock region for approaching exhaustion on every
      // wake-up, so a load spike is warned about long before it can wedge the
      // installation.
      CheckLockPressure( bdbEnv, f, lockWarned );
      WinUnix::sleep( 5 );
    }
  }


  // --- Clean up the environment
  rc = bdbEnv->close( 0 );
  
  int result = EXIT_CHECKPOINT_OK;

  if (rc != 0) {
    bdbEnv->err(rc, "%s", "env close failed!");
    result = EXIT_CHECKPOINT_FAIL;
  }

  delete bdbEnv;
  bdbEnv = nullptr;

  return result;
}


int main( const int argc, const char* argv[] )
{
  SecondoCheckpoint* appPointer = new SecondoCheckpoint( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}

