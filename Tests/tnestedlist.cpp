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

//paragraph [1] Title: [{\Large \bf ] [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[->] [$\rightarrow $]

[1] Module TestNL: Tests with Stable Nested Lists


April 1996 Carsten Mund

Februar 2002 M. Spiekermann, Environment for the SMI was added.

August 2003 M. Spiekermann, The test code for the persistent and 
main-memory implementations were merged into one code file. 

Jan 2005 M. Spiekermann, Some general useful functions were moved to class
~CTestFrame~


1 Introduction

In this module the nested list functions from the module NestedList are 
called to test them and to demonstrate how to use them.

*/

#include <string>
#include <iostream>
#include <fstream>

#include "NestedList.h"
#include "SecondoSMI.h"
#include "WinUnix.h"
#include "CTestFrame.h"


using namespace std;

struct IntPairs {

   int v;
   ListExpr l;
   IntPairs(int val) : v(val), l(0) {};
};


/*

3.2 Local Procedures

*/

class TestNestedList : public CTestFrame {

private:

  int recCounter;
  NestedList* nl;
  SmiRecordFile* rf;
  const string filePrefix;

public:

  TestNestedList(char x, const string& prefixStr) : CTestFrame(x), 
    recCounter(0),
    nl(0),
    rf(0),
    filePrefix(prefixStr) {}

