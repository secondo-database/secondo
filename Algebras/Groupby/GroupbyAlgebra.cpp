/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]


[1] Implementation of Module Groupby Algebra

[1] Using Storage Manager Berkeley DB

December 2011 Dieter Capek

[TOC]

1 Includes and defines

*/


#include <vector>
#include <deque>
#include <sstream>
#include <stack>
#include <limits.h>
#include <set>

//#define TRACE_ON
#undef TRACE_ON
#include "LogMsg.h"
#define TRACE_OFF

#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "DateTime.h"
#include "Stream.h"
#include "FTextAlgebra.h"
#include "SecondoCatalog.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace listutils;



//=================================================================== CAPs Code


// Type Mapping Funktion für groupby2
ListExpr GroupByTypeMapCap(ListExpr args)
{
  ListExpr first, second, third, fourth;     // list used for analysing input
  ListExpr listn, lastlistn, listp;  // list used for constructing output
  ListExpr first2;
  string err = 
    "stream(tuple(X)) x (g1..gn) x (tuple(X)xtxt -> t), t in DATA expected";
  string resstring;


  nl->WriteToString(resstring, args);
  cout << "GroupbyTypeMapCap Input = " << resstring << endl;

  first = second = third = fourth = nl->TheEmptyList();
  listn = lastlistn = listp = nl->TheEmptyList();

  string relSymbolStr = Relation::BasicType();
  string tupleSymbolStr = Tuple::BasicType();

 
  // Die Anzahl der Eingabewerte muss gleich vier sein
  if(! nl->HasLength(args,3))	
    return listutils::typeError("Need to specify three parameters.");

  // get the three arguments
  first  = nl->First(args);         // input stream
  second = nl->Second(args);        // Liste der Gruppierungsattribute
  third  = nl->Third(args);         // Funktionen für die Aggregation

  // Fehlende Werte sind nur für die Gruppierungsattribute erlaubt
  if ( nl->IsEmpty(first) || nl->IsEmpty(third))
    return listutils::typeError("Mandatory argument is missing.");

  // first argument must be of type stream
  if(!Stream<Tuple>::checkType(first))	
    return listutils::typeError("First argument must be of type stream.");


  // Each grouping attribute must be part of the input stream
  // Diese Prüfung habe ich aus dem groupby übernommen
  ListExpr rest = second;
  ListExpr lastlistp = nl->TheEmptyList();
  bool firstcall = true;

  while (!nl->IsEmpty(rest))
  {
    ListExpr attrtype = nl->TheEmptyList();
    first2 = nl->First(rest);
    if(nl->AtomType(first2)!=SymbolType){
      ErrorReporter::ReportError("Wrong format for an attribute name");
      return nl->TypeError();
    }

    string attrname = nl->SymbolValue(first2);

    // calculate index of attribute in tuple
    int j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);

    if (j)
    {
      if (!firstcall)
      {
        lastlistn = nl->Append(lastlistn,nl->TwoElemList(first2,attrtype));
        lastlistp = nl->Append(lastlistp,nl->IntAtom(j));
      } else {
        firstcall = false;
        listn = nl->OneElemList(nl->TwoElemList(first2,attrtype));
        lastlistn = listn;
        listp = nl->OneElemList(nl->IntAtom(j));
        lastlistp = listp;
      }
    }
    else // grouping attribute not in input stream
    {
      string errMsg = "groupby2: Attribute " + attrname + 
        " not present in input stream";
      ErrorReporter::ReportError(errMsg);
      return nl->SymbolAtom(Symbol::TYPEERROR());
    }
    rest = nl->Rest(rest);
  } // end while; Prüfung der grouping Attribute


  // Es muss mindestens eine Funktion angegeben sein
  if(nl->ListLength(third) < 1)
    return listutils::typeError("Must specify one aggregation function.");

  rest = third;         // List of functions
  // Prüfung der Aggregatfunktionen
  while (!(nl->IsEmpty(rest))) 
  {
    // iterate over function list and initial values
    ListExpr firstr = nl->First(rest);  // functions
    rest = nl->Rest(rest);

    // Es muss mindestens eine Funktion angegeben sein
    if(nl->ListLength(firstr) != 3)
      return listutils::typeError("Each function must have three elements.");

    ListExpr newAttr  = nl->First(firstr);
    ListExpr mapDef   = nl->Second(firstr);
    ListExpr firstInit = nl->Third(firstr);

    // Prüfung des Attributnames
    if ( !(nl->IsAtom(newAttr)) || !(nl->AtomType(newAttr) == SymbolType) )
      return listutils::typeError("Attribut name for function is not valid.");

    // Prüfung Initialwert
    if(!listutils::isDATA(firstInit))   
      return listutils::typeError("Initial values must all be of kind DATA.");


    // Prüfung der Funktion
    if(!listutils::isMap<2>(mapDef))  
      return listutils::typeError("Aggregation function is not valid.");

    // zuvor muss die Gültigkeit der Funktion auf typeerror geprüft sein
    ListExpr mapOut = nl->Third(mapDef);

    // Das Tupel muss das erste Funktionsargument sein
    ListExpr t = nl->Second(first);
    if(!nl->Equal(t, nl->Second(mapDef)))
      return listutils::typeError("Map argument 1 must be tuple from stream.");
   
    // Zweites Funktionsargument und Initialwert müssen gleichen Typ haben
    if(! nl->Equal(firstInit, nl->Third(mapDef)))
      return listutils::typeError(
      "Map argument 2 and start value must have same type.");

    // Ergebnis der Funktion und Initalwert müssen gleichen Typ haben
    if(! nl->Equal(firstInit, nl->Fourth(mapDef)))
      return listutils::typeError(
      "Map result and start value must have same type.");

    if (    (nl->EndOfList( lastlistn ) == true)
         && (nl->IsEmpty( lastlistn ) == false)
         && (nl->IsAtom( lastlistn ) == false)
       )
    { // list already contains group-attributes (not empty)
      lastlistn = nl->Append(lastlistn,(nl->TwoElemList(newAttr,mapOut)));
    } else { 
      // no group attribute (list is still empty)
      listn = nl->OneElemList(nl->TwoElemList(newAttr,mapOut));
      lastlistn = listn;
    }
  } // end while for aggregate functions

  // Prüfe, ob der Name für das Aggregat bereits vorkommt
  if ( !CompareNames(listn) )
    return listutils::typeError("Attribute names are not unique.");

  // Type mapping is correct, return result type.
  ListExpr result =
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->Cons(nl->IntAtom(nl->ListLength(listp)), listp),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList( nl->SymbolAtom(tupleSymbolStr), listn))
    );
 
  // Testausgabe
  nl->WriteToString(resstring, result);
  cout << "groupbyTypeMapCap Result = " << resstring << endl;

  // Hilfe für den Test der Type Mapping Funktion
  // return listutils::typeError("Ende des groupby2 Type Mapping.");

  return result;
}  // Ende Type Mapping groupby2



