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

void reportTableSizes(const std::string& name, NestedList& nl){
  cout << "---------  " << name << "  ----------" << endl;
  cout << " used list storage " << endl;
  cout << "Nodes   : " << nl.sizeOfNodeTable()
       << " (in " << nl.chunksOfNodeTable() << " chunks)" << endl;
  cout << "Strings : " << nl.sizeOfStringTable() << endl;
  cout << "Texts   : " << nl.sizeOftextTable() << endl;
  cout << "----------------------------" << endl << endl; 
}



class TestNestedList : public CTestFrame {

private:

  int recCounter;
  NestedList* nl;
  const string filePrefix;

public:

  TestNestedList(char x, const string& prefixStr) : CTestFrame(x), 
    recCounter(0),
    nl(0),
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

    //cerr << "( rec:" << recCounter << " , first:" 
    //<< first << ", rest:" << rest << " )"; 

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
   NestedList nA;
   NestedList nB;

   TestCase("Copy Lists between NL-Instances nA, nB");
   reportTableSizes("nA", nA);
   reportTableSizes("nB", nB);


   vector<string> listStr(4);

   listStr[0]=string("(create fifty : (rel(tuple((n int)))))");
   listStr[1]=string("(0 (1 2) (3 4))"); 
   listStr[2]=string("(1 2 (3 5) 7 (9 2 3 4 5) (2 (3 4 (5 6)) 4))");
   listStr[3]=string("(1 \"jhgjhg\" 6 <text> Hallo! Dies ist ein recht langer"
      " und langweiliger Text, hier gibt es nichts zu erfahren. Wir wollen nur"
      " testen, ob Text-Atome korrekt behandelt werden. </text---> "
      "\"anbsdfklsd sksdjf sdksdf sdfj asdkjf sdkjssd\" 7 (9 10))");

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
  reportTableSizes("nA", nA);
  reportTableSizes("nB", nB);


  return 0;
}

/*

Destroying one of two lists that share a tail must leave the other alone.

The spine of a list is a chain of nodes linked through their right sons, and
two lists share a tail when one of them is built on a node the other already
reaches. ~DestroyRec~ handles the *head* of the list it is given correctly --
it lowers the reference count, writes it back, and stops if anyone else is
still holding the node -- but it used to do neither for the nodes further down
the spine: their decrement was applied to a copy that was never written back,
and the recursion into their children ran whether or not the node had actually
died. Destroying one list therefore released the elements of the other.

There is no way to see this from the list contents. Slots are not recycled, so
the released records still hold their values and both lists still read back
correctly; only the bookkeeping is wrong, and the first symptom is the
~assert~ at the top of ~DestroyRec~ firing on some later, unrelated destroy.
So this checks the reference counts directly, and only destroys the second
list once they look right -- on a broken build it reports and returns rather
than aborting the run.

*/

void
TestSharedTailDestroy(CTestFrame& t)
{
   NestedList nl;

   t.TestCase("Destroying a list that shares its tail with another");

   // list1 = (1 2 3), then list2 = (99 2 3) built on list1's own tail, so the
   // second node of the spine is reachable from both.
   ListExpr list1 = nl.ThreeElemList(nl.IntAtom(1), nl.IntAtom(2),
                                     nl.IntAtom(3));
   ListExpr tail  = nl.Rest(list1);            // the shared spine node
   ListExpr list2 = nl.Cons(nl.IntAtom(99), tail, true);

   const ListExpr shared2 = nl.First(tail);              // the atom 2
   const ListExpr shared3 = nl.First(nl.Rest(tail));     // the atom 3

   const uint32_t tailBefore  = nl.ReferenceCount(tail);
   const uint32_t elem2Before = nl.ReferenceCount(shared2);
   const uint32_t elem3Before = nl.ReferenceCount(shared3);

   cout << "*** before: tail=" << tailBefore
        << " elem2=" << elem2Before << " elem3=" << elem3Before << endl;

   nl.Destroy(list1);

   const uint32_t tailAfter  = nl.ReferenceCount(tail);
   const uint32_t elem2After = nl.ReferenceCount(shared2);
   const uint32_t elem3After = nl.ReferenceCount(shared3);

   cout << "*** after destroying the first list: tail=" << tailAfter
        << " elem2=" << elem2After << " elem3=" << elem3After << endl;

   // The shared node loses exactly the one reference the destroyed list held.
   const bool tailOk = (tailAfter + 1 == tailBefore);
   // ... and the elements below it, which only the surviving list reaches,
   // are not touched at all.
   const bool elemsOk = (elem2After == elem2Before)
                     && (elem3After == elem3Before);

   t.CheckResult("the shared spine node loses one reference", tailOk, true);
   t.CheckResult("the elements below it keep theirs", elemsOk, true);

   // Reading the survivor is not evidence either way -- the records still hold
   // their values whatever the counts say -- but it should hold regardless.
   t.CheckResult("the surviving list still reads back",
                 nl.ToString(list2) == "(99 2 3)", true);

   if (!tailOk || !elemsOk) {
     cout << "*** counts are wrong; not destroying the second list, "
             "since that is what would abort" << endl;
     return;
   }

   // Now the survivor can go, and taking it down must be clean: with the old
   // behaviour its elements were already at zero and this tripped the assert.
   nl.Destroy(list2);
   t.CheckResult("destroying the survivor afterwards is clean", true, true);
}