  ~TestNestedList() {}

ListExpr
ConcatLists( ListExpr list1, ListExpr list2)
{
  recCounter++;
  if (nl->IsEmpty(list1))
  {
    cerr << endl << "### ConcatLists - terminated." << endl;
    return list2;
  }
  else
  {
    ListExpr first = nl->First(list1);
    ListExpr rest = nl->Rest(list1);

    //cerr << "( rec:" << recCounter << " , first:" << first << ", rest:" << rest << " )"; 

    ListExpr second =  ConcatLists(rest, list2);

    ListExpr newnode = nl->Cons(first,second);
    return newnode;
  }
}




/*

4 Module body


4.1 Copy lists between two C++ NestedList-Objects

*/



int
TestNLCopy()
{
   int testcase = 0;
   NestedList nA(rf,1000,1000,1000,1000);
   NestedList nB(rf,1000,1000,1000,1000);

   TestCase("Copy Lists between NL-Instances nA, nB");

   cout  << " listA: " << endl << nA.ReportTableSizes() << endl
         << " listB: " << endl << nB.ReportTableSizes() << endl;

   vector<string> listStr(4);

   listStr[0]=string("(create fifty : (rel(tuple((n int)))))");
   listStr[1]=string("(0 (1 2) (3 4))"); 
   listStr[2]=string("(1 2 (3 5) 7 (9 2 3 4 5) (2 (3 4 (5 6)) 4))");
   listStr[3]=string("(1 \"jhgjhg\" 6 <text> hallo dies ist ein recht langer und langweiliger Text, hier gibt es nichts zu erfahren. Wir wollen nur testen ob Text-Atome korrekt behandelt werden. </text---> \"anbsdfklsd sksdjf sdksdf sdfj asdkjf sdkjssd\" 7 (9 10))");

   vector<string>::iterator it;

   for ( it = listStr.begin(); it != listStr.end(); it++ ) {

     ListExpr listA = 0;
     cout << ++testcase << " nA.ReadFromString(): " << endl;
     nA.ReadFromString(*it, listA);
     cout << " nA.CopyList(): A->B " << endl;
     ListExpr listB = nA.CopyList(listA, &nB);

     cout << " B.WriteToString: A->B " << endl;
     string strB = "";
     nB.WriteToString(strB, listB);
     cout << "  strA: " << *it << endl;
     cout << "  strB: " << strB << endl;
     ListExpr copyOfA;
     nA.ReadFromString(strB, copyOfA);
     CheckResult("Equal", nA.Equal(listA, copyOfA), true);

  }

     cout << " ReportTableSizes(): " << endl
	  << " listA: " << endl << nA.ReportTableSizes() << endl
	  << " listB: " << endl << nB.ReportTableSizes() << endl;

  return 0;
}

/*

The next functions contain code which is extraced from
the secondo system to isolate bugs.

*/

void
StringAtom_bug() {

   //NestedList nl(10,10,10,10);
   NestedList nl(rf,10,10,10,10);

   cout << "Test of String Atoms." << endl << endl;
   cout << "MemoryModel: " << nl.MemoryModel() << endl;

   NestedList* pnl = &nl;

   /* An example of an algebra property */
   ListExpr examplelist = pnl->TextAtom();
   pnl->AppendText(examplelist,"<relation> createbtree [<attrname>] where "
   "<attrname> is the key");

   ListExpr result = pnl->TwoElemList(
            pnl->TwoElemList(pnl->StringAtom("Creation"), 
			     pnl->StringAtom("Example Creation")),    
            pnl->TwoElemList(examplelist, 
			     pnl->StringAtom("(let mybtree = ten "
			     "createbtree [no])")));
   cerr << endl << "### BTreeProp(): " << pnl->ToString(result) << endl;

  exit(0);

}

void
ConcatLists_bug() {

   //NestedList Nl(10,10,10,10);
   NestedList Nl(rf,10,10,10,10);
   nl = &Nl;

   cout << "Test of String Atoms." << endl << endl;
   cout << "MemoryModel: " << nl->MemoryModel() << endl;

   ListExpr headerlist=0;
   ListExpr concatenatedlist = 0;
   nl->ReadFromFile("ListOperators.nl", headerlist);
   headerlist = nl->Second(headerlist);

  concatenatedlist = nl->TheEmptyList();
  concatenatedlist = nl->Second( nl->First(headerlist) );
  headerlist = nl->Rest(headerlist);

  recCounter = 0;
  while (!nl->IsEmpty( headerlist ))
  {
    cout << " header: " << nl->ToString(headerlist) << endl; 
    concatenatedlist = 
      ConcatLists( concatenatedlist, nl->Second(nl->First(headerlist)) );
    headerlist = nl->Rest(headerlist);
    recCounter++;
  }
 
  cerr << endl << "### concatenatedlist: " << nl->ToString(concatenatedlist);

}

void empty_textResult() {

   NestedList nl(rf,10,10,10,10);

   //string s1("(0 0 <text></text---> ())");
   string s1("()");
  
   ListExpr list1=0;

   nl.ReadFromString(s1,list1);
   cout << "string s1: " <<  nl.ToString(list1) << endl;
   cout << "WriteListExpr: " << list1 << endl;
   nl.WriteListExpr(list1); 
   
   string outname("empty_text.bnl");
   cout << endl << "Writing " + outname << endl;
   ofstream outFile2(outname.c_str(), ios::out|ios::trunc|ios::binary); 
   nl.WriteBinaryTo(list1, outFile2);   
   outFile2.close();

}



int 
TestBasicOperations()
{
   ListExpr  ListExpr1 = 0,  ListExpr2 = 0,  ListExpr3 = 0, 
	     ListExpr4 = 0,  ListExpr5 = 0,  ListExpr6 = 0,
	     ListExpr8 = 0,  ListExpr9 = 0, ListExpr15 = 0,
	     EmptyListVar = 0,
	     IntAtomVar = 0,
	     BoolAtomVar = 0,
	     StringAtomVar = 0,
	     SymbolAtomVar = 0,
	     TextAtomVar = 0, 
             TextAtomVar2 = 0;

   long ErrorVar = 0;
   bool BoolValue = false, BoolValue2 = false;

   string NLStringValue = "", NLStringValue2 = "", SymbolValue = "", SymbolValue2 = "";
   string String1 = "", String2 = "", String3 = "", Chars = "";
   TextScan TextScan1;
   Cardinal Position = 0;

   
   NestedList nl(rf,1000,1000,1000,1000);

   ListExpr sym2 = nl.OneElemList(nl.SymbolAtom("ERRORS"));
   nl.ReadFromString( "(open database opt)", sym2 );
   nl.WriteListExpr(sym2);

/*

4.1 Elementary methods 

4.1.1 Empty List 

*/
   
   TestCase("Empty List");
   bool ok = false;
   ok = BeginCheck("TheEmptyList, IsEmpty, ReadFromString, ToString, WriteListExpr, WriteBinaryTo ");
   EmptyListVar = nl.TheEmptyList(); 
   ok = nl.IsEmpty(EmptyListVar) && ( !nl.IsEmpty(7) );

   //string s1("(0 0 <text></text---> ())");
   string s1("()");
   ListExpr list1=0;

   nl.ReadFromString(s1,list1);
   cout << "ToString s1 = " <<  nl.ToString(list1) << endl;
   cout << "WriteListExpr: " << list1 << endl;
   nl.WriteListExpr(list1); 
   
   string outname("empty_text.bnl");
   cout << endl << "Writing " + outname << endl;
   ofstream outFile2(outname.c_str(), ios::out|ios::trunc|ios::binary); 
   nl.WriteBinaryTo(list1, outFile2);   
   outFile2.close();


   EndCheck(ok);

/*

4.1.2 Integer atoms

*/
   
   TestCase("Integer Atoms");
   vector<IntPairs> IntValues;
      
   IntValues.push_back( IntPairs(0) );
   IntValues.push_back( IntPairs(1) );
   IntValues.push_back( IntPairs(-1) );
   IntValues.push_back( IntPairs(255) );
   IntValues.push_back( IntPairs(-255) );
   IntValues.push_back( IntPairs(32536) );
   IntValues.push_back( IntPairs(-32536) );
  
   ok = BeginCheck("IntAtom(), IntValue(), AtomType() ");
   for ( vector<IntPairs>::iterator it = IntValues.begin(); 
         it != IntValues.end();
	 it++ )
   {
      it->l = nl.IntAtom(it->v); // create Integer Atoms
   }
   
   ok = true;
   for ( vector<IntPairs>::iterator it = IntValues.begin(); 
         it != IntValues.end();
	 it++ )
   {
      
     if ( nl.AtomType(it->l) != IntType)
     {
       cout << "  Error: AtomType != int";
       ok = false;
     }
     ok = ok && ( nl.IntValue(it->l) == it->v );
     cout << "   " << nl.IntValue(it->l) << " == " << it->v << endl;
   }
   EndCheck(ok);
   
   

/*

4.1.3 Real atoms 

*/
  
   TestCase("Real Atoms");
 
   typedef pair<ListExpr, double> RealPair;
   vector<RealPair> realValues;
   vector<RealPair>::iterator rvit;
   realValues.push_back( RealPair(0, 87654.321) );
   realValues.push_back( RealPair(0, 0.0000001) );
   realValues.push_back( RealPair(0, -49857392587452345.01) );

   ok = BeginCheck("RealAtom(), RealValue(), AtomType() ");
   for ( rvit = realValues.begin(); 
         rvit != realValues.end();
	 rvit++ )
   {
      rvit->first = nl.RealAtom(rvit->second); // create Real Atoms
   }
   
   ok = true;
   for ( rvit = realValues.begin(); 
         rvit != realValues.end();
	 rvit++ )
   {
      
     if ( nl.AtomType(rvit->first) != RealType)
     {
       cout << "  Error: AtomType != int";
       ok = false;
     }
     ok = ok && ( nl.RealValue(rvit->first) == rvit->second );
     cout << "   " << nl.RealValue(rvit->first) << " == " << rvit->second << endl;
   }
   EndCheck(ok);


/*
ste
4.1.4 Bool atoms 

*/
   
   TestCase("Bool Atoms");
   BoolValue = true;
   BoolAtomVar = nl.BoolAtom (BoolValue);
   if ( (nl.AtomType (BoolAtomVar) == BoolType) )
   {
     cout << "BoolAtomVar is a bool atom: "; 
     BoolValue2 = nl.BoolValue (BoolAtomVar);
     cout << CBool(BoolValue2) << endl << endl;
   }

/*

4.1.5 String atoms

*/

   TestCase("String Atoms");
   /////////////////1234567890123456789012345
   NLStringValue = "Here I go again and again";
   StringAtomVar = nl.StringAtom (NLStringValue);
   if (nl.AtomType(StringAtomVar) == StringType)
   {
     cout << "StringAtomVar is a string atom: "; 
     NLStringValue2 = nl.StringValue (StringAtomVar);
     cout << """" << NLStringValue2 << """" << endl << endl;
   }

/*

4.1.6 Symbol atoms 

*/
   
   TestCase("Symbol Atoms");
   SymbolValue = "<=";
   SymbolAtomVar = nl.SymbolAtom(SymbolValue);

   if ( (nl.AtomType (SymbolAtomVar) == SymbolType) )
   {
     cout << endl;
     cout << "SymbolAtomVar is a symbol atom: ";
     SymbolValue2 = nl.SymbolValue (SymbolAtomVar);
     cout << SymbolValue2 << endl << endl;
   }
  cout.flush();  


/*

4.1.6 Text atoms and text scans 

*/

   TestCase("Text Atoms");
   cout << endl << " Short text (one fragment only)" << endl; 
   TextAtomVar = nl.TextAtom();
   Chars = "1__4__7__10__4__7__20__4__7__30__4__7__40__4__7__50__4__7";
   nl.AppendText (TextAtomVar, Chars);

   ListExpr textList = nl.OneElemList(TextAtomVar);
   s1="";
   nl.WriteToString(s1, textList);
   cout << endl << "A one element list with text atom: " << s1 << endl;

   TextScan1 = nl.CreateTextScan(TextAtomVar);
   Chars = "";
   Position = 0;
   nl.GetText (TextScan1, 30, Chars);
   cout << "(TextScan1, 30, Chars): " << Chars << endl;
   nl.GetText (TextScan1, 17, Chars);
   cout << "(TextScan1, 17, Chars): " << Chars << endl;
   CheckResult("EndOfText", nl.EndOfText(TextScan1), false);
   nl.GetText (TextScan1, 10, Chars);
   cout << "(TextScan1, 10, Chars): " << Chars << endl;
   
   CheckResult("EndOfText", nl.EndOfText(TextScan1), true);
   CheckResult("EndOfText", nl.EndOfText(TextScan1), true);
   nl.DestroyTextScan(TextScan1);

   cout << "After Text with one fragment: Memory-Usage: " << nl.ReportTableSizes() << endl;

   cout << endl << "Text in several fragments!" << endl; 
   TextAtomVar2 = nl.TextAtom();
   Chars = 
     "1__4__7__10__4__7__20__4__7__30__4__7__40__4__7__50__4__7__60__4__7__70__4__7_";
   nl.AppendText (TextAtomVar2, Chars);
 
   Chars = 
     "1++4++7++10++4++7++20++4++7++30++4++7++40++4++7++50++4++7++60++4++7++70++4++7+";
   nl.AppendText (TextAtomVar2, Chars);
   textList = nl.OneElemList(TextAtomVar2);
   int textLen = nl.TextLength(TextAtomVar2);

   cout << "Chars.length() == Text Length: " << textLen << " == " <<  2 * Chars.length() << endl;

   s1 = "";
   nl.WriteToString(s1, textList);
   cout << endl << "A list with a bigger text atom: " << s1 << endl;
   
   TextScan1 = nl.CreateTextScan (TextAtomVar2);
   Chars = "";
   Position = 0;
   while ( !nl.EndOfText (TextScan1) )
   {   
     nl.GetText (TextScan1, 50, Chars);
     cout << endl <<  "GetText(TextScan1, 50, Chars): " << Chars << endl;
   }
   cout << "After While" << endl;
   nl.DestroyTextScan (TextScan1);

   cout << "After Text with more than one fragment, Memory-Usage: " << nl.ReportTableSizes() << endl;
   
   ListExpr15 = nl.TwoElemList (TextAtomVar, TextAtomVar2);
   cout << "ListExpr15" << nl.ToString(ListExpr15) << endl;


/*

4.2 List Construction and Traversal

*/
   TestCase("List Construction and Traversal");

   ListExpr1 = nl.SixElemList (EmptyListVar, IntAtomVar, 
			       BoolAtomVar,  StringAtomVar,
			       SymbolAtomVar, ListExpr15);

   cout << "ListExpr1" << nl.ToString(ListExpr1) <<  endl;

   ListExpr2 = nl.Cons (nl.StringAtom(NLStringValue2), ListExpr1);
   cout << "ListExpr2" << endl;

   ListExpr3 = nl.TwoElemList (ListExpr1, ListExpr2);
   cout << "ListExpr3" << endl;

   ListExpr4 = nl.Cons (StringAtomVar, nl.Second (ListExpr3));
   cout << "ListExpr4" << endl;


/*

4.3 In-/Output 

4.3.1 In-/Output of a small list expression

The following steps are executed with a small list expression. 

  1 ListExpr [->] File   [->] ListExpr [->] String ; PrintString

  2 ListExpr [->] String [->] ListExpr [->] String ; PrintString

*/
   TestCase("Input/Output from/to files");

   ok = BeginCheck("WriteListExpr, WriteToFile, ReadFromFile, WriteToString, Equal");
   nl.WriteListExpr (ListExpr3);
   
   ErrorVar = nl.WriteToFile ("testout_SmallListFile", ListExpr3);
   cout << "WriteToFile" << endl;
   ErrorVar = nl.WriteToString ( String1, ListExpr3 );
   cout << "WriteToString" << endl;
   cout << endl;
   cout << "Small list - File test. Written to File: SmallList = "; 
   cout << String1 << endl << endl;

   ErrorVar = nl.ReadFromFile ("testout_SmallListFile", ListExpr5);
   ErrorVar = nl.WriteToString (String1, ListExpr5);
   cout << endl;
   cout << "Small list - File test. Found in File: SmallList = "; 
   cout << String1 << endl << endl;

   ErrorVar = nl.WriteToString (String2, ListExpr3);
   ErrorVar = nl.ReadFromString (String2, ListExpr6);
   ErrorVar = nl.WriteToString (String3, ListExpr6);
   cout << endl;
   cout << "Small list- String test. SmallList = ";
   cout << String3 << endl << endl;
   ok = ok && CheckResult("Equal(ListExpr3, ListExpr6)", nl.Equal(ListExpr5, ListExpr6), true);

   ErrorVar = nl.WriteToFile ("testout_RestExpr5", nl.Rest(ListExpr5));
   cout << endl;

   ErrorVar = nl.WriteToFile ("testout_FirstExpr5", nl.First(ListExpr5));
   cout << endl;

   cout << "After String <-> List Conversions, Memory-Usage: " << nl.ReportTableSizes() << endl;
   
  
/*

4.4 Destruction 

*/

   nl.Destroy(ListExpr3);
   nl.Destroy(ListExpr4);
   nl.Destroy(ListExpr5);
   nl.Destroy(ListExpr6);
   nl.Destroy(ListExpr8);
   nl.Destroy(ListExpr9);

   ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210-------</text--->)", ListExpr1);
   ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210-------</text--->)", ListExpr2);
   ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210---xxxxx----</text--->)", ListExpr3);
   ErrorVar = nl.WriteToString(String1, ListExpr1);
   ErrorVar = nl.WriteToString(String2, ListExpr2);
   ErrorVar = nl.WriteToString(String3, ListExpr3);
   cout << String1 << endl;
   cout << String2 << endl;
   cout << String3 << endl;

   cout << "After Destruction, Memory-Usage: " << nl.ReportTableSizes() << endl;

   ok = ok && CheckResult("Equal(text1, text1)", nl.Equal(ListExpr1, ListExpr1), true);
   ok = ok && CheckResult("Equal(text1, text3)", nl.Equal(ListExpr1, ListExpr3), false);

   EndCheck(ok);
   return (0);
}