#define CAPBUCKETS 99997

struct GroupByLocalInfo2
{
  Tuple *t;
  TupleType *resultTupleType;
  long MAX_MEMORY;
  bool FirstREQUEST;
  TupleBuffer gBucket[CAPBUCKETS];
  GenericRelationIterator* ReturnRit;
  int ReturnBucket;

  GroupByLocalInfo2() : t(0), resultTupleType(0), MAX_MEMORY(0), 
  FirstREQUEST(true), ReturnRit(0), ReturnBucket(0) {}
};


// Value Mapping für groupby2
int GroupByValueMapping2 (Word* args, Word& result, int message, Word& local, 
                          Supplier supplier)
{
  // The argument vector contains the following values:
  // args[0] = input stream of tuples
  // args[1] = list of grouping attributes
  // args[2] = list of functions (with elements name, function, initial value)
  // args[3] = number of grouping attributes (added by APPEND)
  // args[4..] =  position of grouping attributes (added by APPEND);    

  /* Beispiel für drei Gruppierungsattribute: 
     es wird APPEND (3 1 2 3) im Type Mapping erzeugt
     Dies ergibt arg[3] = 3    Anzahl der Guppierungsattribute
                 arg[4] = 1    Index des ersten Gruppierungsattributs im Tupel
                 arg[5] = 2    Index des zweiten Gruppierungsattributs im Tupel
                 arg[6] = 3    Index des dritten Gruppierungsattributs im Tupel
  */

  Word sWord(Address(0));
  GroupByLocalInfo2 *gbli = 0;
  ListExpr resultType;
  Tuple *current, *s, *tres;
  int numberatt = 0;		// Anzahl Gruppenattriute
  int attribIdx = 0;
  int indexOfCountArgument = 3;	// Anzahl Gruppierungs Attribute
  // Beginn der Indizes der Gruppierungsattribute
  int startIndexOfExtraArguments = indexOfCountArgument + 1; 
  int i, j, k; 
  Attribute *sattr, *tattr;
  GenericRelationIterator* rit;
  int AnzahlTupelimBucket = 0;
  size_t myhash1, myhash2;
  bool GruppeGleich, GruppeDoppelt;
  ArgVectorPointer funargs;
  Word funres;
  Supplier supp1, supp2, supp3, supp4;
  int noOffun;
  Supplier value2;


  switch(message)
  {
    case OPEN:
      // open the stream and get the first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, sWord);
     
      // allokiere lokale Info und speichere das erste Tupel
      if (qp->Received(args[0].addr)) {
        gbli = new GroupByLocalInfo2();
        gbli->t = (Tuple*)sWord.addr;
        (gbli->t)->IncReference();

        resultType = GetTupleResultType(supplier);
        gbli->resultTupleType = new TupleType(nl->Second(resultType));

        local.setAddr(gbli);
      } else {
        local.setAddr(0);	// kein Tupel erhalten
      }
      return 0;

    case REQUEST:
      // Adresse zur lokalen Information
      gbli = (GroupByLocalInfo2 *)local.addr;
      if (!gbli) return CANCEL;                 // empty input stream
      if (gbli->t == 0) return CANCEL;		// stream has ended
      numberatt = ((CcInt*)args[indexOfCountArgument].addr)->GetIntval();

      if (gbli->FirstREQUEST) {	
        // erster REQUEST Aufruf der Value Mapping Funktion:
        // Führe die Aggregation durch und gib das erste Ergebnis Tupel zurück
        s = gbli->t;	// das erste Tupel kommt aus dem OPEN
        gbli->FirstREQUEST = false;

        // get the number of functions
        value2 = (Supplier) args[2].addr;
        noOffun = qp->GetNoSons(value2);


        // Keine Gruppierung angegeben, berechne eine Gesamtsumme
        if (numberatt == 0) {
          // Bilde das Gruppentupel: nur Funktionswerte
          tres = new Tuple(gbli->resultTupleType);

          // Berechne die ersten Funktionswerte aus Tupel und Initialwerten
          for (i=0; i < noOffun; i++) {
            // qp für die Funktionsauswertung aurufen
            supp1 = (Supplier) args[2].addr;  // funlist: Liste der Funktionen
            supp2 = qp->GetSupplier( supp1, i);
            supp3 = qp->GetSupplier( supp2, 1);
            funargs = qp->Argument(supp3);    // get argument vector of function

            supp4 = qp->GetSupplier( supp2, 2); // supp4 ist der Initialwert
            qp->Request( supp4, funres);        // Funktionsauswertung
            tattr = ((Attribute*)funres.addr)->Clone();

            (*funargs)[0].setAddr(s);     // erstes Argument muss das Tupel sein
            (*funargs)[1].setAddr(tattr);
            qp->Request( supp3, funres);        // Funktionsauswertung

            sattr = ((Attribute*)funres.addr)->Clone();
            tres->PutAttribute( i, sattr);
          } // end-for    

          s->DeleteIfAllowed();

          qp->Request(args[0].addr, sWord);	// lies das nächste Tupel
          s = (Tuple*) sWord.addr;

          while (qp->Received(args[0].addr)) {
            // Berechne die Funktionswerte aus Tupel und bisherigem Wert
            for (i=0; i < noOffun; i++) {
              // qp für die Funktionsauswertung aurufen
              supp1 = (Supplier) args[2].addr;	// Liste der Funktionen
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector 

              sattr = tres->GetAttribute(i);     

              (*funargs)[0].setAddr(s);   // erstes Argument muss das Tupel sein
              (*funargs)[1].setAddr(sattr);
              qp->Request( supp3, funres);        // call the parameter function

              sattr = ((Attribute*)funres.addr)->Clone();
              tres->PutAttribute( i, sattr);
            } // end-for 
 
            s->DeleteIfAllowed();

            qp->Request(args[0].addr, sWord);	
            s = (Tuple*) sWord.addr;
          }  // end-while Tupel lesen
           
          result.addr = tres;
          return YIELD;
        } // end-if keine Gruppierung


        while (s) {
/*        // Test: Ausgabe des Input Tuple
          cout << "Input tuple: ";
          s->Print(cout);
*/

          // Berechne den Hash Wert des Tupel aus den Gruppenattributen
          myhash1 = 0;
          for (k = 0; k < numberatt; k++) {
            attribIdx = 
              ((CcInt*)args[startIndexOfExtraArguments+k].addr)->GetIntval();
            j = attribIdx-1;		// 0 basiert
            myhash1 += s->HashValue(j);
          }
          myhash2 = myhash1 % CAPBUCKETS;
          // myhash2 kann überlaufen und ist dann negativ
          if (myhash2 < 0) myhash2 = -1 * myhash2;	

          // Durchsuche das zugehörige Hash Bucket, ob es die Gruppe schon gibt
          GruppeDoppelt = false;
          rit = gbli->gBucket[myhash2].MakeScan();
          AnzahlTupelimBucket = gbli->gBucket[myhash2].GetNoTuples();

          // Vergleich des neuen Tupels (s) mit denen im Bucket (current)
          // s: Gruppenattribute Gi o Nichtgruppenattribute Ai
          // Tuple im Tuple Buffer: Gruppenattribute Gi o Funktionswert
          for (i=0; (i<AnzahlTupelimBucket) && !GruppeDoppelt; i++) {
            current = rit->GetNextTuple();
            if (!current) break;
            // Vergleich des neuen Tupel mit einem aus dem Bucket 
            GruppeGleich = true;

            // Vergleich über die Gruppierungsattribute
            for (j=0; (j < numberatt) && GruppeGleich && current; j++) {
              attribIdx = 
                ((CcInt*)args[startIndexOfExtraArguments+j].addr)->GetIntval();
              k = attribIdx - 1;		// 0 basiert
              // Vergleich je Attribut. Die Strukturen sind unterschiedlich
              // k stimmt für das Input Tupel. j für das Aggregat
              if (s->GetAttribute(k)->Compare(current->GetAttribute(j)) != 0){
                GruppeGleich = false;
                break;
              }
            } // end-for
            if (GruppeGleich) GruppeDoppelt = true;
          } // end for
          delete rit;

          if (GruppeDoppelt == false) {  // Gruppe ist neu
            // Bilde das Gruppentupel: Gruppierungsattribute o Funktionswerte
            tres = new Tuple(gbli->resultTupleType);

            // Berechne die ersten Funktionswerte aus Tupel und Initialwert 
            for (i=0; i < noOffun; i++) {
              // qp für die Funktionsauswertung aurufen
              supp1 = (Supplier) args[2].addr;	// Liste der Funktionen
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector 

              supp4 = qp->GetSupplier( supp2, 2); // supp4 ist der Initialwert
              qp->Request( supp4, funres);        // Funktionsauswertung
              tattr = ((Attribute*)funres.addr)->Clone();

              (*funargs)[0].setAddr(s);          
              (*funargs)[1].setAddr(tattr);
              qp->Request( supp3, funres);        // Funktionsauswertung

              sattr = ((Attribute*)funres.addr)->Clone();
              // hinter den Gruppenattributen
              tres->PutAttribute( numberatt+i, sattr);     
            } // end-for 

            // copy grouping attributes 
            for(i = 0; i < numberatt; i++) {
              attribIdx = 
                ((CcInt*)args[startIndexOfExtraArguments+i].addr)->GetIntval();
              tres->CopyAttribute(attribIdx-1, s, i);
            }

            // Speichere das Gruppentupel im Hash Bucket
            gbli->gBucket[myhash2].AppendTuple(tres);

          } else {  // Gruppe kommt schon vor
            // Berechne Funktionswerte n+1 aus neuen Tupel und Funktionswert n 
            for (i=0; i < noOffun; i++) {
              // qp für die Funktionsauswertung aurufen
              supp1 = (Supplier) args[2].addr;	// Liste der Funktionen
              supp2 = qp->GetSupplier( supp1, i);
              supp3 = qp->GetSupplier( supp2, 1);
              funargs = qp->Argument(supp3);      // get argument vector

              sattr = current->GetAttribute( numberatt+i);     

              (*funargs)[0].setAddr(s);          
              (*funargs)[1].setAddr(sattr);
              qp->Request( supp3, funres);        // call the parameter function

              sattr = ((Attribute*)funres.addr)->Clone();
              current->PutAttribute( numberatt+i, sattr);
            } // end-for 
          }  // end-if

          // Lösche das Tupel des Input Stream
          s->DeleteIfAllowed();

          // lies das nächste Tupel
          qp->Request(args[0].addr, sWord);
          s = (Tuple*)sWord.addr;
          if (qp->Received(args[0].addr)) s->IncReference();
        } // end-while 

        // Die Aggregate sind jetzt vollständig berechnet
        // Finde das erste Ergebnis Tupel und gib es zurück
	result.addr = 0;
	// Suche das erste Hash Bucket, das ein Ergebnis Tupel enthält
        for(i = 0; i<CAPBUCKETS; i++) {
          rit = gbli->gBucket[i].MakeScan();
          AnzahlTupelimBucket = gbli->gBucket[i].GetNoTuples();

          if (AnzahlTupelimBucket > 0) {
            gbli->ReturnRit = rit;       // diesen Scan weitergeführen
            gbli->ReturnBucket = i+1;    // dieses Bucket muss als nächstes 
            current = rit->GetNextTuple();
            result.setAddr(current);

            // Test: Ausgabe des Ergebnis Tuple
            // if (current) current->Print(cout);

            return YIELD;
          }
          delete rit;
        } // end-for
        // Das Ergebnis des Operators ist leer
        return CANCEL;

      // folgende REQUEST Aufrufe: Ausgabe der Ergebnis Tupel 
      } else {	
        result.addr = 0;	// noch kein Ergebnis Tupel gefunden 

        // Keine Gruppierung. Es gibt ein einziges Ergebnis Tupel, 
        // das wurde bereits ausgegeben
        if (numberatt == 0) return CANCEL;

        // Falls noch ein Bucket Scan aktiv ist oder 
        // noch Buckets zu verarbeiten sind
        if (gbli->ReturnRit || gbli->ReturnBucket < CAPBUCKETS) {
          // Finde das nächste Ergebnis Tupel
          // Verarbeite einen aktiven Scan
          if (gbli->ReturnRit) { 
            current = (gbli->ReturnRit)->GetNextTuple();
            if (current)            
              result.setAddr(current);            
            else
              delete gbli->ReturnRit;
          } // end-if

          // Noch kein Tupel gefunden, suche in den nächsten Buckets
          if (result.addr == 0 && (gbli->ReturnBucket) < CAPBUCKETS) {
            for(i = gbli->ReturnBucket; i<CAPBUCKETS; i++) {
              gbli->ReturnRit = gbli->gBucket[i].MakeScan();
              AnzahlTupelimBucket = gbli->gBucket[i].GetNoTuples();

              if (AnzahlTupelimBucket > 0) {
                gbli->ReturnBucket = i+1;    // nächstes Bucket
                current = (gbli->ReturnRit)->GetNextTuple();
                result.setAddr(current);
                break;
              } else {
                delete gbli->ReturnRit;
              }
            } // end-for
          }  // end-if
        } // end-if

/*      // Test: Ausgabe des Ergebnis Tuple
        if (current) current->Print(cout);
*/
        if (result.addr)
          return YIELD;
        else
          return CANCEL;
     } // end-if REQUEST Verarbeitung

    case CLOSE: 
      // Löschen der lokalen Informationen
      if(local.addr != 0) {
        gbli = (GroupByLocalInfo2 *) local.addr;
        if(gbli->resultTupleType != 0) gbli->resultTupleType->DeleteIfAllowed();
        if(gbli->t != 0) gbli->t->DeleteIfAllowed();

        delete gbli;
        local.setAddr(0);
      }
      qp->Close(args[0].addr);		// close des Stream
      return 0;
  } // end message switch


  return(0);
} // Ende GroupByValueMapping2


