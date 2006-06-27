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
#include <db_cxx.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "StopWatch.h"

using namespace std;

void
check_bdb_error(const int rc, DbEnv& bdb) {

    if (     (rc == DB_SECONDARY_BAD) 
	  || (rc == EINVAL)
	  || (rc == ENOMEM) ) 
    {
      bdb.err(rc, "cursor.get()");
    } 
}


int
main( int argc, char** argv )
{
  if ( argc == 1 ) {
   
    cout << endl;
    cout << "Usage: " << argv[0] << " -mode [NUM_INSERT | NUM_FIND | BUFFER]" 
         << endl
         << "Operational modes are listed below: " << endl
         << "  -1 : Create B-Tree with key integer and insert NUM_INSERT data"
         << " records of size 100 b" << endl
         << "  -2 : Retrieve randomly NUM_FIND of NUM_INSERT integers." << endl
         << "  -3 : Scan database with cursor." << endl
         << "  -4 : Scan BTREE database using bulk retrieval with buffer "
         << "size BUFFER kb" << endl
         << "  -5 : Create a DB_RECNO database with NUM_INSERT entries"
         << " of size 100 b" << endl
         << "  -6 : Scan RECNO database using bulk retrieval with buffer"
         << " size BUFFER kb." << endl;
        
    cout << endl;
    exit(1);
  }

  int param1 = 0;
  int param2 = 0;
  
  const int mode = -1 * atoi(argv[1]);

  if ( (mode == 1) || (mode == 4) || (mode == 5) || (mode == 6) ) {
    if (!(argc == 3)) {
      cout << "Arguments not correct!" << endl;
      exit(2);
    }
    param1 = atoi(argv[2]);
  }

  if ( mode == 2 ) {
    if (!(argc == 4)) {
      cout << "Arguments not correct!" << endl;
      exit(2);
    }
    param1 = atoi(argv[2]);
    param2 = atoi(argv[3]);
  }


  const int pageSize = 4096;

  DbEnv bdb(DB_CXX_NO_EXCEPTIONS);
  const int CACHE_SIZE = 300 * pageSize;
  assert( !bdb.set_cachesize( 0 ,CACHE_SIZE, 0 ) );
  //assert( !bdb.set_flags(DB_DIRECT_DB | DB_REGION_INIT, 1) );
  assert( !bdb.open( 0, DB_CREATE | DB_INIT_MPOOL, 0) );
  cout << "Environment opened. Cache size: " 
       << CACHE_SIZE << " bytes." << endl;        

  string HOME("bdb-performance-test");
  string DATABASE("access-btree.db");
  string DATABASE2("access-recno.db");

  Db db1(&bdb, 0);
  Db db2(&bdb, 0);
  assert( !db1.set_pagesize(4096) );

  u_int32_t openFlags;

  if ( mode==1 || mode==5 ) {
    openFlags = DB_CREATE;
    
  } else {
    openFlags = DB_RDONLY;
  }

  assert( !db1.open( 0, (DATABASE).c_str(), 
		     0, DB_BTREE, openFlags, 0664) );
  cout << "Database " << DATABASE << " opened." << endl;

  assert( !db2.open( 0, (DATABASE2).c_str(), 
		     0, DB_RECNO, openFlags, 0664) );
  cout << "Database " << DATABASE << " opened." << endl;


  const int recSize = 100;
  char dummyRec[recSize];


  StopWatch runTime;
  const int TEST_MAX = param1;
  if ( mode == 1 ) {
    cout << "Inserting " << TEST_MAX 
         << " integer values into DB_BTREE ..." << endl;
    for ( int j=1 ; j < TEST_MAX; j++ ) {

      Dbt key( (void*) &j, sizeof(int) );
      Dbt data( (void*) dummyRec, recSize );
      assert( !db1.put(0, &key, &data, 0) );

    } // end of for j ...
  }

  if ( mode == 5 ) {
    cout << "Inserting " << TEST_MAX << " records of size " 
         << recSize <<" into DB_RECNO ..." << endl;
    for ( int j=1 ; j < TEST_MAX; j++ ) {

      Dbt key( (void*) &j, sizeof(int) );
      Dbt data( (void*) dummyRec, recSize );
      assert( !db2.put(0, &key, &data, 0) );

    } // end of for j ...
  }


  if ( mode == 2 ) {
    const int READ_MAX = param2;
    cout << "Random access of " << READ_MAX << " items ..." << endl;
    for ( int j=1 ; j < READ_MAX; j++ ) {
      
      int keyval = (int)(TEST_MAX * 1.0 * rand() / (RAND_MAX + 1.0)) + 1;
      //cout << keyval << ", ";
      Dbt key( (void *) &(keyval), sizeof(int) );  
      Dbt data;      
      assert( !db1.get(0, &key, &data, 0) ); 
    }
  } 

  if ( mode == 3 ) {
    Dbc* cursor = 0;
    assert( !db1.cursor(0, &cursor, 0) );
    Dbt key;
    Dbt data;
    int rc = 0;

    cout << "Scanning database " << DATABASE << "..." << endl;
    while ( (rc = cursor->get(&key, &data, DB_NEXT)) == 0 ) {
    }
    check_bdb_error(rc, bdb);
    cursor->close();
  }

    Dbc* cursor = 0;
    assert( !db1.cursor(0, &cursor, 0) );
    Dbt key;
    int rc = 0;
    
    void* bufptr = 0; // maintained by Berkeley-DB
    Dbt retdata;
    void* retkey = 0;
    size_t retklen = 0;
    size_t retdlen = 0;

    Dbt buf;
    size_t cBufLength = 1;
    if ( mode==4 || mode==6 ) {
      cBufLength = param1 * 1024;
      cout << "Buffer for bulk retrieval set to " 
           << cBufLength << " bytes." << endl;
    }
    char cBuf[cBufLength];

    buf.set_data(cBuf);
    buf.set_ulen(cBufLength);
    buf.set_dlen(cBufLength);
    buf.set_doff(0);
    buf.set_flags(DB_DBT_USERMEM);



  /*
  if ( mode == 4 ) {
    assert( !db1.cursor(0, &cursor, 0) );
    cout << "Bulk retrieval of " << DATABASE << "..." << endl;

    for (;;) {
     //cerr << "Getting next bulk of data ..." << endl;
     

     rc = cursor->get(&key, &buf, DB_MULTIPLE_KEY | DB_NEXT);
     if ( rc != 0) {
       //cerr << "finished!" << endl;
       check_bdb_error(rc, bdb);
       break;
     }

      int ctr = 0;		  
      for ( DB_MULTIPLE_INIT( bufptr, &buf );; ) {
        
	DB_MULTIPLE_KEY_NEXT( bufptr, &buf, retkey, retklen, retdata, retdlen );
	if (bufptr == 0) {
          //cerr << "Iterating buffer finished!" << endl;
	  break;
        } 

	  //cout << ctr << " - key, data: " <<  *((int*)retkey) 
          //     << ", " << *((int*)retdata)<< endl;
          //ctr++;
	}
      }

    cursor->close();
  }
  */

  if ( mode == 6 ) {


    assert( !db2.cursor(0, &cursor, 0) );
    db_recno_t recno = 0;

    cout << "Bulk retrieval of " << DATABASE2 << "..." << endl;

    for (;;) {
     //cerr << "Getting next bulk of data ..." << endl;
     

     rc = cursor->get(&key, &buf, DB_MULTIPLE_KEY | DB_NEXT);
     if ( rc != 0) {
       //cerr << "finished!" << endl;
       check_bdb_error(rc, bdb);
       break;
     }

      int ctr = 0;		  
      DbMultipleRecnoDataIterator it(buf);
      while ( it.next(recno, retdata) ) {};
     }

    cursor->close();
  }

  


  assert ( !db1.close(DB_NOSYNC) );
  assert ( !db2.close(DB_NOSYNC) );

  cout << "Time: " << runTime.diffTimes() << endl;       
  assert( !bdb.close(0) );

}