/*

4.3.2 In-/Output of a complex list expression

The complex list expression is saved in the file ~testin-simple~.
The following five steps are executed:

File [->] ListExpr [->] File 
File [->] ListExpr [->] String [->] File

*/

ListExpr 
TestFile(const string& fileBaseName, NestedList& nl) {
   
   string fileIn = fileBaseName + ".nl";
   string fileOut = filePrefix + fileBaseName + ".nl";
   string fileBinOut = filePrefix + fileBaseName + ".bnl";

   ListExpr list = nl.TheEmptyList();


   cout << nl.ReportTableSizes() << endl;
   cout << endl << "Reading " << fileIn << " ..." << endl;
   nl.ReadFromFile( fileIn, list );
   cout << nl.ReportTableSizes() << endl;
   cout << endl << "Writing " << fileOut << " ..." << endl;
   nl.WriteToFile( fileOut, list );

   cout << endl << "Writing " << fileBinOut << " ..." << endl;
 
   ofstream outFile2(fileBinOut.c_str(), ios::out|ios::trunc|ios::binary); 
   nl.WriteBinaryTo(list, outFile2);   

   outFile2.close();

   return list;
}


void
TestInputOutput() {

   NestedList nl(rf,10000,10000,10000,10000);

   TestCase("Input/OutPut of Complex Expressions");

   ListExpr sList = TestFile("simpleList", nl);
   nl.WriteListExpr(sList); 
   TestFile("geo", nl);

}