/*

A streamed list and a written list are the same bytes.

~WriteBinaryHeader~ / ~WriteBinaryListOpen~ / ~WriteBinaryElem~ exist so a
producer can write a result it never holds -- a relation of 212,099 tuples,
written one tuple at a time. The only thing that makes that safe is that a
reader cannot tell the difference, so the test is a byte comparison against
~WriteBinaryTo~ of the finished list, not a read-back: a read-back would pass
for any encoding both sides agreed on, including a wrong one.

Both length prefixes are covered, because they are the one place the streaming
writer has to reproduce a decision ~GetBinaryType~ makes elsewhere: a list of
255 elements takes the one-byte form and 256 takes the four-byte one, and
getting that boundary wrong shifts every following byte.

*/

void
TestStreamedBinaryWrite(CTestFrame& t)
{
   NestedList nl;

   t.TestCase("A streamed binary list is byte-identical to a written one");

   // 255 and 256 are the two sides of the SHORTLIST/LONGLIST boundary; 3 is
   // an ordinary short list, and its elements are nested so the element
   // writer is exercised on more than atoms.
   const int lengths[] = {3, 255, 256};

   for (unsigned int i = 0; i < sizeof(lengths)/sizeof(lengths[0]); i++) {
     const int n = lengths[i];

     // Build the list the ordinary way, and write it the ordinary way.
     ListExpr list = nl.TheEmptyList();
     ListExpr last = list;
     for (int k = 0; k < n; k++) {
       ListExpr elem = nl.TwoElemList(nl.IntAtom(k),
                                      nl.StringAtom("tuple"));
       if (nl.IsEmpty(list)) {
         list = nl.Cons(elem, nl.TheEmptyList());
         last = list;
       } else {
         last = nl.Append(last, elem);
       }
     }
     stringstream whole;
     nl.WriteBinaryTo(list, whole);

     // Now the same thing without ever holding it: the header, the opening of
     // a list of known length, then the elements one at a time.
     stringstream streamed;
     NestedList::WriteBinaryHeader(streamed);
     NestedList::WriteBinaryListOpen(n, streamed);
     for (int k = 0; k < n; k++) {
       ListExpr elem = nl.TwoElemList(nl.IntAtom(k),
                                      nl.StringAtom("tuple"));
       nl.WriteBinaryElem(elem, streamed);
     }

     stringstream label;
     label << "streamed == written for a list of " << n;
     t.CheckResult(label.str(), streamed.str() == whole.str(), true);

     if (streamed.str() != whole.str()) {
       cout << "*** written  " << whole.str().size() << " bytes" << endl;
       cout << "*** streamed " << streamed.str().size() << " bytes" << endl;
     }
   }

   // And the bytes are not merely equal to each other but readable: a reader
   // that was handed the streamed form gets the list back.
   stringstream streamed;
   NestedList::WriteBinaryHeader(streamed);
   NestedList::WriteBinaryListOpen(2, streamed);
   nl.WriteBinaryElem(nl.IntAtom(7), streamed);
   nl.WriteBinaryElem(nl.StringAtom("seven"), streamed);

   ListExpr readBack = nl.TheEmptyList();
   const bool ok = nl.ReadBinaryFrom(streamed, readBack);
   t.CheckResult("a streamed list reads back", ok, true);
   t.CheckResult("and holds what was streamed into it",
                 nl.ToString(readBack) == "(7 \"seven\")", true);
}