// noch alt;
const string GroupBySpec2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" ) "
                            "( <text>((stream (tuple (a1:d1 ... an:dn))) "
                            "(ai1 ... aik) ((bj1 (fun (rel (tuple (a1:d1"
                            " ... an:dn))) (_))) ... (bjl (fun (rel (tuple"
                            " (a1:d1 ... an:dn))) (_))))) -> (stream (tuple"
                            " (ai1:di1 ... aik:dik bj1 ... bjl)))</text--->"
                            "<text>_ groupby2 [list; funlist]</text--->"
              "<text>Capek groupby: Groups a relation according to attributes "
                            "ai1, ..., aik and feeds the groups to other "
                            "functions. The results of those functions are "
                            "appended to the grouping attributes. The empty "
                            "list is allowed for the grouping attributes (this "
                            "results in a single group with all input tuples)."
                            "</text--->"
                            "<text>query Employee feed "
                            "groupby2[DeptNr; anz : group feed count] consume"
                            "</text--->"
                            ") )";


Operator groupby2 (
         "groupby2",             // name
         GroupBySpec2,           // specification
         GroupByValueMapping2,   // value mapping
         Operator::SimpleSelect, // trivial selection function
         GroupByTypeMapCap       // type mapping; 
);


/*

3 Class ~ExtRelationAlgebra~

A new subclass ~GroupbyAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~
is defined.

*/


class GroupbyAlgebra : public Algebra
{
 public:
  GroupbyAlgebra() : Algebra()
  {
    AddOperator(&groupby2);    
  }

  ~GroupbyAlgebra() {};
};

/*

4 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeGroupbyAlgebra(     NestedList* nlRef,
                              QueryProcessor* qpRef,
                              AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new GroupbyAlgebra());
}

