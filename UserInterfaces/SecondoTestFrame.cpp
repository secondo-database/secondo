using namespace std;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Application.h"
//#include "Processes.h"
#include "Profiles.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "SecondoInterface.h"
#include "SecondoSMI.h"
#include "NestedList.h"
#include "DisplayTTY.h"


// for testing the Tuple Manager
#include "StandardTypes.h"
#include "Tuple.h"
#include "../Algebras/Polygon/PolygonAlgebra.cpp"


namespace {

static const bool needIdent = false;

class SecondoTestFrame : public Application
{
 public:
  SecondoTestFrame( const int argc, const char** argv );
  virtual ~SecondoTestFrame() {};
  int  Execute();
  bool CheckConfiguration();
  
 private:
  string            parmFile;
  string            user;
  string            pswd;
  string            host;
  string            port;
  string            iFileName;
  string            oFileName;
  string            cmd;
//  bool              isStdInput;
  bool              quit;
//  NestedList*       nl;
//  AlgebraLevel      currentLevel;
//  bool              isQuery;
  SecondoInterface* si;
  
  /* Test methods for the tuple manager. */
  void Test01(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test02(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test03(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test04(const TupleAttributes *attributes, SmiRecordFile *recFile, bool &trans);
  void Test05(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test06(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test07(const TupleAttributes *attributes, SmiRecordFile *recFile);
  void Test08(const TupleAttributes *attributes, SmiRecordFile *recFile);
};

SecondoTestFrame::SecondoTestFrame( const int argc, const char** argv )
  : Application( argc, argv )
{
  parmFile      = "";
  user          = "";
  pswd          = "";
  host          = "";
  port          = "";
  iFileName     = "";
  oFileName     = "";
  string cmd    = "";
//  isStdInput    = true;
  quit          = false;
//  nl            = 0;
//  currentLevel  = DescriptiveLevel;
  si            = 0;
}


/*
13 SecondoMain

This is the function where everything is done. If one wants to use the
storage manager provided by SHORE, one should struture its program
like this. Since using SHORE makes necessary to be on top of a thread, 
we cannot just initiate the storage manager and then keep in the same
function. Using SHORE we must use this workaround. Call the initiation 
method from the main function, and then we know that the functions
SecondoMain will be "called back"

*/

/*
1 CheckConfiguration

This function checks the Secondo configuration. First it looks for the name
of the configuration file on the command line. If no file name was given on
the command line or a file with the given name does not exist, the environment
variable SECONDO\_HOME is checked. If this variable is defined it should point
to a directory where the configuration file can be found. If the configuration
file is not found there, the current directory will be checked. If no configuration
file can be found the program terminates.

If a valid configuration file was found initialization continues.

*/

bool
SecondoTestFrame::CheckConfiguration()
{
  bool ok = true;
  int i = 1;
  string argSwitch, argValue;
  bool argOk;
  while (i < GetArgCount())
  {
    argSwitch = GetArgValues()[i];
    if ( i < GetArgCount()-1)
    {
      argValue  = GetArgValues()[i+1];
      argOk = (argValue[0] != '-');
    }
    else
    {
      argValue = "";
      argOk = false;
    }
    if ( argSwitch == "-?" || argSwitch == "--help" )  // Help
    {
      cout << "Usage: SecondoTTY{BDB|ORA|CS} [options]" << endl << endl
           << "Options:                                             (Environment)" << endl
           << "  -c config  : Secondo configuration file            (SECONDO_CONFIG)" << endl
           << "  -i input   : Name of input file  (default: stdin)" << endl
           << "  -o output  : Name of output file (default: stdout)" << endl
           << "  -u user    : User id                               (SECONDO_USER)" << endl
           << "  -s pswd    : Password                              (SECONDO_PSWD)" << endl
           << "  -h host    : Host address of Secondo server        (SECONDO_HOST)" << endl
           << "  -p port    : Port of Secondo server                (SECONDO_PORT)" << endl << endl
           << "Command line options overrule environment variables." << endl;
      ok = false;
      break;
    }
    else if ( argOk && argSwitch == "-c" )  // Configuration file
    {
      parmFile = argValue;
    }
    else if ( argOk && argSwitch == "-i" )  // Input file
    {
      iFileName = argValue;
    }
    else if ( argOk && argSwitch == "-o" )  // Output file
    {
      oFileName = argValue;
    }
    else if ( argOk && argSwitch == "-u" )  // User id
    {
      user = argValue;
    }
    else if ( argOk && argSwitch == "-s" )  // Password
    {
      pswd = argValue;
    }
    else if ( argOk && argSwitch == "-h" )  // Host
    {
      host = argValue;
    }
    else if ( argOk && argSwitch == "-p" )  // Port
    {
      port = argValue;
    }
    else
    {
      cout << "Error: Invalid option: '" << argSwitch << "'." << endl;
      if ( argOk )
      {
        cout << "  having option value: '" << argValue << "'." << endl;
      }
      cout << "Use option -? or --help to get information about available options." << endl;
      ok = false;
    }
    i++;
    if ( argOk )
    {
      i++;
    }
  }
  char* envValue;
  if ( parmFile.length() == 0 )
  {
    envValue = getenv( "SECONDO_CONFIG" );
    if ( envValue != 0 )
    {
      parmFile = envValue;
    }
  }
  if ( user.length() == 0 )
  {
    envValue = getenv( "SECONDO_USER" );
    if ( envValue != 0 )
    {
      user = envValue;
    }
  }
  if ( pswd.length() == 0 )
  {
    envValue = getenv( "SECONDO_PSWD" );
    if ( envValue != 0 )
    {
      pswd = envValue;
    }
  }
  if ( host.length() == 0 )
  {
    envValue = getenv( "SECONDO_HOST" );
    if ( envValue != 0 )
    {
      host = envValue;
    }
  }
  if ( port.length() == 0 )
  {
    envValue = getenv( "SECONDO_PORT" );
    if ( envValue != 0 )
    {
      port = envValue;
    }
  }
  if ( needIdent ) // Is user identification needed?
  {
    int count = 0;
    while (count <= 3 && user.length() == 0)
    {
      count++;
      cout << "Enter user id: ";
      getline( cin, user );
    }
    ok = user.length() > 0;
    if ( !ok )
    {
      cout << "Error: No user id specified." << endl;
    }
    if ( ok && pswd.length() == 0 )
    {
      count = 0;
      while (count <= 3 && user.length() == 0)
      {
        count++;
        cout << "Enter password: ";
        getline( cin, pswd );
      }
      if ( pswd.length() == 0 )
      {
        cout << "Error: No password specified." << endl;
        ok = false;
      }
    }
  }
  else
  {
    user = "SECONDO";
    pswd = "SECONDO";
  }
  if ( ok )
  {
    // config file or (host and port) must be specified
    ok = parmFile.length() > 0 || (host.length() > 0 && port.length() > 0);
    if ( !ok )
    {
      cout << "Error: Neither config file nor host and port of Secondo server specified." << endl;
    }
  }
  return (ok);
}

/*

Test 01: create new tuple and save to file

*/
void SecondoTestFrame::Test01(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;	
	float realv;
	int intv;
	char boolv;
	bool bboolv;
	int numberOfPoints;	
	int *X;
	int *Y;
	int i;
	CcReal *real1;
	CcInt *int1;
	CcBool *bool1;
	SmiRecordFile *lobFile;
	bool lobFileOpen;
	Polygon* polygon1;
	SmiRecordId recId;

	myTuple = new Tuple(attributes);
	cout << "\ta float value, please: "; cin >> realv;
	cout << "\tan int value, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value, please: "; cin >> boolv;
	bboolv = ((boolv == 't') ? true : false);
	cout << "\thow many points, please: "; cin >> numberOfPoints;
	X = new int[numberOfPoints];
	Y = new int[numberOfPoints];
						
	for (i = 0; i < numberOfPoints; i++) {
		cout << "\t" << (i+1) << ". Point:" << endl;
		cout << "\t\tX: "; cin >> X[i];
		cout << "\t\tY: "; cin >> Y[i];
	}
						
	real1 = new CcReal(true, realv);
    int1 = new CcInt(true, intv);
	bool1 = new CcBool(true, bboolv);
					
	lobFile = new SmiRecordFile(false);
	lobFileOpen = lobFile->Open("LOBFILE");

	polygon1 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
	myTuple->Put(0, int1);
	myTuple->Put(1, bool1);
	myTuple->Put(2, real1);
	myTuple->Put(3, polygon1);
					
	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
	cout << "\tSave tuple into recFile. Persistent id = ";

	myTuple->SaveTo(recFile, lobFile);
	recId = myTuple->GetPersistentId();
	cout << recId << endl;
	lobFile->Close();
	
	delete polygon1;
	delete lobFile;
	delete real1;
	delete int1;
	delete bool1;
	delete[] X;
	delete[] Y;
	delete myTuple;
}

/*

Test 02: read tuple from file

*/
void SecondoTestFrame::Test02(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;	
	SmiRecordId recId;

	cout << "\tID:";
	cin >> recId;
	cout << "\trecId = " << recId << endl;
	cout << "\treading tuple." << endl;
						
	myTuple = new Tuple(recFile, recId, attributes, SmiFile::ReadOnly);
						
	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
						
	delete(myTuple);
}

/*

Test 03: read tuple, change core attributes and resave to file." << endl;

*/
void SecondoTestFrame::Test03(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;	
	float realv;
	int intv;
	char boolv;
	bool bboolv;
	CcReal *real1;
	CcInt *int1;
	CcBool *bool1;
	SmiRecordId recId;
	
	cout << "\tID:";
	cin >> recId;
	cout << "\trecId = " << recId << endl;
						
	cout << "\treading tuple." << endl;
	myTuple = new Tuple(recFile, recId, attributes, SmiFile::ReadOnly);

	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;

	cout << "\ta new float value, please: "; cin >> realv;
	cout << "\ta new int value, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta new boolean value, please: "; cin >> boolv;
	bboolv = ((boolv == 't') ? true : false);
	real1 = new CcReal(true, realv);
    int1 = new CcInt(true, intv);
	bool1 = new CcBool(true, bboolv);
					
	myTuple->Put(0, int1);
	myTuple->Put(1, bool1);
	myTuple->Put(2, real1);
						
	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
	myTuple->Save();
					
	delete bool1;
	delete int1;
	delete real1;	
	delete(myTuple);
}

/*

Test 04: create lots of tuples and save to file

*/
void SecondoTestFrame::Test04(const TupleAttributes *attributes, SmiRecordFile *recFile, bool& trans) {
	Tuple* myTuple;	
	float realv;
	int intv;
	char boolv;
	bool bboolv;
	int numberOfPoints;
	int numberOfTuples;
	int *X;
	int *Y;
	int i;
	int j;
	CcReal *real1;
	CcInt *int1;
	CcBool *bool1;
	SmiRecordFile *lobFile;
	bool lobFileOpen;
	Polygon* polygon1;
	SmiRecordId recId;
	
	cout << "\tnumber of tuples, please: "; cin >> numberOfTuples;
	
	cout << "\ta start float value, please: "; cin >> realv;
	cout << "\tan start int value, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value, please: "; cin >> boolv;
	bboolv = ((boolv == 't') ? true : false);
	cout << "\thow many points, please: "; cin >> numberOfPoints;

	X = new int[numberOfPoints];
	Y = new int[numberOfPoints];
		
	for (i = 0; i < numberOfPoints; i++) {
		cout << "\t" << (i+1) << ". Point." << endl;
		cout << "\t\tX: "; cin >> X[i];
		cout << "\t\tY: "; cin >> Y[i];
	}

	for (j = 0; j < numberOfTuples; j++) {
		myTuple = new Tuple(attributes);					
		real1 = new CcReal(true, realv + j);
    	int1 = new CcInt(true, intv + j);
		bool1 = new CcBool(true, bboolv);
					
		lobFile = new SmiRecordFile(false);
		lobFileOpen = lobFile->Open("LOBFILE");

		polygon1 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
		myTuple->Put(0, int1);
		myTuple->Put(1, bool1);
		myTuple->Put(2, real1);
		myTuple->Put(3, polygon1);
					
		cout << "\ttest tuple values" << endl;
		cout << "\t" << *myTuple << endl;
		cout << "\tSize: " << myTuple->GetSize() << endl;
		cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
		cout << "\tSave tuple into recFile. Persistent id = ";

		myTuple->SaveTo(recFile, lobFile);
		recId = myTuple->GetPersistentId();
		cout << recId << endl;
		lobFile->Close();
		
		delete real1;
		delete int1;
		delete bool1;
		delete lobFile;
		delete(myTuple);
		delete(polygon1);
	}
	
	delete[] X;
	delete[] Y;
}
/*

Test 05: show all tuples in the file. 

*/
void SecondoTestFrame::Test05(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	cout << "\tnot yet implemented." << endl;
	/*
	it = new SmiRecordFileIterator();
	myTuple = new Tuple(&tupleType1);
	do {
		more = it->Next(*myTuple);
		cout << "\t" << *myTuple << endl;
	} while (more); */
}

/*

Test 06: create fresh tuple, destroy it and '(re-)SaveTo' it

*/
void SecondoTestFrame::Test06(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;	
	float realv;
	int intv;
	char boolv;
	bool bboolv;
	int numberOfPoints;	
	int *X;
	int *Y;
	int i;
	CcReal *real1;
	CcInt *int1;
	CcBool *bool1;
	SmiRecordFile *lobFile;
	bool lobFileOpen;
	Polygon* polygon1;
	SmiRecordId recId;

	myTuple = new Tuple(attributes);
	cout << "\ta float value, please: "; cin >> realv;
	cout << "\tan int value, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value, please: "; cin >> boolv;
	bboolv = ((boolv == 't') ? true : false);
	cout << "\thow many points, please: "; cin >> numberOfPoints;
	X = new int[numberOfPoints];
	Y = new int[numberOfPoints];
						
	for (i = 0; i < numberOfPoints; i++) {
		cout << "\t" << (i+1) << ". Point:" << endl;
		cout << "\t\tX: "; cin >> X[i];
		cout << "\t\tY: "; cin >> Y[i];
	}
						
	real1 = new CcReal(true, realv);
    int1 = new CcInt(true, intv);
	bool1 = new CcBool(true, bboolv);
					
	lobFile = new SmiRecordFile(false);
	lobFileOpen = lobFile->Open("LOBFILE");

	polygon1 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
	myTuple->Put(0, int1);
	myTuple->Put(1, bool1);
	myTuple->Put(2, real1);
	myTuple->Put(3, polygon1);
					
	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
	cout << "\tSave tuple into recFile. Persistent id = ";

	myTuple->SaveTo(recFile, lobFile);
	recId = myTuple->GetPersistentId();
	cout << recId << endl;
	cout << "\tDestroying the tuple..." << endl;
	myTuple->Destroy();
	cout << "\tNow this tuple is a fresh one." << endl;	
	cout << "\tResave this tuple..." << endl;
	myTuple->SaveTo(recFile, lobFile);
	recId = myTuple->GetPersistentId();
	cout << "\tnew Persistent id = " << recId << endl;
	lobFile->Close();
	
	delete polygon1;
	delete lobFile;
	delete real1;
	delete int1;
	delete bool1;
	delete[] X;
	delete[] Y;
	delete myTuple;
}

/*

Test 07: create two new tuples with one shared float component, save both to file.

*/
void SecondoTestFrame::Test07(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;
	Tuple* myTuple2;
	float realv;
	int intv;
	int intv2;
	char boolv;
	char boolv2;
	bool bboolv;
	bool bboolv2;
	int numberOfPoints;	
	int *X;
	int *Y;
	int i;
	CcReal *real1;
	CcInt *int1;
	CcInt *int2;
	CcBool *bool1;
	CcBool *bool2;
	SmiRecordFile *lobFile;
	bool lobFileOpen;
	Polygon* polygon1;
	Polygon* polygon2;
	SmiRecordId recId;

	myTuple = new Tuple(attributes);
	myTuple2 = new Tuple(attributes);
	
	cout << "\ta float value for both tuples, please: "; cin >> realv;
						
	cout << "\tan int value for the first tuple, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value for the first tuple, please: "; cin >> boolv;
						
	cout << "\tan int value for the second tuple, please: "; cin >> intv2;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value for the second tuple, please: "; cin >> boolv2;
	bboolv = ((boolv == 't') ? true : false);
	bboolv2 = ((boolv2 == 't') ? true : false);
	
	cout << "\thow many points, please: "; cin >> numberOfPoints;
	X = new int[numberOfPoints];
	Y = new int[numberOfPoints];
						
	for (i = 0; i < numberOfPoints; i++) {
		cout << "\t" << (i+1) << ". Point:" << endl;
		cout << "\t\tX: "; cin >> X[i];
		cout << "\t\tY: "; cin >> Y[i];
	}

	lobFile = new SmiRecordFile(false);
	lobFileOpen = lobFile->Open("LOBFILE");
	
	real1 = new CcReal(true, realv);
    int1 = new CcInt(true, intv);
	bool1 = new CcBool(true, bboolv);
	int2 = new CcInt(true, intv2);
	bool2 = new CcBool(true, bboolv2);

	polygon1 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
	polygon2 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
	
	myTuple->DelPut(0, int1);
	myTuple->DelPut(1, bool1);
	myTuple->DelPut(2, real1);
	myTuple->Put(3, polygon1);
	myTuple2->DelPut(0, int2);
	myTuple2->DelPut(1, bool2);
	myTuple2->AttrPut(2, myTuple, 2);
	myTuple2->Put(3, polygon2);
					
	cout << "\ttest tuple values" << endl;
	cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
	cout << "\tSave tuple into recFile. Persistent id = ";

	myTuple->SaveTo(recFile, lobFile);
	recId = myTuple->GetPersistentId();
	cout << recId;
	
	myTuple2->SaveTo(recFile, lobFile);
	recId = myTuple2->GetPersistentId();
	cout << ", Persistent id = " << recId << endl;
	
	lobFile->Close();
	
	delete polygon1;
	delete polygon2;
	delete lobFile;
	delete real1;
	delete int1;
	delete bool1;
	delete[] X;
	delete[] Y;
	delete myTuple2;
	delete myTuple;
}


/*

Test 08: create new tuple with lots of points and save to file

*/
void SecondoTestFrame::Test08(const TupleAttributes *attributes, SmiRecordFile *recFile) {
	Tuple* myTuple;	
	float realv;
	int intv;
	char boolv;
	bool bboolv;
	int numberOfPoints;	
	int *X;
	int *Y;
	int i;
	CcReal *real1;
	CcInt *int1;
	CcBool *bool1;
	SmiRecordFile *lobFile;
	bool lobFileOpen;
	Polygon* polygon1;
	SmiRecordId recId;

	myTuple = new Tuple(attributes);
	cout << "\ta float value, please: "; cin >> realv;
	cout << "\tan int value, please: "; cin >> intv;
	cout << "\tt = true, f = false" << endl;
	cout << "\ta boolean value, please: "; cin >> boolv;
	bboolv = ((boolv == 't') ? true : false);
	cout << "\thow many points, please: "; cin >> numberOfPoints;
	X = new int[numberOfPoints];
	Y = new int[numberOfPoints];
						
	for (i = 0; i < numberOfPoints; i++) {
		X[i] = i;
		Y[i] = i * i;
	}
						
	real1 = new CcReal(true, realv);
    int1 = new CcInt(true, intv);
	bool1 = new CcBool(true, bboolv);
					
	lobFile = new SmiRecordFile(false);
	lobFileOpen = lobFile->Open("LOBFILE");

	polygon1 = new Polygon(lobFile, numberOfPoints, (char *)X, (char *)Y);
	myTuple->Put(0, int1);
	myTuple->Put(1, bool1);
	myTuple->Put(2, real1);
	myTuple->Put(3, polygon1);
					
	//cout << "\ttest tuple values" << endl;
	//cout << "\t" << *myTuple << endl;
	cout << "\tSize: " << myTuple->GetSize() << endl;
	cout << "\tAttributes: " << myTuple->GetAttrNum() << endl;
	cout << "\tSave tuple into recFile. Persistent id = ";

	myTuple->SaveTo(recFile, lobFile);
	recId = myTuple->GetPersistentId();
	cout << recId << endl;
	lobFile->Close();
	
	delete polygon1;
	delete lobFile;
	delete real1;
	delete int1;
	delete bool1;
	delete[] X;
	delete[] Y;
	delete myTuple;
}

//

/*
1 Execute

This function checks the configuration of the Secondo system. If the configuration
seems to be ok the system is intialized. If the initialization succeeds the user
commands are processed. If the initialization fails or the user finishes work
the system is terminated.

*/

int SecondoTestFrame::Execute() {
	int rc = 0;
	cout << endl << "*** Secondo Testframe ***" << endl << endl;
	
	if (CheckConfiguration()) {
		si = new SecondoInterface();
		if (si->Initialize(user, pswd, host, port, parmFile)) {
			/* start of test code. */
            SecondoCatalog* exCatalog = 0;
	    	int algIdStandard = 0;
	    	int CcRealId = 0, CcIntId =0, CcStringId = 0, CcBoolId =0;

	    	cout << "* Try to get catalog of exec. level. " << endl;
	    	exCatalog = SecondoSystem::GetCatalog(ExecutableLevel);
		
			cout << "************************************************" << endl;
			cout << "* Try to get algebra and type ids for polygon. *" << endl;
			cout << "************************************************" << endl;
			
			int algIdPolygon = 0;
			int CcPolygonId = 0;

			if ((exCatalog->GetTypeId("polygon", algIdPolygon, CcPolygonId) == true)) {
				cout << "* Polygon --> algId: " << algIdPolygon << ", typeid: " << CcPolygonId << endl;
			}
			else {
				cout << "* failed." << endl;
			}

	    	cout << "*---------- Try to get algebra and type ids. " << endl;
	    	if ( (exCatalog->GetTypeId("int", algIdStandard, CcIntId) == true) ) {
	      		cout << "* int --> algId: " << algIdStandard << ", typeId: " << CcIntId << endl;
            } 
	    	else {
	      		cout << "* failed" << endl;
	    	}
	    	if ((exCatalog->GetTypeId("string", algIdStandard, CcStringId) == true)) {
	      		cout << "* string --> algId: " << algIdStandard << ", typeId: " << CcStringId << endl;
            }
	    	else {
	      		cout << "* failed" << endl;
	    	}
	   		if ( (exCatalog->GetTypeId("bool", algIdStandard, CcBoolId) == true) ) {
	      		cout << "* bool --> algId: " << algIdStandard << ", typeId: " << CcBoolId << endl;
            } 
	    	else {
	      		cout << "* failed" << endl;
	    	}
	    	if ((exCatalog->GetTypeId("real", algIdStandard, CcRealId) == true)) {
	      		cout << "* real --> algId: " << algIdStandard << ", typeId: " << CcRealId << endl;
            } 
	    	else {
	      		cout << "* failed" << endl;
	    	}
			cout << "*************************************************************************" << endl;
		
	    	cout << "* Get a reference of the Algebra Manager" << endl;
	    	AlgebraManager* algM = 0;
	    	algM = SecondoSystem::GetAlgebraManager();
	    
	    	cout << "* Get cast function for type real" << endl;
	    	ObjectCast oca;
	    	oca = algM->Cast(algIdStandard,CcRealId);
	    
	    	cout << "* assemble attribute types" << endl;
	    
	    	AttributeType realAttr = {algIdStandard, CcRealId, sizeof(CcReal)};
	      	AttributeType intAttr = {algIdStandard, CcIntId, sizeof(CcInt)};
	      	AttributeType boolAttr = {algIdStandard, CcBoolId, sizeof(CcBool)};
			AttributeType polygonAttr = {algIdPolygon, CcPolygonId, sizeof(Polygon)};

         	AttributeType attrTypes[] = {intAttr, boolAttr, realAttr, polygonAttr};

	    	cout << "* create tuple attribute description" << endl;
	    
        	TupleAttributes tupleType1(4, attrTypes);
	   
	    	cout << "* create tuple" << endl;

			bool cdb = SmiEnvironment::OpenDatabase("LOBDB");
			if (cdb == false) {
				cout << "* Database opened: NO!!" << endl;
				cdb = SmiEnvironment::CreateDatabase("LOBDB");
				cout << "* Database created:";
				if (cdb == true) {
					cout << "yes" << endl;
				}
				else {
					cout << "NO DATABASE CREATED." << endl;
				}
			}
			else {
				 cout << "* Database opened: yes" << endl;
			}
			
			SmiRecordFile *recFile = new SmiRecordFile(false);

			bool recFileOpen = recFile->Open("RECFILE");
			int recFileId = recFile->GetFileId();

			cout << "* Database opened/created: " << (cdb ? "yes" : 		"NO!!!!!!") << endl;
			cout << "* File for record opened:  " << ((recFileOpen == true) ? "yes" : "NO!!!!!!") << endl;
					
			bool trans = SmiEnvironment::BeginTransaction();
			cout << "* begin of transactions: ";
			cout << ((trans == true) ? " OK" : " failed.") << endl;
			
			int choice;
			 		
			do {
				cout << "* Test menu: " << endl << endl;
				cout << "1) create new tuple and save to file" << endl;
				cout << "2) read tuple from file" << endl;
				cout << "3) read tuple, change core attributes and resave to file." << endl;
				cout << "4) create lots of tuples and save to file" << endl;
				cout << "5) show all tuples in the file. " << endl;
				cout << "6) create fresh tuple, destroy it and '(re-)SaveTo' it" << endl;
				cout << "7) create two new tuples with one shared float component, save both to file." << endl;
				cout << "8) create new tuple with lots of points and save to file" << endl;

				cout << endl;
				cout << "9) end." << endl;
			
				cout << "Your choice: ";
				cin >> choice;
				
				switch (choice) {
					case 1: Test01(&tupleType1, recFile); break;
					case 2: Test02(&tupleType1, recFile); break;
					case 3: Test03(&tupleType1, recFile); break;
					case 4: Test04(&tupleType1, recFile, trans); break;
					case 5: Test05(&tupleType1, recFile); break;
					case 6: Test06(&tupleType1, recFile); break;
					case 7: Test07(&tupleType1, recFile); break;
					case 8: Test08(&tupleType1, recFile); break;
					case 9: break;
				}
			} while (choice != 9);
					
			trans = SmiEnvironment::CommitTransaction();
			cout << "* end (commit) of transactions: ";
			cout << ((trans == true) ? " OK" : " failed.") << endl;
		
			recFileId = recFile->GetFileId();
			recFile->Close();
			
			delete recFile;
    	}
    	si->Terminate();
		
    	delete si;
    	cout << "*** SecondoTestFrame terminated. ***" << endl;
  	}
  	else {
    	rc = 1;
	}
  return rc;
}

} // end of namespace{

/*
14 main

The main function creates the Secondo TTY application and starts its execution.

*/
int main( const int argc, const char* argv[] ) {
  SecondoTestFrame* appPointer = new SecondoTestFrame( argc, argv );
  int rc = appPointer->Execute();
  delete appPointer;
  return (rc);
}