/*

The cached ~typeerror~ node survives being handed around.

Every type mapping in the system returns `nl->TypeError()`, which is one node
index shared by every caller and every thread. Putting it into a list used to
be a read-modify-write on that node -- lower its isRoot, raise its count -- and
no discipline on the caller's part could make two threads doing that at once
safe, because they are not co-operating and do not know about each other.

It is marked immortal instead, so nothing writes it at all. This checks the
consequences: the count does not move however many lists it goes into, the node
survives those lists being destroyed, and it is still the same node afterwards
-- two call sites in DBService compare against it by identity.

*/

void
TestImmortalTypeError(CTestFrame& t)
{
   NestedList nl;

   t.TestCase("The cached typeerror node is never written");

   const ListExpr te = nl.TypeError();
   const uint32_t before = nl.ReferenceCount(te);

   ListExpr l1 = nl.OneElemList(te);
   ListExpr l2 = nl.TwoElemList(te, nl.IntAtom(7));
   ListExpr l3 = nl.Cons(nl.IntAtom(8), nl.OneElemList(te));
   ListExpr again = te;
   nl.IncReferences(again);                    // the explicit path, too

   cout << "*** references before=" << before
        << " after building three lists=" << nl.ReferenceCount(te) << endl;

   t.CheckResult("the count does not move when it is put into lists",
                 nl.ReferenceCount(te) == before, true);

   nl.Destroy(l1);
   nl.Destroy(l2);
   nl.Destroy(l3);

   t.CheckResult("the count does not move when those lists are destroyed",
                 nl.ReferenceCount(te) == before, true);
   t.CheckResult("it is still the same node", nl.TypeError() == te, true);
   t.CheckResult("and still reads as typeerror",
                 nl.IsEqual(nl.TypeError(), "typeerror"), true);
}


/*

The next functions contain code which is extraced from
the secondo system to isolate bugs.

*/

void
StringAtom_bug() {

   //NestedList nl(10,10,10,10);
   NestedList nl;

   cout << "Test of String Atoms." << endl << endl;

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
   NestedList Nl;
   nl = &Nl;

   cout << "Test of String Atoms." << endl << endl;

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

   NestedList nl;

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

   bool BoolValue = false, BoolValue2 = false;

   string NLStringValue = "", NLStringValue2 = "", 
          SymbolValue = "", SymbolValue2 = "";
   string String1 = "", String2 = "", String3 = "", Chars = "";
   TextScan TextScan1;

   
   NestedList nl;

   reportTableSizes("nl:0", nl);



   ListExpr sym2 = nl.OneElemList(nl.SymbolAtom("ERRORS"));
   nl.ReadFromString( "(open database opt)", sym2 );
   nl.WriteListExpr(sym2);

/*

4.1 Elementary methods 

4.1.1 Empty List 

*/
   reportTableSizes("nl:1", nl);
   
   TestCase("Empty List");
   bool ok = false;
   ok = BeginCheck("TheEmptyList, IsEmpty, ReadFromString,"
                   "ToString, WriteListExpr, WriteBinaryTo ");
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
   reportTableSizes("nl:2", nl);
   
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
   
   reportTableSizes("nl:3", nl);
   

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
     cout << "   " << nl.RealValue(rvit->first) 
          << " == " << rvit->second << endl;
   }
   EndCheck(ok);


/*
ste
4.1.4 Bool atoms 

*/
   reportTableSizes("nl:4", nl);
   
   TestCase("Bool Atoms");
   BoolValue = true;
   BoolAtomVar = nl.BoolAtom (BoolValue);
   if ( (nl.AtomType (BoolAtomVar) == BoolType) )
   {
     cout << "BoolAtomVar is a bool atom: "; 
     BoolValue2 = nl.BoolValue (BoolAtomVar);
     cout << CBool(BoolValue2) << endl << endl;
   }

   reportTableSizes("nl:5", nl);
