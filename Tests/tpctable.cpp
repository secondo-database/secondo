#include <string>
#include <iostream>
#include "SecondoSMI.h"

#define CTABLE_PERSISTENT
#include "CTable.h"


/*   Important Note: Currently the CTable is implemented as a temporary datastructure 
 *   which uses main memory and Berkeley-DB records as disk memory. Functions for
 *   storing (and restoring) a CTable completley on disk are not implemented yet.
 */

SmiRecordFile* rf = 0;

using namespace std;

void pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}

bool
RecordBufferTest() {

  cout << "Tests for the Record Buffer" << endl;
  RecordBuffer rb(rf, 200, 4, true);
  
  cout << "Show the buffer addresses for 100 pages" << endl;
  for (int i=0; i<100; i++) {

    bool change=false;
    void* ptr =  rb.GetBufPtr(i, change);
    cout << " " << (void*) ptr << "(" << change << "), " << endl; 
  }
  cout << endl;
  
  return true;

}


bool
PArrayTest() {
	
  cout << "Tests for the PArray template class!" << endl << endl;
  PagedArray<int> pa(rf);

  int max = 1000000;
  cout << "Storing numbers from 1 to " << max << ", read back, and sum them up ... " << endl;
  for (int j = 0; j < max; j++) {

  int val = j+1;
  pa.Put(j, val);	  
  }	 

  double sum = 0.0;
  for (int k = 0; k < max; k++) {
  int val;
  pa.Get(k, val);
  sum += val;
  }
  cout << "Sum (" 
       << ((double) max)/((double) 2.0) * ((double) max+1) 
       << ") = " << sum << endl; 
  
  return true;
}

bool
PCTableTest() {
	
  cout << "Tests for the PCTable class!" << endl;
	
  bool test = true;
  cout << "true  : " << test << endl;
  test = false;
  cout << "false : " << test << endl;
  
  CTable<bool> bt( 3, rf );
  bool b1;
  
  Cardinal index1=0, index2=0;
  index1 = bt.EmptySlot();
  b1 = false;
  bt.Put(index1, b1);
 
  index2 = bt.EmptySlot();
  b1 = true;
  bt.Put(index2, b1);
  
  cout << "bt[" << index1 << "] false : " << bt[index1] << endl;
  cout << "bt[" << index2 << "] true  : " << bt[index2] << endl;
  
  CTable<int> ct( 5, rf );
  cout << "size = 5: " << ct.Size() << endl;
 
  int val = 1;
  index2 = ct.EmptySlot();
  ct.Put(index2, val);
  int intRef = ct[index2];
  cout << "intRef = 1: " << intRef << endl;
  intRef = 2;
  val = 40;
  index1 = ct.EmptySlot();
  ct.Put(index1, val);
  cout << "ct[2] = 1: " << ct[index2] << endl;
  cout << "size = 5: " << ct.Size() << endl;
  cout << "ct[4] = 40: " << ct[index1] << endl;
  
  cout << "1: " << ct.IsValid( 1 ) << endl;
  cout << "2: " << ct.IsValid( 2 ) << endl;
  ct.Add( 10 );
  ct.Add( 11 );
  ct.Add( 12 );
  ct.Add( 13 );
  ct.Add( 14 );

  // show values
  cout << "values : (1,40,10,11,12,13,14,) --> (";
  for (int k = 1; k <= 7; k++) {
    
     cout << ct[k] << ","; 
  }
  cout << ")" << endl;
  
  // show sizes
  cout << "size 7: " << ct.Size() << endl;
  cout << "NoEntries(): " << ct.NoEntries() << endl;
  
  
  cout << "Test ot iterators. Print 1st and 2nd element of the table" << endl;
  CTable<int>::Iterator it, it2;
  it2 = ct.Begin();
  it = it2++;
  cout << "*it  " << *it  << ", i= " << it.GetIndex()  << endl;
  cout << "*it2 " << *it2 << ", i2=" << it2.GetIndex() << endl;

  cout << "Set first element to value 5 and remove the 5th element." << endl;
  ct.Get(it.GetIndex(), val);
  val = 5;
  ct.Put(it.GetIndex(), val);
  ct.Remove( 5 );

  cout << "Enumerate all used slots ..." << endl;
  for ( it = ct.Begin(); it != ct.End(); ++it )
  {
    cout << "it " << *it << ", i=" << it.GetIndex() << endl;
  }
  cout << "Test for end of scan! Result of it.EndOfScan() && (++it).EndOfScan(): " 
       << (it.EndOfScan() && (++it).EndOfScan())
       << endl;

  //cout << "Id of CTable: " << ct.Id() << endl;

  char inChar;
  cout << "Delete CTable from Disk when terminating (y/n)?";
  cin >> inChar;

  if ( inChar == 'y' || inChar == 'Y' ) {
  // ct.MarkDelete();
  }

  return true;
}


int
main() {

  SmiError rc;
  bool ok;

  rc = SmiEnvironment::StartUp( SmiEnvironment::MultiUser,
                                "SecondoConfig.ini", cerr );
  cout << "StartUp rc=" << rc << endl;
  if ( rc == 1 )
  {
    string dbname;
    cout << "*** Start list of databases ***" << endl;
    SmiEnvironment::ListDatabases( dbname );
    cout << dbname << endl;
    cout << "*** End list of databases ***" << endl;
 
    ok = SmiEnvironment::OpenDatabase( "PARRAY" );   
    if ( ok )
    {
      cout << "OpenDatabase PARRAY ok." << endl;
    }
    else
    {
      cout << "OpenDatabase PARRAY failed, try to create." << endl;
      ok = SmiEnvironment::CreateDatabase( "PARRAY" );
      if ( ok )
        cout << "CreateDatabase PARRAY ok." << endl;
      else
        cout << "CreateDatabase PARRAY failed." << endl;
      
      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase PARRAY ok." << endl;
      else
        cout << "CloseDatabase PARRAY failed." << endl;
      
      if ( ok = SmiEnvironment::OpenDatabase( "PARRAY" ) )
        cout << "OpenDatabase PARRAY ok." << endl;
      else
        cout << "OpenDatabase PARRAY failed." << endl;
    }
    pause();
    if ( ok )
    {
      //cout << "Begin Transaction: " << SmiEnvironment::BeginTransaction() << endl;
    
      int recSize=512;
 
      rf = new SmiRecordFile(true, recSize, true);
      if ( !(rf->Create()) ) {
       string errMsg;
       SmiError errCode = SmiEnvironment::GetLastErrorCode(errMsg);
	 cerr << "SmiError: " << "SmiErrorCode " << errCode << " Msg: " << errMsg << endl;
       exit(0);
      } else {
       cout << "Temporary SmiRecordFile (recSize=" << recSize << ") created!" << endl;
      }

      pause();
      RecordBufferTest(); 
      pause();
      PArrayTest();
      pause();
      PCTableTest();
      
      rf->Close();
      //cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
      cout << "*** Closing Database ***" << endl;
      if ( SmiEnvironment::CloseDatabase() )
        cout << "CloseDatabase ok." << endl;
      else
        cout << "CloseDatabase failed." << endl;
      pause();
    }
  }
  rc = SmiEnvironment::ShutDown();
  cout << "ShutDown rc=" << rc << endl;
  
 
  return 0;
 

}



