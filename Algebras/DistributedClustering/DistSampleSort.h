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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Spatial Algebra

Jun 2015, Daniel Fuchs 

[TOC]

1 Overview


This file contains the implementation of the class dbDacScanAlgebra

2 Includes


*/


#include "AlgebraTypes.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Stream.h"
#include <utility>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream> //TODO for log
#include "FTextAlgebra.h"
#include "Member.h"
#include <limits>  

#ifndef SISTRIBUTEDSORT_H
#define SISTRIBUTEDSORT_H

using namespace std;

namespace distributedClustering{
  
/*
class Distsamp

*/
  template <class MEMB_TYP_CLASS, class TYPE>
  class Distsamp{
    
//     ---------------------------------------------
//     update:
//     nur noch sample sortieren
//     grenzpunkte bestimmen
//     anhand derer workerid zur großen relation hiinzufügen
//     dabei muss rel nur noch einmal durchlaufen werdeen
//     membArray fällt weg -> nur noch tuplebuffer
//     
//     
//     http://stackoverflow.com/questions/223545/
//     probability-of-selecting-an-element-from-a-set
//     
//     http://stackoverflow.com/questions/1152732/
//     how-does-the-mapreduce-sort-algorithm-work
//     
//     http://de.slideshare.net/tungld/terasort
//     https://en.wikipedia.org/wiki/Trie
//     
//     in TeraSort.Java ist die auswahl von samples und sortietung 
//     implementiert
//     http://grepcode.com/file/repository.cloudera.com/content/
//     repositories/releases/com.cloudera.hadoop/hadoop-examples/
//     0.20.2-320/org/apache/hadoop/examples/terasort/TeraSort.java?av=f
//     
//     
//     n...Mächtigkeit der Problemgröße
//     m...Mächtigkeit der Verteilten Problemgröße - Teilmengen
//     t...Anzahl der Maschinen
//     
//     Auswahl der Sample Elemente:
//     
//     roh = 1/m * ln(n*t) -> Wahrscheinlichkeit um die Elemente zu wählen
//     roh^-1 -> Anzahl der Menge für Sample
//     
//     Wenn jede Zahl aus der Menge mit Wahrscheinlichkeit 
//     roh ausgewählte werden 
//     kann - so werden genau roh^-1 zahlen aus dieser Menge ausgewählt
//     
//     
//     Voraussetzung - Daten sind bereits gleichmäßig auf Knoten Verteilt
//     Übergabeparameter - Dateiname der Daten
//     
//     Runde 1
//     -erstellt zuerst Samples. Die größe eines Samples ist r=m*ln(n*t). 
//     Dabei wählt man die ersten r oder jedes m/r te element aus
//     
//     - die Samples werden an alle anderen KNoten geschickt und von allen
//     Knoten wird ihr Sample empfangen
//     
//     - alle Knoten sortieren ihr gesamt Sample welches sich aus dem eigenen
//     und den zugesandten zusammensetzt. 
//     s= |SampleGesamt|
//     
//     - definiere die grenzpunkte b1...bt-1, dabei ist bi das i*⌈s/t⌉ 
//     kleinste element aus Ssamp - also an stelle i*⌈s/t⌉
//     
//     Runde 2
//     Every Mi sends the objects in ( bj −1, bj] 
//     from its local storage to M j , for each 1 ≤ j ≤ t , where 
//     b0 = −∞ and bt = ∞ are dummy boundary objects. 
//     
//     Reduce: 
//     Every Mi sorts the objects received in the previous phase.)
//     TODO
    
  public: //TODO siehe DistClOp.h -> query script für samplen
   
       
/*
constructor

*/
    Distsamp(Word& _inSampStream, Word& _sampStream, ListExpr& _tupleType,
             int _attrPos, int _cntWorkers, size_t _maxMem):
             cntWorkers(_cntWorkers),resIt(0),tt(0),buffer(0),sampBuff(0)
             ,attrPos(_attrPos)
    {
      tt = new TupleType(_tupleType);
      init(_maxMem,_inSampStream, _sampStream);
//       sort sample
      // cout << "sample sort" << endl;
      mergeSort(sampleArray,0,sampleArray.size());
//       //sort relation
//       // cout << "rel sort" << endl;
//       mergeSort(membArray,0,membArray.size());
      
      // cout << "initOutput" << endl;
      initOutput();
      
      //man erhält 2 Streams 
        //1. ist teilrelaiton 2. ist die kokatenation von den samples
      
      // sortiere zuerst die samples
      
      //bestimme die border points
      
      //sortiere relation anhand der borderpoints
      
      //füge attribute workerid zu relation
      // siehe ~sortby~ -> relationen erstellen
      
    }
    
/*
destructor

*/
    ~Distsamp(){
      if(buffer)
        delete buffer;
      if(sampBuff)
        delete sampBuff;
      if(resIt) 
        delete resIt;
      if(tt) 
        tt->DeleteIfAllowed();
      
    }
    
/*
next

*/    
    Tuple* next(){
      if(resIt){
        Tuple* tuple = resIt->GetNextTuple();
        if(!tuple){
          return 0;
        }
        TupleId id = resIt->GetTupleId();
        Tuple* resTuple = new Tuple(tt);
        int noAttr = tuple->GetNoAttributes();
        for(int i = 0; i<noAttr; i++){
          resTuple->CopyAttribute(i,tuple,i);
        }
        tuple->DeleteIfAllowed();
        
        //TODO position bestimmen
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        MEMB_TYP_CLASS dummy(obj);
        resTuple->PutAttribute(noAttr, 
                               new CcInt(true,
                                      getWorkerID((double)dummy.getXVal())));
        return resTuple;
      } else {
        return 0;
      }
  
    }
    
    
    