/*

4.1.5 String atoms

*/

   string NLStringValue3="";
   
   TestCase("String Atoms");
   /////////////////123456789012345678901234567890123456789012345678
   NLStringValue  = "Here I go again and again";
   NLStringValue2 = "1 ere I go again and again again again again 48";
   NLStringValue3 = "Here I go again and again again again again again";
   
   StringAtomVar = nl.StringAtom(NLStringValue);
   cout << nl.StringValue(StringAtomVar) << endl;
   
   CHECK(nl.AtomType(StringAtomVar) == StringType, true);
   
   StringAtomVar = nl.StringAtom(NLStringValue2);
   cout << nl.StringValue(StringAtomVar) << endl;
   
   CHECK(nl.AtomType(StringAtomVar) == StringType, true);
 
   //The test below will raise an assertion in the module NestedList
   //StringAtomVar = nl.StringAtom(NLStringValue3);
   //cout << nl.StringValue(StringAtomVar) << endl;
   //
   //CHECK(nl.AtomType(StringAtomVar) == StringType, true);


   
/*

4.1.6 Symbol atoms 

*/
   reportTableSizes("nl:6", nl);
   
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
   reportTableSizes("nl:7", nl);

   TestCase("Text Atoms");
   cout << endl << " Short text (one fragment only)" << endl; 
   TextAtomVar = nl.TextAtom();
   string TextChars 
             = "1__4__7__10__4__7__20__4__7__30__4__7__40__4__7__50__4__7";
   int TextSize=TextChars.length();
   nl.AppendText (TextAtomVar, TextChars);

   ListExpr textList = nl.OneElemList(TextAtomVar);
   s1="";
   nl.WriteToString(s1, textList);
   cout << endl << "A one element list with text atom: " << s1 << endl;

   TextScan1 = nl.CreateTextScan(TextAtomVar);
   Chars = "";
   int sum = 0;
   for (int i=1; i<TextSize; i++) {
     nl.GetText (TextScan1, i, Chars);
     sum += i;
     //cout << "(TextScan1, i,  Chars): " << Chars << endl;
     //cout << "(SubStr 0,i TextChars): " << TextChars.substr(0,sum) << endl;
     string subText = TextChars.substr(0,sum);

     bool endOfText = false;
     if (sum >= TextSize) 
       endOfText = true;             

     bool rc = CheckResult("EndOfText", nl.EndOfText(TextScan1), endOfText);
     if (!rc) {
       cerr << "End of text check after retrieving " 
            << sum << " characters failed" << endl;
     }              

     rc = CheckResult("Equal-Strings?", Chars == subText, true);
     if (!rc) {
       cerr << "Test nl.GetText(TextScan1, " << i << ", Chars) failed!" << endl;
       cerr << "Expected: <" << subText << ">" << endl;
       cerr << "Computed: <" << Chars << ">" << endl;
       exit(1);
     }             
   }
   nl.DestroyTextScan(TextScan1);


   cout << endl << "Text in several fragments!" << endl; 
   TextAtomVar2 = nl.TextAtom();
   Chars = 
     "1__4__7__10__4__7__20__4__7__30__4__7__"
     "40__4__7__50__4__7__60__4__7__70__4__7_";
   nl.AppendText (TextAtomVar2, Chars);
 
   Chars = 
     "1++4++7++10++4++7++20++4++7++30++4++7++"
     "40++4++7++50++4++7++60++4++7++70++4++7+";
   nl.AppendText (TextAtomVar2, Chars);
   textList = nl.OneElemList(TextAtomVar2);

   s1 = "";
   nl.WriteToString(s1, textList);
   cout << endl << "A list with a bigger text atom: " 
                << s1 << endl;
   
   int textLen = nl.TextLength(TextAtomVar2);
   int charLen = 2 * Chars.length();
   CheckResult("Equal-Length?", textLen == charLen, true);

   TextScan1 = nl.CreateTextScan (TextAtomVar2);
   Chars = "";
   while ( !nl.EndOfText (TextScan1) )
   {   
     nl.GetText (TextScan1, 50, Chars);
     cout << endl <<  "GetText(TextScan1, 50, Chars): " 
                  << Chars << endl;
   }
   nl.DestroyTextScan (TextScan1);

   string str = nl.Text2String(TextAtomVar2);
   CheckResult("Equal-String",  textLen == charLen, true);
   
   
   ListExpr15 = nl.TwoElemList (TextAtomVar, TextAtomVar2);
   cout << "ListExpr15" << nl.ToString(ListExpr15) << endl;

   reportTableSizes("nl:8", nl);

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

   reportTableSizes("nl:9", nl);

