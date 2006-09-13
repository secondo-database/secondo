/*

07.11.2004 M. Spiekermann

This is a simple test of the Berkeley-Db C++ API. It was 
programmed in order to test the performance of Berkeley-DB
B-Trees.

It was observed that the PrefetchingIterator implemented in
Secondos SMI is much slower than the bulk retrieval, but it
does call the same bulk-retrieval APIs as in this code.

*/


#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <db_cxx.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "StopWatch.h"

using namespace std;

void
openDb(Db& database, string& name, u_int32_t flags) {

  database.open( 0, name.c_str(), 
	         0, DB_RECNO, flags, 0664);

  //cout << "Database " << name << " opened." << endl;
}


int
main( int argc, char** argv )
{
  if ( argc == 1 ) {
   
    cout << endl;
    cout << "Usage: " << argv[0] << " -mode [NUM_INSERT | BUFSIZE]" << endl
         << "Operational modes are listed below: " << endl
         << "  -1 : Create a DB_RECNO database with NUM_INSERT entries "
         << "of size 100 b" << endl
         << "  -2 : Scan a DB_RECNO database using bulk retrieval "
         << "with buffer size BUFSIZE kb." << endl;
        
    cout << endl;
    exit(1);
  }

  int param1 = 0;
  
  const int mode = -1 * atoi(argv[1]);

  if (!(argc == 3)) {
      cout << "Arguments not correct!" << endl;
      exit(1);
  }
  param1 = atoi(argv[2]);

  DbEnv bdb(0); // Environment
  //Db db(&bdb, 0);  // Database
 
  const int pageSize = 4096;
  const int CACHE_SIZE = 300 * pageSize;

  int rc = 0;

  ofstream bdbErrOs("bdb-err.log");

  try {

  bdb.set_error_stream(&bdbErrOs);
  bdb.set_cachesize( 0, CACHE_SIZE, 0 );
  bdb.open( 0, DB_CREATE | DB_INIT_MPOOL, 0);

  cout << "Environment opened. Cache size: " 
       << CACHE_SIZE << " bytes." << endl;        

  string DATABASE("btree.db");

  u_int32_t openFlags;

  if ( mode==1 ) {
    openFlags = DB_CREATE;
  } else {
    openFlags = DB_RDONLY;
  }

  // generate filenames
  const int DB_MAX = 100;
  vector<string> databases;
  vector<string>::iterator it = databases.begin();
  vector<Db*> dbHandles;
  vector<Db*>::iterator dit = dbHandles.begin();

  for (int k=0; k<DB_MAX; k++) {
  
    ostringstream ostr;
    ostr << DATABASE << k << ends;
    databases.push_back(ostr.str());
  }

  // open databases
  for ( it=databases.begin(); it != databases.end(); it++ ) {

    Db* db = new Db(&bdb,0);
    dbHandles.push_back( db  );
    openDb(*db, *it, openFlags);
    
  }

  const int recSize = 100;
  char dummyRec[recSize];

  if ( mode == 1 ) {
    const int TEST_MAX = param1/DB_MAX;
    cout << "Inserting " << TEST_MAX << " records of size " 
         << recSize << " into "<< DB_MAX << " database(s)." << endl;

    for ( dit=dbHandles.begin(); dit != dbHandles.end(); dit++ ) {

      //Db& db = *( new Db(&bdb,0) );
      //openDb(db, *it, openFlags);
      for ( int j=1 ; j < TEST_MAX; j++ ) {

	Dbt key( (void*) &j, sizeof(int) );
	Dbt data( (void*) dummyRec, recSize );
	(*dit)->put(0, &key, &data, 0);

      } // end of for j ...
      //db.close(DB_NOSYNC);
    }
  }

  if ( mode == 2 ) {
  
    Dbt key;
    Dbt data;
    Dbt buf;
    db_recno_t recno = 0;
    
    size_t  cBufLength = param1 * 1024;
    char cBuf[cBufLength];
    cout << "Buffer for bulk retrieval set to " 
         << cBufLength << " bytes." << endl;

    buf.set_data(cBuf);
    buf.set_ulen(cBufLength);
    buf.set_dlen(cBufLength);
    buf.set_doff(0);
    buf.set_flags(DB_DBT_USERMEM);
    
    cout << "Bulk retrieval of " << DATABASE << "..." << endl;

    int rc=0;    
    for ( dit=dbHandles.begin(); dit != dbHandles.end(); dit++ ) {

      //StopWatch open;
      //openDb(db, *it, openFlags);
      //cout << "Time for opening: " << open.diffTimes() << endl;
      Db& db = **dit;
      Dbc* cursor = 0;
      db.cursor(0, &cursor, 0);

      while ( (rc = cursor->get(&key, &buf, DB_MULTIPLE_KEY | DB_NEXT)) == 0 ) {

       DbMultipleRecnoDataIterator rit(buf);
       while ( rit.next(recno, data) ) {};
      }

      // check possible errors
      if (     (rc == DB_SECONDARY_BAD) 
	    || (rc == EINVAL)
	    || (rc == ENOMEM) ) 
      {
	bdb.err(rc, "cursor.get()");
      }

      //StopWatch close;
      cursor->close();
      //db.close(DB_NOSYNC);
      //cout << "Time for closing: " << close.diffTimes() << endl;
    }
  }

  // close databases
  for ( dit=dbHandles.begin(); dit != dbHandles.end(); dit++ ) {

    (*dit)->close(DB_NOSYNC);
  }



  //db.close(DB_NOSYNC);
  bdb.close(0);

  } catch (DbException e) {

    cout << "Error: " << e.what() << endl;
  }

}
