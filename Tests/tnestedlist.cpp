/*

//paragraph [1] Title: [{\Large \bf ] [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[->] [$\rightarrow $]

[1] Module TestNL: Tests with Stable Nested Lists


April 1996 Carsten Mund

\tableofcontents

1 Introduction

In this module the nested list functions from the module NestedList are called to test them and to demonstrate how to use them.

2 Imports

******************************************************************************/

#include <string>

#include "NestedList.h"


/******************************************************************************

3 Preliminaries

3.1 Variables

******************************************************************************/


/******************************************************************************

3.2 Local Procedures

******************************************************************************/

string WriteBool( bool b )
{
  if ( b )
    return "TRUE";
  else
    return "FALSE";
}

/******************************************************************************

4 Module body

******************************************************************************/

int main()
{
  ListExpr  ListExpr1,  ListExpr2,  ListExpr3, 
            ListExpr4,  ListExpr5,  ListExpr6,
            ListExpr7,  ListExpr8,  ListExpr9, 
            ListExpr10, ListExpr11, ListExpr12,
            ListExpr13, ListExpr14, ListExpr15,
            EmptyListVar,  EmptyListVar2,  EmptyListVar3,
            IntAtomVar,    IntAtomVar2,    IntAtomVar3,
            RealAtomVar,   RealAtomVar2,   RealAtomVar3,
            BoolAtomVar,   BoolAtomVar2,   BoolAtomVar3,
            StringAtomVar, StringAtomVar2, StringAtomVar3,
            SymbolAtomVar, SymbolAtomVar2, SymbolAtomVar3,
            TextAtomVar,   TextAtomVar2,   TextAtomVar3;
  long IntValue, IntValue2, ErrorVar;
  double RealValue, RealValue2;
  bool BoolValue, BoolValue2, success;
  string NLStringValue, NLStringValue2, SymbolValue, SymbolValue2;
  string String1, String2, String3, Chars;
  char ch;
  TextScan TextScan1, TextScan2, TextScan3, ts;
  Cardinal Position;
  NestedList nl(100);

  cout << "Test of Nested Lists." << endl << endl;

/******************************************************************************

4.1 Elementaries 

4.1.1 Empty List 

******************************************************************************/
  EmptyListVar   = nl.TheEmptyList(); 
  if ( nl.IsEmpty (EmptyListVar) )
  {
    cout << "EmptyListVar is the empty list." << endl << endl;
  }

/******************************************************************************

4.1.2 Integer atoms

******************************************************************************/
  IntValue = 123;
  IntAtomVar = nl.IntAtom (IntValue);
  if ( nl.AtomType (IntAtomVar) == IntType)
  {
    cout << "IntAtomVar is an INTEGER atom: ";
    IntValue2 = nl.IntValue (IntAtomVar);
    cout << IntValue2 << endl << endl;
  }

/****************************************************************************** 

4.1.3 Real atoms 

******************************************************************************/
  RealValue = 87654.321;
  RealAtomVar = nl.RealAtom (RealValue);
  if ( nl.AtomType(RealAtomVar) == RealType )
  {
    cout << "RealAtomVar is a REAL atom: ";
    RealValue2 = nl.RealValue (RealAtomVar);
    cout << RealValue2 << endl << endl;
  }

/******************************************************************************

4.1.4 Bool atoms 

******************************************************************************/
  BoolValue = true;
  BoolAtomVar = nl.BoolAtom (BoolValue);
  if ( (nl.AtomType (BoolAtomVar) == BoolType) )
  {
    cout << "BoolAtomVar is a bool atom: "; 
    BoolValue2 = nl.BoolValue (BoolAtomVar);
    cout << WriteBool(BoolValue2) << endl << endl;
  }

/******************************************************************************

4.1.5 String atoms

******************************************************************************/
  NLStringValue = "Here I go again and again";
  StringAtomVar = nl.StringAtom (NLStringValue);
  if (nl.AtomType(StringAtomVar) == StringType)
  {
    cout << "StringAtomVar is a string atom: "; 
    NLStringValue2 = nl.StringValue (StringAtomVar);
    cout << """" << NLStringValue2 << """" << endl << endl;
  }

/****************************************************************************** 

4.1.6 Symbol atoms 

******************************************************************************/
  SymbolValue = "<=";
  SymbolAtomVar = nl.SymbolAtom(SymbolValue);

  if ( (nl.AtomType (SymbolAtomVar) == SymbolType) )
  {
    cout << endl;
    cout << "SymbolAtomVar is a symbol atom: ";
    SymbolValue2 = nl.SymbolValue (SymbolAtomVar);
    cout << SymbolValue2 << endl << endl;
  }


/****************************************************************************** 

4.1.6 Text atoms and text scans 

******************************************************************************/

/* Short text (one fragment only) ********************************************/
  TextAtomVar = nl.TextAtom();
  Chars = "1__4__7__10__4__7__20__4__7__30__4__7__40__4__7__50__4__7";
  nl.AppendText (TextAtomVar, Chars);

  TextScan1 = nl.CreateTextScan (TextAtomVar);
  Chars = "";
  Position = 0;
  nl.GetText (TextScan1, 30, Chars);
  cout << Chars << endl;
  nl.GetText (TextScan1, 17, Chars);
  cout << Chars << endl;
  nl.GetText (TextScan1, 10, Chars);
  cout << Chars << endl;

  nl.DestroyTextScan (TextScan1); 

/* Text in several fragments *************************************************/
  TextAtomVar2 = nl.TextAtom();
  Chars = 
    "1__4__7__10__4__7__20__4__7__30__4__7__40__4__7__50__4__7__60__4__7__70__4__7_";
  nl.AppendText (TextAtomVar2, Chars);
  
  Chars = 
    "1++4++7++10++4++7++20++4++7++30++4++7++40++4++7++50++4++7++60++4++7++70++4++7+";
  nl.AppendText (TextAtomVar2, Chars);

  TextScan1 = nl.CreateTextScan (TextAtomVar2);
  Chars = "";
  Position = 0;
  while ( !nl.EndOfText (TextScan1) )
  {
    cout << "Before GetText" << endl;
    nl.GetText (TextScan1, 50, Chars);
    cout << endl << Chars << endl;
  }
  cout << "After While" << endl;
  nl.DestroyTextScan (TextScan1);

  ListExpr15 = nl.TwoElemList (TextAtomVar, TextAtomVar2);
  cout << "ListExpr15" << endl;


/****************************************************************************** 

4.2 List Construction and Traversal

******************************************************************************/
  ListExpr1 = nl.SixElemList (EmptyListVar, IntAtomVar, 
                              BoolAtomVar,  StringAtomVar,
                              SymbolAtomVar, ListExpr15);
  cout << "ListExpr1" << endl;

  ListExpr2 = nl.Cons (nl.StringAtom(NLStringValue2), ListExpr1);
  cout << "ListExpr2" << endl;

  ListExpr3 = nl.TwoElemList (ListExpr1, ListExpr2);
  cout << "ListExpr3" << endl;

  ListExpr4 = nl.Cons (StringAtomVar, nl.Second (ListExpr3));
  cout << "ListExpr4" << endl;


/******************************************************************************

4.3 In-/Output 

4.3.1 In-/Output of a small list expression

The following steps are executed with a small list expression. 

  1 ListExpr [->] File   [->] ListExpr [->] String ; PrintString

  2 ListExpr [->] String [->] ListExpr [->] String ; PrintString

*/

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

/*

4.3.2 In-/Output of a complex list expression

The complex list expression is saved in the file ~testin-simple~.
The following five steps are executed:

File [->] ListExpr [->] File 
File [->] ListExpr [->] String [->] File

*/

  ErrorVar = nl.ReadFromFile ("testin_simple", ListExpr8);
  ErrorVar = nl.WriteToFile ("testout_simple", ListExpr8);

  ErrorVar = nl.WriteToString (String1, ListExpr8);
  ErrorVar = nl.ReadFromString (String1, ListExpr9);
  ErrorVar = nl.WriteToFile ("testout_simple2", ListExpr9);

/*

4.3.3 Test of nested instructions

*/

  ErrorVar = nl.WriteToFile ("testout_RestExpr5", nl.Rest (ListExpr5));
  cout << endl;

  ErrorVar = nl.WriteToFile ("testout_FirstExpr5", nl.First (ListExpr5));
  cout << endl;


/******************************************************************************

4.4 Destruction 

******************************************************************************/

  nl.Destroy(ListExpr3);
  nl.Destroy(ListExpr4);
  nl.Destroy(ListExpr5);
  nl.Destroy(ListExpr6);
  nl.Destroy(ListExpr8);
  nl.Destroy(ListExpr9);

  ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210-------</text--->)", ListExpr1);
  ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210-------</text--->)", ListExpr2);
  ErrorVar = nl.ReadFromString("(<text>--------10--------20--------30--------40--------50--------60--------70--------80-------90-------100-------110-------120-------130-------140-------150-------160-------170-------180-------190-------200-------210-------</text--->)", ListExpr3);
  ErrorVar = nl.WriteToString(String1, ListExpr1);
  ErrorVar = nl.WriteToString(String2, ListExpr2);
  ErrorVar = nl.WriteToString(String3, ListExpr3);
  cout << String1 << endl;
  cout << String2 << endl;
  cout << String3 << endl;
  return (0);
}