  private:
/* 
1 .3 members                                *

*/
    int  cntWorkers,pos,sizeOfBuffer,attrPos;
    TupleBuffer* buffer, *sampBuff;
    GenericRelationIterator* resIt;  // iterator 
//     typename vector<MEMB_TYP_CLASS*>::iterator it;
    TupleType* tt;   // the result tuple type
//     vector <MEMB_TYP_CLASS*> membArrayUntouched;
    vector <MEMB_TYP_CLASS*> membArray;
    vector <MEMB_TYP_CLASS*> sampleArray;
    vector <double> border;

/*
initialize

*/
    void init(size_t maxMem, Word& _inStream, Word& _sampStream){
      Tuple* tuple;
      buffer = new TupleBuffer(maxMem);
      Stream<Tuple> inStream(_inStream);
      inStream.open();
      while((tuple = inStream.request())){
        buffer->AppendTuple(tuple);
//         TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
//         if(obj->IsDefined()){
//           MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
// //           member->setTuple(tuple);
//           member->setTuplePos(buffer->GetNoTuples()-1);
// //           membArrayUntouched.push_back(member);
//           membArray.push_back(member);
//         }
        tuple->DeleteIfAllowed();
      }
      inStream.close();
      
      sampBuff = new TupleBuffer();
      Stream<Tuple> sampStream(_sampStream);
      sampStream.open();
      while((tuple = sampStream.request())){
        sampBuff->AppendTuple(tuple);
        TYPE* obj = (TYPE*) tuple->GetAttribute(attrPos);
        if(obj->IsDefined()){
          MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
          sampleArray.push_back(member);
        }
        tuple->DeleteIfAllowed();
      }
      sampStream.close();
    }
    
/*
getWorkerID
returns the WorkerId which is putted to the result relation

*/
    int getWorkerID(double val){
      //TODO carefully -> values descending!!
      
      //prüfe für alle variablen borders ob val 
      // <= > ist gib aktuelle pos zurück
      bool workerFound = false;
      int worker = 0;
      for (int i=0; i< cntWorkers && !workerFound; i++){
        if(val < border.at(i) && val >= border.at(i+1)){
          workerFound = true;
          worker = i;
          
          // cout << "Value= " << val << " index i= " << i << endl;
          // cout << "border.at(i)= " << border.at(i) 
          // << "  \nborder.at(i+1)= " << border.at(i+1) << endl;
        }
      }
      return worker;
    }
    
/*
init Output()
Starts the begin of returning tuples.

*/
    void initOutput(){
      if(resIt) delete resIt;
      resIt = buffer->MakeScan(); 
      
      //make bordervektor
      border.clear();
      border.push_back(numeric_limits<double>::max());
      for(int i = 1; i < cntWorkers; i++){
        border.push_back(sampleArray.at(b(i))->getXVal());
      }
      border.push_back(-1 * numeric_limits<double>::max());
      
      // cout << "boerder Values" << endl;
      for(int i =0; i< border.size(); i++){
        // cout << "border.at(i)= " << border.at(i) << endl;
      }
      
//       it = membArray.begin();
//       pos=0;
//       sizeOfBuffer = buffer->GetNoTuples();
     
      //TODO grenzpunkte bestimmen
    }

/*
mergeSort
sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       MEMB_TYP_CLASS ** auxiliaryArray = new MEMB_TYP_CLASS*[right-left+1];
       if(auxiliaryArray!= 0){
         mergeSort(array,left,right,auxiliaryArray);
         delete [] auxiliaryArray;
       }
     }
     
/*
mergeSort
sort an array in ascending order

*/
     void mergeSort(vector<MEMB_TYP_CLASS*>& array,int left, 
                    int right,MEMB_TYP_CLASS** auxiliaryArray){
       if(right == left+1)
         return ; //mergeSort finisch
         else{
           int i = 0;
           int length = right - left;
           int median = (right - left)/2;
           int l = left; //position to the left subarray
           int r = left + median; //position to the right subarray
           
           //divide array
           mergeSort(array, left, r, auxiliaryArray);
           mergeSort(array, r, right, auxiliaryArray);
           
           //merge array
           /* Check to see if any elements remain in the left array; if so,
            * we check if there are any elements left in the right array; if
            * so, we compare them.  Otherwise, we know that the merge must
            * use take the element from the left array */
           for(i = 0; i < length; i++){
             if(l < left+median && (r==right || leftIsMax(array, l, r))){
               auxiliaryArray[i]=array[l];
               l++;
             }
             else{
               auxiliaryArray[i]= array[r];
               r++;
             }
           }
           //Copy the sorted subarray back to the input array
           for(i=left; i < right; i++){
             array[i]=auxiliaryArray[i-left];
           }
         }
     }
     
/*
leftIsMax()
auxiliary fuction to compare the maximum Object with the left object

*/
     bool leftIsMax(vector<MEMB_TYP_CLASS*>& array,int left, int right){
       bool retVal = false;
       
       double leftXVal = array[left]->getXVal();
       double rightXVal = array[right]->getXVal();
       
       leftXVal > rightXVal ? retVal = true : retVal = false;
       return retVal;
     }
     
/*
b(int i)
return the position of Borderpoint i
bi = i x ⌈s/t⌉

*/
    int b(int i){
      
      int retVal = i* (int) 
      floor((double)sampleArray.size() / (double) cntWorkers);
      // cout << "in b(i)" << endl;
      // cout << "i = " << i << " sampleArray.size(): "
      // << sampleArray.size() << " cntWorkers: " << cntWorkers << endl;
      // cout << " retVal = " <<retVal << endl;
      // cout << "retVal%smpleSize() = " 
      // << (retVal % sampleArray.size()) << endl;
      
      return retVal;
    }
    
  };