bool
openDB(string dbname ) {
   
   bool ok=false;   
   
   cout << "OpenDatabase " << dbname;
   if ( (ok=SmiEnvironment::OpenDatabase( dbname )) == true )
   {
      cout << " ok." << endl;
   }
   else
   {
      cout << " failed, try to create." << endl;
   
      cout << "CreateDatabase " << dbname; 
      if ( (ok=SmiEnvironment::CreateDatabase( dbname )) == true )
         cout << " ok." << endl;
      else
         cout << " failed." << endl;
   }
 
   return ok;
   
}

bool
closeDB()
{
    bool ok=false;   

    cout << "CloseDatabase "; 
    if ( (ok=SmiEnvironment::CloseDatabase()) == true )
      cout << " ok." << endl;
    else
      cout << " failed." << endl;

    return ok;
}

void
listDB() {
   
   string dbname;

   cout << "*** Start list of databases ***" << endl;
   SmiEnvironment::ListDatabases( dbname );
   cout << dbname << endl;
   cout << "*** End list of databases ***" << endl;
 
}

int
TestRun_Persistent() {
  
   cout << endl << "Test run persistent" << endl;
 
   SmiError rc = 0;
   
   rc = SmiEnvironment::StartUp( SmiEnvironment::MultiUser,
   "SecondoConfig.ini", cerr );
   listDB();
   pause();
   
   assert( openDB("PARRAY") ); 
   //cout << "Begin Transaction: " << SmiEnvironment::BeginTransaction() << endl;
   
   pause();
   TestBasicOperations();
   
   pause();
   TestInputOutput();

   cout << endl << "Test list copy function" << endl;
   
   pause();
   TestNLCopy();
   
   //cout << "Commit: " << SmiEnvironment::CommitTransaction() << endl;
   
   assert( closeDB() );

   pause();
   
   rc = SmiEnvironment::ShutDown();
   cout << "ShutDown rc=" << rc << endl;
   
   return rc;
}


int
TestRun_MainMemory() {
   
   bool ok = true;
   
   cout << endl << "Test run main memory" << endl;
  
   pause();
   TestBasicOperations();
   
   pause();
   TestInputOutput();
   //StringAtom_bug(); 
   
   pause();
   TestNLCopy();
   
   return ok;
}
   
}; // end of class TestNestedList

int
main() {

  TestNestedList test('*', "testout-");

  test.TestCase("Comprehensive test of the class NestedList");
  
  cout << endl;
  cout << "Internal used Sizes (bytes): " << endl;
  cout << NestedList::SizeOfStructs() << endl;
  cout << "STRING_INTERNAL_SIZE: " << (int) STRING_INTERNAL_SIZE << endl;

  bool isPersistent = NestedList::IsPersistentImpl();
 
  cout << "Implementation Model: "; 
  if ( isPersistent ) {
    cout << " PERSISTENT";
  } else {
    cout << "NON-PERSISTENT";
  }
  cout << endl << endl;

  if ( isPersistent ) { 
   test.TestRun_Persistent();
  } else {
   test.TestRun_MainMemory();
  }

  test.ShowErrors();

  exit( test.GetNumOfErrors() );
}