/*

4.3 In-/Output 

4.3.1 In-/Output of a small list expression

The following steps are executed with a small list expression. 

  1 ListExpr [->] File   [->] ListExpr [->] String ; PrintString

  2 ListExpr [->] String [->] ListExpr [->] String ; PrintString

*/
   TestCase("Input/Output from/to files");

   ok = BeginCheck("WriteListExpr, WriteToFile, "
                   "ReadFromFile, WriteToString, Equal");
   nl.WriteListExpr (ListExpr3);
   
   nl.WriteToFile ("testout_SmallListFile", ListExpr3);
   cout << "WriteToFile" << endl;
   nl.WriteToString ( String1, ListExpr3 );
   cout << "WriteToString" << endl;
   cout << endl;
   cout << "Small list - File test. Written to File: SmallList = "; 
   cout << String1 << endl << endl;

   nl.ReadFromFile ("testout_SmallListFile", ListExpr5);
   nl.WriteToString (String1, ListExpr5);
   cout << endl;
   cout << "Small list - File test. Found in File: SmallList = "; 
   cout << String1 << endl << endl;

   nl.WriteToString (String2, ListExpr3);
   nl.ReadFromString (String2, ListExpr6);
   nl.WriteToString (String3, ListExpr6);
   cout << endl;
   cout << "Small list- String test. SmallList = ";
   cout << String3 << endl << endl;
   ok = CHECK( nl.Equal(ListExpr5, ListExpr6), true);

   nl.WriteToFile ("testout_RestExpr5", nl.Rest(ListExpr5));
   cout << endl;

   nl.WriteToFile ("testout_FirstExpr5", nl.First(ListExpr5));
   cout << endl;

   
   reportTableSizes("nl:10", nl);
  
/*

4.4 Destruction 

*/

   nl.Destroy(ListExpr3);
   nl.Destroy(ListExpr4);
   nl.Destroy(ListExpr5);
   nl.Destroy(ListExpr6);
   nl.Destroy(ListExpr8);
   nl.Destroy(ListExpr9);

   reportTableSizes("nl:11", nl);
/*

4.5 Reading an writing text atoms 

*/

   const string tagS("('");
   const string tagE("')");
   string text = 
   "--------10--------20--------30--------40--------50--------"
   "60--------70--------80-------90-------100-------110-------120-------"
   "130-------140-------150-------160-------170-------180-------190-------"
   "200-------210-------";
   
   string text1 = tagS + text + tagE;
   string text2 = tagS + text + text + tagE;
   
   nl.ReadFromString(text1, ListExpr1);
   nl.ReadFromString(text1, ListExpr2);
   nl.ReadFromString(text2, ListExpr3);
   
   nl.WriteToString(String1, ListExpr1);
   nl.WriteToString(String2, ListExpr2);
   nl.WriteToString(String3, ListExpr3);
   
   CHECK( String1 == text1, true);
   CHECK( String3 == text2, true);


   CHECK( nl.Equal(ListExpr1, ListExpr2), true);
   CHECK( nl.Equal(ListExpr1, ListExpr3), false);

   EndCheck(ok);
   reportTableSizes("nl:9", nl);
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
TestFile(const string& dir, const string& fileBaseName, NestedList& nl) {
   
   string fileIn = dir+"/"+fileBaseName;
   string fileOut = filePrefix + fileBaseName + ".nl";
   string fileBinOut = filePrefix + fileBaseName + ".bnl";

   ListExpr list = nl.TheEmptyList();


   cout << endl << "Reading " << fileIn << " ..." << endl;
   nl.ReadFromFile( fileIn, list );
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

   NestedList nl;

   TestCase("Input/OutPut of Complex Expressions");

   ListExpr sList = TestFile(".", "simpleList.nl", nl);
   nl.WriteListExpr(sList); 
   TestFile("../bin", "opt", nl);

}



int
TestRun_Persistent() {
  
   cout << endl << "Test run persistent" << endl;
 
   //pause();

   
   //pause();
   TestBasicOperations();
   
   //pause();
   TestInputOutput();

   cout << endl << "Test list copy function" << endl;
   
   //pause();
   TestNLCopy();

   TestSharedTailDestroy(*this);

   TestImmortalTypeError(*this);

   TestStreamedBinaryWrite(*this);
   
   //cout << "Commit: " 
   //<< SmiEnvironment::CommitTransaction() << endl;

   return 1;
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
  cout << "MAX_STRINGSIZE: " << (int) MAX_STRINGSIZE << endl;

  test.TestRun_Persistent();

  test.ShowErrors();

  exit( test.GetNumOfErrors() );
}


