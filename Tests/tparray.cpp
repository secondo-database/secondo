#include <iostream>
#include <string>
#include "SecondoSMI.h"
#include "PArray.h"
using namespace std;

void pause()
{
  char buf[80];
  cout << "<<< Press return to continue >>>" << endl;
  cin.getline( buf, sizeof(buf) );
}

int main( int argc, char* argv[] )
{
  SmiError rc;
  bool ok;

  rc = SmiEnvironment::StartUp( SmiEnvironment::MultiUser,
  				"SecondoConfig.ini", cerr );
  cout << "StartUp rc = " << rc << endl;
  if ( rc == 1 )
  {
    string dbname;
    cout << "*** Start list of databases ***" << endl;
    while ( SmiEnvironment::ListDatabases( dbname ))
    {
      cout << dbname << endl;
    }
    cout << "*** End list of databases ***" << endl;

    pause();
  }

  ok = SmiEnvironment::OpenDatabase("parray");
  if ( ok )
  {
    cout << "OpenDatabase parray ok." << endl;
  }
  else
  {
    cout << "Creating database parray." << endl;
    ok = SmiEnvironment::CreateDatabase("parray");
    if ( ok )
    {
      cout << "Database parray created." << endl;
    }
    else
    {
      cout << "Creating database parray failed." << endl;
    }

  }
  pause();

  cout << "Begin Transaction: "
  	<< SmiEnvironment::BeginTransaction() << endl;
  	
/*
  SmiRecordFile rf(false, 0);
  ok = rf.Open("parrayfile");

  	cout << "parrayfile opened: " << ok << endl;
  	
  ok = rf.Drop();

  	cout << "parrayfile dropped: " << ok << endl;
  	
*/

/*
Test of Parray

*/

  PArray<int> *intarray = new PArray<int>( 2000000 );

        cout << "array created"  << endl;

  for (int i = 0; i < 100; i++)
  {
    int j = i;
    intarray->Put(i, j);

    	cout << "number entered " << j << endl;
    	
    int k;
    intarray->Get(i, k);

    	cout << "number read " << k << endl;    	
  }

  int thousand = 1000;
  intarray->Put(1000000, thousand);
  SmiRecordId id = intarray->Id();

  delete intarray;

  cout << "Numbers written into PArray, record id = " << id << endl;

  cout << "Commit: "
  	<< SmiEnvironment::CommitTransaction() << endl;
  	
  cout << "Begin Transaction: "
  	<< SmiEnvironment::BeginTransaction() << endl;


  intarray = new PArray<int>( id, false );         		

  cout << "intarray opened." << endl;

  int i1, i2, i3;
  intarray->Get(16, i1);
  cout << "i1 = " << i1 << endl;

  intarray->Get(52, i2);
  cout << "i2 = " << i2 << endl;

  intarray->Get(52399, i3);
  cout << "i3 = " << i3 << endl;


//   intarray->MarkDelete();
  delete intarray;

  cout << "Commit: "
  	<< SmiEnvironment::CommitTransaction() << endl;




  if ( SmiEnvironment::CloseDatabase() )
  {
    cout << "Database parray closed." << endl;
  }
  else
  {
    cout << "Closing database parray failed." << endl;
  }

  rc = SmiEnvironment::ShutDown();
  cout << "ShutDown rc = " << rc << endl;
}

  				
  				
  				
  				
  				
  				
  				
  				
  				
  				