  //   Gesamtbeschreibung:
  //    aufruf des Operatos mit ffeed5 operator 
  //              -> somit bekommt der sorting operator
  //    einen stream als input -> ausgabe ist ebenfalls ein stream der
  //    mit fconsume5 gespeicher wird.
  //    
  //    fconsume5
  //    Signature:  stream(TUPLE) x {string, text} -> stream(TUPLE)
  //    
  //    ffeed5
  //    Signature:  {string, text} -> stream(TUPLE) 
  //    
  //  
  //   
  //  
  //    
  //    1. wie bei dbdacscan die datei öffnen und den stream einlesen
  //    
  //    2. samples erstellen in bin datei speichern und via query versenden
  //       --> dabei SecondoHome() + getSendFolder verzeichnis verwenden
  //    
  //    3. sample dateien einlesen und sortieren und grenzen bestimmen
  //  
  //   
  //   
  //  
  //    zu implementieren in diesem Operator
  //    
  //    distinguish between sample sort und relation sort with boreder points
  //    sampleSort:
  //    sort the part relation
  //    stream x bool -> stream
  //    tupelStr x isSample -> outTupleStr
  //    
  //    
  //    relationSort:
  //    sort the relation in order to the border boints
  //    stream x stream x bool -> stream
  //    relStream x sampleStream x isSample -> outTupleStr
  //    
  //    --> sihe concat operator in ExtRelationAlgebra.cpp
  //    --> joinRIndex in TindexAlgebra
  //  
  //   
  //   
  //  
  //    zu implementieren in diesem Operator
  //    
  //    distinguish between sample sort und relation sort with boreder points
  //    sampleSort:
  //    sort the part relation
  //    stream x bool -> stream
  //    tupelStr x isSample -> outTupleStr
  //    
  //    
  //    relationSort:
  //    sort the relation in order to the border boints
  //    stream x stream x bool -> stream
  //    relStream x sampleStream x isSample -> outTupleStr
  //    
  //    --> sihe concat operator in ExtRelationAlgebra.cpp
  //    --> joinRIndex in TindexAlgebra
  //  
  //   
  //   
  //   
  //  
  //    saveInFile
  //    saves the stream in a bin file
  //    TODO wie bei class fconsume5Info in Distributed2Algbebra.cpp
  //  
  //  
  //    void saveInFile(Word& stream, const string& filename,
  //                   const ListExpr typeList)
  //    {
  //     out.open(filename.c_str(),ios::out|ios::binary);
  //     ok = out.good();
  //     buffer = new char[FILE_BUFFER_SIZE];
  //     out.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
  //     if(ok){
  //       BinRelWriter::writeHeader(out,typeList);
  // } 
  // ok = out.good();
  // stream.open();
  // }
  // 
  // Tuple* next(){
  // Tuple* tuple = in.request();
  // if(!tuple){
  //   return 0;
  // }
  // if(ok){
  //   if(!BinRelWriter::writeNextTuple(out,tuple)){
  //     ok = false;
  //     if(firstError){
  //       cerr << "Problem in writing tuple" << endl;
  //       firstError = false;
  // }
  // }
  // }
  // return tuple;
  // }
  
  
  
}

#endif
