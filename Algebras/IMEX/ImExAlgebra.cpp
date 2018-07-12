/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen,
Faculty of Mathematics and Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]
//[_] [\_]
//[TOC] [\tableofcontents]

[1] ImExAlgebra

This algebra provides import export functions for different
data formats.


*/

/*

[TOC]

1 Overview

This file contains the implementation import / export operators.

2 Defines, includes, and constants

*/


#include <cmath>
#include <stack>
#include <limits>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#include <fcntl.h>
#include "SocketIO.h"

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "SecondoSystem.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "DateTime.h"
#include "Algebras/TopOps/TopOpsAlgebra.h"
#include "Algebras/BinaryFile/BinaryFileAlgebra.h"
#include "Tools/Flob/DbArray.h"
#include "Symbols.h"
#include "FileSystem.h"
#include "ListUtils.h"

#include "version.h"
#include "DbVersion.h"
#include "Algebras/Spatial/RegionTools.h"
#include "NMEAImporter.h"
#include "Algebras/Stream/Stream.h"
#include "aisdecode.h"
#include "Base64.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

using namespace std;


#define FILE_BUFFER_SIZE 1048576 


   uint32_t readBigInt32(ifstream& file){
      uint32_t res;
      file.read(reinterpret_cast<char*>(&res),4);
      if(WinUnix::isLittleEndian()){
         res = WinUnix::convertEndian(res);
      }
      return res;
   }

   uint32_t readLittleInt32(ifstream& file){
      uint32_t res;
      file.read(reinterpret_cast<char*>(&res),4);
      if(!WinUnix::isLittleEndian()){
         res = WinUnix::convertEndian(res);
      }
      return res;
   }
   
   uint16_t readLittleInt16(ifstream& file){
      uint16_t res;
      file.read(reinterpret_cast<char*>(&res),2);
      if(!WinUnix::isLittleEndian()){
         res = WinUnix::convertEndian(res);
      }
      return res;
   }

   double readLittleDouble(ifstream& file){
      uint64_t tmp;
      file.read(reinterpret_cast<char*>(&tmp),8);
      if(!WinUnix::isLittleEndian()){
         tmp = WinUnix::convertEndian(tmp);
      }
      void* tmpv = (void*) &tmp;
      double res = * (reinterpret_cast<double*>(tmpv));
      return res;
   }

/*
1 Operator ~csvexport~

1.1 Type Mapping

   stream(CSVEXPORTABLE) x text x bool -> stream(CSVEXPORTABLE)
   stream ( tuple ( (a1 t1) ... (an tn))) x string x
                bool x bool -> stream (tuple(...))
   stream ( tuple ( (a1 t1) ... (an tn))) x string x bool x
                bool x string -> stream (tuple(...))

*/

ListExpr csvexportTM(ListExpr args){
  int len = nl->ListLength(args);
  if(len != 3 && len != 4 && len!=5){
    ErrorReporter::ReportError("wrong number of arguments");
    return nl->TypeError();
  }
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(len == 3){ // stream(CSV) x string x bool
    if(!nl->IsEqual(nl->Second(args),FText::BasicType()) ||
       !nl->IsEqual(nl->Third(args),CcBool::BasicType())){
       ErrorReporter::ReportError("stream x text x bool expected");
       return nl->TypeError();
    }
    ListExpr stream = nl->First(args);
    if(nl->ListLength(stream)!=2){
       ErrorReporter::ReportError("stream x text x bool expected");
       return nl->TypeError();
    }
    if(!nl->IsEqual(nl->First(stream),Symbol::STREAM())){
       ErrorReporter::ReportError("stream x text x bool expected");
       return nl->TypeError();
    }
    if(!am->CheckKind(Kind::CSVEXPORTABLE(),nl->Second(stream),errorInfo)){
       if(listutils::isTupleStream(stream)){
          return listutils::typeError("for processing a tuple stream one  "
                                     "additional parameter is required"
                                     " at least");

       } else {
         return listutils::typeError("stream element not in "
                                     "kind csvexportable");
       }
    }
    return stream;
  } else { // stream(tuple(...) )
    if( !nl->IsEqual(nl->Second(args),FText::BasicType()) ||
        !nl->IsEqual(nl->Third(args),CcBool::BasicType()) ||
        !nl->IsEqual(nl->Fourth(args),CcBool::BasicType())  ){
       ErrorReporter::ReportError("stream x text x bool x"
                                  " bool [x string] expected");
       return nl->TypeError();
    }
    if(len==5 && !nl->IsEqual(nl->Fifth(args),CcString::BasicType())){
       ErrorReporter::ReportError("stream x text x bool x "
                                  "bool [x string] expected");
       return nl->TypeError();
    }
    ListExpr stream = nl->First(args);
    if(nl->ListLength(stream)!=2 ||
       !nl->IsEqual(nl->First(stream),Symbol::STREAM())){
       ErrorReporter::ReportError("stream x text x bool [ x bool] expected");
       return nl->TypeError();
    }
    ListExpr tuple = nl->Second(stream);
    if(nl->ListLength(tuple)!=2 ||
       !nl->IsEqual(nl->First(tuple),Tuple::BasicType())){
       ErrorReporter::ReportError("stream x text x bool [ x bool] expected");
       return nl->TypeError();
    }
    ListExpr attrList = nl->Second(tuple);
    if(nl->ListLength(attrList) < 1 ){
       ErrorReporter::ReportError("stream x text x bool [ x bool] expected");
       return nl->TypeError();
    }
    // check attrList
    while(!nl->IsEmpty(attrList)){
      ListExpr attr = nl->First(attrList);
      attrList = nl->Rest(attrList);
      if(nl->ListLength(attr)!=2){
         ErrorReporter::ReportError("invalid tuple stream");
         return nl->TypeError();
      }
      ListExpr aName = nl->First(attr);
      ListExpr atype = nl->Second(attr);
      if(nl->AtomType(aName)!=SymbolType){
         ErrorReporter::ReportError("invalid tuple stream");
         return nl->TypeError();
      }
      if(!am->CheckKind(Kind::CSVEXPORTABLE(), atype,errorInfo)){
         ErrorReporter::ReportError("invalid kind in tuple");
         return nl->TypeError();
      }
    }
    return stream;
  }
}

/*
1.2 Value Mappings for csvexport

*/

class CsvExportLocalInfo {

public:

   CsvExportLocalInfo(string fname, bool append){
     if(append){
       f.open(fname.c_str(), ios::out | ios::app);
     } else {
       f.open(fname.c_str(),ios::out | ios::trunc);
     }
   }

   CsvExportLocalInfo(string fname, bool append,bool names,
                      const string& sep,
                      ListExpr type){
     this->sep = sep;
     if(append){
       f.open(fname.c_str(), ios::out | ios::app);
     } else {
       f.open(fname.c_str(),ios::out | ios::trunc);
     }
     if(names){
        ListExpr rest = nl->Second(nl->Second(type));
        ListExpr first = nl->First(rest);
        rest = nl->Rest(rest);
        f << nl->SymbolValue(nl->First(first));
        while(!nl->IsEmpty(rest)){
           f << ",";
           first = nl->First(rest);
           rest = nl->Rest(rest);
           f << nl->SymbolValue(nl->First(first));
        }
        f << endl;
     }
   }

   ~CsvExportLocalInfo(){
     f.close();
   }

   bool isOk(){
      return f.good();
   }

   void write(Attribute* attr){
     f << attr->getCsvStr() << endl;
   }

   void write(Tuple* tuple){
     int s = tuple->GetNoAttributes();
     for(int i=0;i<s;i++){
        if(i>0){
          f << sep;
        }
        f << tuple->GetAttribute(i)->getCsvStr();
     }
     f << endl;
   }

private:
  fstream f;
  string sep;
};

int CsvExportVM(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
      qp->Open(args[0].addr);
      FText* fname = static_cast<FText*>(args[1].addr);
      CcBool* append = static_cast<CcBool*>(args[2].addr);
      if(!fname->IsDefined() || !append->IsDefined()){
         local.setAddr(0);
      } else {
         CsvExportLocalInfo* linfo;
         linfo = (new CsvExportLocalInfo(fname->GetValue(),
                                         append->GetBoolval()));
         if(!linfo->isOk()){
            delete linfo;
            local.setAddr(0);
         } else {
             local.setAddr(linfo);
         }
      }
      return 0;
    }

    case REQUEST:{
       if(local.addr==0){
         return CANCEL;
       } else {
         Word elem;
         qp->Request(args[0].addr,elem);
         if(qp->Received(args[0].addr)){
           CsvExportLocalInfo* linfo;
           linfo  = static_cast<CsvExportLocalInfo*>(local.addr);
           linfo->write( static_cast<Attribute*>( elem.addr));
           result = elem;
           return YIELD;
         } else {
           return CANCEL;
         }
       }
    }

    case CLOSE:{
      qp->Close(args[0].addr);
      if(local.addr){
        CsvExportLocalInfo* linfo;
        linfo  = static_cast<CsvExportLocalInfo*>(local.addr);
        delete linfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}


int CsvExportVM2(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
      qp->Open(args[0].addr);
      FText* fname = static_cast<FText*>(args[1].addr);
      CcBool* append  = static_cast<CcBool*>(args[2].addr);
      CcBool* names   = static_cast<CcBool*>(args[3].addr);
      string sep = ",";
      if(qp->GetNoSons(s)==5){
        CcString* ccSep = static_cast<CcString*>(args[4].addr);
        sep =  ccSep->GetValue();
        if(sep.length()==0){
           sep =",";
        }
      }
      if(!fname->IsDefined() || !append->IsDefined() || !names->IsDefined()){
         local.setAddr(0);
      } else {
         CsvExportLocalInfo* linfo;
         linfo = (new CsvExportLocalInfo(fname->GetValue(),
                                         append->GetBoolval(),
                                         names->GetBoolval(),
                                         sep,
                                         qp->GetType(s)));
         if(!linfo->isOk()){
            delete linfo;
            local.setAddr(0);
         } else {
             local.setAddr(linfo);
         }
      }
      return 0;
    }

    case REQUEST:{
       if(local.addr==0){
         return CANCEL;
       } else {
         Word elem;
         qp->Request(args[0].addr,elem);
         if(qp->Received(args[0].addr)){
           CsvExportLocalInfo* linfo;
           linfo  = static_cast<CsvExportLocalInfo*>(local.addr);
           linfo->write( static_cast<Tuple*>( elem.addr));
           result = elem;
           return YIELD;
         } else {
           return CANCEL;
         }
       }
    }

    case CLOSE:{
      qp->Close(args[0].addr);
      if(local.addr){
        CsvExportLocalInfo* linfo;
        linfo  = static_cast<CsvExportLocalInfo*>(local.addr);
        delete linfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

ValueMapping csvexportmap[] =
{  CsvExportVM, CsvExportVM2 };

int csvExportSelect( ListExpr args )
{
  if(nl->ListLength(args)==3){
   return 0;
  } else {
   return 1;
  }
}

/*
1.3 Specification

*/


const string CsvExportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(X) x  text x bool -> stream(X)\n"
    "stream(tuple(...))) x text x bool x bool "
    "[x string]-> stream(tuple...)</text--->"
    "<text> stream csvexport [ file , append, "
    "[writenames, [separator]]]</text--->"
    "<text> Exports stream content to a csv file </text--->"
    "<text>query ten feed exportcsv['ten.csv', "
    "FALSE,TRUE,\";\"] count</text--->"
    ") )";

/*
1.4 Operator instance

*/

Operator csvexport( "csvexport",
                     CsvExportSpec,
                     2,
                     csvexportmap,
                     csvExportSelect,
                     csvexportTM);



/*
1 CSV Import

1.1 Type Mapping

  rel(tuple(a[_]1 : t[_]1)...(a[_]n : t[_]n)) x text x
                          int x string [x string] [x bool] -> stream(tuple(...))

  where t[_]i is CSVIMPORTABLE.

  text            :  indicates the filename containing the csv data
  int             :  number of lines to ignore (header)
  string          :  ignore Lines starting with this string (comment)
                     use an empty string for disable this feature

  optional string : use another separator, default is ","
  optional bool   : if set to true, ignore separators within quotes, 
                    for example,
                    treat "hello, hello" as a single item , default is false
  optional bool : multiline. If set to true, values within quotes can contain 
                  linebreaks

*/

ListExpr csvimportTM(ListExpr args){
  string err = " rel(tuple(a_1 : t_1)...(a_n : t_n)) x "
               " text x int x string [x string [x bool [ x bool]]] expected";

  int len = nl->ListLength(args);
  if((len!=4) && (len!=5) && (len !=6) && (len !=7)){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!Relation::checkType(nl->First(args)) ||
     (   !FText::checkType(nl->Second(args) ) 
      && !BinaryFile::checkType(nl->Second(args))) ||
     !CcInt::checkType(nl->Third(args)) ||
     !CcString::checkType(nl->Fourth(args) )){
    return listutils::typeError(err);
  }

  // check attribute types for csv importable
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  ListExpr attrlist = nl->Second(nl->Second(nl->First(args)));
  if(nl->IsEmpty(attrlist)){
    return listutils::typeError(err);
  }
  while(!nl->IsEmpty(attrlist)){
     ListExpr type = nl->Second(nl->First(attrlist));
     if(!am->CheckKind(Kind::CSVIMPORTABLE(),type,errorInfo)){
        ErrorReporter::ReportError("attribute type '" + nl->ToString(type) + 
                                    "' not in kind CSVIMPORTABLE");
        return nl->TypeError();
     }
     attrlist = nl->Rest(attrlist);
  }

  // create result list
  ListExpr resList = nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                     nl->Second(nl->First(args)));



  if(len==4){ // append two default values
     ListExpr defaults = nl->ThreeElemList(
                              nl->StringAtom(","),
                              nl->BoolAtom(false),
                              nl->BoolAtom(false)
                            );
     return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                               defaults, resList);
  }  

  
  // the fifth element must be of type string
 if(!CcString::checkType(nl->Fifth(args))){
    return listutils::typeError(err);
  }

  if(len==5){ // set the optional booleans to be false 
     ListExpr defaults = nl->TwoElemList(nl->BoolAtom(false),
                                             nl->BoolAtom(false));

     return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                               defaults, resList);
  }

  if(len==6){
    if(!CcBool::checkType(nl->Sixth(args))){
      return listutils::typeError(err);
    }
    ListExpr defaults = nl->OneElemList(nl->BoolAtom(false));
   return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                               defaults, resList);
  }

  // len = 7, no defaults
  if(!CcBool::checkType(nl->Seventh(args))){
    return listutils::typeError("7th attribute must be of type bool");
  }

  return resList;
}


/*
1.2 Specification


*/


const string csvimportSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>rel(tuple(...) x text x int x string [x string [x bool]] "
   "-> stream (tuple(...))</text--->"
   "<text>Rel csvimport[ FileName, HeaderSize, Comment, Separator, "
   "quotes, multiline]"
   " </text--->"
   "<text> Returns the content of the CSV (Comma Separated Value) file "
   "'FileName' as a stream of tuples. The types are defined by the first "
   "argument, 'Rel', whose tuple type acts as a template for the result tuple "
   "type. 'HeaderSize' specifies an amount of lines to ignore at the head of "
   "the imported textfile. 'Comment' defines a character that marks comment "
   "lines within the textfile. 'Separator' defines the character that is "
   "expected to separate the fields within each line of the textfile (default "
   "is \",\"."
   "quotes means that separators within quotes are ignored. If multiline is set"
   "to true, attributes within quotes can span more than 1 line.   "
   "</text--->"
   "<text> not tested !!!</text--->"
   ") )";


/* 
1.3 Abstract class for parsing csv input streams

  The methods open(), close(), readNextData()
  and isDataAvailable() must be implemented
  in subclasses
  
*/

class CSVInputStream {

// Activate debug messages
//#define _CSVInputStream_DEBUG

enum parsermode { UNQUOTED_ENVIRONMENT, QUOTED_ENVIRONMENT }; 

public:
      CSVInputStream() { }
 
/*
1.3 Create a new CSVInputStream. The 1st parameter is the
  source to read, the 2nd parameter is the separator character,
  the 3rd parameter is the comment character

*/
      CSVInputStream(string* source, string* separator, string* comment) {
          if(source){
              this -> source = string(*source);
          } else {
              this->source = "";
          }
          this -> separator = string(*separator);
          this -> comment = string(*comment);
          
          bufferPos = sizeof(buffer);
          bufferSize = sizeof(buffer);
          
          memset(buffer, 0, sizeof(buffer)); 
      }
      
      
      virtual ~CSVInputStream() { 
          close(); 
      }
      
      // Template methods, sould be overwritten in subclasses
      virtual void open() {};
      virtual void close() {};
      
      // callback after a completed tuple
      virtual void tupleComplete() {
         if(buffer[bufferPos-1] != '\n'){
           skipLine();
         }
      }   
      
      // is unprocessed data available
      virtual bool isDataAvailable() { return false; }   
      
      // Read new unprocessed data into the buffer
      virtual size_t readNextData
         (char *buffer, size_t start, size_t maxBytes) { 
         
         return 0; 
      }

      // Are unprocessed bytes in the buffer or new data
      // available?
      bool isUnprocessedDataAvailable() {
          return ((bufferSize != bufferPos) || isDataAvailable());
      }

      // Is the multiline support active?
      bool getMultiline() {
          return multiline;
      }

      // set multiline support
      void setMultiline(bool myMultiline) {
          multiline = myMultiline;
      }

      // Is the input quoted?
      bool getQuotes() {
         return quotes;
      }

      // set quoting support
      void setQuotes(bool myQuotes) {
         quotes = myQuotes;      
      }
      
      // get the configured separator symbol
      string getSeparator() {
          return separator;
      }

/*
1.3.1 Fill buffer with new data

*/
      size_t fillBuffer() {
         
         // copy unprocessed data to the beginning of the array
         size_t remainingBytes = bufferSize - bufferPos;
         if(remainingBytes > 0) {
            memcpy(buffer, buffer+bufferPos, remainingBytes);
         }
         
         #ifdef _CSVInputStream_DEBUG
         cout << "fillBuffer() : Copied " << remainingBytes;
         cout << " bytes to the beginning of the buffer" << endl;
         #endif 
         
         bufferSize = remainingBytes;
         bufferPos = 0;
         
         // Fill buffer
         if(isDataAvailable()) {
         
             size_t readData = readNextData(buffer, bufferPos, 
               sizeof(buffer) - bufferSize );
               
             bufferSize = bufferSize + readData;
             
             #ifdef _CSVInputStream_DEBUG
             cout << "fillBuffer() Filled " << readData << endl;
             cout << "Buffersize is now: " << bufferSize << " bufferPos ";
             cout << bufferPos << endl;
             #endif 
             
             return readData;
         }
         
         return 0;
      }
      
/*
1.3.2 Skip all bytes until we reach the line end

*/
      bool skipLine() {
          
          // Process bytes
          while(true) {
    
             // Are all Bytes inside our buffer processed?
             if(bufferPos == bufferSize){         
               
                // No new bytes available?
                if(fillBuffer() == 0){
                   return false;
                }
             }
             
             char c = buffer[bufferPos];
             bufferPos++;
             
             if(c == '\n') {
                 #ifdef _CSVInputStream_DEBUG
                 cout << "Skipped one line" << endl;
                 cout << "BufferPos is " << bufferPos << endl;
                 cout << "BufferSize is " << bufferSize << endl;
                 #endif
                 return true;
             }
         }
      }

/*
1.3.3 Get next csv field

*/
      string getNextValue() {
        
         parsermode mode = UNQUOTED_ENVIRONMENT;  
          
         string result;
         
         bool done = false;
         
         // Process bytes
         while(! done) {
    
            // Are all Bytes inside our buffer processed?
            if(bufferPos == bufferSize){            
               // No new bytes available?
               if(! isDataAvailable() || fillBuffer() == 0){
                   done = true;
                   continue;
               }
            }
            
            char c = buffer[bufferPos];
            bufferPos++;
   
            // normal mode (unquoted environment)
            if(mode == UNQUOTED_ENVIRONMENT) {
                
                // If this is the first byte of our field
                // and it is a comment char - skip the whole line
                if((result.empty()) && (comment.size() > 0) 
                  && (comment.find(c) != string::npos)) {
                  
                    skipLine();
                    continue;
                }
                
                // If c is a quote char and we are handling
                // quote chars
                if (c == '"' && quotes) {
                    // switch into quoted environment
                    // and skip quote char
                    mode = QUOTED_ENVIRONMENT; 
                    continue;
                }
                
                // Field separator or newline read?
                if ((separator.find(c)!=string::npos) || (c == '\n')) {
                    done = true;
                } else {
                    result += c;
                }
                
            } else {
              
                // We are inside a quoted environment
                // switch back to the normal mode
                // as soon as we read the next " char
                if (c == '"') {
                    mode = UNQUOTED_ENVIRONMENT;
                    continue;
                } 
                
                // Handling of newline chars
                if(c == '\n') { 
                  
                  // Append newline chars to output?
                  if(! multiline) {                
                    // We are not in multiline mode,
                    // skip newline char
                    continue;
                  }
                }

                // Append char to output
                result += c;
            }
         }
         
         #ifdef _CSVInputStream_DEBUG
         cout << "Return: " << ss.str() << endl;
         #endif

         return result;
      }

    protected:
       bool multiline;      // process multiline fields 
       bool quotes;         // use quotes
       string separator;    // our separator (e.g. , or ;)
       string comment;      // comment (e.g. //)
       string source;       // name of the source
       char buffer[102400]; // buffer (100 KB)
       size_t bufferPos;    // position of next unprocessed char
       size_t bufferSize;   // last position of unprocessed chars in buffer 
};


/* 
1.4 Class for processing file csv input streams

*/
class CSVFileInputStream : public CSVInputStream {

   public:
   CSVFileInputStream(string *mySource, string *mySeparator, 
      string *myComment) 
       : CSVInputStream(mySource, mySeparator, myComment) {
           
      file = NULL;
   }
   
   ~CSVFileInputStream() {
      close();
   }

   virtual void open() {
      // Is file accessible?
      if( access( source.c_str(), R_OK ) != 0 ) { 
         cout << "Unable to open file: " << source << endl;
      } else {
         file = fopen(source.c_str(), "rb");
      }
   }

   virtual void close() {
      if(file != NULL) {
         fclose(file);
         file = NULL;
      }
   }

   virtual bool isDataAvailable() {
      // Is file open?
      if(file == NULL) {
         return false;
      }
      
      return ! feof(file);
   }
 
   virtual size_t readNextData(char *buffer, size_t start, 
      size_t maxBytes) {

      if(! isDataAvailable() ) {
         return 0;
      }
      
      size_t readBytes = fread(buffer + start, 1, maxBytes, file);
      
      #ifdef _CSVInputStream_DEBUG
      cout << "readNextDataCalled read: " << readBytes;
      cout << " eof " << feof(file) << endl;
      #endif
   
      return readBytes;
   }
 
   private:
      FILE* file;

};


/* 
1.4 Class for processing binary file csv input streams

*/
class CSVBinFileInputStream : public CSVInputStream {

   public:
   CSVBinFileInputStream(BinaryFile *source, string *mySeparator, 
      string *myComment) 
       : CSVInputStream(0, mySeparator, myComment) {
       size = source->GetSize();
       this->source = source;
       pos = 0;
   }
   
   ~CSVBinFileInputStream() {
   }

   virtual void open() {
     // we cna just read binary files
   }

   virtual void close() {
      // we don't have to close binary files
   }

   virtual bool isDataAvailable() {
       return pos < size;
   }
 
   virtual size_t readNextData(char *buffer, size_t start, 
      size_t maxBytes) {

      if(! isDataAvailable() ) {
         return 0;
      }
      size_t abytes = size-pos;
      size_t readBytes = min(abytes,maxBytes);
      source->Get(pos,readBytes,buffer+start); 
      pos += readBytes;
      return readBytes;
   }
 
   private:
     BinaryFile* source;
     size_t pos;
     size_t size;
     
};



/* 
1.5 Class for processing network csv input streams

*/
class CSVNetworkInputStream : public CSVInputStream {
   
   public:
   CSVNetworkInputStream(string *mySource, string *mySeparator, 
      string *myComment) 
       : CSVInputStream(mySource, mySeparator, myComment) {
           
           readSocket = NULL;
           listenSocket = NULL;
   }

   virtual string extractPortFromSource() {
   
      // tcp:// - 6 chars
      string stringport = source.substr(6);
      return stringport;
   }

   virtual void open() {
      string stringport = extractPortFromSource();
      
      cout << "Open TCP Port: " << stringport << endl;
      
      listenSocket = Socket::CreateGlobal("0.0.0.0", stringport.c_str());

      if (!listenSocket || !listenSocket->IsOk()) {
        cerr <<  "unable listening to port: " << stringport.c_str();
        return;
      }
    
      #if defined(unix) || defined(__unix__) || defined(__unix)
      // Set socket option "reuse"
      // Prevents bind errors if the query is
      // executed multiple times
      int yes = 1;
      if (setsockopt(listenSocket -> GetDescriptor(), 
                   SOL_SOCKET, SO_REUSEADDR, &yes, 
        sizeof(int)) == -1 ) {
      
        cerr << "Set socket options failed" << endl;
      }
      #endif
      
      readSocket = listenSocket->Accept();  
      
      if(! readSocket && ! readSocket -> IsOk()) {
         cerr <<  "Accept on socket failed" << endl;
         return;
      }
      
      // Close server socket
      listenSocket -> Close();
      
   }
   
   virtual size_t readNextData(char *buffer, 
      size_t start, size_t maxBytes) {

      if(! isDataAvailable() ) {
         return 0;
      }
      
      #ifdef _CSVInputStream_DEBUG
      cout << "readNextData() start " << start << endl;
      #endif
      
      // Read data (socket is set to blocking mode)
      int readBytes = readSocket -> Read(buffer + start, 1, maxBytes);
            
      #ifdef _CSVInputStream_DEBUG
      cout << "readNextDataCalled read (from socket): " << readBytes << endl;
      #endif
      
      // Check for last read byte is
      // EOT (End of Transmission / ASCII 004)
      if(*(buffer + start + readBytes -1) == '\004') {
          readBytes--;
          close();
      } else if(readBytes <= 0) {
         
         #ifdef _CSVInputStream_DEBUG
         cout << "Read 0 bytes from socket in blocking mode ";
         cout << ", closing socket" << endl;
         #endif
         
         close();
         return 0;
      }
      
      return readBytes;
   }

   virtual void close() {
      #ifdef _CSVInputStream_DEBUG
      cout << "Closing TCP Port: " << source << endl;
      #endif
      
      if(readSocket != NULL) {
         readSocket -> Close();
         delete readSocket;
         readSocket = NULL;
      }
      
      if(listenSocket != NULL) {
         listenSocket -> Close();
         delete listenSocket;
         listenSocket = NULL;
      }
   }

/*
1.3 Is unprocessed data available?
    
*/  
   virtual bool isDataAvailable() {
   
      // Client is connected
      if(readSocket != NULL) {
         return true;
      }
      
      // There are unprocessed bytes in the buffer
      if(bufferPos != bufferSize) { 
         return true;
      }
      
      return false;
   }
   
   protected:
          Socket* listenSocket;           // FD for server listen
          Socket* readSocket;             // FD for client handling
          struct sockaddr_in serv_addr;   // Server address  
          struct sockaddr_in client_addr; // Client address

};

/* 
1.6 Class for processing reliable network csv input streams

*/
class CSVReliableNetworkInputStream : public CSVNetworkInputStream {

  public:
   CSVReliableNetworkInputStream(string *mySource, 
      string *mySeparator, string *myComment) 
       : CSVNetworkInputStream(mySource, mySeparator, myComment) {
        
        unacknowledgedTuples = 0;
        acknowledgeAfter = 1;
   }

/*
1.6.1 process source url
    "tcplb://port/acknowledgeAfter"
    
*/   
   virtual string extractPortFromSource() {
   
      // tcplb:// - 8 chars
      string stringport = source.substr(8);
      
      size_t pos = stringport.find("/");
      
      // Process optional acknowledgeAfter field
      if(pos != string::npos) {

         string acknowledgeAfterString 
            = stringport.substr(pos + 1, stringport.length());
         acknowledgeAfter = atoi(acknowledgeAfterString.c_str());

         stringport = stringport.substr(0, pos);         
      }
      
      return stringport;
   }


/*
1.6.2 send an ack to server after
  every ~acknowledgeAfter~ tuples
    
*/
   virtual void tupleComplete() { 

        unacknowledgedTuples++;

        if(unacknowledgedTuples >= acknowledgeAfter) {     
           // Send an ack char to the server
           readSocket -> Write("\006", sizeof(char));
           unacknowledgedTuples = 0;
        }
   }   
   
   protected:
      size_t unacknowledgedTuples;  // Number of
                                    // unacknowledged Tuples
      
      size_t acknowledgeAfter;      // Send an ack char to server
                                    // after every n tuples 
};

/*
1.7 This is a factory class for CSVInportStream Objects

*/
class CSVInputStreamFactory {

   public:
      static CSVInputStream* getStreamForInput(string* input, 
         string* separator, string* comment) {
          
         // Create a network intance
         if(input -> compare(0, 6, "tcp://") == 0) {
            return new CSVNetworkInputStream
               (input, separator, comment);
         } 
         
         // Create a reliable network instance
         if(input -> compare(0, 8, "tcplb://") == 0) {
            return new CSVReliableNetworkInputStream
               (input, separator, comment);
         }

         // Default: Create file instance
         return new CSVFileInputStream(input, separator, comment);
      }
};


/*
1.3 Value Mapping for csvimport

*/
class CsvImportInfo{
public:
  CsvImportInfo(ListExpr type, FText* filename,
                CcInt* hSize , CcString* comment,
                CcString* separator,
                CcBool* quotes,
                CcBool* multiline = 0
                ){

      this->separator = ",";
      this->quotes = false;

      if(separator->IsDefined() ){
         string sep = separator->GetValue();
         sep = stringutils::replaceAll(sep,"\\t","\t");
         if(sep.length()>0){
           this->separator=sep;
         }
      } 
      if(quotes->IsDefined()){
         this->quotes = quotes->GetValue();
      }

      BasicTuple = 0;
      tupleType = 0;
      defined = filename->IsDefined() && hSize->IsDefined() &&
                comment->IsDefined();
      if(!defined){
          return;
      }
      string name = filename->GetValue();
      this->multiLine = false;
      if(multiline && multiline->IsDefined()){
         this->multiLine = multiline->GetBoolval();
      }


      this->comment = comment->GetValue();
      useComment = this->comment.size()>0;

      // Create csvinputstream
      csvinputstream = CSVInputStreamFactory::getStreamForInput(&name, 
         &this->separator, &this->comment);
         
      csvinputstream -> setMultiline(multiline);
      csvinputstream -> setQuotes( quotes -> GetBoolval( ));

      csvinputstream -> open();

      if(! csvinputstream -> isUnprocessedDataAvailable() ){
         defined = false;
         return;
      }

      // skip header
      int skip = hSize->GetIntval();

      for(int i=0;i<skip && csvinputstream -> isUnprocessedDataAvailable(); 
          i++) {
        
         csvinputstream -> skipLine();
         // Callback (needed for tcplb protocol)
         csvinputstream -> tupleComplete();
      }

      if(!csvinputstream -> isUnprocessedDataAvailable()){
        defined=false;
        return;
      }

      ListExpr numType = nl->Second(
                         SecondoSystem::GetCatalog()->NumericType((type)));
      tupleType = new TupleType(numType);
      BasicTuple = new Tuple(tupleType);
      // build instances for each type
      ListExpr attrList = nl->Second(nl->Second(type));
      while(!nl->IsEmpty(attrList)){
         ListExpr attrType = nl->Second(nl->First(attrList));
         attrList = nl->Rest(attrList);
         int algId;
         int typeId;
         string tname;
         if(! ((SecondoSystem::GetCatalog())->LookUpTypeExpr(attrType,
                                               tname, algId, typeId))){
           defined = false;
           return;
         }
         Word w = am->CreateObj(algId,typeId)(attrType);
         instances.push_back(static_cast<Attribute*>(w.addr));
      }
  }


  CsvImportInfo(ListExpr type, BinaryFile* file,
                CcInt* hSize , CcString* comment,
                CcString* separator,
                CcBool* quotes,
                CcBool* multiline = 0
                ){

      this->separator = ",";
      this->quotes = false;

      if(separator->IsDefined() ){
         string sep = separator->GetValue();
         sep = stringutils::replaceAll(sep,"\\t","\t");
         if(sep.length()>0){
           this->separator=sep;
         }
      } 
      if(quotes->IsDefined()){
         this->quotes = quotes->GetValue();
      }

      BasicTuple = 0;
      tupleType = 0;
      defined = file->IsDefined() && hSize->IsDefined() &&
                comment->IsDefined();
      if(!defined){
          return;
      }
      this->multiLine = false;
      if(multiline && multiline->IsDefined()){
         this->multiLine = multiline->GetBoolval();
      }
      this->comment = comment->GetValue();
      useComment = this->comment.size()>0;

      // Create csvinputstream
      csvinputstream =  new CSVBinFileInputStream(
            file, &this->separator, &this->comment);

      csvinputstream -> setMultiline(multiline);
      csvinputstream -> setQuotes( quotes -> GetBoolval( ));

      csvinputstream -> open();

      if(! csvinputstream -> isUnprocessedDataAvailable() ){
         defined = false;
         return;
      }

      // skip header
      int skip = hSize->GetIntval();

      for(int i=0;i<skip && csvinputstream -> isUnprocessedDataAvailable(); 
          i++) {
        
         csvinputstream -> skipLine();
         // Callback (needed for tcplb protocol)
         csvinputstream -> tupleComplete();
      }

      if(!csvinputstream -> isUnprocessedDataAvailable()){
        defined=false;
        return;
      }

      ListExpr numType = nl->Second(
                         SecondoSystem::GetCatalog()->NumericType((type)));
      tupleType = new TupleType(numType);
      BasicTuple = new Tuple(tupleType);
      // build instances for each type
      ListExpr attrList = nl->Second(nl->Second(type));
      while(!nl->IsEmpty(attrList)){
         ListExpr attrType = nl->Second(nl->First(attrList));
         attrList = nl->Rest(attrList);
         int algId;
         int typeId;
         string tname;
         if(! ((SecondoSystem::GetCatalog())->LookUpTypeExpr(attrType,
                                               tname, algId, typeId))){
           defined = false;
           return;
         }
         Word w = am->CreateObj(algId,typeId)(attrType);
         instances.push_back(static_cast<Attribute*>(w.addr));
      }
  }




  ~CsvImportInfo(){
     if(BasicTuple){
       delete BasicTuple;
       BasicTuple = 0;
     }
     if(tupleType){
        tupleType->DeleteIfAllowed();
        tupleType = 0;
     }
     for(unsigned int i=0; i<instances.size();i++){
        delete instances[i];
     }
     instances.clear();

     if(csvinputstream) {
        delete csvinputstream;
        csvinputstream = NULL;
     }
   }


   bool isComment(string line){
      if(!useComment){
        return false;
      } else {
        return line.find(comment)==0;
      }
   }

   Tuple* getNext(){
     if(! csvinputstream -> isUnprocessedDataAvailable()){
        return 0;
     }
     
     Tuple* res = createTuple();

     if( ! defined ) {
       return 0;
     }

     return res;
   }

private:
  bool defined;
  bool useComment;
  string comment;
  TupleType* tupleType;
  Tuple* BasicTuple;
  vector<Attribute*> instances;
  string separator;
  bool quotes;
  // support of multiline mode
  bool multiLine;
  CSVInputStream* csvinputstream;

  Tuple* createTuple(){
      static char c = 0;
      static string nullstr(&c, 1);
      Tuple* result = BasicTuple->Clone();

      for(unsigned int i=0;i<instances.size();i++){
         Attribute* attr = instances[i]->Clone();

         if(csvinputstream -> isUnprocessedDataAvailable()){
            attr->ReadFromString(csvinputstream -> getNextValue());
         } else {
            attr->SetDefined(false);
            defined = false;
         }
         result->PutAttribute(i,attr);
      }
      
      // Callback
      csvinputstream -> tupleComplete();
      
      return result;
  }
};


template<class SType>
int csvimportVM1(Word* args, Word& result,
               int message, Word& local, Supplier s){


  CsvImportInfo* info = static_cast<CsvImportInfo*>(local.addr);
  switch(message){
    case OPEN: {
      ListExpr type = qp->GetType(qp->GetSon(s,0));
      SType* source = static_cast<SType*>(args[1].addr);
      CcInt* skip = static_cast<CcInt*>(args[2].addr);
      CcString* comment = static_cast<CcString*>(args[3].addr);
      CcString* separator = static_cast<CcString*>(args[4].addr);
      CcBool* quotes = static_cast<CcBool*>(args[5].addr);
      CcBool* multiline =  static_cast<CcBool*>(args[5].addr);
      if(info){
        delete info;
      }
      local.setAddr(new CsvImportInfo(type,source,skip,comment,separator, 
                                      quotes,multiline));
      return 0;
    }
    case REQUEST: {
      if(!info){
        return CANCEL;
      } else {
        result.addr = info->getNext();
        return result.addr?YIELD:CANCEL;
      }
    }
    case CLOSE: {
      if(info){
        delete info;
        local.addr=0;
      }

    }
    default: {
     return 0;
    }
  }
}


/*
Value Mapping array and Selection

*/
ValueMapping csvimportVM[] = {
   csvimportVM1<FText>,
   csvimportVM1<BinaryFile>
};

int csvimportSelect(ListExpr args){

  return FText::checkType(nl->Second(args))?0:1;

}

/*
1.4 Operator Instance

*/
Operator csvimport( "csvimport",
                    csvimportSpec,
                    2,
                    csvimportVM,
                    csvimportSelect,
                    csvimportTM);



/*
1.5 Operator csvimport2

This operator does the same thing as the csv import operator.
The difference is that this version of the operator does not require
the relation scheme. Instead, the names of the attributes are read form the 
first line of the csv file and all elements are assumed to be of type string.

1.5.1 Type Mapping

The signature is:

 ftext x int x string x string x bool -> 
 stream(tuple( (X string) (Y string) ...))

 filename, headersize, comment, separator, uses quotes, multiline

*/



ListExpr csvimport2TM(ListExpr args){
   string err = "ftext x int x string x string x bool  x bool expected";
   if(!nl->HasLength(args,6)){
      return listutils::typeError(err + " (wrong number of args)");
   }
   // this operator uses the arg evaluation within type mappings
   // thus, each argument is a two elem list (type value)
  ListExpr tmp = args;
  while(!nl->IsEmpty(tmp)){
    if(!nl->HasLength(nl->First(tmp),2)){
       return listutils::typeError("expected (type value)");
    }
    tmp = nl->Rest(tmp);
  }
  // now, check the types
  if(!FText::checkType(nl->First(nl->First(args)))){
      return listutils::typeError(err + " (1st is not a text)");
  } 

  if(!CcInt::checkType(nl->First(nl->Second(args)))){
      return listutils::typeError(err + " (2nd is not an int)");
  } 
  if(!CcString::checkType(nl->First(nl->Third(args)))){
      return listutils::typeError(err + " (3th is not a string)");
  } 
  if(!CcString::checkType(nl->First(nl->Fourth(args)))){
      return listutils::typeError(err + " (4th is not a string)");
  } 
  if(!CcBool::checkType(nl->First(nl->Fifth(args)))){
      return listutils::typeError(err + " (5th is not a bool)");
  } 
  if(!CcBool::checkType(nl->First(nl->Sixth(args)))){
      return listutils::typeError(err + " (6th is not a bool)");
  } 
  // ok, the types are fine, now, we have to evaluate the filename
  // as well as the separator
  string filenameExpr = nl->ToString(nl->Second(nl->First(args)));
  Word res;
  bool fnok = QueryProcessor::ExecuteQuery(filenameExpr,res);
  if(!fnok){
     return listutils::typeError("Could not get filename from expression");
  }
  FText* fn = (FText*) res.addr;
  if(!fn->IsDefined()){
     fn->DeleteIfAllowed();
     return listutils::typeError("filename is undefined");
  }
  string filename = fn->GetValue();
  fn->DeleteIfAllowed();
  fn = 0;
  res.setAddr(0);
  // get the separator 
  string separatorExpr = nl->ToString(nl->Second(nl->Fourth(args)));
  if(!QueryProcessor::ExecuteQuery(separatorExpr,res)){
     return listutils::typeError("separator string could not be evaluated");
  }
  CcString* ccseparators = (CcString*) res.addr;
  string separators = ","; // standard separator
  if(ccseparators->IsDefined() && ccseparators->GetValue().size()>0){
    separators = ccseparators->GetValue();
  }
  ccseparators->DeleteIfAllowed();
  ccseparators = 0;

  // get the first Line from the file
  ifstream file(filename.c_str());
  if(!file){
     return listutils::typeError("could not open file " + filename);
  } 
  string line;
  getline(file,line);
  if(!file.good()){
     return listutils::typeError("could not read the first line of " 
                                 + filename);
  }
  file.close();
   
  stringutils::StringTokenizer st(line,separators);
  int count = 0;
  set<string> tokens;

  ListExpr attrList= nl->TheEmptyList();
  ListExpr last = attrList;
  char c1 = 0;
  string nullstr(&c1,1);;

  while(st.hasNextToken()){
    string token = st.nextToken();
    stringutils::trim(token);
    token = stringutils::replaceAll(token,nullstr,"");
    if(token.size()<1){
      stringstream ss;
      ss << "Unknown" << count;
      token = ss.str();
    } else {
      // convert token into a valid symbol
      char f = token[0];
      if(!stringutils::isLetter(f)){
         token = "A_"+token;
      } else {
         token[0] = toupper(token[0]);
      }
      for(size_t i=1;i<token.size();i++){
         char c = token[i];
         if(!stringutils::isLetter(c) && !stringutils::isDigit(c)){
             token[i] = '_';
         }
      }
   }
   string err;
   if(!SecondoSystem::GetCatalog()->IsValidIdentifier(token,
                                                      true)){
      stringstream ss;
      ss << token << count;
      token = ss.str();
   }

   // if there is a sequence of underlines within token,
   // replace it by a single underline
   while(token.find("__")!=string::npos){
     token = stringutils::replaceAll(token,"__","_");
   }


   // avoid double names
   int c2 = 0;
   string t = token;
   while( tokens.find(token)!=tokens.end()){
      stringstream ss;
      ss << t << "_" << c2;
      c2++;
      token = ss.str();   
   }
   tokens.insert(token);

   token = token.substr(0,MAX_STRINGSIZE);
   ListExpr attr = nl->TwoElemList( nl->SymbolAtom(token), 
                                    listutils::basicSymbol<FText>());
   if(count == 0){
     attrList = nl->OneElemList( attr );
     last = attrList;
   } else {
      last = nl->Append(last,attr);
   }
   count++; 
  } 

  return nl->TwoElemList(
            listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(
                  listutils::basicSymbol<Tuple>(),
                   attrList)); 
}


int csvimport2VM(Word* args, Word& result,
                 int message, Word& local, Supplier s){

  
  CsvImportInfo* li = (CsvImportInfo*) local.addr;
  switch(message){
     case OPEN: {
        if(li){
          delete li;
        }
        local.addr = new CsvImportInfo( qp->GetType(s),
                                        (FText*) args[0].addr,
                                        (CcInt*) args[1].addr,
                                        (CcString*) args[2].addr,
                                        (CcString*) args[3].addr,
                                        (CcBool*) args[4].addr,
                                        (CcBool*) args[5].addr);
        return 0;
     }
     case REQUEST :{
         result.addr=li?li->getNext():0;
         return result.addr?YIELD:CANCEL;
     }
     case CLOSE: {
         if(li){
           delete li;
           local.addr=0;  
         }
         return 0;
     }
  }
  return -1; 


}

/*
1.5.3 Specification

*/

OperatorSpec csvimport2Spec(
  "ftext x int x string x string x bool x bool-> stream(tuple(...))",
  "csvimport2(filename, headersize, comment, separator, quotes, multilines)",
  "csvimport2 imports the csv file given by filename into a relation"
  " having text attributes exclusively. The file must contain "
  " the attribute names within its first row separated by the same "
  " character as the values. The second argument defines the size of"
  " the header, this means the given number of rows is omitted from "
  " the file. This number should be 1 at least. "
  "  Each line of the file starting with the string comment is ignored also."
  " All characters within the separator argument are treat as attribute"
  " separators. If this string is empty or undefined, a comma is used as "
  "separator."
  " Sometimes, the values are given in quotes (for example because "
  " a string contains the separator char). If so,just set the "
  " quotes argument to TRUE. "
  " The parameter multilines can be used if attributes span more than 1 line."
  "These attributes must be included in double quotes. If the multilines"
  "argument is set to true, also the quotes argument is true.",
  "query csvimport2('kinos.csv',1,\"\",\",\",TRUE,TRUE)");


/*
1.4 Operator Instance

*/
Operator csvimport2( "csvimport2",
                    csvimport2Spec.getStr(),
                    csvimport2VM,
                    Operator::SimpleSelect,
                    csvimport2TM);


/*
2 Operator nmeaImport


2.1 Type Mapping for nmeaimport


The signature is text x string -> stream(tuple(...))

The tuple type depends on the value of the second argument.
Because this operator evaulates the second argument in type mapping,
the types are extended to be pairs of (type, list), where list
is the representation of the value within the query.

Note: This type mapping is also used for operator nmeaimport[_]line.
If changes are made within this type mapping ensure to check for compatibility
with the nmeaimport[_]line.operator.

*/
static NMEAImporter* nmeaImporter=0;

ListExpr nmeaimportTM(ListExpr args){

    string err = "text x string expected";
   if(!nl->HasLength(args,2)){
      return listutils::typeError(err);
   }
   ListExpr first = nl->First(args);
   ListExpr second = nl->Second(args);

   if(!nl->HasLength(first,2) || !nl->HasLength(second,2)){
     return listutils::typeError("invalid types for operator"
                                 " evaluating arguments in type mapping");
   }
   ListExpr first1 = nl->First(first);
   if(!listutils::isSymbol(first1,FText::BasicType())){
      return listutils::typeError(err);
   }
   ListExpr second1 = nl->First(second);
   if(!listutils::isSymbol(second1,CcString::BasicType())){
      return listutils::typeError(err);
   }
   // Evaulate the String value
   ListExpr val = nl->Second(second);

   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(val),res);
   if(!success){
     return listutils::typeError("could not evaluate the value of string");
   }

   CcString* value = static_cast<CcString*>(res.addr);

   if(!value->IsDefined()) {
      value->DeleteIfAllowed();
      return listutils::typeError("string argument evaluated to be undefined");
   }
   string v = value->GetValue();
   value->DeleteIfAllowed();

   if(!nmeaImporter){
      nmeaImporter = new NMEAImporter();
   }
   if(!nmeaImporter->setType(v)){
     return listutils::typeError(v + " is not a known nmea type id, known are "
                                 + nmeaImporter->getKnownTypes());
   }

   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nmeaImporter->getTupleType());
}




/*
2.2 Value Mapping for nmeaimport

*/
int nmeaimportVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  switch(message){
    case OPEN:{
      if(local.addr){
        delete (NMEAImporter*)local.addr;
        local.addr=0;
      }
      FText* fn = static_cast<FText*>(args[0].addr);
      CcString* t = static_cast<CcString*>(args[1].addr);
      if(!fn->IsDefined() || !t->IsDefined()){
         return 0;
      }
      NMEAImporter* imp = new NMEAImporter();
      if(!imp->setType(t->GetValue())){
        delete imp;
        return 0;
      }
      string err;
      if(!imp->scanFile(fn->GetValue(),err)){
        delete imp;
        return 0;
      }
      local.addr = imp;
      return 0;
    }

   case REQUEST:{
      if(!local.addr){
         return CANCEL;
       }
       NMEAImporter* imp = static_cast<NMEAImporter*>(local.addr);
       Tuple* t = imp->nextTuple();
       result.addr=t;
       return t?YIELD:CANCEL;
    }

    case CLOSE:{
      if(local.addr){
          NMEAImporter* imp = static_cast<NMEAImporter*>(local.addr);
          delete imp;
          local.addr=0;
      }
      return 0;
    }

    default: assert(false);

  }

  return -1;
}



/*
2.3 Specification for nmeaimport

*/

const string nmeaimportSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text> text x string -> streamtuple( TL )) </text--->"
   "<text>nmeaimport( FileName, Type) </text--->"
   "<text>Extracts data from the given text file 'FileName' as a tuple stream "
   "(or an empty stream if the file contains no valid sentence). For each line"
   "represents a correct NMEA 0183 sentence of of the specified 'Type', a "
   "result tuple is created. The result tuple type TL depends on the specified "
   "sentence type 'Type'. The NMEA 0183 sender prefix (first 2 "
   "characters after the starting '$') is not considered! The known sentence "
   "types are:"
//    +nmeaImporter->getKnownTypes()+ // generic solution crashes at startup
   "{GGA, GLL, GNS, GSA, GSV, RMC, ZDA}" // using hard-coded list instead!
   ".</text--->"
   "<text> query nmeaimport('trip.nmea',\"GGA\") count</text--->"
   ") )";

/*
2.4 Operator instance for nmeaimport

*/

Operator nmeaimport( "nmeaimport",
                    nmeaimportSpec,
                    nmeaimportVM,
                    Operator::SimpleSelect,
                    nmeaimportTM);



/*
3 Operator nmeaimport[_]line

3.1 Type Mapping

Here, we just use the typoe mapping of the nmeaimport[_]line operator


3.2 Value Mapping


*/

int nmeaimport_lineVM(Word* args, Word& result,
               int message, Word& local, Supplier s){
  switch(message){
    case OPEN:{
      CcString* t = static_cast<CcString*>(args[1].addr);
      if(!t->IsDefined()){
         return 0;
      }
      string* type = new string(t->GetValue());
      local.addr = type;
      return 0;
    }

   case REQUEST:{
      if(!local.addr){
         return CANCEL;
       }
       string* type = (string*) local.addr;
       if(!nmeaImporter){
          nmeaImporter = new NMEAImporter();
       }
       nmeaImporter->setType(*type);
       delete type;
       local.addr = 0;
       FText* line = static_cast<FText*>(args[0].addr);
       if(!line->IsDefined()){
          return CANCEL;
       }
       result.addr =  nmeaImporter->getTuple(line->GetValue());
       return result.addr?YIELD:CANCEL;
    }

    case CLOSE:{
      if(local.addr){
          string* type = static_cast<string*>(local.addr);
          delete type;
          local.addr=0;
      }
      return 0;
    }
    default: assert(false);
  }
  return -1;
}


const string nmeaimport_lineSpec  =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
 "( <text>text x string -> stream(tuple( TL )) </text--->"
 "<text>nmeaimport_line( Line, Type) </text--->"
 "<text>Extracts data from the given text 'Line' as a single tuple (or an empty"
 " stream if the line is not a valid sentence). The call is successful, if Line"
 " represents a correct NMEA 0183 sentence of of the given Type. The result "
 "tuple type TL depends on the specified sentence type ' Type'. The "
 "NMEA 0183 sender prefix (first 2 characters after the starting '$') is not"
 "considered! The known sentence types are: "
 //    +nmeaImporter->getKnownTypes()+ // generic solution crashes at startup
 "{GGA, GLL, GNS, GSA, GSV, RMC, ZDA}" // using hard-coded list instead!
 ".</text--->"
 "<text> query nmeaimport_line('$GPGGA,090150.383,"
 "5131.2913,N,00726.9363,E,0,0,,102.5,M,47.5,M,,*45', \"GGA\") "
 "tconsume</text--->"
 ") )";


/*
2.4 Operator instance for nmeaimport[_]line

*/

Operator nmeaimport_line( "nmeaimport_line",
                    nmeaimport_lineSpec,
                    nmeaimport_lineVM,
                    Operator::SimpleSelect,
                    nmeaimportTM);


/*
4 Operator ~get[_]lines~

This operator reads a file and returns all lines
found in this files into a stream.


4.1 Type Mapping


*/
ListExpr get_linesTM(ListExpr args){
  string err = " string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + "(wrong number of args)");
  }

  ListExpr arg = nl->First(args);
  if(!listutils::isSymbol(arg,CcString::BasicType()) &&
     !listutils::isSymbol(arg,FText::BasicType())){
    return listutils::typeError(err);
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM() ),
                         nl->SymbolAtom(FText::BasicType()));

}

/*
4.2 ValueMapping for ~get[_]lines~


*/
template <class T>
int get_linesVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  switch(message){
     case OPEN:{
         if(local.addr){
           delete (ifstream*)local.addr;
           local.addr = 0;
         }
         T* arg = static_cast<T*>(args[0].addr);
         if(!arg->IsDefined()){
           return 0;
         }
         ifstream* in = new ifstream();
         in->open(arg->GetValue().c_str());
         local.addr = in;
         return 0;
     }

     case REQUEST:{
        if(!local.addr){
           return CANCEL;
        }
        ifstream* in = static_cast<ifstream*>(local.addr);
        if(!in->good()){
           return CANCEL;
        }
        string line;
        getline(*in,line);
        if(!in->good()){
          return CANCEL;
        }
        result.addr = new FText(true, line);
        return YIELD;
     }

     case CLOSE:{
        if(local.addr){
          ifstream* in = static_cast<ifstream*>(local.addr);
          in->close();
          delete in;
          local.addr = 0;
        }
        return 0;
     }
  }
  return -1;
}

ValueMapping get_linesMAPS[] = {get_linesVM<CcString>,
                              get_linesVM<FText>};

int get_linesSel(ListExpr args){
   return listutils::isSymbol(nl->First(args), CcString::BasicType())?0:1;
}



const string get_linesSpec  =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
 "( <text>{text,string} -> stream(text)</text--->"
 "<text>get_lines( FileName ) </text--->"
 "<text>Returns the content of file 'FileName' line by line as text stream."
 "</text--->"
 "<text> query get_lines('ten.csv') count</text--->"
 ") )";


Operator get_lines( "get_lines",
                     get_linesSpec,
                     2,
                     get_linesMAPS,
                     get_linesSel,
                     get_linesTM);


/*
2 Operator shpExport

2.1 Type Mapping

*/

ListExpr shpexportTM(ListExpr args){
  int len = nl->ListLength(args);
  if((len!=2) && (len != 3) && (len !=4)){
    ErrorReporter::ReportError("wrong number of arguments");
    return nl->TypeError();
  }
  string err = "   stream(SHPEXPORTABLE) x text [ x text ]\n "
               " or stream(tuple(...)) x text x attrname [x text] expected";

  // the second argument must be a text
  if(!FText::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }

  ListExpr stream = nl->First(args);
  if(Stream<Tuple>::checkType(stream)){
     if(len<3){
        return listutils::typeError(err);
     }
     // case stream(tuple) x text x attrname [ x text]
     ListExpr an = nl->Third(args);
     if(!listutils::isSymbol(an)){
       return listutils::typeError(err + ": invalid attribute name");
     }
     ListExpr type;
     string name = nl->SymbolValue(an);
     ListExpr attrList = nl->Second(nl->Second(stream));
     int pos = listutils::findAttribute(attrList, name, type);
     if(pos<1){
       return listutils::typeError(err + ": attribute " + name +
                                   " not member of tuple");
     }
     pos--;
     ListExpr errorInfo = listutils::emptyErrorInfo();
     // type of attribute must be in KIND SHPEXORTABLE
     if(!am->CheckKind(Kind::SHPEXPORTABLE(),type, errorInfo)){
       return listutils::typeError(err + ":Attribute "+ name +
                                  "not in kind SHPEXPORTABLE");
     }
     if(len==4){
        ListExpr shx = nl->Fourth(args);
        if(!FText::checkType(shx)){
           return listutils::typeError(err);
        }
        // all the things are ok
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList( nl->IntAtom(pos)),
                             stream);
     } else {
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                                 nl->TwoElemList(nl->TextAtom(""),
                                                 nl->IntAtom(pos)),
                                 stream);
     }
  }
  // check for stream<SHPEXPORTABLE>
  if(!nl->HasLength(stream,2)){
    return listutils::typeError(err);
  }

  if(!listutils::isSymbol(nl->First(stream), Stream<Attribute>::BasicType())){
    return listutils::typeError(err);
  }

  if(len==3){ // having file name for idx file
     ListExpr idx = nl->Third(args);
     if(!FText::checkType(idx)){
        return listutils::typeError(err);
     }
     return stream;
  } else {
     // no name for idx file is given
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->TextAtom("")),
                             stream);
  }
}

/*
2.2 Value Mapping

*/

class shpLInfo{
 public:
    shpLInfo(FText* name, FText* idxname){
      if(!name->IsDefined()){
         defined = false;
      } else {
         defined = true;
         first = true;
         baseName = (name->GetValue());
         recno = 1;
         boxdef = false;
         xMin = 0;
         xMax = 0;
         yMin = 0;
         yMax = 0;
         zMin = 0;
         zMax = 0;
         mMin = 0;
         mMax = 0;
         if(!idxname->IsDefined() ||
            (idxname->GetValue()).length()==0){
            produceIdx = false;
         } else {
            produceIdx = true;
            idxName = idxname->GetValue();
         }
      }
    }

    bool writeHeader(uint32_t type){
       file.open((baseName ).c_str(),
                  ios::out | ios::trunc | ios_base::binary);
       if(!file.good()){
         return false;
       }
       uint32_t code = 9994;
       WinUnix::writeBigEndian(file,code);
       uint32_t unused = 0;
       for(int i=0;i<5;i++){
         WinUnix::writeBigEndian(file,unused);
       }
       // dummy for file length
       uint32_t length = 100;
       WinUnix::writeBigEndian(file,length);
       uint32_t version = 1000;
       WinUnix::writeLittleEndian(file,version);
       WinUnix::writeLittleEndian(file,type);
       double boxpos = 0.0;
       for(int i=0;i<8;i++){
         WinUnix::writeLittle64(file,boxpos);
       }

       if(produceIdx){
         idxfile.open((idxName ).c_str(),
                    ios::out | ios::trunc | ios_base::binary);
         if(!idxfile.good()){
            produceIdx = false;
           return file.good();
         }
         uint32_t code = 9994;
         WinUnix::writeBigEndian(idxfile,code);
         uint32_t unused = 0;
         for(int i=0;i<5;i++){
           WinUnix::writeBigEndian(idxfile,unused);
         }
         // dummy for file length
         uint32_t length = 100;
         WinUnix::writeBigEndian(idxfile,length);
         uint32_t version = 1000;
         WinUnix::writeLittleEndian(idxfile,version);
         WinUnix::writeLittleEndian(idxfile,type);
         double boxpos = 0.0;
         for(int i=0;i<8;i++){
           WinUnix::writeLittle64(idxfile,boxpos);
         }
       }
       return file.good();
    }

    bool write(Attribute* value) {
       if(!file.good() || ! defined ){
         return false;
       }
       if(first){
          if(!writeHeader(value->getshpType())){
             return false;
          }
          first = false;
       }
       uint32_t offset = file.tellp() /2; // measured as 16 bit words
       value->writeShape(file,recno++);
       if(value->hasBox()){
          if(!boxdef){
             xMin = value->getMinX();
             xMax = value->getMaxX();
             yMin = value->getMinY();
             yMax = value->getMaxY();
             zMin = value->getMinZ();
             zMax = value->getMaxZ();
             mMin = value->getMinM();
             mMax = value->getMaxM();
             boxdef=true;
          } else {
             xMin = min(xMin,value->getMinX());
             xMax = max(xMax,value->getMaxX());
             yMin = min(yMin,value->getMinY());
             yMax = max(yMax,value->getMaxY());
             zMin = min(zMin,value->getMinZ());
             zMax = max(zMax,value->getMaxZ());
             mMin = min(mMin,value->getMinM());
             mMax = max(mMax,value->getMaxM());
          }
       }
       uint32_t contentlength = file.tellp()/2 - 4 - offset;
       if(produceIdx){
          WinUnix::writeBigEndian(idxfile, offset);
          WinUnix::writeBigEndian(idxfile, contentlength);
       }

       return file.good();
    }



    void close(){
       // write correct file-length into file
       // write correct bounding box into file
       uint32_t len = file.tellp() / 2; // measured in 16 bit words
       file.seekp(24,ios_base::beg);
       WinUnix::writeBigEndian(file,len);
       file.seekp(36,ios_base::beg);
       WinUnix::writeLittle64(file,xMin);
       WinUnix::writeLittle64(file,yMin);
       WinUnix::writeLittle64(file,xMax);
       WinUnix::writeLittle64(file,yMax);
       WinUnix::writeLittle64(file,zMin);
       WinUnix::writeLittle64(file,zMax);
       WinUnix::writeLittle64(file,mMin);
       WinUnix::writeLittle64(file,mMax);
       file.close();

       if(produceIdx){
         uint32_t len = idxfile.tellp() / 2;
         idxfile.seekp(24,ios_base::beg);
         WinUnix::writeBigEndian(idxfile,len);
         idxfile.seekp(36,ios_base::beg);
         WinUnix::writeLittle64(idxfile,xMin);
         WinUnix::writeLittle64(idxfile,yMin);
         WinUnix::writeLittle64(idxfile,xMax);
         WinUnix::writeLittle64(idxfile,yMax);
         WinUnix::writeLittle64(idxfile,zMin);
         WinUnix::writeLittle64(idxfile,zMax);
         WinUnix::writeLittle64(idxfile,mMin);
         WinUnix::writeLittle64(idxfile,mMax);
         idxfile.close();
       }

    }

    int getIndex(){ return index; }
    void setIndex(int index) {this->index = index;}

 private:
    string baseName;
    bool first;
    bool defined;
    ofstream file;
    int recno;
    double xMin;
    double xMax;
    double yMin;
    double yMax;
    double zMin;
    double zMax;
    double mMin;
    double mMax;
    bool boxdef;
    int index;
    bool produceIdx;
    string idxName;
    ofstream idxfile;

};

int shpexportVM1(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
       qp->Open(args[0].addr);
       FText* fname = static_cast<FText*>(args[1].addr);
       FText* idxname = static_cast<FText*>(args[2].addr);
       local.setAddr(new shpLInfo(fname,idxname));
       return 0;
    }
    case REQUEST: {
      shpLInfo* linfo= static_cast<shpLInfo*>(local.addr);
      if(!linfo){
         return CANCEL;
      }
      Word elem;
      qp->Request(args[0].addr, elem);
      if(qp->Received(args[0].addr)){
        bool ok;
        ok = linfo->write( static_cast<Attribute*>(elem.addr));
        ok = true;
        if(ok){
          result = elem;
          return YIELD;
        } else {
          Attribute* del = static_cast<Attribute*>(elem.addr);
          del->DeleteIfAllowed();
          result.setAddr(0);
          return CANCEL;
        }
      } else {
         result.setAddr(0);
         return CANCEL;
      }
    }
    case CLOSE:{
      qp->Close(args[0].addr);
      if(local.addr){
        shpLInfo* linfo = static_cast<shpLInfo*>(local.addr);
        linfo->close();
        delete linfo;
        local.addr=0;
      }
      return 0;
    }
    default: return 0;
  }
}

int shpexportVM2(Word* args, Word& result,
                           int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN:{
       qp->Open(args[0].addr);
       FText* fname = static_cast<FText*>(args[1].addr);
       FText* idxname = static_cast<FText*>(args[3].addr);

       cout << "fname = " << fname->GetValue() << endl;
       cout << "idxname = " << idxname->GetValue() << endl;

       shpLInfo* linfo  = new shpLInfo(fname, idxname);
       int attrPos = (static_cast<CcInt*>(args[4].addr))->GetIntval();

       cout << " attrIndex = " << attrPos << endl;

       linfo->setIndex( attrPos);
       local.setAddr(linfo);
       return 0;
    }
    case REQUEST: {
      shpLInfo* linfo= static_cast<shpLInfo*>(local.addr);
      if(!linfo){
         return CANCEL;
      }
      Word elem;
      qp->Request(args[0].addr, elem);
      if(qp->Received(args[0].addr)){
        Tuple* tuple = static_cast<Tuple*>(elem.addr);
        Attribute* attr;
        int attrPos = linfo->getIndex();
        attr  = tuple->GetAttribute(attrPos);
        linfo->write(attr);
        result = elem;
        return YIELD;
      } else {
         result.setAddr(0);
         return CANCEL;
      }
    }
    case CLOSE:{
      qp->Close(args[0].addr);
      if(local.addr){
        shpLInfo* linfo = static_cast<shpLInfo*>(local.addr);
        linfo->close();
        delete linfo;
        local.addr=0;
      }
      return 0;
    }
    default: return 0;
  }
}

/*
2.3 Value Mapping Array

*/

ValueMapping shpexportmap[] =
{  shpexportVM1, shpexportVM2};

/*
2.4 Selection Function

*/

int shpexportSelect( ListExpr args )
{ if(!listutils::isTupleStream(nl->First(args))){
    return 0;
  } else {
    return 1;
  }
}

/*
2.5 Specification

*/
const string shpexportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>stream(T) x  text [x text] -> stream(T), X in SHPEXPORTABLE\n"
    "stream(tuple(T))) x text x attrname [x text] -> stream(tuple(T))"
    ", (attrname W) in T, W in SHPEXPORTABLE</text--->"
    "<text> stream shpexport [ shpfile [, idxfile] ]\n"
    "tuplestream shpexport[ shpfile, attrname [, idxfile] ]</text--->"
    "<text> Exports stream content to the specified shp file. If the optional"
    "argument is provided, also the according shape index file is created."
    "</text--->"
    "<text>query  Kinos feed projecttransformstream[geoData] "
    "shpexport['kinos.shp'] count</text--->"
    ") )";

/*
2.6 Operator Instance

*/

Operator shpexport( "shpexport",
                     shpexportSpec,
                     2,
                     shpexportmap,
                     shpexportSelect,
                     shpexportTM);


/*
3 DB3-export

3.1 Type Mapping

  stream(tuple ( ... )) x string -> stream(tuple(...))

*/



ListExpr db3exportTM(ListExpr args){
  string err =   "stream(tuple ( ... )) x text  expected";
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(!IsStreamDescription(nl->First(args)) ||
     !nl->IsEqual(nl->Second(args),FText::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  return nl->First(args);
}

/*
3.2 Value Mapping

*/

class Db3LInfo{
public:
   Db3LInfo(FText* fname, ListExpr type){
     if(!fname->IsDefined()){ // no correct filename
        defined=false;
     }  else {
        if(!f.good()){
          defined = false;
        } else {
          this->fname = (fname->GetValue());
          first = true;
          firstMemo=true;
          defined = true;
          memNumber = 0;
          recNumber = 0;
          ListExpr rest = nl->Second(nl->Second(type));
          while(!nl->IsEmpty(rest)){
            string  name = nl->SymbolValue(nl->First(nl->First(rest)));
            rest = nl->Rest(rest);
            name = name.substr(0,10);// allow exactly 11 chars
            names.push_back(name);
          }
        }
     }
   }

   void writeHeader(Tuple* tuple){
      bool hasMemo = false;
      uint16_t recSize = 1; // deleted marker
      int exportable = 0;
      for(int i=0;i<tuple->GetNoAttributes();i++){
        Attribute* attr = tuple->GetAttribute(i);
        if(attr->hasDB3Representation()){
          exportable++;
          bool ismemo = false;
          if(attr->getDB3Type() == 'M'){
            hasMemo = true;
            ismemo = true;
          }
          unsigned char len = attr->getDB3Length();
          if(ismemo){
            len = 10;
          }
          recSize += len;
          exps.push_back(true);
          lengths.push_back(len);
          dbtypes.push_back(attr->getDB3Type());
          decCnts.push_back(attr->getDB3DecimalCount());
          isMemo.push_back(ismemo);
        } else {
          exps.push_back(false);
          lengths.push_back(0);
          dbtypes.push_back('L');
          decCnts.push_back(0);
          isMemo.push_back(false);
        }
      }
      if(exportable == 0){ // no exportable attributes found
          defined = false;
          return;
      }
      // open the file
      f.open((this->fname).c_str(),
              ios::out | ios::trunc | ios::binary);

      if(!f.good()){
          defined = false;
          return;
      }

      unsigned char code = hasMemo?0x83:0x03;
      WinUnix::writeLittleEndian(f,code);
      // wrint an dummy date
      unsigned char yy = 99;
      unsigned char mm = 01;
      unsigned char dd = 01;
      f << yy << mm << dd;
      uint32_t num = 1;
      WinUnix::writeLittleEndian(f,num); // dummy number of records
      // header size

      uint16_t headersize = 33 + exportable*32;
      WinUnix::writeLittleEndian(f,headersize);

      //  record size
      WinUnix::writeLittleEndian(f,recSize);
      unsigned char reserved=0;
      for(int i=0;i<20;i++){
          WinUnix::writeLittleEndian(f,reserved);
      }
      unsigned char zero = 0;
      for(unsigned int i=0; i< names.size(); i++){
         if(exps[i]){
           // write Field descriptor
           string name = names[i];
           int len = name.length();
           f << name;
           // fill with zeros
           for(int j=len;j<11;j++){
             f << zero;
           }
           // type
           WinUnix::writeLittleEndian(f,dbtypes[i]);
           // address in memory
           uint32_t adr = 0;
           WinUnix::writeLittleEndian(f,adr);
           // len
           WinUnix::writeLittleEndian(f,lengths[i]);
           // decimal count
           WinUnix::writeLittleEndian(f,decCnts[i]);
           for(int j=0;j< 14; j++){
              f << reserved;
           }
         }

      }
      unsigned char term = 0x0D;
      WinUnix::writeLittleEndian(f,term);
      if(!f.good()){
         f.close();
         defined = false;
      }
   }


   void writeMemo(string memo){
     string no ="          "; // 10 spaces
     if(firstMemo){
        string memoname;
        bool endsWith = true;
        if(fname.size() < 4){
           endsWith = false;
        } else { 
           size_t s = fname.size();
           endsWith = (fname[s-4] == '.') &&
                      ((fname[s-3]=='d') || (fname[s-3]=='D') ) &&
                      ((fname[s-2]=='b') || (fname[s-2]=='B') ) &&
                      ((fname[s-1]=='f') || (fname[s-1]=='F') ) ;
        }
        if(endsWith){
           memoname = fname;
           size_t p = fname.size()-1;
           if(memoname[p] == 'f'){
             memoname[p] = 't';
           } else {
             memoname[p] = 'T';
           }
        } else {
          memoname = fname +".dbt"; 
        }
        fmemo.open(memoname.c_str(),
                   ios::out | ios::trunc | ios::binary);
        memNumber = 1;
        firstMemo = false;
        // write the header
        unsigned char zero = 0;
        unsigned char version = 3;
        for(int  i=0;i<512;i++){
           if(i==16){
              fmemo << version;
           } else {
              fmemo << zero;
           }
        }
     }
     // empty memo or in
     if(!fmemo.good() || memo.length()==0){
       f << no;
       return;
     }
     // write block number to f
     stringstream ss;
     ss << memNumber;
     no = extendString(ss.str(),10,0);
     f << no;
     // write data to fmemo
     fmemo << memo;
     unsigned char term = 0x1A;
     fmemo << term << term;
     unsigned int len = memo.length() + 2 ;
     unsigned int blocks = len / 512;
     unsigned int os = len % 512;
     if(os!=0){
        blocks++;
     }
     unsigned char zero = 0;
     // fill the block with zero
     while(os!=0){
       fmemo << zero;
       os = (os + 1 ) % 512;
     }
     memNumber += blocks;

   }

   void write(Tuple* tuple){
      if(!defined){ // invalid filename input
        return;
      }
      if(first){
         writeHeader(tuple);
         first = false;
      }
      if(!defined){ // no exportable attributes
         return;
      }
      recNumber++;

      f << ' ';  // mark record as non-deleted
      for(unsigned int i=0;i<names.size();i++){
        if(exps[i]){
          unsigned char len = lengths[i];
          string s = (tuple->GetAttribute(i))->getDB3String();
          bool ismemo = isMemo[i];
          if(!ismemo){
            char e = dbtypes[i]=='C'?0:' ';
            s = extendString(s,len,e);
            f << s;
          } else {
            writeMemo(s);
          }
        }
      }
   }

   string extendString(string s, int len,const char c){
      s = s.substr(0,len); // shrink to maximum length
      int e = len-s.length();
      stringstream ss;
      ss << s;
      for(int i=0;i<e; i++){
       ss << c;
      }
      return ss.str();
   }

   void close() {
     if(defined){
       unsigned char eof = 0x1A;
       f << eof;
       f.seekp(4,ios::beg);
       WinUnix::writeLittleEndian(f,recNumber);
       f.seekp(0,ios::end);
       f.close();

       // close memo if used
       if(!firstMemo){
         fmemo.seekp(0,ios::beg);
         WinUnix::writeLittleEndian(fmemo,memNumber);
         fmemo.seekp(0,ios::end);
         fmemo.close();
       }
     }
   }

private:
  fstream f;
  fstream fmemo;
  bool defined;
  bool first;
  bool firstMemo;
  uint32_t memNumber;
  vector<string> names; // names for the attributes
  vector<unsigned char> lengths;  // lengths for the attributes
  vector<bool> exps;     // flags whether the attribute is exportable
  vector<unsigned char> dbtypes;
  vector<unsigned char> decCnts;
  vector<bool> isMemo;
  string fname;

  uint32_t recNumber;        // number of contained records
};


int db3exportVM(Word* args, Word& result,
                int message, Word& local, Supplier s){

  switch( message )
  {
    case OPEN:{
       qp->Open(args[0].addr);
       FText* fname = static_cast<FText*>(args[1].addr);
       Db3LInfo* linfo  = new Db3LInfo(fname,qp->GetType(s));
       local.setAddr(linfo);
       return 0;
    }
    case REQUEST: {
      Db3LInfo* linfo= static_cast<Db3LInfo*>(local.addr);
      if(!linfo){
         return CANCEL;
      }
      Word elem;
      qp->Request(args[0].addr, elem);
      if(qp->Received(args[0].addr)){
        Tuple* tuple = static_cast<Tuple*>(elem.addr);
        linfo->write(tuple);
        result = elem;
        return YIELD;
      } else {
         result.setAddr(0);
         return CANCEL;
      }
    }
    case CLOSE:{
      qp->Close(args[0].addr);
      if(local.addr){
        Db3LInfo* linfo = static_cast<Db3LInfo*>(local.addr);
        linfo->close();
        delete linfo;
        local.addr=0;
      }
      return 0;
    }
    default: return 0;
  }
}

/*
3.3 Specification

*/

const string db3exportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> stream(tuple(...))) x text -> stream(tuple...)</text--->"
    "<text> stream db3export [ file ]</text--->"
    "<text> Exports stream content to a db3 file </text--->"
    "<text> not tested !!!</text--->"
    ") )";

/*
3.4 Operator Instance

*/
Operator db3export( "db3export",
                     db3exportSpec,
                     db3exportVM,
                     Operator::SimpleSelect,
                     db3exportTM);



/*
4 Operator shptype


4.1 Type Mapping

text -> text

*/

ListExpr shptypeTM(ListExpr args){
  if(nl->ListLength(args)==1 && nl->IsEqual(nl->First(args),
                                            FText::BasicType())){
     return nl->SymbolAtom(FText::BasicType());
  }
  ErrorReporter::ReportError("text expected");
  return nl->TypeError();
}

/*
4.2 Value mapping

*/

/*
This function retrieves the type of a

*/
string getShpType(const string fname, bool& correct, string& errorMessage){
  correct = false;
  if(fname.length()==0){
     errorMessage = "invalid filename";
     return "";
  }

  ifstream f(fname.c_str(),std::ios::binary);
  if(!f.good()){
     errorMessage = "problem in reading file";
     return "";
  }

  f.seekg(0,ios::end);
  streampos flen = f.tellg();
  if(flen < 100){
     errorMessage =  "not a valid shape file";
     f.close();
     return "";
  }


  f.seekg(0,ios::beg);
  uint32_t code = 0;
  f.read(reinterpret_cast<char*>(&code),4);
  if(WinUnix::isLittleEndian()){
     code = WinUnix::convertEndian(code);
  }
  if(code!=9994){
     errorMessage = "invalid file code  detected";
     f.close();
     return "";
  }

  uint32_t version;
  f.seekg(28,ios::beg);
  f.read(reinterpret_cast<char*>(&version),4);
  if(!WinUnix::isLittleEndian()){
      version = WinUnix::convertEndian(version);
  }
  if(version != 1000){
    errorMessage = "invalid version detected";
    f.close();
    return "";
  }
  uint32_t type;
  f.read(reinterpret_cast<char*>(&type),4);
  if(!WinUnix::isLittleEndian()){
    type = WinUnix::convertEndian(type);
  }
  f.close();

  switch(type){
    case 0 : { errorMessage = "null shape, no corresponding secondo type";
               return "";
             }
    case 1 : { correct = true;
               return Point::BasicType();
             }
    case 3 : { correct = true;
               return Line::BasicType();
             }
    case 5 : { correct = true;
               return Region::BasicType();
             }
    case 8 : { correct = true;
               return Points::BasicType();
             }
    case 11 : { errorMessage = "PointZ, no corresponding secondo type";
               return "";
             }
    case 13 : { errorMessage = ("PolyLineZ, no corresponding secondo type");
               return "";
             }
    case 15 : { errorMessage = ("PolygonZ, no corresponding secondo type");
               return "";
             }
    case 18 : { errorMessage=("MultiPointZ, no corresponding secondo type");
               return "";
             }
    case 21 : { errorMessage=("PointM, no corresponding secondo type");
               return "";
             }
    case 23 : { errorMessage =("PolyLineM, no corresponding secondo type");
               return "";
             }
    case 25 : {errorMessage=("PolygonM, no corresponding secondo type");
               return "";
             }
    case 28 : { errorMessage=("MultiPointM, no corresponding secondo type");
               return "";
             }
    case 31 : { errorMessage = ("MultiPatch, no corresponding secondo type");
               return "";
             }
    default : errorMessage = " not a valid shape type";
              return "";
  }

}




int shptypeVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  FText* FN = static_cast<FText*>(args[0].addr);
  if(!FN->IsDefined()){
     res->Set(false,"");
     return 0;
  }
  bool correct;
  string shpType;
  string errMsg;
  shpType = getShpType(FN->GetValue(), correct, errMsg);
  if(!correct){
    res->Set(true, errMsg);
    return 0;
  }
  string value = "()";
  if(shpType==Point::BasicType()){
    value = "(0 0)";
  }
  res->Set(true, "[const "+shpType+" value "+value+"]");
  return 0;
}

/*
4.3 Specification

*/

const string shptypeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text -> text</text--->"
    "<text> shptype(filename) </text--->"
    "<text> returns an object description of the secondo object"
    " stored within the shape file specified by the name</text--->"
    "<text> not tested !!!</text--->"
    ") )";



/*
4.4 Operator instance

*/

Operator shptype( "shptype",
                   shptypeSpec,
                   shptypeVM,
                   Operator::SimpleSelect,
                   shptypeTM);



/*
5 Operator shpimport

5.1 Type Mapping

s x text -> stream(s), s in {point, points, line, region}

*/

ListExpr shpimportTM(ListExpr args){
  string err =
  " s x text -> stream(s), s in {point, points, line, region} expected";
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!nl->IsEqual(nl->Second(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  if(nl->AtomType(arg1)!=SymbolType){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  string t1 = nl->SymbolValue(arg1);

  if(t1!=Point::BasicType() &&
     t1!=Points::BasicType()  &&
     t1!=Line::BasicType()  &&
     t1!=Region::BasicType()){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), arg1);
}



/*
5.2 Value mapping

*/

class shpimportInfo{

 public:

   shpimportInfo( const ListExpr allowedType1, const FText* fname){
      buffer = 0;
      int allowedType = -1;
      if(listutils::isSymbol(allowedType1,Point::BasicType())){
           allowedType=1;
       } else  if(listutils::isSymbol(allowedType1,Line::BasicType())){
           allowedType=3;
       } else  if(listutils::isSymbol(allowedType1,Region::BasicType())){
           allowedType=5;
       } else  if(listutils::isSymbol(allowedType1,Points::BasicType())){
           allowedType=8;
       }


     if(!fname->IsDefined()){
       defined = false;
     } else {
       defined = true;
       string name = fname->GetValue();
       this->filename = name;
       file.open(name.c_str(),ios::binary);
       buffer = new char[FILE_BUFFER_SIZE];
       file.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);

       if(!file.good()){
         defined = false;
         file.close();
       } else {
         defined = readHeader(allowedType);
         if(!defined){
           file.close();
         } else {
           file.seekg(100,ios::beg); // to the first data set
         }
       }
     }
   }

   ~shpimportInfo(){
      if(buffer){
         delete[] buffer;
      }
   }


   Attribute* getNext(){
      if(!defined){
        return 0;
      }
      switch(type){
        case 1: return getNextPoint();
        case 3: return getNextLine();
        case 5: return getNextPolygon();
        case 8: return getNextMultiPoint();
        default: return 0;
      }
   }
   void close(){
     if(defined){
        file.close();
        defined = false;
     }
   }

 private:

   bool defined;
   ifstream file;
   string filename;
   uint32_t type;
   streampos fileend;
   char* buffer;

   bool readHeader(unsigned int allowedType){
      file.seekg(0,ios::end);
      streampos p = file.tellg();
      fileend = p;
      if(p< 100){ // minimum size not reached
         return false;
      }
      file.seekg(0,ios::beg);
      uint32_t code;
      uint32_t version;
      file.read(reinterpret_cast<char*>(&code),4);
      file.seekg(28,ios::beg);
      file.read(reinterpret_cast<char*>(&version),4);
      file.read(reinterpret_cast<char*>(&type),4);
      if(WinUnix::isLittleEndian()){
        code = WinUnix::convertEndian(code);
      } else {
         version = WinUnix::convertEndian(version);
         type = WinUnix::convertEndian(type);
      }
      if(code!=9994){
        return false;
      }
      if(version != 1000){
        return false;
      }
      return type==allowedType;
   }

   uint32_t readBigInt32(){
      uint32_t res;
      file.read(reinterpret_cast<char*>(&res),4);
      if(WinUnix::isLittleEndian()){
         res = WinUnix::convertEndian(res);
      }
      return res;
   }

   uint32_t readLittleInt32(){
      uint32_t res;
      file.read(reinterpret_cast<char*>(&res),4);
      if(!WinUnix::isLittleEndian()){
         res = WinUnix::convertEndian(res);
      }
      return res;
   }

   double readLittleDouble(){
      uint64_t tmp;
      file.read(reinterpret_cast<char*>(&tmp),8);
      if(!WinUnix::isLittleEndian()){
         tmp = WinUnix::convertEndian(tmp);
      }
      void* tmpv = (void*) &tmp;
      double res = * (reinterpret_cast<double*>(tmpv));
      return res;
   }

   Attribute* getNextPoint(){
      if(file.tellg() == fileend){
        return 0;
      }
     // uint32_t recNo =
      readBigInt32();
      uint32_t recLen = readBigInt32();
      uint32_t type = readLittleInt32();
      if(type == 0){ // null shape
        if(recLen!=2 || !file.good()){
          cerr << "Error in shape file detected " << __LINE__ << endl;
          defined = false;
          file.close();
          return 0;
        } else {
          return new Point(false,0,0);
        }
      }
      if(type != 1 || recLen != 10){
          cerr << "Error in shape file detected " << __LINE__ << endl;
          defined = false;
          file.close();
          return 0;
      }
      double x = readLittleDouble();
      double y = readLittleDouble();
      if(!file.good()){
          cerr << "Error in shape file detected " << __LINE__ << endl;
          defined = false;
          file.close();
          return 0;
      }
      return new Point(true,x,y);
   }

   Attribute* getNextMultiPoint(){
      if(file.tellg()==fileend){
         return 0;
      }
      uint32_t recNo = 0;
      recNo =  readBigInt32();
      uint32_t len = 0;
      len = readBigInt32();
      uint32_t type = 0;
      type = readLittleInt32();

      if(!file.good()){
         cerr << " problem in reading file " << __LINE__ << endl;
         cerr << "recNo = " << recNo << endl;
         cerr << "len = " << len << endl;
         cerr << "type = " << type << endl;
         defined = false;
         return 0;
      }

      if(type==0){
        if(len!=2){
           cerr << "Error in shape file detected " << __LINE__ << endl;
           defined = false;
           file.close();
           return 0;
        } else {
           return new Points(1);
        }
      }
      if(type!=8){
         cerr << "Error in shape file detected " << __LINE__ << endl;
         cout << "type = " << type << endl;
         cout << "file.good = " << file.good() << endl;
         defined = false;
         file.close();
         return 0;
      }
      // ignore Bounding box
      readLittleDouble();
      readLittleDouble();
      readLittleDouble();
      readLittleDouble();
      uint32_t numPoints = readLittleInt32();

      uint32_t expectedLen = (40 + numPoints*16) / 2;
      if(len != (expectedLen)){
         cerr << "Error in file " << __LINE__ << endl;
         cerr << "len = " << len << endl;
         cerr << "numPoints " << numPoints << endl;
         cerr << " expected" << expectedLen << endl;
         file.close();
         defined = false;
         return 0;
      }
      Points* ps = new Points(numPoints);
      Point p;
      ps->StartBulkLoad();
      for(unsigned int i=0;i<numPoints && file.good();i++){
         double x = readLittleDouble();
         double y = readLittleDouble();
         p.Set(x,y);
         (*ps) += p;
      }
      ps->EndBulkLoad();
      if(!file.good()){
        cerr << "Error in file " << __LINE__ << endl;
        delete ps;
        return 0;
      } else {
         return ps;
      }
   }

   Attribute* getNextLine(){
     if(file.tellg()==fileend){
       return 0;
     }
     //uint32_t recNo =
     readBigInt32();
     uint32_t len = readBigInt32();
     uint32_t type = readLittleInt32();
     if(type==0){
       if(len!=2){
          cerr << "SHPIMPORT:: Error in file detected" 
               << filename << endl;
          file.close();
          defined = false;
          return 0;
       } else {
         return new Line(1);
       }
     }
     // a non-null line
     if(type!=3){
       cerr << "Error in file detected" << endl;
       file.close();
       defined = false;
       return 0;
     }
     // ignore box
     readLittleDouble();
     readLittleDouble();
     readLittleDouble();
     readLittleDouble();

     uint32_t numParts  = readLittleInt32();
     uint32_t numPoints = readLittleInt32();
     vector<int> parts;
     for(unsigned int i=0;i<numParts && file.good() ; i++){
        uint32_t part = readLittleInt32();
        parts.push_back(part);
     }
     if(!file.good()){
       cerr << "SHPIMPORT:: error in reading file" << filename <<  endl;
       file.close();
       defined = false;
       return 0;
     }
     Point p1;
     Point p2;
     Line* line = new Line(numPoints);
     int rpoints = 0;
     int lastPoint = 0;
     line->StartBulkLoad();
     int edgeno = -1;
     HalfSegment hs;
     for(unsigned int i=0;i<parts.size() && file.good(); i++){
       lastPoint = i==parts.size()-1?numPoints:parts[i+1];
       int start = rpoints;
       for(int j=start; j<lastPoint && file.good() ;j++){
          double x = readLittleDouble();
          double y = readLittleDouble();
          p2.Set(x,y);
          if(j>start){
            if(!AlmostEqual(p1,p2)){
                hs.Set( true, p1, p2 );
                hs.attr.edgeno = ++edgeno;
                (*line) += hs;
                hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
                (*line) += hs;
            }
          }
          p1 = p2;
          rpoints++;
       }
     }
     line->EndBulkLoad(true,true,true);//sort, realminize, robust
     if(!file.good()){
       cerr << "SHPIMPORT: Error in reading file " << filename << endl;
       delete line;
       file.close();
       defined = false;
       return 0;
     }
     return line;
   }


/*
~getNextPolygon~

This function read the next region value from the file and returns it.
In case of an error or if no more regions are available, the return value
will be 0.

*/


   Attribute* getNextPolygon(){
     if(file.tellg()==fileend){ // end of file reached
       return 0;
     }

     readBigInt32(); // ignore record number
     uint32_t len = readBigInt32();
     uint32_t type = readLittleInt32();
     if(type==0){
       if(len!=2){
          cerr << "Error in file detected" << __LINE__  <<  endl;
          file.close();
          defined = false;
          return 0;
       } else { // NULL shape, return an empty region
         cout << "found null shape" << endl;
         return new Region(0);
       }
     }
     if(type!=5){ // different shapes are not allowed
       cerr << "Error in file detected" << __LINE__ << endl;
       cerr << "Expected Type = 5, but got type: " << type << endl;
       file.close();
       defined = false;
       return 0;
     }
     // ignore box
     readLittleDouble();
     readLittleDouble();
     readLittleDouble();
     readLittleDouble();
     uint32_t numParts  = readLittleInt32();
     uint32_t numPoints = readLittleInt32();
     // for debugging the file
     uint32_t clen = (44 + 4*numParts + 16*numPoints)/2;

     //cout << "region has " << numParts << " parts " << endl;

     if(clen!=len){
        cerr << "File invalid: length given in header seems to be wrong"
             << endl;
        file.close();
        defined = false;
        return 0;
     }
     // read the starts of the cycles
     vector<uint32_t> parts;
     for(unsigned int i=0;i<numParts;i++){
       uint32_t p = readLittleInt32();
       parts.push_back(p);
     }


     // read the cycles
     vector<vector <Point> > cycles;
     uint32_t pos = 0;
     for(unsigned int p=0;p<parts.size(); p++){
        vector<Point> cycle;
        uint32_t start = pos;
        uint32_t end = p< (parts.size()-1) ? parts[p+1]:numPoints;
        Point lastPoint(true,0.0,0.0);
        // read a single cycle
        for(unsigned int c=start; c< end; c++){
            double x = readLittleDouble();
            double y = readLittleDouble();
            Point point(true,x,y);
            if(c==start){ // the first point
               cycle.push_back(point);
               lastPoint = point;
            } else if(!AlmostEqual(lastPoint,point)){
               cycle.push_back(point);
               lastPoint = point;
            }
            pos++;
        }
        //cout << "part " << p << " has " << cycle.size() << " points " << endl;
        cycles.push_back(cycle);
     }
     try{
         return buildRegion2(cycles);
     } catch(const string& error){
        cout << "error occured while creating a polygon" << endl;
        cout << "Region should be created from " << cycles.size() << " cycles"
             << endl;
        Region* res = new Region(0);
        res->SetDefined(false);
        return res;
     }
   }

public:



/*
~numOfNeighbours~

Returns the number of halfsegments having the same dominating point
like the halfsegment at positition pos (exclusive that segment).
The DbArray has to be sorted.

*/
int numOfNeighbours1(const DbArray<HalfSegment>& segs,const int pos){
   HalfSegment hs1;
   HalfSegment hs2;
   segs.Get(pos,&hs1);
   Point dp(hs1.GetDomPoint());
   int num = 0;
   bool done= false;
   int pos2 = pos-1;
   while(pos2>0 && !done){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       num++;
       pos2--;
     }else {
       done = true;
     }
   }
   done = false;
   pos2 = pos+1;
   while(!done && pos2<segs.Size()){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       num++;
       pos2++;
     }else {
       done = true;
     }
   }
   return num;
}
// slower implementation
int numOfNeighbours(const DbArray<HalfSegment>& segs,const int pos){
   HalfSegment hs1;
   HalfSegment hs2;
   segs.Get(pos,hs1);
   Point dp = hs1.GetDomPoint();
   double dpx = dp.GetX();
   int num = 0;
   bool done= false;
   int pos2 = pos-1;
   while(pos2>=0 && !done){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       num++;
       pos2--;
     }else {
       double dpx2 = hs2.GetDomPoint().GetX();
       if(AlmostEqual(dpx,dpx2)){
         pos2--;
       }else{
         done = true;
       }
     }
   }
   done = false;
   pos2 = pos+1;
   while(!done && pos2<segs.Size()){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       num++;
       pos2++;
     }else {
       double dpx2 = hs2.GetDomPoint().GetX();
       if(AlmostEqual(dpx,dpx2)){
         pos2++;
       } else {
         done = true;
       }
     }
   }
   return num;
}

/*
~numOfUnusedNeighbours~

Returns the number of halfsegments having the same dominating point
like the halfsegment at positition pos (exclusive that segment).
The DBArray has to be sorted.

*/
int numOfUnusedNeighbours(const DbArray<HalfSegment>& segs,
                          const int pos,
                          const bool* used){
   HalfSegment hs1;
   HalfSegment hs2;
   segs.Get(pos,hs1);
   Point dp = hs1.GetDomPoint();
   double dpx = dp.GetX();
   int num = 0;
   bool done= false;
   int pos2 = pos-1;
   while(pos2>0 && !done){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       if(!used[pos2]){
         num++;
       }
       pos2--;
     }else {
       double dpx2 = hs2.GetDomPoint().GetX();
       if(AlmostEqual(dpx,dpx2)){
         pos2--;
       } else {
         done = true;
       }
     }
   }
   done = false;
   pos2 = pos+1;
   while(!done && pos2<segs.Size()){
     segs.Get(pos2,hs2);
     if(AlmostEqual(dp,hs2.GetDomPoint())){
       if(!used[pos2]){
         num++;
       }
       pos2++;
     }else {
       double dpx2 = hs2.GetDomPoint().GetX();
       if(AlmostEqual(dpx,dpx2)){
          pos2++;
       } else {
         done = true;
       }
     }
   }
   return num;
}


void removeDeadEnd(DbArray<HalfSegment>* segments,
                   vector<Point>& path, vector<int>& positions,
                   const bool* used){

   int pos = positions.size()-1;
   while((pos>=0) &&
         (numOfUnusedNeighbours(*segments,positions[pos],used) < 1)){
       pos--;
   }
   if(pos<=0){ // no extension found
      positions.clear();
      path.clear();
   } else {
     cout << "Remove path from " << pos << " until its end" << endl;
     positions.erase(positions.begin()+pos, positions.end());
     path.erase(path.begin()+pos,path.end());
   }

}





};




template<int filePos>
int shpimportVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

   switch(message){
     case OPEN: {
       if(local.addr){
          delete (shpimportInfo*)local.addr;
       }
       FText* fname = static_cast<FText*>(args[filePos].addr);
       ListExpr type = nl->Second(qp->GetType(s));
       local.setAddr(new shpimportInfo(type,fname));
       return 0;
     }
     case REQUEST: {
       if(!local.addr){
          return CANCEL;
       }
       shpimportInfo* info = static_cast<shpimportInfo*>(local.addr);
       if(!info){
         return CANCEL;
       }
       Attribute* next = info->getNext();
       result.addr = next;
       return next?YIELD:CANCEL;
     }
     case CLOSE: {
       shpimportInfo* info = static_cast<shpimportInfo*>(local.addr);
       if(info){
         info->close();
         delete info;
         local.addr = 0;
       }
       return 0;
     }
     default: {
       return 0;
     }
   }
}

/*
5.3 Specification

*/
const string shpimportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>T x text -> stream(T), T in {point, points, line, region}"
    "</text--->"
    "<text> _ shpimport [ _ ] </text--->"
    "<text>Produces a stream of spatial objects of type T from a shapefile. "
    "The first argument is a dummy parameter. It is required to determine the "
    "type T of the result stream.</text--->"
    "<text> not tested !!!</text--->"
    ") )";

/*
5.6 Operator instance

*/
Operator shpimport( "shpimport",
                    shpimportSpec,
                    shpimportVM<1>,
                    Operator::SimpleSelect,
                    shpimportTM);


/*
6 Operator shpimport2

imports a shape file without given the type directly.

*/
ListExpr shpimport2TM(ListExpr args){
   if(nl->ListLength(args)!=1){
      return listutils::typeError("one argument expected");
   }
   ListExpr arg = nl->First(args);
   if(nl->ListLength(arg) !=2){
      return listutils::typeError("Error, argument has to consists of 2 parts");
   }
   ListExpr type = nl->First(arg);
   ListExpr value = nl->Second(arg);

   if(!listutils::isSymbol(type,FText::BasicType())){
       return listutils::typeError("text expected");
   }

   // get the value if possible

   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(value),res);
   if(!success){
     return listutils::typeError("could not evaluate the value of  " +
                                  nl->ToString(value) );
   }

   FText* resText = static_cast<FText*>(res.addr);

   if(!resText->IsDefined()){
      resText->DeleteIfAllowed();
       return listutils::typeError("filename evaluated to be undefined");
   }

   string name = resText->GetValue();
   resText->DeleteIfAllowed();

   string shpType;
   bool correct;
   string errmsg;

   shpType = getShpType(name, correct, errmsg);
   if(!correct){
      return listutils::typeError(errmsg);
   }
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(shpType));
}



const string shpimport2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text -> stream(T), T in {point, points, line, region}</text--->"
    "<text>shpimport2(_) </text--->"
    "<text>Produces a stream of spatial objects from a shapefile. The spatial "
    "result stream element type T is determined automatically by inspecting "
    "the import file."
    "</text--->"
    "<text> query shpimport2('kinos.shp') count</text--->"
    ") )";


/*
5.6 Operator instance

*/
Operator shpimport2( "shpimport2",
                    shpimport2Spec,
                    shpimportVM<0>,
                    Operator::SimpleSelect,
                    shpimport2TM);



/*
6 Operator dbtype

6.1 Type Mapping

text -> text

*/

ListExpr dbtypeTM(ListExpr args){
  if(nl->ListLength(args)==1 &&
     nl->IsEqual(nl->First(args),FText::BasicType())){
     return nl->SymbolAtom(FText::BasicType());
  }
  ErrorReporter::ReportError("text expected");
  return nl->TypeError();
}

/*
6.2 Value Mapping

*/

ListExpr getDBAttrList(string name, bool& correct, string& errorMessage){

  ifstream f;
  correct = true;
  f.open(name.c_str(),ios::binary);
  if(!f.good()){
     errorMessage="could not open file";
     correct = false;
     return listutils::typeError();
  }

  unsigned char code= 0;
  f.read(reinterpret_cast<char*>(&code),1);
  if(code!=0x03 && code!=0x83){
     f.close();
     errorMessage = "Not a DBase III file";
     correct = false;
     return listutils::typeError();
  }
  if(!f.good()){
    errorMessage = "problem in reading file";
    correct = false;
    return listutils::typeError();
  }
  f.seekg(8,ios::beg);

  if(!f.good()){
    errorMessage = "problem in reading file";
    correct = false;
    return listutils::typeError();
  }

  uint16_t headerlength;
  f.read(reinterpret_cast<char*>(&headerlength),2);

  if(!WinUnix::isLittleEndian()){
     headerlength = WinUnix::convertEndian(headerlength);
  }
  int check = (headerlength-1) % 32;
  if(check!=0){
    errorMessage = "wrong header length";
    correct = false;
     f.close();
    return listutils::typeError();
  }
  if(!f.good()){
    errorMessage = "problem in reading file";
    f.close();
    correct = false;
    return listutils::typeError();
  }
  int noRecords = (headerlength-32) / 32;


  f.seekg(0,ios::end);
  if(f.tellg() < headerlength){
    errorMessage = "invalid filesize";
    correct = false;
    f.close();
    return listutils::typeError();
  }

  f.seekg(32,ios::beg);
  char buffer[32];

  ListExpr attrList=nl->TheEmptyList();
  ListExpr last = nl->TheEmptyList();
  for(int i=0;i<noRecords;i++){
     f.read(buffer,32);
     stringstream ns;
     for(int j=0;j<11;j++){
        if(buffer[j]){
          ns << buffer[j];
        }
     }
     string name = ns.str();
     name[0] = toupper((unsigned char) name[0]);
     unsigned char typeCode = buffer[11];
     unsigned char dc = buffer[17];
     unsigned char len = buffer[16];
     string type = "unknown";

     switch(typeCode){
      case 'C' : {
         type = len <= MAX_STRINGSIZE?CcString::BasicType():FText::BasicType();
         break;
      }
      case 'D' : {
         type = datetime::DateTime::BasicType();
         break;
      }
      case 'L' : {
         type = CcBool::BasicType();
         break;
      }
      case 'M' : {
         type = FText::BasicType();
         break;
      }
      case 'N' : {
         type = dc==0?CcInt::BasicType():CcReal::BasicType();
         break;
      }
      case 'F' : {
         type = CcReal::BasicType();
         break;
      }
      default : {
         errorMessage = "unknown type ";
         f.close();
         correct = false;
         return listutils::typeError();
      }
     }

     ListExpr attr = nl->TwoElemList(nl->SymbolAtom(name),
                                     nl->SymbolAtom(type));
     if(i==0){ // first elem
       attrList = nl->OneElemList(attr);
       last = attrList;
     } else {
        last = nl->Append(last,attr);
    }
  }
  f.close();
  return attrList;
}



int dbtypeVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  FText* arg = static_cast<FText*>(args[0].addr);
  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }


  string name = arg->GetValue();

  bool correct;
  string errorMessage = "";
  ListExpr attrList = getDBAttrList(name, correct, errorMessage);



  if(!correct){
     res->SetDefined(false);
     cout << errorMessage << endl;
     return 0;
  }

  stringstream resstr;
  resstr << "[const rel(tuple([";
  bool first = true;
  while(!nl->IsEmpty(attrList)){
     ListExpr a = nl->First(attrList);
     if(!first){
        resstr << ", ";
     }
     resstr << nl->SymbolValue(nl->First(a));
     resstr << " : ";
     resstr << nl->SymbolValue(nl->Second(a));
     attrList = nl->Rest(attrList);
     first = false;
  }
  resstr << "])) value ()]";

  resstr.flush();
  res->Set(true, resstr.str());
  return 0;
}

/*
6.3 Specification

*/

const string dbtypeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>text -> text</text--->"
    "<text>dbtype( FileName ) </text--->"
    "<text>Returns an object description of the secondo object"
    " stored within the dbase III file specified by FileName as a text value."
    "</text--->"
    "<text> not tested !!!</text--->"
    ") )";

/*
6.4 Operator instance

*/

Operator dbtype( "dbtype",
                   dbtypeSpec,
                   dbtypeVM,
                   Operator::SimpleSelect,
                   dbtypeTM);


/*
7 Operator dbimport

7.1 Type Mapping

(rel(tuple(...))) x text -> stream(tuple(...)

*/
ListExpr dbimportTM(ListExpr args){
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("rel x text expected");
    return nl->TypeError();
  }
  if(!IsRelDescription(nl->First(args)) ||
     !nl->IsEqual(nl->Second(args),FText::BasicType())){
    ErrorReporter::ReportError("rel x text expected");
    return nl->TypeError();
  }
  ListExpr res =  nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->Second(nl->First(args)));
  return res;
}


/*
7.2 Specification

*/

const string dbimportSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>rel(tuple(...) x text -> stream(tuple(...))</text--->"
    "<text>Rel dbimport[ FileName ] </text--->"
    "<text>Returns the content of the file as a tuple stream. The result tuple "
    "type must the passed as a relation Rel of the same tupletype as the "
    "result stream.</text--->"
    "<text> not tested !!!</text--->"
    ") )";


/*
7.3 Value Mapping

*/
class DbimportInfo{

public:

/*
~Constructor~

*/
   DbimportInfo(ListExpr type, FText* fname){
     BasicTuple = 0;
     tupleType = 0;
     buffer1 = new char[FILE_BUFFER_SIZE];
     buffer2 = new char[FILE_BUFFER_SIZE];

     ListExpr attrList = nl->Second(nl->Second(type));
     while(!nl->IsEmpty(attrList)){
        ListExpr type = nl->Second(nl->First(attrList));
        if(nl->AtomType(type)!=SymbolType){
           cerr << " composite type not allowed " << endl;
           defined = false;
           return;
        }
        types.push_back(nl->SymbolValue(type));
        attrList = nl->Rest(attrList);
     }

     if( !fname->IsDefined()){
        cerr << " undefined filename" << endl;
        defined = false;
        return;
     }
     name = fname->GetValue();
     file.open((name).c_str(),ios::binary);
     file.rdbuf()->pubsetbuf(buffer1,FILE_BUFFER_SIZE);
     if(!file.good()) {
        cerr << "DBIMPORT: error in reading file (open failed)" 
             << name << endl;
        defined = false;
        return;
     }
     if(!checkFile()){
       cerr << "CheckFile failed" << endl;
       defined = false;
       return;
     }
     defined = true;
     ListExpr numType = nl->Second(
                       SecondoSystem::GetCatalog()->NumericType((type)));
     tupleType = new TupleType(numType);
     BasicTuple = new Tuple(numType);
     current = 0;
     file.seekg(headerLength,ios::beg);
     bool found = false;
     for(unsigned int i=0;i<isMemo.size() && !found;i++){
        found = isMemo[i];
     }
     if(found){
        bool endsWith = true;
        if(name.size() < 4){
           endsWith = false;
        } else { 
           size_t s = name.size();
           endsWith = (name[s-4] == '.') &&
                      ((name[s-3]=='d') || (name[s-3]=='D') ) &&
                      ((name[s-2]=='b') || (name[s-2]=='B') ) &&
                      ((name[s-1]=='f') || (name[s-1]=='F') ); 
        }
        string memoname;
        if(endsWith){
           memoname = name;
           size_t p = name.size()-1;
           if(memoname[p] == 'f'){
             memoname[p] = 't';
           } else {
             memoname[p] = 'T';
           }
        } else {
          memoname = name +".dbt"; 
        }
        memofile.open(memoname.c_str(),ios::binary);
        memofile.rdbuf()->pubsetbuf(buffer2,FILE_BUFFER_SIZE);
        if(!memofile.good()){
           cerr << "cannot open dbt file " + memoname << endl;
        }
     }
   }

   Tuple* getNext(){
      if(!defined || current==noRecords  || file.eof() || !file.good()){
         return 0;
      }
      unsigned char buffer[recordSize];
      // read buffer
      file.read(reinterpret_cast<char*>(buffer),recordSize);

      if(!file.good()){ // error in reading file, end of file reached
        defined = false;
        return 0;
      }
      Tuple* ResultTuple = BasicTuple->Clone();

      unsigned int offset = 1; // ignore a deleted flag
      for(int i=0;i<noAttributes;i++){
        if(!store(types[i], ResultTuple, i, buffer,offset, fieldLength[i])){
            delete ResultTuple;
            defined = false;
            return 0;
        }
        int fl = fieldLength[i];
        offset += fl;
      }
      return ResultTuple;
   }

  ~DbimportInfo(){
     if(BasicTuple){
        delete BasicTuple;
        BasicTuple = 0;
     }
     if(tupleType){
       tupleType->DeleteIfAllowed();
       tupleType=0;
     }
     if(buffer1){
        delete[] buffer1;
     }
     if(buffer2){
       delete[] buffer2;
     }
   }  

   void close(){
      file.close();
      memofile.close();
   }

private:
  bool defined;
  string name;
  ifstream file;
  ifstream memofile;
  uint32_t noRecords;
  uint16_t headerLength;
  uint16_t recordSize;
  uint16_t noAttributes;
  vector<string> types;
  vector<bool> isMemo;
  vector<unsigned char> fieldLength;
  Tuple* BasicTuple;
  TupleType* tupleType;
  unsigned int current;
  char* buffer1;
  char* buffer2;

  bool checkFile(){
    file.seekg(0,ios::beg);
    unsigned char code;
    file.read(reinterpret_cast<char*>(&code),1);
    file.seekg(4,ios::beg);
    if( !file.good() || ( (code!=3) && (code!=0x83) ) ){
       cerr << "invalid code" << endl;
       file.close();
       return false;
    }
    file.read(reinterpret_cast<char*>(&noRecords),4);
    file.read(reinterpret_cast<char*>(&headerLength),2);
    file.read(reinterpret_cast<char*>(&recordSize),2);
    if(!WinUnix::isLittleEndian()){
       noRecords = WinUnix::convertEndian(noRecords);
       headerLength = WinUnix::convertEndian(headerLength);
       recordSize = WinUnix::convertEndian(recordSize);
    }
    file.seekg(32,ios::beg);
    if(!file.good()){
       cerr << "Error in reading file" << endl;
       file.close();
       return false;
    }
    if( (headerLength-1) % 32  != 0){
      cerr << " invalid length for the header" << headerLength << endl;
      file.close();
      return false;
    }
    noAttributes = (headerLength - 32) / 32;
    if(noAttributes < 1){
      cerr << "numer of attributes invalid" << noAttributes << endl;
      file.close();
      return false;
    }
    if(noAttributes != types.size()){
      cerr << "numbers of types not match "
           << types.size() << " <-> " << noAttributes << endl;
      file.close();
      return false;
    }
    unsigned char buffer[32];
    for(int i=0;  i < noAttributes; i++){
      file.read(reinterpret_cast<char*>(buffer),32);
      unsigned char t = buffer[11];
      unsigned char len = buffer[16];
      unsigned char dc = buffer[17];
      string type;
      switch(t){
        case 'C' : type = len <= MAX_STRINGSIZE?CcString::BasicType()
                                               :FText::BasicType();
                   isMemo.push_back(false);
                   break;
        case 'D' : type = datetime::DateTime::BasicType();
                   isMemo.push_back(false);
                   break;
        case 'L' : type = CcBool::BasicType();
                   isMemo.push_back(false);
                   break;
        case 'M' : type = FText::BasicType();
                   isMemo.push_back(true);
                   if(len!=10){
                      cerr << "Invalid field length for memo detected ,"
                           << " correct to 10" << endl;
                      len = 10;
                   }
                   break;
        case 'N' : type = dc==0?CcInt::BasicType():CcReal::BasicType();
                   isMemo.push_back(false);
                   break;
        case 'F' : type = CcReal::BasicType();
                   isMemo.push_back(false);
                   break;
        default : cerr << "non supported type code:" << t << endl;
                  file.close();
                  return false;
      }
      if(type!=types[i]){
        cerr << "non-matching types " << type << " <-> " << types[i] << endl;
        cerr << "file is " << name << endl;
        file.close();
        return false;
      } else {
         fieldLength.push_back(len);
      }
    }
    return true;
  }

  void trim(string& str) {
    string::size_type pos = str.find_last_not_of(' ');
    if(pos != string::npos) {
      str.erase(pos + 1);
      pos = str.find_first_not_of(' ');
      if(pos != string::npos){
         str.erase(0, pos);
      }
    } else {
     str.erase(str.begin(), str.end());
    }
}

  bool store(string type, Tuple* tuple, int index,
             unsigned char* buf, int offset, int length){
     stringstream ss;
     for(int i=offset; i<offset+length;i++){
       if(buf[i]!=0){
          ss << buf[i];
       }
     }
     string s = ss.str();
     if(type==CcInt::BasicType()){
        if(s.size()==0){
          tuple->PutAttribute(index, new CcInt(false,0));
        } else {
            trim(s);
            istringstream buffer(s);
            int res_int;
            buffer >> res_int;
            tuple->PutAttribute(index, new CcInt(true,res_int));
        }
     } else if(type==CcReal::BasicType()){
        if(s.size()==0){
          tuple->PutAttribute(index, new CcReal(false,0));
        } else {
          trim(s);
          istringstream buffer(s);
          double res_double;
          buffer >> res_double;
          tuple->PutAttribute(index, new CcReal(true,res_double));
        }
     } else if(type==CcString::BasicType()){
        if(s.size()==0){
          tuple->PutAttribute(index, new CcString(false,""));
        } else {
          tuple->PutAttribute(index, new CcString(true,s));
        }
     } else if(type==CcBool::BasicType()){
        trim(s);
        if(s.size()==0 || s=="?"){
           tuple->PutAttribute(index, new CcBool(false,false));
        }
        bool res_bool = s=="y" || s=="Y" || s=="t" || s=="T";
        tuple->PutAttribute(index,new CcBool(true,res_bool));

     } else if(type==FText::BasicType()){
        bool ismemo = isMemo[index];
        if(ismemo){
           if(s.size()==0){
              tuple->PutAttribute(index, new FText(false,""));
           } else {
              trim(s);
              if(s.size() ==0){
                tuple->PutAttribute(index,new FText(true,""));
              } else { // need access to dbt file
                // compute block number
                istringstream buffer(s);
                int bn;
                buffer >> bn;
                memofile.seekg(512*bn,ios::beg);
                if(memofile.good()){
                   char c;
                   stringstream text;
                   memofile.read(&c,1);
                   while(memofile.good() && c!=0x1A){
                      text << c;
                      memofile.read(&c,1);
                   }
                   if(!memofile.good()){
                     cerr << "Error in reading memo file";
                   }
                   tuple->PutAttribute(index,new FText(true,text.str()));
                } else {
                   tuple->PutAttribute(index,new FText(false,""));
                }
              }
           }
        } else {
           if(s.size()==0){
             tuple->PutAttribute(index, new FText(false,""));
           } else {
             tuple->PutAttribute(index, new FText(true,s));
           }
        }
     } else if(type==datetime::DateTime::BasicType()){
        datetime::DateTime* res =
            new datetime::DateTime(datetime::instanttype);
        if(s.size()==0){
           res->SetDefined(false);
        } else {

          istringstream buffer(s);
          int resInt;
          buffer >> resInt;
          int year = resInt/10000;
          if(year<100) {
              year += 2000;
          }
          int month = (resInt/100)%100;
          int day = resInt%100;
          res->Set(year,month,day);
        }
        tuple->PutAttribute(index,res);
     } else {
          assert(false);
     }
     return true;
  }


};


int dbimportVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  switch(message){
    case OPEN: {
      ListExpr type = qp->GetType(s);
      FText* fname = static_cast<FText*>(args[1].addr);
      local.setAddr(new DbimportInfo(type,fname));
      return 0;
    }
    case REQUEST: {
      DbimportInfo* info = static_cast<DbimportInfo*>(local.addr);
      if(!info){
        return CANCEL;
      } else {
        Tuple* tuple = info->getNext();
        result.addr = tuple;
        return tuple==0?CANCEL:YIELD;
      }
    }
    case CLOSE: {
      DbimportInfo* info = static_cast<DbimportInfo*>(local.addr);
      if(info){
        info->close();
        delete info;
        local.addr=0;
      }

    }
    default: {
     return 0;
    }
  }
}

/*
7.4 Operator Instance

*/
Operator dbimport( "dbimport",
                   dbimportSpec,
                   dbimportVM,
                   Operator::SimpleSelect,
                   dbimportTM);




/*
8 Operator db3import2

This variant of the db3import operator combines the dbtype and the
dbimport operator to a single operator. It exploits a new feature of
secondo, namely accessing values of constant expressions within type mappings
not available during the implementation of db3type and db3import.

*/

/*
8.1 Value Mapping


The value mapping of this function is text -> stream(tuple(....))

The tuple type depends on the content of the file specified by the text.
If the file does not reprensent a valid db3 file, the result will be
a typeerror.

*/
ListExpr dbimport2TM(ListExpr args){
   if(nl->ListLength(args)!=1){
      return listutils::typeError("one argument expected");
   }

   ListExpr arg = nl->First(args);
   // the format must be (text value)
   // where values represents the value of the text as
   // query expression
   if(nl->ListLength(arg)!=2){
      return listutils::typeError("internal error");
   }

   if(!listutils::isSymbol(nl->First(arg), FText::BasicType())){
       return listutils::typeError("the argument must be of type text");
   }

   ListExpr value = nl->Second(arg);

   Word res;
   bool success = QueryProcessor::ExecuteQuery(nl->ToString(value),res);
   if(!success){
     return listutils::typeError("could not evaluate the value of text");
   }

   FText* resText = static_cast<FText*>(res.addr);

   if(!resText->IsDefined()){
      resText->DeleteIfAllowed();
      return listutils::typeError("filename evaluated to be undefined");
   }


   string name = resText->GetValue();
   resText->DeleteIfAllowed();
   resText = 0;
   bool correct;
   string errMsg="";
   ListExpr attrList = getDBAttrList(name,correct, errMsg);
   if(!correct){
       return listutils::typeError(errMsg);
   }
   return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                   nl->TwoElemList(
                       nl->SymbolAtom(Tuple::BasicType()),
                       attrList));


}


const string dbimport2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> text -> stream (tuple(...))</text--->"
    "<text>dbimport2( FileName) </text--->"
    "<text>Returns the content of file FineName as a stream. The stream tuple "
    "type is determined automatically by inspecting the file.</text--->"
    "<text> </text--->"
    ") )";



int dbimport2VM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  switch(message){
    case OPEN: {
      ListExpr type = qp->GetType(s);
      FText* fname = static_cast<FText*>(args[0].addr);
      local.setAddr(new DbimportInfo(type,fname));
      return 0;
    }
    case REQUEST: {
      DbimportInfo* info = static_cast<DbimportInfo*>(local.addr);
      if(!info){
        return CANCEL;
      } else {
        Tuple* tuple = info->getNext();
        result.addr = tuple;
        return tuple==0?CANCEL:YIELD;
      }
    }
    case CLOSE: {
      DbimportInfo* info = static_cast<DbimportInfo*>(local.addr);
      if(info){
        info->close();
        delete info;
        local.addr=0;
      }

    }
    default: {
     return 0;
    }
  }
}



Operator dbimport2( "dbimport2",
                   dbimport2Spec,
                   dbimport2VM,
                   Operator::SimpleSelect,
                   dbimport2TM);


/*
8 Operator saveObject

8.1 TypeMapping

The Type mapping is:

---- T x string x text -> bool
----


*/

ListExpr saveObjectTM(ListExpr args){
  string err = "T x string x text  expected";
  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!nl->IsEqual(nl->Second(args),CcString::BasicType()) |
     !nl->IsEqual(nl->Third(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  ListExpr obj = nl->First(args);

  if(!nl->IsAtom(obj) &&
     !nl->IsEmpty(obj) &&
     nl->IsEqual(nl->First(obj),Symbol::STREAM())){
    ErrorReporter::ReportError("stream not allowes as first argument");
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcBool::BasicType());
}

/*
8.2 Specification

*/
const string saveObjectSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> T x string x text -> bool </text--->"
    "<text> _  saveObject[objName, fileName] </text--->"
    "<text> saves an object to a file in nested list format</text--->"
    "<text> query (3 + 4) saveObject[\"seven\",'seven.obj']  </text--->"
    ") )";

/*
8.3 Value Mapping

*/

int saveObjectVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  CcString* objName = static_cast<CcString*>(args[1].addr);
  FText* fileName = static_cast<FText*>(args[2].addr);
  if(!objName->IsDefined() || !fileName->IsDefined()){
    res->Set(true,false);
    return 0;
  }
  ListExpr type = qp->GetType(qp->GetSon(s,0));
  int algId;
  int typeId;
  string tname;
  if(! SecondoSystem::GetCatalog()->LookUpTypeExpr(type,tname,algId,typeId)){
    res->Set(true,false);
    return 0;
  }

  ListExpr value = (am->OutObj(algId,typeId))(type,args[0]);

  string oname = objName->GetValue();

  ListExpr objList = nl->FiveElemList(
                        nl->SymbolAtom("OBJECT"),
                        nl->SymbolAtom(oname),
                        nl->TheEmptyList(),
                        type,
                        value);
  bool success = nl->WriteToFile(fileName->GetValue(),objList);
  res->Set(true,success);
  return 0;
}

/*
8.4 Operator Instance

*/
Operator saveObject( "saveObject",
                   saveObjectSpec,
                   saveObjectVM,
                   Operator::SimpleSelect,
                   saveObjectTM);


/*
9 Operator ~isFile~

This operator checks, whether a given file exists in the filesystem.
If so, it returns ~TRUE~, otherwise ~FALSE~

9.1 Type Mapping Function ~stringORtext2boolTM~

Used for operators ~isFile~, ~removeFile~, ~isDirectory~

---- {text|string} -> bool
----

*/
ListExpr stringORtext2boolTM(ListExpr args){
  string err = "text or string expected";
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(    !nl->IsEqual(nl->First(args),CcString::BasicType())
      && !nl->IsEqual(nl->First(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  return nl->SymbolAtom(CcBool::BasicType());
}

/*
9.2 Value Mapping Function for ~checkfile~

We implement this as a template function. The template class parameter ~T~
may be either CcString or FText.

*/

template<class T>
int isFileVM(Word* args, Word& result,
                int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  if(!objName->IsDefined()){
    res->Set(false,false);
  } else {
    string fileNameS = objName->GetValue();
    res->Set(true,FileSystem::FileOrFolderExists(fileNameS));
  }
  return 0;
}

ValueMapping isFilevaluemap[] = {isFileVM<CcString>, isFileVM<FText>};


/*
9.3 Specification for ~isFile~

*/
const string isFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} -> bool </text--->"
    "<text> isFile( Name ) </text--->"
    "<text> Checks, whether the file or directory Name exists.\n"
    "If Name is UNDEFINED, nothing is done and the result is "
    "UNDEFINED.</text--->"
    "<text> query isFile('data.csv')  </text--->"
    ") )";

/*
9.4 Selection Function for ~isFile~

*/

int stringORtextSelect( ListExpr args )
{
  if(nl->IsEqual(nl->First(args),CcString::BasicType())) {
    return 0;
  } else if(nl->IsEqual(nl->First(args),FText::BasicType())){
    return 1;
  }
  assert( false );
  return -1;
}

/*
9.5 Operator Instance for operator ~isFile~

*/
Operator isFile ( "isFile",
                   isFileSpec,
                   2,
                   isFilevaluemap,
                   stringORtextSelect,
                   stringORtext2boolTM);


/*
10 Operator ~removeFile~

This operator removes/deletes a given file from the file system

10.1 Type Mapping for ~removeFile~

Uses ~stringORtext2boolTM~.

10.2 Value Mapping for ~removeFile~

*/

template<class T>
int removeFileVM(Word* args, Word& result,
                 int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  if(!objName->IsDefined()){
    res->Set(false,false);
  } else {
    string fileNameS = objName->GetValue();
    res->Set(true,FileSystem::DeleteFileOrFolder(fileNameS));
  }
  return 0;
}



ValueMapping removeFilevaluemap[] = {removeFileVM<CcString>,
                                     removeFileVM<FText>};

/*
10.4 Selection Function for ~removeFile~

Uses ~stringORtextSelect~.

*/

/*
10.4 Specification  for ~removeFile~

*/
const string removeFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} -> bool </text--->"
    "<text> removeFile( Name ) </text--->"
    "<text> Deletes the file or empty folder with the given Name from"
    " the files system. Returns TRUE, if this succeeds, and FALSE if the "
    "file could not be deleted or any error occurs. If the Name is\n"
    "UNDEFINED, nothing is done and the result is UNDEFINED.\n"
    "WARNING: Be extremely careful about removing files!</text--->"
    "<text> query removeFile('data.csv')  </text--->"
    ") )";


/*
10.5 Operator Instance for operator ~removeFile~

*/
Operator removeFile ( "removeFile",
                   removeFileSpec,
                   2,
                   removeFilevaluemap,
                   stringORtextSelect,
                   stringORtext2boolTM);




/*

10.2  Operator ~rtf2text~

This operator converts a given rtf file into a text file and 
save it in txt format 

10.2.1 Type Mapping for ~rtf2txtfile~

Uses ~stringORtext2boolTM~

10.2.2 Value Mapping for ~rtf2txtfile~

*/
#ifndef SECONDO_WIN32
 
template<class T>
int rtf2txtfileVM(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  
  if(!objName->IsDefined())
   {
    res->Set(false,false);
   } 
   
   else 
    {       
      
     string fileNameS = objName->GetValue();    
     
     string::size_type size;
     size = fileNameS.size();   
     string str3 = fileNameS.substr (size-4);     
     
     if (!(str3 == ".rtf")) 
     {
       res->Set(false,false);
       return 0;
     }
     
     
     
     string sourcefile = fileNameS;
     size_t f = fileNameS.find(".rtf");
     string filetarget = sourcefile.replace(f, std::string(".rtf").length(),
                                            ".txt");
    
     string call = "unrtf --text " + fileNameS + " > " + filetarget;        
     const char * cll = call.c_str();
     string deltxt = "rm " + filetarget;
     const char * del = deltxt.c_str();
     
     
     int back = system(cll);    
    
         
    
     if (back == 0) 
     {
       res->Set(true,true);
       
       return 0;
     }
    
      else 
       {
         res->Set(true,false);
         system(del);
         return 0;
       } 
    
     
  }
  return 0;
} 
  
  
  

ValueMapping rtf2txtfilevaluemap[] = {rtf2txtfileVM<CcString>,
                                  rtf2txtfileVM<FText>};

/*
10.2.3 Selection Function for ~rtf2txtfile~

Uses ~stringORtextSelect~

*/



/*
10.2.4 Specification  for ~rtf2txtfile~

*/



const string rtf2txtfileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} -> bool </text--->"
    "<text> rtf2text ( Name ) </text--->"
    "<text>Converts a given rtf file and creats a txt file with the same name" 
    " using unrtf linux tool. Returns TRUE, if this succeeds and FALSE if the "
    "file could not be converted or opened. And UNDEFINED if any "
    "error occurs.</text--->"
    "<text> query rtf2txtfile('text.rtf')  </text--->"
    ") )";


 
 
 


/*
10.2.5 Operator Instance for operator ~rtf2txtfile~

*/


Operator rtf2txtfile ( "rtf2txtfile",
                   rtf2txtfileSpec,
                   2,
                   rtf2txtfilevaluemap,
                   stringORtextSelect,
                   stringORtext2boolTM);








#endif






/*
11 Operator ~createDirectory~

The operator creates the passed directory within the file system.

11.1 Type Mapping for ~createDirectory~

*/

ListExpr createDirectoryTM(ListExpr args){
   string err = "{string,text} [ x bool] expected";
   if(!nl->HasLength(args,2) && !nl->HasLength(args,1)){
     return listutils::typeError("wrong number of arguments");
   }
   ListExpr arg1 = nl->First(args);
   if(!FText::checkType(arg1) && !CcString::checkType(arg1)){
     return listutils::typeError(err);
   }
   if(nl->HasLength(args,2)){
     if(!CcBool::checkType(nl->Second(args))){
        return listutils::typeError(err);
     } 
     return listutils::basicSymbol<CcBool>();
   }
   return nl->ThreeElemList(
              nl->SymbolAtom(Symbols::APPEND()),
              nl->OneElemList( nl->BoolAtom(false)),
              listutils::basicSymbol<CcBool>());
}


/*
11.2 Value Mapping for ~createDirectory~

*/



template<class T>
int createDirectoryVM(Word* args, Word& result,
                 int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  CcBool* parents = (CcBool*) args[1].addr;
  if(!objName->IsDefined()|| !parents->IsDefined()) {
    res->Set(false,false);
  } else {
    string fileNameS = objName->GetValue();
    bool p = parents->GetValue();
    if(p){
       res->Set(true,FileSystem::CreateFolderEx(fileNameS));
    } else {
       res->Set(true,FileSystem::CreateFolder(fileNameS));
    }
  }
  return 0;
}

ValueMapping createDirectoryvaluemap[] = {createDirectoryVM<CcString>,
                                          createDirectoryVM<FText>};

/*
11.3 Specification  for ~createDirectory~

*/
const string createDirectorySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} [x bool]-> bool </text--->"
    "<text> createDirectory( Name ) </text--->"
    "<text> Creates a directory with the given Name on"
    " the files system. Returns TRUE, if this succeeds, and FALSE if the "
    "directory could not be created or any error occurs. If the Name is\n"
    "UNDEFINED, nothing is done and the result is UNDEFINED. If the oprional"
    "boolean argument is specified, non-existent parent directories are"
    " craeted automatocally.</text--->"
    "<text> query createDirectory('my_csv_directory')  </text--->"
    ") )";

/*
11.4 Selection Function for ~createDirectory~

Uses ~stringORtextSelect~

11.5
Operator Instance for operator ~createDirectory~

*/
Operator createDirectory ( "createDirectory",
                   createDirectorySpec,
                   2,
                   createDirectoryvaluemap,
                   stringORtextSelect,
                   createDirectoryTM);


/*
12 Operator ~fileSize~

Returns the size of a file or directory on the file system

12.1 Type Mapping for ~fileSize~

---- {text|string} [ x bool ] -> int
----

*/
ListExpr stringORtextOPTIONbool2intTM(ListExpr args){
  string err = "{text|string} [ x bool ] expected";
  int listLength = nl->ListLength(args);
  if(listLength!=1 && listLength !=2){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(    !nl->IsEqual(nl->First(args),CcString::BasicType())
      && !nl->IsEqual(nl->First(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if (listLength == 2 && !nl->IsEqual(nl->Second(args),CcBool::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcInt::BasicType());
}

/*
12.2 Value Mapping for ~fileSize~

*/

template<class T>
int fileSizeVM(Word* args, Word& result,
                 int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  if(!objName->IsDefined()){
    res->Set(false,0);
    return 0;
  }
  bool recursive = false;
  if(qp->GetNoSons(s)==2){
     CcBool* rec = static_cast<CcBool*>(args[1].addr);
    if(!rec->IsDefined()){
      res->Set(false,0);
    } else {
      recursive = rec->GetBoolval();
    }
  }
  if(recursive){
    cerr<<"Recursive file size calculation still not implemented!\n"<<endl;
  }
  string fileNameS = objName->GetValue();
  int32_t size = FileSystem::GetFileSize(fileNameS);
  if(size<0){
    res->Set(false,0);
    return 0;
  }
  res->Set(true,size);
  return 0;
}

ValueMapping fileSizevaluemap[] = {fileSizeVM<CcString>,
                                   fileSizeVM<FText>};

/*
12.3 Selection Function for ~fileSize~

Uses ~stringORtextSelect~.

12.3 Specification for ~fileSize~

*/
const string fileSizeSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} [ x bool ] -> bool </text--->"
    "<text> fileSize( Name, Recurse ) </text--->"
    "<text> Returns the file's or directory's size in Bytes.\n"
    "If Name specifies a directory and the optional parameter Recurse is TRUE, "
    "the operator recurses into subdirectories aggregating the total size.\n"
    "If Name or Recurse is UNDEFINED, nothing is done and the result is "
    "UNDEFINED.</text--->"
    "<text> query fileSize('data.csv')  </text--->"
    ") )";

/*
12.5 Operator Instance for operator ~fileSize~

*/
Operator fileSize ( "fileSize",
                   fileSizeSpec,
                   2,
                   fileSizevaluemap,
                   stringORtextSelect,
                   stringORtextOPTIONbool2intTM);

/*
13 Operator ~isDirectory~

This operator checks, whether a given file is a directory in the filesystem.
If so, it returns ~TRUE~, otherwise ~FALSE~

13.1 Type Mapping Function for ~isDirectory~

Uses ~stringORtext2boolTM~.

13.2 Value Mapping Function for ~isDirectory~

We implement this as a template function. The template class parameter ~T~
may be either CcString or FText.

*/

template<class T>
int isDirectoryVM(Word* args, Word& result,
                int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* objName = static_cast<T*>(args[0].addr);
  if(!objName->IsDefined()){
    res->Set(false,false);
  } else {
    string fileNameS = objName->GetValue();
    res->Set(true,FileSystem::IsDirectory(fileNameS));
  }
  return 0;
}

ValueMapping isDirectoryvaluemap[] = {isDirectoryVM<CcString>,
                                      isDirectoryVM<FText>};


/*
13.3 Specification for ~isDirectory~

*/
const string isDirectorySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} -> bool </text--->"
    "<text> isDirectory( Name ) </text--->"
    "<text> Checks, whether Name is a directory.\n"
    "If Name is UNDEFINED, nothing is done and the result is "
    "UNDEFINED.</text--->"
    "<text> query isDirectory('data.csv')  </text--->"
    ") )";

/*
13.4 Selection Function for ~isDirectory~

Uses ~stringORtextSelect~.

13.5 Operator Instance for operator ~isDirectory~

*/
Operator isDirectory ( "isDirectory",
                   isDirectorySpec,
                   2,
                   isDirectoryvaluemap,
                   stringORtextSelect,
                   stringORtext2boolTM);


/*
14 Operator ~writeFile~

The operator writes a ~text~ to a (text-) file on the file system.

14.1 Type Mapping for ~writeFile~

---- text x {text|string} [ x bool ] -> bool
----

*/
ListExpr stringORtext_stringORtext_OPTIONALbool2boolTM(ListExpr args){
  string err = "{text|string} x {text|string} [ x bool ] expected";
  int listLength = nl->ListLength(args);
  if(listLength!=2 && listLength !=3){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(    !nl->IsEqual(nl->First(args),CcString::BasicType())
      && !nl->IsEqual(nl->First(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(    !nl->IsEqual(nl->Second(args),CcString::BasicType())
      && !nl->IsEqual(nl->Second(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }


  if (listLength == 3 && !nl->IsEqual(nl->Third(args),CcBool::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->SymbolAtom(CcBool::BasicType());
}


/*
14.2 Value Mapping for ~writeFile~

*/
template<class T, class S>
int writeFileVM(Word* args, Word& result,
                int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* content = static_cast<T*>(args[0].addr);
  S* fileName = static_cast<S*>(args[1].addr);

  if(!content->IsDefined() || !fileName->IsDefined()){
    res->Set(false,false);
    return 0;
  }
  string fileNameS = fileName->GetValue();
  if(fileNameS == ""){
    res->Set(true,false);
    return 0;
  }
  bool append = false;
  if(qp->GetNoSons(s)==3){
    CcBool* app = static_cast<CcBool*>(args[2].addr);
    if(!app->IsDefined()){
      res->Set(false,false);
      return 0;
    } else {
      append = app->GetBoolval();
    }
  }
  bool fileExists = FileSystem::FileOrFolderExists(fileNameS);
  bool fileIsDirectory = FileSystem::IsDirectory(fileNameS);
  if(fileExists && fileIsDirectory){
    cerr << "writeFile: Cannot write to existing directory!\n"<<endl;
    res->Set(false,false);
    return 0;
  }
  string contentS = content->GetValue();
  ofstream file;
  if(append){
    file.open(fileNameS.c_str(), ios::out | ios::app | ios_base::binary);
  } else {
    file.open(fileNameS.c_str(), ios::out | ios::trunc | ios_base::binary);
  }
  if(!file.good()){
    cerr << "writeFile: Cannot open file '"<< fileName << "'!\n"<<endl;
    res->Set(false,false);
    return 0;
  }
  file << contentS;
  if(file.good()){
    res->Set(true,true);
  } else {
    res->Set(true,false);
  }
  file.close();
  return 0;
}

ValueMapping writeFilevaluemap[] = {writeFileVM<CcString, CcString>,
                                    writeFileVM<CcString, FText   >,
                                    writeFileVM<FText,    CcString>,
                                    writeFileVM<FText,    FText   >};



/*
14.3 Specification for ~writeFile~

*/
const string writeFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} x {text|string} [ x bool ] -> bool </text--->"
    "<text> writeFile( Content, Name, Append ) </text--->"
    "<text> Writes text Content to file Name. If the file does not exist, it\n"
    "is created. If optional argument Append = TRUE, Content will be appended\n"
    "to the file. Otherwise the file gets possibly overwritten.\n"
    "If any argument is UNDEFINED, nothing is done and the result is "
    "UNDEFINED. If writing succeeds, result is TRUE.</text--->"
    "<text> query writeFile('This is content', 'data.csv')  </text--->"
    ") )";

/*
14.4 Selection Function for ~writeFile~

*/
int stringORtext_stringORtext_Select( ListExpr args )
{
  int index = 0;
  // first arg
  if(nl->IsEqual(nl->First(args),CcString::BasicType())) {
    index += 0;
  } else if(nl->IsEqual(nl->First(args),FText::BasicType())){
    index +=  2;
  } else {
    assert(false);
    return -1;
  }
  // second arg
  if(nl->IsEqual(nl->Second(args),CcString::BasicType())) {
    index += 0;
  } else if(nl->IsEqual(nl->Second(args),FText::BasicType())){
    index += 1;
  } else {
    assert(false);
    return -1;
  }
  return index;
}


/*
14.5 Operator Instance for operator ~isDirectory~

*/
Operator writeFile ( "writeFile",
                   writeFileSpec,
                   4,
                   writeFilevaluemap,
                   stringORtext_stringORtext_Select,
                   stringORtext_stringORtext_OPTIONALbool2boolTM);


/*
15 Operator ~readFile~

Reads a file into a text object. If the file contains a NULL, the result is
UNDEFINED.

15.1 TypeMapping for ~readFile~

---- {text|string} -> text
----

*/

ListExpr stringORtext2textTM(ListExpr args){
  string err = "{text|string} expected";
  int listLength = nl->ListLength(args);
  if(listLength!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(    !nl->IsEqual(nl->First(args),CcString::BasicType())
      && !nl->IsEqual(nl->First(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  return nl->SymbolAtom(FText::BasicType());
}

/*
15.2 Value Mapping for ~readFile~

*/

template<class T>
int readFileVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  T* fileName = static_cast<T*>(args[0].addr);
  if(!fileName->IsDefined()){
    res->Set(false,"");
    return 0;
  }
  string fileNameS = fileName->GetValue();
  if(   fileNameS == ""
     || !FileSystem::FileOrFolderExists(fileNameS)
     || FileSystem::IsDirectory(fileNameS)
    ){
    cerr << "readFile: Cannot open file '" << fileNameS << "'!" << endl;
    res->Set(false,"");
    return 0;
  }
  string Content = "";
  ifstream file;
  file.open(fileNameS.c_str(),ios::binary);
  if(!file.good()){
    cerr << "readFile: Error opening file '" << fileNameS << "'!" << endl;
    res->Set(false,"");
    return 0;
  }
  // read file character by character and check for NULL
  char      currChar;
  bool      ok = true;
  file.seekg(0,ios::end);
  streampos fileend = file.tellg();
  file.seekg(0,ios::beg);
  while( ok && (file.tellg() != fileend)){
    file.read(reinterpret_cast<char*>(&currChar),1);
    ok = (currChar != '\0');
    if(ok){ // append char to string
      Content.append(1,currChar);
    } else{
      cerr << "readFile: Encountered NULL at position " << file.tellg()
           << "." << endl;
    }
  }
  file.close();
  if(ok){
    res->Set(true,Content);
  } else {
    res->Set(false,"");
  }
  return 0;
}

ValueMapping readFilevaluemap[] = {readFileVM<CcString>,
                                   readFileVM<FText>};

/*
15.3 Specification for ~readFile~

*/
const string readFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} -> text </text--->"
    "<text> readFile( Name ) </text--->"
    "<text> Reads text from file Name. If the file does not exist, \n"
    "contains a NULL character or some error occurs, the result is "
    "UNDEFINED.</text--->"
    "<text> query readFile('data.csv')  </text--->"
    ") )";

/*
15.4 Selection Function for ~writeFile~

Uses ~stringORtextSelect~.

15.5 Operator Instance for operator ~readFile~

*/
Operator readFile ( "readFile",
                   readFileSpec,
                   2,
                   readFilevaluemap,
                   stringORtextSelect,
                   stringORtext2textTM);


/*
16 Operator ~moveFile~

Move a file to another location within the file system. Returns TRUE,
iff this succeeds.

16.1 TypeMapping for ~moveFile~ and copyFile

---- {text|string} x {text|string} -> bool
----

*/

ListExpr moveOrCopyFileTM(ListExpr args){
  string err = "{text,string}  x {string,text} [x bool] expected";
  int listLength = nl->ListLength(args);
  if(listLength!=2 && listLength!=3){
    ErrorReporter::ReportError(err + " (wrong number of args)");
    return nl->TypeError();
  }
  if(listLength==3 && !CcBool::checkType(nl->Third(args))){
     return listutils::typeError(err + " ( third arg is not a bool)");
  }
  if(   !CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
   return listutils::typeError(err + " (first arg is not a strinbg or text)");
  }
  if(   !CcString::checkType(nl->Second(args))
     && !FText::checkType(nl->Second(args))){
   return listutils::typeError(err + " (2nd arg is not a strinbg or text)");
  }
  if(listLength==3){
    return listutils::basicSymbol<CcBool>();
  }
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            nl->OneElemList(nl->BoolAtom(false)),
                            listutils::basicSymbol<CcBool>());
}

/*
16.2 Value Mapping for ~moveFile~

*/

template<class T, class S, bool isMOVE>
int moveOrCopyFileVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  T* fileNameOld = static_cast<T*>(args[0].addr);
  S* fileNameNew = static_cast<S*>(args[1].addr);
  CcBool* createDir = static_cast<CcBool*>(args[2].addr);
  bool create = createDir->IsDefined()?createDir->GetValue():false;
  if(!fileNameOld->IsDefined() || !fileNameNew->IsDefined()){
    res->Set(false,false);
    return 0;
  }
  string fileNameOldS = fileNameOld->GetValue();
  string fileNameNewS = fileNameNew->GetValue();
  if(   fileNameOldS == "" || fileNameNewS == ""
     || !FileSystem::FileOrFolderExists(fileNameOldS)
    ){
    cerr << "moveFile: Cannot open file '" << fileNameOldS << "'!" << endl;
    res->Set(true,false);
    return 0;
  }
  if(create){
    string parent = FileSystem::GetParentFolder(fileNameNewS);
    if(parent!=""){
      if(!FileSystem::FileOrFolderExists(parent)){
          FileSystem::CreateFolderEx(parent);
      }
    }
  }

  bool  boolresult = isMOVE 
                     ?  FileSystem::RenameFileOrFolder(fileNameOldS,
                                                       fileNameNewS)
                     : FileSystem::Copy_File(fileNameOldS, fileNameNewS); 
  res->Set(true,boolresult);
  return 0;
}

ValueMapping moveFilevaluemap[] = {moveOrCopyFileVM<CcString, CcString, true>,
                                   moveOrCopyFileVM<CcString, FText, true>,
                                   moveOrCopyFileVM<FText,    CcString, true>,
                                   moveOrCopyFileVM<FText,    FText, true>};

/*
16.3 Specification for ~moveFile~

*/
const string moveFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} [ x bool ] -> text </text--->"
    "<text> moveFile( OldName, NewName ) </text--->"
    "<text> Move file OldName to file NewName. Can also be used to rename a "
    "file.\nReturns TRUE, iff move-command succeeds.\n If the optional boolean"
    " argument is present and TRUE, the parent directory of the target is "
    "created automatically.\n"
    "WARNING: Be extremely carefully about moving files!\n"
    "         The operator also overwrites existing files!</text--->"
    "<text> query moveFile('data.csv', 'data.csv.old')  </text--->"
    ") )";

/*
16.4 Selection Function for ~moveFile~

Uses ~stringORtext\_stringORtextSelect~.


16.5 Operator Instance for operator ~moveFile~

*/
Operator moveFile ( "moveFile",
                   moveFileSpec,
                   4,
                   moveFilevaluemap,
                   stringORtext_stringORtext_Select,
                   moveOrCopyFileTM);


OperatorSpec copyFileSpec(
  "{string,text} x {string,text} -> bool ",
  "copyFile(source, dest)",
  "Copyies the file source to a file named dest. "
  "Returns the success of this operation.",
  "query copyFile('data.csv','data1.csv')"
);

ValueMapping copyFileVM[] = {moveOrCopyFileVM<CcString, CcString, false>,
                             moveOrCopyFileVM<CcString, FText, false>,
                             moveOrCopyFileVM<FText,    CcString, false>,
                             moveOrCopyFileVM<FText,    FText, false>};

Operator copyFile( "copyFile",
                   copyFileSpec.getStr(),
                   4,
                   copyFileVM,
                   stringORtext_stringORtext_Select,
                   moveOrCopyFileTM);

/*
17 Operator ~basename~

*/
ListExpr basenameTM(ListExpr args){
  string err ="string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + "(wrong number of args)");
  }
  ListExpr arg = nl->First(args);
  if(!FText::checkType(arg) && ! CcString::checkType(arg)){
    return listutils::typeError(err);
  }
  return arg;
}

template<class T>
int basenameVMT(Word* args, Word& result,
               int message, Word& local, Supplier s){

  result =  qp->ResultStorage(s);
  T* res = (T*) result.addr;
  T* arg = (T*) args[0].addr;
  if(!arg->IsDefined()){
     res->SetDefined(false);
     return 0;
  }
  res->Set(true, FileSystem::Basename(arg->GetValue()));
  return 0;
}

ValueMapping basenameVM[] = {
   basenameVMT<CcString>,
   basenameVMT<FText>
};

int basenameSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec basenameSpec(
  "n -> n, n in {string,text}",
  "basename(_)",
  "Extract the filename without leading path from a pathname",
  "query basename('/home/secondo/secondo.txt')"
);

Operator basenameOp(
  "basename",
  basenameSpec.getStr(),
  2,
  basenameVM,
  basenameSelect,
  basenameTM
);





                   

/*
17 Operator ~getDirectory~

Get a stream of text with the listing of the given directory.

17.1 TypeMapping for ~getDirectory~

---- {text|string} -> stream(text)
----

*/

ListExpr stringORtext_OPTIONALint2textstreamTM(ListExpr args){
  string err = "{text|string} [x int] expected";
  int listLength = nl->ListLength(args);
  if(listLength!=1 && listLength!=2){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(    !nl->IsEqual(nl->First(args),CcString::BasicType())
      && !nl->IsEqual(nl->First(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(    listLength == 2
      && !nl->IsEqual(nl->Second(args),CcInt::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(FText::BasicType()));
}

/*
17.2 Value Mapping for ~getDirectory~

*/

class GetDirectoryLocalInfo
{ public:
    GetDirectoryLocalInfo(){
      elements = FilenameList(0);
      iter = elements.begin();
    }

    ~GetDirectoryLocalInfo(){}

    void SetElements(FilenameList L){
      elements = L;
      iter = elements.begin();
    }

    FilenameList elements;
    FilenameList::iterator iter;
};

template<class T>
int getDirectoryVM(Word* args, Word& result,
                   int message, Word& local, Supplier s){
  GetDirectoryLocalInfo* li;
  result = qp->ResultStorage(s);
  T* dirName = static_cast<T*>(args[0].addr);
  FText* resText;
  int levels = 1;

  switch(message){
    case OPEN:{
      li = new GetDirectoryLocalInfo();
      local.setAddr(li);
      if(!dirName->IsDefined()){
        cerr << "getDirectory: Directory parameter undefined!"
             << endl;
        return 0;
      }
      string dirNameS = dirName->GetValue();
      if(   dirNameS == ""
         || !FileSystem::FileOrFolderExists(dirNameS)
         || !FileSystem::IsDirectory(dirNameS)
        ){
        cerr << "getDirectory: Cannot open directory '" << dirNameS << "'!"
             << endl;
        return 0;
      }
      if(qp->GetNoSons(s)==2){ // get optional 2nd argument
        CcInt* levelsCc = static_cast<CcInt*>(args[1].addr);
        if(!levelsCc->IsDefined()){
          cerr << "getDirectory: Optional recursion parameter undefined!"
               << endl;
          return 0;
        } else {
          levels = levelsCc->GetIntval();
          if(levels<1){
            levels = 1;
          }
        }
      }
      FilenameList L(0);
      if(FileSystem::FileSearch(dirNameS, L, 0, levels, true, true, 0)){
        li->SetElements(L);
      }
      return 0;
    }

    case REQUEST:{
      if(local.addr){
        li = static_cast<GetDirectoryLocalInfo*>(local.addr);
      }else{
        result.setAddr(0);
        return CANCEL;
      }
      if(li->iter == li->elements.end()){
        result.setAddr(0);
        return CANCEL;
      }
      resText = new FText(true, *(li->iter));
      result.setAddr(resText);
      ++(li->iter);
      return YIELD;
    }

    case CLOSE:{
      result.setAddr(0);
      if(local.addr){
        li = static_cast<GetDirectoryLocalInfo*>(local.addr);
        delete li;
        local.setAddr(0);
      }
      return 0;
    }
  }
  // this line should never be reached!
  return -1;
}

ValueMapping getDirectoryvaluemap[] = {getDirectoryVM<CcString>,
                                       getDirectoryVM<FText>};

/*
17.3 Specification for ~getDirectory~

*/
const string getDirectorySpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> {text|string} [ x int] -> stream(text) </text--->"
    "<text> getDirectory( DirectoryName [, N]) </text--->"
    "<text> Create a stream of text containing all file names from directory\n"
    "DirectoryName and its subfolders. The function recurses down to the Nth\n"
    "level. If N is not specified, or N is set to 1 or smaller, meaning that\n"
    "only the direct contents of DirectoryName are listed.</text--->"
    "<text> query getDirectory('tmp', 2)</text--->"
    ") )";

/*
17.4 Selection Function for ~getDirectory~

Uses ~stringORtextSelect~.


17.5 Operator Instance for operator ~getDirectory~

*/
Operator getDirectory( "getDirectory",
                   getDirectorySpec,
                   2,
                   getDirectoryvaluemap,
                   stringORtextSelect,
                   stringORtext_OPTIONALint2textstreamTM);

/*
18 Operator ~toCSVtext~

Converts any object of a Type from CSVEXPORTABLE to a text object with the
corresponding CSV text representation.

18.1 TypeMapping for ~toCSVtext~

---- CSVEXPORTABLE -> text
----

*/

ListExpr CSVEXPORTABLEtoTextTM(ListExpr args){
  string err = "CSVEXPORTABLE expected";
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  int listLength = nl->ListLength(args);
  if(listLength!=1){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!am->CheckKind(Kind::CSVEXPORTABLE(),nl->First(args),errorInfo)){
    ErrorReporter::ReportError("stream element not in kind csvexportable");
    return nl->TypeError();
  }
  return nl->SymbolAtom(FText::BasicType());
}



/*
18.2 Value Mapping for ~toCSVtext~

*/

int toCSVtextVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  FText* res = static_cast<FText*>(result.addr);
  Attribute* object = static_cast<Attribute*>(args[0].addr);
  if(!object->IsDefined()){
    res->Set(false,"");
    return 0;
  }
  string objStr = object->getCsvStr();
  res->Set(true,objStr);
  return 0;
}

/*
18.3 Specification for ~toCSVtext~

*/
const string toCSVtextSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> X -> text, for X in CSVEXPORTABLE </text--->"
    "<text> toCSVtext( Obj ) </text--->"
    "<text>Converts any object of a CSVEXPORTABLE type to its CSV text\n"
    "representation.</text--->"
    "<text> query toCSVtext(3.14137) </text--->"
    ") )";

/*
18.4 Selection Function for ~toCSVtext~

Uses ~simpleselect~.


18.5 Operator Instance for operator ~toCSVtext~

*/
Operator toCSVtext( "toCSVtext",
                    toCSVtextSpec,
                    toCSVtextVM,
                    Operator::SimpleSelect,
                    CSVEXPORTABLEtoTextTM);


/*
19 Operator ~fromCSVtext~

Converts any object of a Type from CSVEXPORTABLE to a text object with the
corresponding CSV text representation.

19.1 TypeMapping for ~fromCSVtext~

---- CSVEXPORTABLE x text -> CSVEXPORTABLE
----

*/

ListExpr CSVIMPORTABLE_textORstring2CSVIMPORATABLETM(ListExpr args){
  string err = "CSVIMPORTABLE x {text|string} expected";
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  int listLength = nl->ListLength(args);
  if(listLength!=2){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!am->CheckKind(Kind::CSVIMPORTABLE(),nl->First(args),errorInfo)){
    ErrorReporter::ReportError("stream element not in kind csvimportable");
    return nl->TypeError();
  }
  if(    !nl->IsEqual(nl->Second(args),CcString::BasicType())
      && !nl->IsEqual(nl->Second(args),FText::BasicType())){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  return nl->First(args);
}



/*
19.2 Value Mapping for ~fromCSVtext~

*/

template<class T>
int fromCSVtextVM(Word* args, Word& result,
               int message, Word& local, Supplier s){

  T* InText= static_cast<T*>(args[1].addr);
  result = qp->ResultStorage( s );
  Attribute* Res = static_cast<Attribute*>(result.addr);

  if(!InText->IsDefined()){
    Res->SetDefined(false);
    return 0;
  }
  string myText = InText->GetValue();
  Res->ReadFromString(myText);
  return 0;
}

ValueMapping fromCSVtextvaluemap[] = {fromCSVtextVM<FText>,
                                      fromCSVtextVM<CcString>};

/*
19.3 Specification for ~fromCSVtext~

*/
const string fromCSVtextSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> X x {text|string} -> X, for X in CSVIMPORTABLE </text--->"
    "<text> fromCSVtext( Pattern, Text ) </text--->"
    "<text>Converts the textual CSV representation of the CSVIMPORTABLE type\n"
    "specified by the type of Pattern to a secondo object.</text--->"
    "<text> query fromCSVtext(0.0, '3.141') </text--->"
    ") )";

/*
19.4 Selection Function for ~fromCSVtext~

*/

int fromCSVtextSelect(ListExpr args){
  if(nl->IsEqual(nl->Second(args),FText::BasicType())){
    return 0;
  } else if(nl->IsEqual(nl->Second(args),CcString::BasicType())){
    return 1;
  }
  return -1;
}

/*
19.4 Operator instance

*/

Operator fromCSVtext( "fromCSVtext",
                   fromCSVtextSpec,
                   2,
                   fromCSVtextvaluemap,
                   fromCSVtextSelect,
                   CSVIMPORTABLE_textORstring2CSVIMPORATABLETM);

/*
20 Operator ~getPID~

20.1 Type Mapping for ~getPID~

---- getPID:    --> int
----

*/
ListExpr getPIDTM(ListExpr args){
  if( nl->ListLength(args)==0 ){
     return nl->SymbolAtom(CcInt::BasicType());
  }
  ErrorReporter::ReportError("No argument expected");
  return nl->TypeError();
}

/*
20.2 Value Mapping

*/

int getPIDVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  res->Set(true, WinUnix::getpid());
  return 0;
}

/*
20.3 Specification

*/

const string getPIDSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> -> int</text--->"
    "<text>getPID( ) </text--->"
    "<text>Returns the process identifier of the running Secondo kernel "
    "process. The result is always defined.</text--->"
    "<text>query getPID()</text--->"
    ") )";

/*
20.4 Operator instance

*/

Operator getPID( "getPID",
                   getPIDSpec,
                   getPIDVM,
                   Operator::SimpleSelect,
                   getPIDTM);

/*
21 Operator ~getSecondoVersion~

21.1 Type Mapping for ~getSecondoVersion~

---- getSecondoVersion:    --> string
----

*/
ListExpr getSecondoVersionTM(ListExpr args){
  if( nl->ListLength(args)==0 ){
     return nl->SymbolAtom(CcString::BasicType());
  }
  ErrorReporter::ReportError("No argument expected");
  return nl->TypeError();
}

/*
21.2 Value Mapping

*/

int getSecondoVersionVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  ostringstream os;
  os.precision(47);
  os << SECONDO_VERSION_MAJOR << "."
     << SECONDO_VERSION_MINOR << "."
     << SECONDO_VERSION_REVISION;
  res->Set( true, os.str() );
  return 0;
}

/*
21.3 Specification

*/

const string getSecondoVersionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> -> int</text--->"
    "<text>getSecondoVersion( ) </text--->"
    "<text>Returns the version string (version.subversion.revision) for the "
    "running Secondo kernel process. The result is always defined.</text--->"
    "<text>query getSecondoVersion()</text--->"
    ") )";

/*
21.4 Operator instance

*/

Operator getSecondoVersion( "getSecondoVersion",
                   getSecondoVersionSpec,
                   getSecondoVersionVM,
                   Operator::SimpleSelect,
                   getSecondoVersionTM);


/*
22 Operator ~getBDBVersion~

21.1 Type Mapping for ~getBDBVersion~

---- getBDBVersion:    --> string
----

*/
ListExpr getBDBVersionTM(ListExpr args){
  if( nl->ListLength(args)==0 ){
     return nl->SymbolAtom(CcString::BasicType());
  }
  ErrorReporter::ReportError("No argument expected");
  return nl->TypeError();
}

/*
22.2 Value Mapping

*/

int getBDBVersionVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  ostringstream os;
  os.precision(47);
  os << DB_VERSION_MAJOR << "."
     << DB_VERSION_MINOR;
  res->Set( true, os.str() );
  return 0;
}

/*
22.3 Specification

*/

const string getBDBVersionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> -> int</text--->"
    "<text>getBDBVersion( ) </text--->"
    "<text>Returns the version string (version.subversion) for the "
    "running Secondo kernel process' BerkeleyDB version. "
    "The result is always defined.</text--->"
    "<text>query getBDBVersion()</text--->"
    ") )";

/*
22.4 Operator instance

*/

Operator getBDBVersion( "getBDBVersion",
                   getBDBVersionSpec,
                   getBDBVersionVM,
                   Operator::SimpleSelect,
                   getBDBVersionTM);


/*
23 Operator ~getSecondoPlatform~

23.1 Type Mapping for ~getSecondoPlatform~

---- getSecondoPlatform:    --> string
----

*/
ListExpr getSecondoPlatformTM(ListExpr args){
  if( nl->ListLength(args)==0 ){
     return nl->SymbolAtom(CcString::BasicType());
  }
  ErrorReporter::ReportError("No argument expected");
  return nl->TypeError();
}

/*
23.2 Value Mapping

*/

int getSecondoPlatformVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcString* res = static_cast<CcString*>(result.addr);
  res->Set(true, WinUnix::getPlatformStr());
  return 0;
}

/*
23.3 Specification

*/

const string getSecondoPlatformSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> -> string</text--->"
    "<text>getSecondoPlatform( ) </text--->"
    "<text>Returns the platform, this Secondo kernel is running on. "
    "The result is always defined.</text--->"
    "<text>query getSecondoPlatform()</text--->"
    ") )";

/*
23.4 Operator instance

*/

Operator getSecondoPlatform( "getSecondoPlatform",
                   getSecondoPlatformSpec,
                   getSecondoPlatformVM,
                   Operator::SimpleSelect,
                   getSecondoPlatformTM);

/*
24 Operator ~getPageSize~

24.1 Type Mapping for ~getPageSize~

---- getPageSize:    --> int
----

*/
ListExpr getPageSizeTM(ListExpr args){
  if( nl->ListLength(args)==0 ){
     return nl->SymbolAtom(CcInt::BasicType());
  }
  ErrorReporter::ReportError("No argument expected");
  return nl->TypeError();
}

/*
24.2 Value Mapping

*/

int getPageSizeVM(Word* args, Word& result,
            int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  res->Set(true, WinUnix::getPageSize());
  return 0;
}

/*
24.3 Specification

*/

const string getPageSizeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text> -> int</text--->"
    "<text>getPageSize( ) </text--->"
    "<text>Returns the file-system's page size. "
    "The result is always defined.</text--->"
    "<text>query getPageSize()</text--->"
    ") )";

/*
24.4 Operator instance

*/

Operator getPageSize( "getPageSize",
                   getPageSizeSpec,
                   getPageSizeVM,
                   Operator::SimpleSelect,
                   getPageSizeTM);




/*
25 Operator SQLExport

This operator takes a tupel stream and writes a SQL script into a file.


25.1 TypeMapping


The signature is (stream(tuple)) x string x {text,string} [x bool] -> bool

All attributes within the tuple stream must be in KIND SQLexportable

*/
ListExpr sqlExportTM(ListExpr args){
 int len = nl->ListLength(args);
 string err = "stream(tuple) x {text,string} x string  [x bool] expected";
 if((len!=3) && (len!=4) ){
   return listutils::typeError(err);
 }
 ListExpr stream = nl->First(args);
 ListExpr fileName = nl->Second(args);
 ListExpr tabName = nl->Third(args);

 if(!Stream<Tuple>::checkType(stream)){
   return listutils::typeError(err);
 }
 if(!CcString::checkType(fileName) && !FText::checkType(fileName)){
   return listutils::typeError(err);
 }
 if(!CcString::checkType(tabName)){
   return listutils::typeError(err);
 } 

 if(len==4){
   if(!CcBool::checkType(nl->Fourth(args))){
     return listutils::typeError(err);
   }
 }
 ListExpr attrList = nl->Second(nl->Second(stream));
 while(!nl->IsEmpty(attrList)){
   ListExpr first = nl->First(attrList);
   attrList = nl->Rest(attrList);
   ListExpr type = nl->Second(first);
   if(!listutils::isKind(type,Kind::SQLEXPORTABLE())){
      string name = nl->SymbolValue(nl->First(first));
      return listutils::typeError("Attribute " + name + 
                                  " is not in kind SQLEXPORTABLE");
   }
 } 
 if(len==4){
     return listutils::basicSymbol<CcBool>();
 } else {
    return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                              nl->OneElemList(nl->BoolAtom(false)),
                              listutils::basicSymbol<CcBool>());
 }
}


template<class T>
int sqlExportVM1(Word* args, Word& result,
               int message, Word& local, Supplier s){

    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;

    T* FileName  = (T*) args[1].addr;
    CcString* TabName = (CcString*) args[2].addr;
    CcBool* Overwrite = (CcBool*) args[3].addr;
    if(!FileName->IsDefined() || ! Overwrite->IsDefined() ||
       !TabName->IsDefined()){
        res->SetDefined(false);
        return 0;
    }

    string filename = FileName->GetValue();
    bool overwrite = Overwrite->GetBoolval();

    if(FileSystem::FileOrFolderExists(filename) && !overwrite){
      res->Set(true,false);
      return 0;
    }
    fstream out;
    out.open(filename.c_str(), ios::out);
    if(!out.good()){
      res->Set(true,false);
      return 0;
    }

    Stream<Tuple> stream(args[0]);
    stream.open();
    Tuple* tuple = 0;

    string tabname = TabName->GetValue();

    string namelist="";

    bool first = true;
    while(out.good() && ( (tuple = stream.request() )!=0)){
      if(first){ // write the table create command to out
         out << "CREATE TABLE " 
             << TabName->GetValue() <<  " ( " << endl;


         ListExpr list = qp->GetType(qp->GetSon(s,0));
         ListExpr attrList = nl->Second(nl->Second(list));
         assert(nl->ListLength(attrList) == tuple->GetNoAttributes());

         for(int i=0;i<tuple->GetNoAttributes();i++){
             string name = nl->SymbolValue(nl->First(nl->First(attrList)));
             if(i>0) {
                namelist += ", "; 
             } else {
                namelist += " ";
             }
             namelist += name;
             attrList = nl->Rest(attrList);
             out << name << "  " << tuple->GetAttribute(i)->getSQLType() 
                 << " ," << endl;
         }
         out << ");" << endl;
        first = false;
      }

      out << "INSERT INTO " << tabname <<"(" << namelist << ") Values (";
      for(int i=0;i<tuple->GetNoAttributes();i++){
         if(i>0){ 
          out << ", ";
         } else {
           out << " ";
         }
         out << tuple->GetAttribute(i)->getSQLRepresentation(); 
      }
      out << ");" << endl;
      tuple->DeleteIfAllowed();
    }

    out.close();
    stream.close();
    res->Set(true,tuple==0);
    return 0;
}

/*
Value Mapping Array 

*/
ValueMapping sqlExportVM[] = {
    sqlExportVM1<CcString>,
    sqlExportVM1<FText>
 };


/*
Selection function

*/
int sqlExportSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

/*
Specification

*/
OperatorSpec sqlExportSpec(
  "stream(tuple) x {text,string} x string [ x bool] ", // signature
  " <stream> sqlExport[ fileName, tabName [, overwriteExisting] ] " , //syntax
  " writes an sql script fot importing a stream" ,
  " query ten feed sqlExport[\"ten.sql\",\"ten\",TRUE]"
);

/*
Operator Instance

*/
Operator sqlExport(
  "sqlExport",
  sqlExportSpec.getStr(),
  2,
  sqlExportVM,
  sqlExportSelect,
  sqlExportTM
);


/*
24.25 Operator importHGT1

24.25.1 TypeMapping

Signature is text -> stream(tuple((R : rect, V : int))

*/
ListExpr importHGT1TM(ListExpr args){
  string err = "text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(FText::checkType(nl->First(args))){
    return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
               nl->TwoElemList(listutils::basicSymbol<Tuple>(),
               nl->TwoElemList(
                  nl->TwoElemList(nl->SymbolAtom("R"),
                                  listutils::basicSymbol<Rectangle<2> >()),
                  nl->TwoElemList(nl->SymbolAtom("V"),
                                  listutils::basicSymbol<CcInt>()))));
  }
  return listutils::typeError(err);
}

/*
24.25.2 Value Mapping 

*/

class ImportHGT1Local{

public:
  ImportHGT1Local(FText* name, ListExpr t){
     little = WinUnix::isLittleEndian();
     undefvalue = -32768; // given by file format
     tt = new TupleType(t);
     f = 0;
     if(!name->IsDefined()){
        return;
     }
     string names = name->GetValue();
     if(names.length() < 4){
        return;
     }
     stringstream ss(names);
     char c;
     ss.get(c);
     int signum1 = tolower(c)=='s'?-1:1;

     int y;
     ss >> y;
     ss.get(c);
     int signum2 = tolower(c)=='w'?-1:1;
     int x;
     ss >> x;
     if(  (y>90) || (x > 180)){
        return;
     }
     x = x * signum2;
     y = y * signum1;
     lbx = x;
     lby = y+1;
     f = new ifstream(names.c_str(), ios::in|ios::binary|ios::ate);
     if(!f->is_open()){
         delete f;
         f = 0;
         return;
     }
     ifstream::pos_type size = f->tellg();
     if(size==25934402) { // == 3601 * 3601 * 2
        gridsize = 3601;
     } else if(size==2884802) { // 1201 * 1201 * 2
       gridsize = 1201;
     } else {
       f->close();
       delete f;
       f = 0;
     }
     posx = 0;
     posy = 0;
     cellsize = 1.0 / (double) gridsize;
     f->seekg (0, ios::beg);
  }

  
 ~ImportHGT1Local(){
     if(f){
       f->close();
       delete f;
       f = 0;
     }
     tt->DeleteIfAllowed();
  }

  Tuple*   next(){
     if(!f){
       // error during initialization
       return 0;
     }
     if( posy==gridsize){
       return 0;
     }
     if(!f->good()){
       cout << " file not good" << endl;
       return 0;
     }

     if(posx==0){ // read next line from file
        f->read((char*) buffer, gridsize*2);
        if(!f->good()){
           return 0;
        }
     }
  
     int16_t v = buffer[posx];
  
     if(little){
       //cout << "convert " << v ;
       v = WinUnix::convertEndian((uint16_t)v);
       //cout << " into " << v << endl;
     }
   
     double min[2];
     double max[2];
     min[0] = lbx + cellsize*posx;
     min[1] = lby - cellsize*(posy+1);
     max[0] = lbx + cellsize*(posx+1);
     max[1] = lby - cellsize*(posy);

     Tuple* res = new Tuple(tt);
     res->PutAttribute(0, new Rectangle<2>(true,min,max));
     if(v!=undefvalue){
        res->PutAttribute(1, new CcInt(true,v));
     } else {
        res->PutAttribute(1, new CcInt(false));
     }
     posx++;
     if(posx==gridsize){
        posy++;
        posx=0;
     }
     return res;
  }



private:
  ifstream* f;
  int gridsize;
  double cellsize;
  int lbx;
  int lby;
  int posx;
  int posy;
  TupleType* tt;
  bool little;
  int16_t buffer[3601]; // buffer for reading from file 
  int undefvalue;

};

int importHGT1VM(Word* args, Word& result,
                  int message, Word& local, Supplier s){

    ImportHGT1Local* li  = (ImportHGT1Local*) local.addr;
    switch(message){
         case  OPEN : { if(li){
                            delete li;
                         }
                      local.addr = new ImportHGT1Local( (FText*) args[0].addr,
                                           nl->Second(GetTupleResultType(s)));
                         return 0;
                        }
         case REQUEST : { result.addr = li?li->next():0;
                          return  result.addr?YIELD:CANCEL;
                        }
         case CLOSE : {
                         if(li){ delete li; }
                         local.addr = 0;
                         return 0;
                       }
    }
    return  -1;

}


/*
Specification

*/
OperatorSpec importHGT1Spec(
  " text -> stream(tuple([R : rect, V : int)) ", // signature
  "  importGHT1(_)" , //syntax
  " imports an HGT file" ,
  " query importGHT1('N55W51.hgt') consume"
);

/*
Operator instance

*/
Operator importGHT1(
   "importHGT1",
   importHGT1Spec.getStr(),
   importHGT1VM,
   Operator::SimpleSelect,
   importHGT1TM);



/*
24.26 Operator ~removeDirectory~

*/

ListExpr removeDirectoryTM(ListExpr args){

 string err = "{string,text} [ x bool] expected";
 if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
   return listutils::typeError(err + " (wrong number of args)");
 }
 ListExpr f = nl->First(args);
 if(!CcString::checkType(f) && !FText::checkType(f)){
   return listutils::typeError(err);
 } 

 if(nl->HasLength(args,1)){
    return nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->OneElemList(nl->BoolAtom(false)),
                listutils::basicSymbol<CcBool>());
 }
  
 if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError(err);
 }
 return listutils::basicSymbol<CcBool>();
}

template<class T>
int removeDirectoryVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){

  T* file = (T*) args[0].addr;
  CcBool* rek = (CcBool*) args[1].addr;
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;

  if(!file->IsDefined() || !rek->IsDefined()){
    res->SetDefined(false);
    return 0;
  }

  string fn = file->GetValue();
  bool r = rek->GetValue();

  if(!FileSystem::IsDirectory(fn)){
     res->Set(true,false);
     return 0;
  }

  if(r){
     res->Set(true,FileSystem::EraseFolder(fn));
  } else {
     res->Set(true,FileSystem::DeleteFileOrFolder(fn));
  }
  return 0;
}

int removeDirectorySelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

ValueMapping removeDirectoryVM[] = {
   removeDirectoryVMT<CcString>,
   removeDirectoryVMT<FText>
};

OperatorSpec removeDirectorySpec(
  "{string,text} [x bool] -> bool",
  "removeDirectory( filename, recursive)",
  "Removes a directory on the filesystem. "
  "If the boolean argument is true, all subfolders "
  "of that directory will be removed too. Otherwise "
  "only empty directories will be deleted. The default "
  "value for the flag is false.",
  "query removeDirectory('Blub.dir', TRUE)"
);

Operator removeDirectoryOp(
  "removeDirectory",
  removeDirectorySpec.getStr(),
  2,
  removeDirectoryVM,
  removeDirectorySelect,
  removeDirectoryTM
);



ListExpr shpBoxTM(ListExpr args){
  if(!nl->HasLength(args,1)){
     return listutils::typeError("One argument expected");
  }
  ListExpr arg = nl->First(args);
  if(!CcString::checkType(arg) && !FText::checkType(arg)){
     return listutils::typeError("string or text expected");
  }
  return listutils::basicSymbol<Rectangle<2> >();
}


template<class T>
int shpBoxVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){

    T* f = (T*) args[0].addr;
    result = qp->ResultStorage(s);
    Rectangle<2>* res = (Rectangle<2>*) result.addr;
    if(!f->IsDefined()){
      res->SetDefined(false);
      return 0;
    }
    ifstream in(f->GetValue().c_str(), ios::in|ios::binary);
    if(!in.good()){
       res->SetDefined(false);
       return 0;
    }

    in.seekg(0,ios::beg);
    uint32_t code;
    uint32_t version;
    uint32_t type;
    in.read(reinterpret_cast<char*>(&code),4);
    in.seekg(28,ios::beg);
    in.read(reinterpret_cast<char*>(&version),4);
    in.read(reinterpret_cast<char*>(&type),4);
    if(WinUnix::isLittleEndian()){
        code = WinUnix::convertEndian(code);
    } else {
       version = WinUnix::convertEndian(version);
       type = WinUnix::convertEndian(type);
    }
    if(code!=9994){ // not a shapefile
      res->SetDefined(false);
      return 0;
    } 
    uint64_t xmin;
    uint64_t ymin;
    uint64_t xmax;
    uint64_t ymax;
    in.read(reinterpret_cast<char*>(&xmin),8);
    in.read(reinterpret_cast<char*>(&ymin),8);
    in.read(reinterpret_cast<char*>(&xmax),8);
    in.read(reinterpret_cast<char*>(&ymax),8);
    if(!in.good()){ // corrupt file
      res->SetDefined(false);
      return 0;
    }
    in.close();
    if(!WinUnix::isLittleEndian()){
       xmin = WinUnix::convertEndian(xmin);
       ymin = WinUnix::convertEndian(ymin);
       xmax = WinUnix::convertEndian(xmax);
       ymax = WinUnix::convertEndian(ymax);
    } 
    void* d=&xmin;
    double xmind = *(reinterpret_cast<double*>(d));
    d = &ymin;
    double ymind = *(reinterpret_cast<double*>(d));
    d = &xmax;
    double xmaxd = *(reinterpret_cast<double*>(d));
    d = &ymax;
    double ymaxd = *(reinterpret_cast<double*>(d));
    double minD[] = {xmind,ymind};
    double maxD[] = {xmaxd, ymaxd};
    res->Set(true, minD, maxD );
    return 0;
}

ValueMapping shpBoxVM[] = {
    shpBoxVMT<CcString>,
    shpBoxVMT<FText>
} ;

int shpBoxSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec shpBoxSpec(
   " string|text -> rect",
   " shpBox(_)",
   "Extracts the bounding box information from a shapefile.",
   "query shpBox('kinos.shp')"
);

Operator shpBoxOp(
   "shpBox",
   shpBoxSpec.getStr(),
   2,
   shpBoxVM,
   shpBoxSelect,
   shpBoxTM
);

  
/*
24.26 collectShpFiles

This operator connects a set of shape file into a single one.
The arguments are a stream of names and the file name for
the result file. 

The output is a tuple stream containing the original file names, 
a boolean value reporting the success for this file and a text 
representing an error message if the was no success.

*/

ListExpr shpCollectTM(ListExpr args){
  if(!nl->HasLength(args,3)){
    return listutils::typeError("invalid number of arguments");
  }
  if(!Stream<CcString>::checkType(nl->First(args))
     && !Stream<FText>::checkType(nl->First(args))){
    return listutils::typeError("the first argument has to be a stream "
                                "of string or a stream of text");
  }
  if(!CcBool::checkType(nl->Second(args))){
    return listutils::typeError("second argument has to be of type bool");
  }

  if(!CcString::checkType(nl->Third(args))
     &&!FText::checkType(nl->Third(args))){
    return listutils::typeError("the third arg ist neihter "
                                "a string nor a text");
  }
  return nl->TwoElemList(
            listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                 nl->ThreeElemList(
                    nl->TwoElemList(
                         nl->SymbolAtom("File"),
                         nl->Second(nl->First(args))),
                    nl->TwoElemList(
                         nl->SymbolAtom("Success"),
                         listutils::basicSymbol<CcBool>()),
                    nl->TwoElemList(
                         nl->SymbolAtom("ErrMsg"),
                         listutils::basicSymbol<FText>())))); 
}



template<class T>
class shpCollectInfo{
  public:
    shpCollectInfo(Word& _stream, 
                   const string& fn, 
                   bool createShx,
                   ListExpr _tt):
     stream(_stream){
     outshp = new ofstream(fn.c_str(), ios::binary| ios::out );
     char* p = realpath(fn.c_str(),0);
     outabsolutepath = string(p);
     free(p);
     outbuf = new char[FILE_BUFFER_SIZE];
     outshp->rdbuf()->pubsetbuf(outbuf,FILE_BUFFER_SIZE);

     if(!createShx){
       outshx = 0;
       shxbuf = 0;
     } else {
        string name = fn;
        stringutils::toLower(name);
        if(stringutils::endsWith(name,".shp")){
          name = fn.substr(0,fn.length()-4);
        } else {
          name = fn;
        }
        name += ".shx";
        outshx = new ofstream(name.c_str(), ios::binary | ios::out);
        shxbuf = new char[FILE_BUFFER_SIZE];
        outshx->rdbuf()->pubsetbuf(shxbuf,FILE_BUFFER_SIZE); 
     }


     stream.open();
     tt = new TupleType(_tt);
     shpType = -1;
     noRecords = 0;
    } 

    ~shpCollectInfo(){
       // write new bounding box into output file
       uint64_t s = outshp->tellp();
       uint32_t s2 = s/2;
       if(s>100){ // header length of a shape file
          outshp->seekp(24);
          WinUnix::writeBigEndian(*outshp,s2); // store file size
          outshp->seekp(36); // position of XMin
          WinUnix::writeLittle64(*outshp,minX);
          WinUnix::writeLittle64(*outshp,minY);
          WinUnix::writeLittle64(*outshp,maxX);
          WinUnix::writeLittle64(*outshp,maxY);
       }
       outshp->close();
       delete[] outbuf;
       delete outshp; 
       
       if(outshx){ // write the same information into shx file if required
         s  = outshx->tellp();
         s2 = s/2;
         if(s>100){
          outshx->seekp(24);
          WinUnix::writeBigEndian(*outshx,s2); // store file size
          outshx->seekp(36); // position of XMin
          WinUnix::writeLittle64(*outshx,minX);
          WinUnix::writeLittle64(*outshx,minY);
          WinUnix::writeLittle64(*outshx,maxX);
          WinUnix::writeLittle64(*outshx,maxY);
         }
         outshx->close();
         delete[] shxbuf;
         delete outshx;
       }

       stream.close();
       tt->DeleteIfAllowed();
     }

     Tuple* next(){
       T* f = stream.request();
       if(!f){
         return 0;
       }
       Tuple* res = new Tuple(tt);
       res->PutAttribute(0,f);
       if(!f->IsDefined()){
         return error(res,"undefined file name",0);
       }
       appendFile(res, f->GetValue());
       return res;
     }

  private:
    Stream<T> stream;
    ofstream* outshp;
    ofstream* outshx;
    int32_t shpType;
    double minX;
    double minY;
    double maxX;
    double maxY;
    TupleType* tt;
    uint32_t noRecords;
    char* outbuf;
    char* shxbuf;
    string outabsolutepath;

    Tuple* error(Tuple* res, string message, ifstream* inshp=0, 
                 char* inbuf = 0 ){
        res->PutAttribute(1, new CcBool(true,false));
        res->PutAttribute(2, new FText(true, message));
        if(inshp) delete inshp;
        if(inbuf) delete[] inbuf;
        return res;
    }

    Tuple* appendFile(Tuple* res, const string& fileName){
      char* inpath = realpath(fileName.c_str(),0);
      string inp = string(inpath);
      free(inpath);
      if(inp == outabsolutepath){
         return error(res, "file identically to output");
      }

      ifstream* inshp = new ifstream(fileName.c_str(), ios::binary);
      if(!inshp->good()){
         return error(res, "could not open shape file", inshp);
      }
      char* inbuf = new char[FILE_BUFFER_SIZE];
      inshp->rdbuf()->pubsetbuf(inbuf,FILE_BUFFER_SIZE);
      // the file can be opened, now check the type
      int32_t code = readBigInt32(*inshp);
      if(!inshp->good()){
        return error(res,"problem in shape file", inshp, inbuf);
      }
      if(code !=9994){
        return error(res,"not a shape file", inshp,inbuf);
      }
      // read shape type and bounding box
      inshp->seekg(32);
      int32_t type = readLittleInt32(*inshp);
      double _minX = readLittleDouble(*inshp);
      double _minY = readLittleDouble(*inshp);
      double _maxX = readLittleDouble(*inshp);
      double _maxY = readLittleDouble(*inshp);
      if(!inshp->good()){
        return error(res,"problem in shape file", inshp,inbuf);
      }
      if(type!=1 && type!=3 && type!=5 && type!=8){
        return error(res, "unsupported shape type", inshp,inbuf);
      }

      if(noRecords==0){ // first file
         shpType = type;
         minX = _minX;
         maxX = _maxX;
         minY = _minY;
         maxY = _maxY;
         // copy header into the new file
         char header[100];
         outshp->seekp(0); // if there was an empty shape file
         inshp->seekg(0);
         inshp->read(header, 100);
         outshp->write(header,100);
         if(outshx){
           outshx->seekp(0);
           outshx->write(header,100);
         }
      } else {
          if(shpType != type){
            return error(res, "shape type differs", inshp,inbuf);
          }
          if(_minX < minX) minX = _minX;
          if(_maxX > maxX) maxX = _maxX;
          if(_minY < minY) minY = _minY;
          if(_maxY > maxY) maxY = _maxY;
          inshp->seekg(100);  // overread header
      }
      uint32_t buffersize = 16*1024;  
      char buffer[buffersize]; // maximum buffer size for a single value
      bool done = false;
      while(!inshp->eof() && !done && inshp->good()){
        // read record number from in
        readBigInt32(*inshp);
        if(!inshp->eof() && inshp->good()){
          // read contents length
          uint32_t length = readBigInt32(*inshp);
          // write output recno and length to output
          if(inshp->good()){
             noRecords++;
             uint32_t offset = outshp->tellp();
             WinUnix::writeBigEndian(*outshp, noRecords);
             WinUnix::writeBigEndian(*outshp, length);
             uint32_t chars = length*2;
             // copy content
             while(chars>0){
               uint32_t toCopy = (uint32_t)(chars>buffersize?buffersize:chars);
               inshp->read(buffer,toCopy);
               outshp->write(buffer,toCopy);
               chars -= toCopy;
             }
             // write info to shx file
             if(outshx){
               WinUnix::writeBigEndian(*outshx,offset/2);
               WinUnix::writeBigEndian(*outshx,length);
             }
          }
        } else {
          done = true;
        }
      }


      inshp->close();
      delete inshp;
      delete[] inbuf;
      res->PutAttribute(1, new CcBool(true,true));
      res->PutAttribute(2, new FText(true,""));
      return res;
    }
};


template<class T, class F>
int shpCollectVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){

  shpCollectInfo<T>* li = (shpCollectInfo<T>*) local.addr;
  switch(message){
    case OPEN : {
                  if(li) {
                    delete li;
                    local.addr=0;
                  }
                  CcBool* x = (CcBool*) args[1].addr;
                  bool createShx = x->IsDefined() && x->GetValue();

                  F* fn = (F*) args[2].addr;
                  if(!fn->IsDefined()){
                    return 0;
                  }
                  ListExpr tt = nl->Second(GetTupleResultType(s));
                  local.addr = new shpCollectInfo<T>(args[0], 
                                               fn->GetValue(),
                                               createShx,
                                               tt);
                  return 0;
                }
    case REQUEST : {
                    result.addr = li?li->next():0;
                    return result.addr?YIELD:CANCEL;
                 }
    case CLOSE : {
                   if(li){
                     delete li;
                     local.addr = 0;
                   }
                   return 0;
                 }
  } 
  return -1;

}

ValueMapping shpCollectVM[] = {
   shpCollectVMT<CcString,CcString>,
   shpCollectVMT<CcString,FText>,
   shpCollectVMT<FText,CcString>,
   shpCollectVMT<FText,FText>
};

int shpCollectSelect(ListExpr args){
  int n1 = Stream<CcString>::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Third(args))?0:1;
  return n1+n2;
}

OperatorSpec shpCollectSpec(
  " stream({text,string}) x bool x {text, string} -> stream(tuple) ",
  " _ shpcollect[_,_]",
  "Puts some shape files whose names comes from the stream "
  "into a single shape file. If the boolean argument is true, "
  "the corresponding shx file is created. "
  "The output is a tuple stream "
  "reporting for each file whether the connect was successful "
  "and in case of an error with an error message.",
  " query getdirectory(\".\") filter[ . endsWith \".shp\"] "
  "shpCollect[TRUE, 'all.shp']"
);

Operator shpCollectOp(
  "shpCollect",
  shpCollectSpec.getStr(),
  4,
  shpCollectVM,
  shpCollectSelect,
  shpCollectTM
);



/*
4.26 Operator ~db3Collect~

4.26.1 Type Mapping

*/
ListExpr db3CollectTM(ListExpr args){

  if(!nl->HasLength(args,2)){
    return listutils::typeError("invalid number of arguments");
  }
  if(!Stream<CcString>::checkType(nl->First(args))
     && !Stream<FText>::checkType(nl->First(args))){
    return listutils::typeError("the first argument has to be a stream "
                                "of string or a stream of text");
  }
  if(!CcString::checkType(nl->Second(args))
     &&!FText::checkType(nl->Second(args))){
    return listutils::typeError("the second arg ist neihter "
                                "a string nor a text");
  }
  return nl->TwoElemList(
            listutils::basicSymbol<Stream<Tuple> >(),
            nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                 nl->ThreeElemList(
                    nl->TwoElemList(
                         nl->SymbolAtom("File"),
                         nl->Second(nl->First(args))),
                    nl->TwoElemList(
                         nl->SymbolAtom("Success"),
                         listutils::basicSymbol<CcBool>()),
                    nl->TwoElemList(
                         nl->SymbolAtom("ErrMsg"),
                         listutils::basicSymbol<FText>())))); 
}

/*
4.26.2 The LocalInfo Class

*/

class db3AttrType{
  public:
   db3AttrType(char* info): name(info,11u),type(info[11]),
                                     length(info[16]),
                                     noDecimals(info[17]){
     name = string(name.c_str());
   } 

   bool operator==(const db3AttrType& rhs){
     return    name==rhs.name 
            && type==rhs.type
            && length==rhs.length
            && noDecimals==rhs.noDecimals;
   }
   bool operator!=(const db3AttrType& rhs){
     return    name!=rhs.name 
            || type!=rhs.type
            || length!=rhs.length
            || noDecimals!=rhs.noDecimals;
   }

   bool isValid(){
     if( type!= 'C' && type!='N' && type!='L'
         && type != 'D' && type!='M'){
        print(cerr);
        return false;
     }
     bool res = stringutils::isIdent(name);
     if(!res){
         print(cerr); cerr << endl;
     }
     return  res;
   }


   void print(ostream& out){
     out << name << ", T:" << type << ", L:" << length 
         << ", D : " << noDecimals << endl;
   }

  
  private:
    string name;
    unsigned char type;
    unsigned char length;
    unsigned char noDecimals;
};


template<class T>
class db3CollectInfo{
  public:
    db3CollectInfo(Word _stream, const string& _fileName,
                   ListExpr _tt): stream(_stream){
       tt = new TupleType(_tt);
       char* outpath = (char*) malloc(PATH_MAX); 
       realpath(_fileName.c_str(),outpath);
       outabsolutepath = string(outpath);
       free(outpath);
       out = new ofstream(_fileName.c_str(),ios_base::binary);
       outbuf = new char[FILE_BUFFER_SIZE];
       out->rdbuf()->pubsetbuf(outbuf,FILE_BUFFER_SIZE);
       noRecords = 0;
       stream.open();
    }
 
    ~db3CollectInfo(){
       if(out->good() && out->tellp()>8){
          // TODO: change modification date (not important)
          out->seekp(4);
          WinUnix::writeLittleEndian(*out,noRecords);       
       }
       out->close();
       delete out;
       delete [] outbuf;
       tt->DeleteIfAllowed();
       stream.close();
    }


    Tuple* next() {
       T* fn = stream.request();
       if(!fn){
          return 0;
       }
       Tuple* res = new Tuple(tt);
       res->PutAttribute(0,fn);
       if(!fn->IsDefined()){
         return error(res,"Undefined file name");
       }
       return processFile(res, fn->GetValue());

    }

    


  private:
    Stream<T> stream;
    TupleType* tt;  
    ofstream* out;
    char* outbuf;
    uint32_t noRecords;
    vector<db3AttrType> dbType;
    bool hasMemo;
    string outabsolutepath;

    Tuple* error(Tuple* res, const string& msg, 
                 ifstream* in1 = 0, ifstream* in2=0,
                 char* in1buf=0, char* in2buf=0){
        res->PutAttribute(1, new CcBool(true,false));
        res->PutAttribute(2, new FText(true,msg));
        if(in1){
          in1->close();
        }
        if(in2){
          in2->close();
        }
        if(in1buf) delete[] in1buf;
        if(in2buf) delete[] in2buf;
        return res;
    }


    Tuple* processFile(Tuple* res, const string& name){
       char* inpath = realpath(name.c_str(),0);
       string inabs = string(inpath);
       free(inpath);
       if(inabs==outabsolutepath){
          return error(res,"input file identically to output");
       }


        ifstream in1(name.c_str(), ios_base::binary);
        if(!in1.good()){
           return error(res,"Cannot open file", &in1);
        }
        char* in1buf = new char[FILE_BUFFER_SIZE];
        in1.rdbuf()->pubsetbuf(in1buf, FILE_BUFFER_SIZE);
        // read first byte to detect used version
        char version;
        in1.read(&version,1);
        if(!in1.good()){
           return error(res,"problem in reading file", &in1,0,in1buf);
        }
        if(version!=0x02 && version!=0x03 && version!=0x83){
           return error(res,"file does not contain a supported "
                            "dbase version", &in1,0,in1buf);
        }
        bool hasMemoLocal = false;
        if(version==0x83){
           return error(res,"memo fields are not supported yet",&in1,0,in1buf);
           hasMemoLocal = true;
        }


        // read global header information
        in1.seekg(4);
        uint32_t inNoRecords = readLittleInt32(in1);
        uint16_t headerLength = readLittleInt16(in1);
        uint16_t recordLength = readLittleInt16(in1);
        in1.seekg(32); // jump over the 20 reserved bytes
        // read the field
        char field[32];
        vector<db3AttrType> type;
        while(in1.good() && (in1.tellg() < (headerLength - 1))){
           in1.read(field,32);
           db3AttrType t(field);
           type.push_back(t);  
           if(!t.isValid()){
             return error(res,"found invalid field definition",&in1,0,in1buf);
           }
        }
        // read end of header mark
        char mark;
        in1.read(&mark,1);
        if(mark!=0x0d){
           return error(res,"invalid structure found",&in1,0,in1buf);
        }
        // copy header information if this is the first file
        if(out->tellp()==0){
          in1.seekg(0);
          char head[headerLength];
          in1.read(head,headerLength);
          out->write(head, headerLength);
          dbType = type; // overtake type information
          hasMemo = hasMemoLocal;
        } else {
          if(dbType.size()!=type.size()){
            return error(res,"different types in files",&in1,0,in1buf);
          }
          for(size_t i=0;i<type.size();i++){
            if(type[i]!=dbType[i]){
               return error(res,"different types in files",&in1,0,in1buf);
            } 
          }
        }
        // copy content for non-Memo
        if(!hasMemo){
          char* buffer = new char[recordLength];
          uint32_t writtenRecords=0;
          while(writtenRecords<inNoRecords && in1.good()){
            in1.read(buffer,recordLength);
            if(in1.good()){
              out->write(buffer,recordLength);
              writtenRecords++;
            }
          } 
          delete[] buffer;
          if(writtenRecords != inNoRecords){
             stringstream ss;
             ss << "written " << writtenRecords 
                << " but according to the header it should be " 
                << inNoRecords;
             return error(res,ss.str(),&in1,0,in1buf);
          }
          in1.close();
          noRecords += writtenRecords; 
          res->PutAttribute(1,new CcBool(true,true));
          res->PutAttribute(2,new FText(true,""));
          delete[] in1buf;
          return res;
        } else {
             return error(res,"Memo fields are not implemented yet",
                          &in1,0,in1buf);
            
        }
    }
};


template<class T, class F>
int db3CollectVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){

  db3CollectInfo<T>* li = (db3CollectInfo<T>*) local.addr;
  switch(message){
    case OPEN : {
                  if(li) {
                    delete li;
                    local.addr=0;
                  }
                  F* fn = (F*) args[1].addr;
                  if(!fn->IsDefined()){
                    return 0;
                  }
                  ListExpr tt = nl->Second(GetTupleResultType(s));
                  local.addr = new db3CollectInfo<T>(args[0],
                                               fn->GetValue(),tt);
                  return 0;
                }
    case REQUEST : {
                    result.addr = li?li->next():0;
                    return result.addr?YIELD:CANCEL;
                 }
    case CLOSE : {
                   if(li){
                     delete li;
                     local.addr = 0;
                   }
                   return 0;
                 }
  } 
  return -1;

}



ValueMapping db3CollectVM[] = {
   db3CollectVMT<CcString,CcString>,
   db3CollectVMT<CcString,FText>,
   db3CollectVMT<FText,CcString>,
   db3CollectVMT<FText,FText>
};

int db3CollectSelect(ListExpr args){
  int n1 = Stream<CcString>::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Second(args))?0:1;
  return n1+n2;
}

OperatorSpec db3CollectSpec(
  " stream({text,string}) x {text, string} -> stream(tuple) ",
  " _ db3collect[_]",
  "Puts some shape files whose names comes from the stream "
  "into a single dbase file. The output is a tuple stream "
  " reporting for each file whether the connect was successful"
  " and in case of an error with an error message.",
  " query getdirectory(\".\") filter[ . endsWith \".dbf\"] "
  "db3Collect['all.db3']"
);

Operator db3CollectOp(
  "db3Collect",
  db3CollectSpec.getStr(),
  4,
  db3CollectVM,
  db3CollectSelect,
  db3CollectTM
);



/*
24,19 Operator ~noShpRecords~

Retrieves the number of records from an shx file indexing 
some shp file. If the shx file is not present, this operator 
counts the number of records by scanning the main shape file.

*/
ListExpr noShpRecordsTM(ListExpr args){
  const string err = "string or text expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err+ " (wrong number of args)");
  }
  if(   !CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();
}


template<class F>
int noShpRecordsVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){

   F* fn = (F*) args[0].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   if(!fn->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   string name = fn->GetValue();
   string n2 = name;
   stringutils::toLower(n2);
   string shpname;
   string shxname;

   if(stringutils::endsWith(n2,".shx")){
     shxname = name;
     shpname = name.substr(0,name.length()-4)+".shp";
   } else if(stringutils::endsWith(n2,"shp")){
      shxname = name.substr(0,name.length()-4)+".shx";
      shpname = name;
   } else {
      shxname = name+".shx";
      shpname = name+".shp";
   }
   // try to get the record number without scanning the shapefile
   ifstream shxin(shxname.c_str(),ios::binary);
   if(shxin.good()){
      shxin.seekg(0,ios::end);
      uint32_t len = shxin.tellg();
      shxin.close();
      if(len>=100){ // assumed to be a shx file
         res->Set(true, (len-100)/8);
         return 0;
      }
   }


   cerr << "no shx file found, scan shp file" << endl;

   // using shx file was not successful, scan shp file
   ifstream shpin(shpname.c_str(),ios::binary);
   if(!shpin.good()){ // cannot open file
      res->SetDefined(false);
      return 0; 
   }
   char* buffer = new char[FILE_BUFFER_SIZE];
   shpin.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
   uint32_t code = readBigInt32(shpin);
   if(!shpin.good() || (code!=9994)){ // not a shape file
      shpin.close();
      delete[] buffer;
      res->SetDefined(false);
      return 0;
   }
   shpin.seekg(24);
   uint64_t len = readBigInt32(shpin);
   if(!shpin.good()){
      shpin.close();
      delete[] buffer;
      res->SetDefined(false);
      return 0;
   }   
   len = len*2;  // length is given in 16 bit words
   shpin.seekg(100); // jump over header
   uint64_t pos = 100;
   uint32_t count = 0;
   while(pos < len && shpin.good()){
     count++;
     uint32_t recNo = readBigInt32(shpin);
     uint32_t cl = readBigInt32(shpin);
     if(recNo!=count){
       cerr << "found wrong numbering in shape file" << endl
            << " should be " << count << endl
            << " but is actually " << recNo << endl;
       cerr << "content length is " << cl << endl;
     }
     shpin.seekg(2*cl,ios::cur);
     pos += 2*(4+cl);
   }
   shpin.close();
   delete[] buffer;
   res->Set(true,count);
   return 0;
}


ValueMapping noShpRecordsVM[] = {
  noShpRecordsVMT<CcString>,
  noShpRecordsVMT<FText>
};


int noShpRecordsSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec noShpRecordsSpec(
   " {string, text} -> int ",
   "noShpRecords(_) ",
   "Retrieves the number of records contained in a shape file. "
   "If the corresponsing index file is present, it will be "
   "used to determine the number of records in constant time. "
   "If the index file is not present, the shape file itself "
   "is scanned to get this number.",
   "query noRecords('Kinos.shp')"

);

Operator noShpRecordsOp(
  "noShpRecords",
  noShpRecordsSpec.getStr(),
  2,
  noShpRecordsVM,
  noShpRecordsSelect,
  noShpRecordsTM
);


/*
24.10 Operator ~noDB3Records~

This operator returns the number of records within a 
d-base III file. If the second argument is given and
TRUE, the file size is checked against the stored number.

*/

ListExpr noDB3RecordsTM(ListExpr args){
   string err = "{string, text} [ x bool] expected";
   if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
     return listutils::typeError(err+ " (wrong number of args)");
   }
   if(   !CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
      return listutils::typeError(err + " (first arg is neither a string "
                                        "nor a text)");
   }
   if(nl->HasLength(args,1)){
     return nl->ThreeElemList(
                  nl->SymbolAtom(Symbols::APPEND()),
                  nl->OneElemList(nl->BoolAtom(false)),
                  listutils::basicSymbol<CcInt>());
   }
   if(!CcBool::checkType(nl->Second(args))){
     return listutils::typeError(err + " (second arg ist not a bool)");
   }
   return listutils::basicSymbol<CcInt>();
}



template<class F>
int noDB3RecordsVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){
   F* f = (F*) args[0].addr;
   CcBool* c = (CcBool*) args[1].addr;
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;

   if(!f->IsDefined() || !c->IsDefined()){
      res->SetDefined(false);
      return 0;
   }
   string fn = f->GetValue();
   ifstream in(fn.c_str(), ios::binary);
   if(!in){
     cerr << "could not open file " << f->GetValue() << endl;
     res->Set(false,0);
     return 0;
   }
   unsigned char version;
   in.read((char*)(&version),1);
   if(version!=0x02 && version!=0x03 && version!=0x83){
     cerr << "file " << f->GetValue() << " seems not to be a dbase file"
          << endl;
     res->Set(false,0);
     return 0;
   }
   in.seekg(4);
   uint32_t noRecords = readLittleInt32(in);
   if(!in.good()){
     cerr << "problem in reading file" << endl;
     return 0;
   }
   if(!c->GetValue()){
     res->Set(true,noRecords);
     return 0;
   }
   uint16_t headerLength = readLittleInt16(in);
   uint16_t recordSize = readLittleInt16(in);
   in.seekg(0,ios::end);
   size_t fileLength = in.tellg();
   if(fileLength != headerLength + noRecords * recordSize &&
      fileLength != headerLength + 1 + noRecords * recordSize ){ 
      // may be there is a 0x1A end marker or not
      cerr << "real file length is " << fileLength << endl;
      cerr << "expected file length is " 
           << (headerLength + noRecords * recordSize) << endl;

      cerr << "headerLength = " << headerLength << endl;
      cerr << "recordSize = " << recordSize << endl;
      cerr << "noRecords = " << noRecords << endl;

      res->Set(true,-1 * (int)noRecords);
   }  else {
      res->Set(true,noRecords);
   }
   return 0;
}

ValueMapping noDB3RecordsVM[] = {
   noDB3RecordsVMT<CcString>,
   noDB3RecordsVMT<FText>
};

int noDB3RecordsSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec noDB3RecordsSpec(
  "{string, text} [x bool] -> int",
  "noDB3Records(filename, check)",
  "Retrieves the number of records stored in a db3 file. "
  "If the optional argument is given and true, the result "
  "read from the file is compared with the expected file "
  "size. If the computed file size and the real file size "
  "differ, the result will be negated.",
  "query noDB3Records('ten.dbf', TRUE) "
);

Operator noDB3RecordsOp(
  "noDB3Records",
  noDB3RecordsSpec.getStr(),
  2,
  noDB3RecordsVM,
  noDB3RecordsSelect,
  noDB3RecordsTM
);



/*
4.27 Operator ~createShx~

This operator scans a shape file and creates a idx file 
from it.

4.27.1 Type Mapping

*/
ListExpr createShxTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two erguments expected");
  }
  if(!CcString::checkType(nl->First(args))
     &&!FText::checkType(nl->First(args))){
    return listutils::typeError("first argument neihter a string nor a text");
  }
  if(!CcString::checkType(nl->Second(args))
     &&!FText::checkType(nl->Second(args))){
    return listutils::typeError("Second argument neihter a string nor a text");
  }
  return listutils::basicSymbol<CcBool>();
}


/*
4.27.2 Value Mapping

*/
template<class S, class T>
int createShxVMT(Word* args, Word& result,
                  int message, Word& local, Supplier s){
   
   result = qp->ResultStorage(s);
   CcBool* res = (CcBool*) result.addr;
   S* src = (S*) args[0].addr;
   T* dest = (T*) args[1].addr;
   if(!src->IsDefined() || !dest->IsDefined()){
     res->SetDefined(false);
     return 0;
   }
   ifstream in(src->GetValue().c_str(), ios::binary);
   if(!in){ // problem in opening input file
     res->Set(true,false);
     return 0;
   }
   ofstream out(dest->GetValue().c_str(), ios::binary);
   if(!out){ // problem in opening output file
     res->Set(true,false);
     return 0;
   }
   char* inbuf = new char[FILE_BUFFER_SIZE];
   char* outbuf = new char[FILE_BUFFER_SIZE];
   in.rdbuf()->pubsetbuf(inbuf,FILE_BUFFER_SIZE);
   out.rdbuf()->pubsetbuf(outbuf,FILE_BUFFER_SIZE);

   uint32_t code = readBigInt32(in);
   if(!in.good() || code!=9994){ // read error or not a shpe file
     res->Set(true,false);
     delete[] inbuf;
     delete[] outbuf;
     return 0;
   }   
   //in.seekg(24);
   //uint32_t length = readBigInt32(in);
   in.seekg(0);
   char header[100];
   in.read(header,100);
   if(!in){
     res->Set(true,false);
     delete[] inbuf;
     delete[] outbuf;
     return 0;
   } 
   // copy header into shx file
   out.write(header,100);
   uint32_t offset = 100;
   while(in.good() && !in.eof() ){
     uint32_t rn;  // record number
     in.read((char*)&rn,4); // read over record number
     uint32_t cl = readBigInt32(in); // content length
     if(in.good()){
        WinUnix::writeBigEndian(out,offset/2);
        WinUnix::writeBigEndian(out,cl);
     }
     offset += 8 + 2*cl;
     in.seekg(cl*2, ios_base::cur); // jump over content
   } 
   uint32_t shxLen = out.tellp() / 2; // in 16 bit words
   out.seekp(24);
   WinUnix::writeBigEndian(out,shxLen);
   in.close();
   out.close();
   res->Set(true,true);
   delete[] inbuf;
   delete[] outbuf;
   return 0;
}

ValueMapping createShxVM[] = {
   createShxVMT<CcString, CcString>,
   createShxVMT<CcString, FText>,
   createShxVMT<FText, CcString>,
   createShxVMT<FText, FText>
};

int createShxSelect(ListExpr args){
   int n1 = CcString::checkType(nl->First(args))?0:2;
   int n2 = CcString::checkType(nl->Second(args))?0:1;
   return n1+n2;
}

OperatorSpec createShxSpec(
   "{string,text} x {string,text} -> bool ",
   "createShx(_,_)",
   "Creates an shx file from a shape file.",
   "query createShx('Kinos.shp', 'Kinos.shx')"
);

Operator createShxOp(
  "createShx",
  createShxSpec.getStr(),
  4,
  createShxVM,
  createShxSelect,
  createShxTM
);



/*

25.9 Operator ~extractShpPart~

25.9.1 Type Mapping

The operator expects 4 arguments. 

  * the file name

  * the index of the first record

  * the index of the last record

  * a boolean value determining whether an index file should be created too

  * a boolean value for allowing a scan if necessary

  * a string or text defining the deistination file

Usually, this operator will use the shx file to determine the
position of the first record to extract. The shape file is 
scanned from this position until the end is reached or the 
maximum number of records have been extracted.  
If there is no shx file or the shape file size exceeds the 
limit of 4GB (maximum adressable file position in the shx file),
the last boolean argument determines whether  a scan of the 
original shape file from the beginning is allowed.

*/

ListExpr extractShpPartTM(ListExpr args){
  string err = "{string,text} x int x int x bool x bool  "
               "x {string, text} expected";
  if(!nl->HasLength(args,6)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(   !CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
    return listutils::typeError(err+ " (first arg is neither a string "
                                 "nor a text");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err+ " (second arg is not an int");
  }
  if(!CcInt::checkType(nl->Third(args))){
    return listutils::typeError(err+ " (Third arg is not an int");
  }
  if(!CcBool::checkType(nl->Fourth(args))){
    return listutils::typeError(err+ " (4th arg is not a bool");
  }
  if(!CcBool::checkType(nl->Fifth(args))){
    return listutils::typeError(err+ " (5th arg is not a bool");
  }
  if(   !CcString::checkType(nl->Sixth(args))
     && !FText::checkType(nl->Sixth(args))){
    return listutils::typeError(err+ " (6th arg is neither a string "
                                 "nor a text");
  }
  return listutils::basicSymbol<CcInt>();
}



void updateBox(const uint32_t type, char* buffer, 
               uint32_t contentLength,
               double& minX, double& minY,
               double& maxX, double& maxY,
               bool& first){

   if(contentLength==0){
     return;
   }
   contentLength = contentLength*2; // in bytes

   double lminX;
   double lminY;
   double lmaxX;
   double lmaxY;
   // determine values for lminX and so on from the block
   bool le = WinUnix::isLittleEndian();
   const uint32_t btype = *(reinterpret_cast<uint32_t*>(buffer));
   if(btype==0){ // a null shape does not contibute to the bbox
     return; 
   }
   if(btype!=type){ // should not happen
     cerr << "the record shape type does not corresponds "
             "to the file shape type" << endl;
     return;
   }
   switch(type){
     case 1: {
               if(contentLength<20){
                 cerr << "found invalid content length for a point" << endl;
                 return;
               }
               uint64_t ix = * (reinterpret_cast<uint64_t*>(buffer+4));
               uint64_t iy = * (reinterpret_cast<uint64_t*>(buffer+12));
               if(!le){
                 ix = WinUnix::convertEndian(ix);
                 iy = WinUnix::convertEndian(iy);
               }
               void* d = &ix;
               lminX = *(reinterpret_cast<double*>(d));
               lmaxX = lminX;
               d = &iy;
               lminY = *(reinterpret_cast<double*>(d));
               lmaxY = lminY;
               break;
             }
     case 3:
     case 5:
     case 8: {
                if(contentLength<36){
                 cerr << "found invalid content length "
                         "for a spatial object" << endl;
                 return;
               }
               uint64_t ix1 = *(reinterpret_cast<uint64_t*>(buffer+4));
               uint64_t iy1 = *(reinterpret_cast<uint64_t*>(buffer+12));
               uint64_t ix2 = *(reinterpret_cast<uint64_t*>(buffer+20));
               uint64_t iy2 = *(reinterpret_cast<uint64_t*>(buffer+28));
               if(!le){
                ix1 = WinUnix::convertEndian(ix1);
                ix2 = WinUnix::convertEndian(ix2);
                iy1 = WinUnix::convertEndian(iy1);
                iy2 = WinUnix::convertEndian(iy2);
               }
               void* d = &ix1;
               lminX = *( reinterpret_cast<double*>(d));
               d = &iy1;
               lminY = *( reinterpret_cast<double*>(d));
               d=&ix2;
               lmaxX = *( reinterpret_cast<double*>(d));
               d = &iy2;
               lmaxY = *( reinterpret_cast<double*>(d));
               break; 
             }
     default: assert(false); // type not supported
   }

   if(first){
      minX = lminX;
      minY = lminY;
      maxX = lmaxX;
      maxY = lmaxY;
      first = false;
   } else {
      minX = min(minX,lminX);
      minY = min(minY,lminY);
      maxX = max(maxX,lmaxX);
      maxY = max(maxY,lmaxY);
   }

} 



int extractShpPart( const string& shpFile,
                    uint32_t i1, uint32_t i2,
                    bool cx, bool as,
                    const string& targetFile){

  if(!stringutils::endsWith(shpFile,".shp" )){
    cerr << "source file name does not end with .shp" << endl;
    return 0;
  }
  if(!stringutils::endsWith(targetFile,".shp")){
    cerr << "target file name does not end with .shp" << endl;
    return 0;
  }

  if(i1>i2){
    swap(i1,i2);
  }
  
  i1++; // force counting at record 1 as in shp file
  i2++;


  // try to open the file and determine the file size
  ifstream inshp(shpFile.c_str(), ios::binary);
  if(!inshp){ // shape file could not be opened
    cerr << "could not open source shape file" << endl;
    return 0;
  }

  // check for shape file of supported type
  uint32_t code = readBigInt32(inshp);
  if(!inshp.good() || code != 9994){
    cerr << "source file not recognized to be a shape file" << endl;
    inshp.close();
    return 0;
  }
  inshp.seekg(32);
  uint32_t type = readLittleInt32(inshp);
  if(!inshp.good()){
    cerr << "could not extract type of the shape file" << endl;
    inshp.close();
    return 0;
  }
  if(type!=1 && type !=3 && type!=5 && type!=8){
    cerr << "unsupported type " << type << endl; 
    inshp.close();
    return 0;
  }
  // determine size of the file
  uint64_t one = 1; 
  uint64_t maxsize = 2*((one << 32)-1); 


  inshp.seekg(0,ios::end);
  uint64_t filesize = inshp.tellg();


  bool indexAvailable = filesize < maxsize;


  uint64_t firstPos = 0;

  if(indexAvailable){
     ifstream inshx((shpFile.substr(0,shpFile.length()-4)+".shx").c_str(),
                    ios::binary);
     if(!inshx){
        indexAvailable = false;
     } else {
       uint64_t i1pos = 100 + 8*(i1-1);
       inshx.seekg(i1pos);
       if(!inshx.good()){ // outside the index file
         inshp.close();
         inshx.close();
         return 0;
       } 
       firstPos = readBigInt32(inshx);
       if(!inshx.good()){ // after the last position
          inshp.close();
          inshx.close();
          return 0;
       }
       inshx.close(); // index file is processed
    }
  }
  if(indexAvailable){
     firstPos = firstPos * 2; // convert 16Bit to byte
  }

  char* buffer = new char[FILE_BUFFER_SIZE];
  inshp.rdbuf()->pubsetbuf(buffer,FILE_BUFFER_SIZE);


  if(!indexAvailable){
    if(!as){
      // impossible to determine the start position
      // without scanning the main file
      cerr << "index not available and scan is not allowed" << endl;
      inshp.close();
      delete[] buffer;
      return 0;
    }
    inshp.seekg(100);

    // search record i1 by scanning the main file
    uint32_t record = 1;
    while(inshp.good() && !inshp.eof() && record < i1){
      uint32_t rec = readBigInt32(inshp);
      if(rec!=record){
        cerr << "wrong record numbering in shape file found" << endl;
      } 
      uint32_t cl = readBigInt32(inshp); // content length
      // jump over content
      inshp.seekg(cl*2,ios::cur);
      record++;
    }
    if(!inshp.good() || record!=i1){
      // index outside file
      cerr << "first index outside the file" << endl;
      inshp.close();
      delete[] buffer;
      return 0;
    }
    firstPos = inshp.tellg();

  }
  // ok, the first index is in the file, at a first step, we 
  // copy the header of the main file into the target file
  // and return the the position of i1  
  ofstream outshp(targetFile.c_str(),ios::binary);
  ofstream outshx;
  char* outBuffer2 = 0;
  if(cx){
    outshx.open((targetFile.substr(0,targetFile.length()-4)+".shx").c_str(),
                  ios::binary);
     
  }
  if(!outshp){
    cerr << "could not open output file" << endl;
    delete[] buffer;
    return 0;
  }
  char* outBuffer1 = new char[FILE_BUFFER_SIZE]; 
  outshp.rdbuf()->pubsetbuf(outBuffer1,FILE_BUFFER_SIZE);

  if(cx && outshx.good()){
    outBuffer2 = new char[FILE_BUFFER_SIZE];
    outshx.rdbuf()->pubsetbuf(outBuffer2, FILE_BUFFER_SIZE);
  }

  // copy header
  inshp.seekg(0);
  char header[100];
  inshp.read(header,100);
  outshp.write(header,100);
  if(outBuffer2){
    outshx.write(header,100);
  } 

  inshp.seekg(firstPos);

  double minX =0;
  double maxX =0;
  double minY =0;
  double maxY = 0;
  bool first = true;
  uint32_t count = 0;

  while(inshp.good() && !inshp.eof() && i1 <= i2){
    // read next element from shape file
    uint32_t rec = readBigInt32(inshp);
    if(rec!=i1){
      cerr << "found invalid numbering in shape file" << endl;
    }
    uint32_t cl = readBigInt32(inshp);
    if(inshp.good()){
      char* recbuf = new char[2*cl];
      inshp.read(recbuf,cl*2);
      if(inshp.good()){
        uint32_t p = outshp.tellp() / 2;
        if(outBuffer2){ // write shx file if required
           WinUnix::writeBigEndian(outshx,p);
           WinUnix::writeBigEndian(outshx,cl);    
        }
        count++;
        WinUnix::writeBigEndian(outshp, count);
        WinUnix::writeBigEndian(outshp, cl);
        outshp.write(recbuf,2*cl); 
        updateBox(type,recbuf,cl,minX,minY,maxX,maxY,first);
        i1++;
      }
      delete[] recbuf;
    }
  }

  // update header information
  uint32_t fileLength = outshp.tellp()/2;
  outshp.seekp(24);
  WinUnix::writeBigEndian(outshp,fileLength);
  outshp.seekp(36);
  WinUnix::writeLittle64(outshp,minX);
  WinUnix::writeLittle64(outshp,minY);
  WinUnix::writeLittle64(outshp,maxX);
  WinUnix::writeLittle64(outshp,maxY);
  outshp.close();
  delete[] outBuffer1;
  if(outBuffer2){
     fileLength = outshx.tellp()/2;
     outshx.seekp(24);
     WinUnix::writeBigEndian(outshx,fileLength);
     outshx.seekp(36);
     WinUnix::writeLittle64(outshx,minX);
     WinUnix::writeLittle64(outshx,minY);
     WinUnix::writeLittle64(outshx,maxX);
     WinUnix::writeLittle64(outshx,maxY);
     outshx.close();
     delete[] outBuffer2;
  } 
  delete[] buffer;
  return count; 
}


template<class T, class F>
int extractShpPartVMT(Word* args, Word& result,
                      int message, Word& local, Supplier s){

  T* fn = (T*) args[0].addr;
  CcInt*  i1 = (CcInt*) args[1].addr;
  CcInt*  i2 = (CcInt*) args[2].addr;
  CcBool* cx = (CcBool*) args[3].addr;
  CcBool* as = (CcBool*) args[4].addr;
  F* fnt = (F*) args[5].addr;

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;


  if(   !fn->IsDefined() || !i1->IsDefined()
     || !i2->IsDefined() || !cx->IsDefined()
     || !as->IsDefined() || !fnt->IsDefined()){
    res->SetDefined(false);
    return 0;
  }

  res->Set(true, extractShpPart( fn->GetValue(),
                                 i1->GetValue(),
                                 i2->GetValue(),
                                 cx->GetValue(),
                                 as->GetValue(),
                                 fnt->GetValue()));
  return 0;
}


ValueMapping extractShpPartVM[] = {
   extractShpPartVMT<CcString, CcString>,
   extractShpPartVMT<CcString, FText>,
   extractShpPartVMT<FText, CcString>,
   extractShpPartVMT<FText, FText>
};

int extractShpPartSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Sixth(args))?0:1;
  return n1+n2;
}

OperatorSpec extractShpPartSpec(
  "{string,text} x int x int x bool x  bool x {string,text} -> int",
  "extractSphPart(source, minRecord, maxRecord, createShx, allowScan, target)",
  "Extracts a part of a shape file and stores this part into another "
  "shape file. The first argument is the name of the source file "
  "including the .shp extension. The secnnd and the third argument "
  "determine the range of records taht should be extracted. Counting of "
  "records starts with zero."  
  "The first boolean argument determines whether an index (shx) file should "
  "be created for the extracted part. "
  "In the normal case, this operator uses an index (shx) file "
  "to determine the begin of the first record within  the shape file."
  "This is impossible if the shape file has a size of more than 8GB or "
  "of course if no index file is present. " 
  "In such a case, a scan within the main file up to the first "
  "record to extract is reqwired. Because this is expensive, the fifth "
  "argument controls whether such a scan is allowed. "
  "The last argument is the name of the target file including the shp "
  "extension.",
  "query extractShpPart('buildings.shp',1000, 1500, TRUE, "
  "TRUE, 'buildings_1000_1500')"
);


Operator extractShpPartOp(
  "extractShpPart",
  extractShpPartSpec.getStr(),
  4,
  extractShpPartVM,
  extractShpPartSelect,
  extractShpPartTM
);


/*
24.93 Operator ~extractDB3Part~

This operator extracts a part of a db3 file into a new db3 file.

24.93.1 Type Mapping

Arguments are:
    
  * the source file name

  * the start index

  * the end index

  * the target file name

*/
ListExpr extractDB3PartTM(ListExpr args){
   string err="{string, text} x int x int x {string, text} expected";
   if(!nl->HasLength(args,4)){
      return listutils::typeError(err + " (wrong number of arguments");
   }
   if(   !CcString::checkType(nl->First(args))
      && !FText::checkType(nl->First(args))){
    return listutils::typeError(err + " (first arg is neither a string"
                                      " nor a text)");
   }
   if(!CcInt::checkType(nl->Second(args))){
     return listutils::typeError(err + " (second arg is not an int)");
   }
   if(!CcInt::checkType(nl->Third(args))){
     return listutils::typeError(err + " (third arg is not an int)");
   }
   if(   !CcString::checkType(nl->Fourth(args))
      && !FText::checkType(nl->Fourth(args))){
    return listutils::typeError(err + " (fourth arg is neither a string"
                                      " nor a text)");
   }
   return listutils::basicSymbol<CcInt>();
}



int extractDB3Part(string source, int first, int last, string target){

   if(first>last){
     swap(first,last);
   }
   if(first<0){
     cerr << "invalid first tuple" << endl;
     return 0;
   }
   ifstream in(source.c_str(),ios::binary);
   if(!in.good()){
     cerr << "could not open source file " << source << endl;
     return 0;
   }
   char* inBuffer = new char[FILE_BUFFER_SIZE];
   in.rdbuf()->pubsetbuf(inBuffer,FILE_BUFFER_SIZE);
   unsigned char version;
   in.read((char*)(&version),1);
   if(!in.good() || (version !=0x02 && version!=0x03 && version!=0x83)){
     cerr << "file " << source << " is not a supported d-base file" << endl;
     delete[] inBuffer;
     return 0;
   }
   if(version==0x83){
     cerr << "memo field support not implemented yet" << endl;
     delete[] inBuffer;
     return 0;
   }
   

   // determine number of records in file
   in.seekg(4, ios::beg);
   uint32_t noRecords = readLittleInt32(in);
   if(!in.good() || (uint32_t)first>=noRecords){
     cerr << "too less records in file " << endl;
     delete[] inBuffer;
     return 0;
   }
   // determine length of the header
   uint16_t headerLength = readLittleInt16(in);
   uint16_t recordLength = readLittleInt16(in);

   if((uint64_t)last>=noRecords){
     last = noRecords - 1;
   }
   // copy db3 header
   char* header = new char[headerLength];
   in.seekg(0,ios::beg);
   in.read(header, headerLength);
   // time to open the output file
   ofstream out(target.c_str(), ios::binary);
   if(!out){
     cerr << "could not open output file";
     delete[] inBuffer;
     delete[] header;
     return 0;
   }
   // write header to output file
   char* outBuffer = new char[FILE_BUFFER_SIZE];
   out.rdbuf()->pubsetbuf(outBuffer, FILE_BUFFER_SIZE);

   out.write(header, headerLength);
   
   uint64_t bytes = ((last + 1) - first) * recordLength;
   // navigate to start position in in file
   in.seekg(first*recordLength, ios::cur);
   size_t bs = 16*1024;
   char buffer[bs]; // copy buffer by buffer
   while(bytes>0 && in.good() && !in.eof()){
      size_t read = bytes>bs?bs:bytes;
      in.read(buffer,read);
      out.write(buffer,read);
      bytes -= read;  
   }  
    
   if(bytes>0){
     cerr << "error in copying data";
     delete[] inBuffer;
     delete[] outBuffer;
     delete[] header;
     return 0;
   } 
   // update record number in target file
   out.seekp(4);
   noRecords = (last+1) - first;
   WinUnix::writeLittleEndian(out,noRecords);
   in.close();
   out.close();
   delete[] inBuffer;
   delete[] outBuffer;
   delete[] header;
   return noRecords; 
}


template<class T, class F>
int extractDB3PartVMT(Word* args, Word& result,
                      int message, Word& local, Supplier s){

  T* fn = (T*) args[0].addr;
  CcInt*  i1 = (CcInt*) args[1].addr;
  CcInt*  i2 = (CcInt*) args[2].addr;
  F* fnt = (F*) args[3].addr;

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;


  if(   !fn->IsDefined() || !i1->IsDefined()
     || !i2->IsDefined() || !fnt->IsDefined()){ 
    res->SetDefined(false);
    return 0;
  }

  res->Set(true, extractDB3Part( fn->GetValue(),
                                 i1->GetValue(),
                                 i2->GetValue(),
                                 fnt->GetValue()));
  return 0;
}


ValueMapping extractDB3PartVM[] = {
   extractDB3PartVMT<CcString, CcString>,
   extractDB3PartVMT<CcString, FText>,
   extractDB3PartVMT<FText, CcString>,
   extractDB3PartVMT<FText, FText>
};

int extractDB3PartSelect(ListExpr args){
  int n1 = CcString::checkType(nl->First(args))?0:2;
  int n2 = CcString::checkType(nl->Fourth(args))?0:1;
  return n1+n2;
}


OperatorSpec extractDB3PartSpec(
  "{string, text} x int x int x {string, text} -> int",
  "extractDB3Part(source, first, last, target)",
  "Extracts a portion given be first and last from a "
  "d-Base III file and stores this part into a "
  "new D-Base II file. As usual counting of records "
  "starts with zero",
  "query extractDB3Part('ten.dbf', 2,8, 'ten_2_8.dbf')"
);


Operator extractDB3PartOp(
  "extractDB3Part",
  extractDB3PartSpec.getStr(),
  4,
  extractDB3PartVM,
  extractDB3PartSelect,
  extractDB3PartTM
);



/*
24.29 Operator ~splitShp~

This operator splits a shape file into a series of shape files 
having at most a certain number of elements stored.

*/
ListExpr splitShpTM(ListExpr args){
  string err = "{string,text} x int [x bool] expected";
  if(!nl->HasLength(args,2) && !nl->HasLength(args,3)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(   !CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
   return listutils::typeError(err + " (first arg is neither a "
                               "string nor a text)");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err + " (second arg in not an int)");
  }
  if(nl->HasLength(args,2)){
     return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                              nl->OneElemList(nl->BoolAtom(false)),
                              listutils::basicSymbol<CcInt>());
  }
  if(!CcBool::checkType(nl->Third(args))){
    return listutils::typeError(err + " ( third arg is not a bool)");
  }
  return listutils::basicSymbol<CcInt>();
}


template<class T>
int splitShpVMT(Word* args, Word& result,
                int message, Word& local, Supplier s){
   result = qp->ResultStorage(s);
   CcInt* res = (CcInt*) result.addr;
   T* fn = (T*) args[0].addr;
   CcInt* norecs = (CcInt*) args[1].addr;
   CcBool* cxs   = (CcBool*) args[2].addr;

   if(!fn->IsDefined() || !norecs->IsDefined() || !cxs->IsDefined()){
      res->SetDefined(false);
      return 0;
   }

   bool cx = cxs->GetValue();
   int maxnorec = norecs->GetValue();
   if(maxnorec < 1){
     res->SetDefined(false);
     return 0;
   }
   string f = fn->GetValue();
   if(!stringutils::endsWith(f,".shp")){
     cerr << "invalid file name, missing extension shp" << endl;
     res->Set(true,0);
     return 0;
   }

   ifstream in(f.c_str(), ios::binary);
   if(!in){
     cerr << "could not open file " << f << endl;
     return 0;
   }  
   in.seekg(0,ios::end);
   streampos fileSize = in.tellg();
   if(fileSize < 100){
     cerr << "invalid file (too short)" << endl;
     res->Set(true,0);
     return 0;
   } 
   in.seekg(0,ios::beg);
   // read some header information
   uint32_t code = readBigInt32(in);
   if(code!=9994){
     cerr << "file " << f << " is not a shape file" << endl;
     res->Set(true,0);
     return 0;
   }

   in.seekg(24);
   streampos fs = readBigInt32(in);
    
   if(fs*2 != fileSize){
     cerr << "invalid  file size stored, give up" << endl;
     res->Set(true,0);
     return 0;
   }


   in.seekg(32);
   uint32_t type = readLittleInt32(in);
   if(type!=1 && type!=3 && type!=5 && type!=8){
     cerr << "unsupported shape type " << type << endl;
     res->Set(true,0);
     return 0;
   }
   // header information ok, get the whole header
   char* inBuffer = new char[FILE_BUFFER_SIZE];
   in.rdbuf()->pubsetbuf(inBuffer, FILE_BUFFER_SIZE);
   char header[100];
   in.seekg(0);
   in.read(header,100);

   uint32_t partition = 0;
   ofstream* out = 0;
   ofstream* outx = 0;

   string base = f.substr(0,f.length()-4);   
   char* outBuffer1 = new char[FILE_BUFFER_SIZE];
   char* outBuffer2 = new char[FILE_BUFFER_SIZE];

   uint32_t recno = 0;
   bool firstRecord=true;

   while(in.good() && (in.tellg() < fileSize)){
      if(!out){ // start a new partition
         out = new ofstream(  (base + "_" 
                             +  stringutils::int2str(partition)
                             + ".shp").c_str(),
                            ios::binary);
         out->rdbuf()->pubsetbuf(outBuffer1, FILE_BUFFER_SIZE);
         out->write(header,100);
         recno = 0;
         if(cx){
           outx = new ofstream(   (base +"_"
                                 + stringutils::int2str(partition)
                                 + ".shx").c_str(),
                               ios::binary);
           outx->rdbuf()->pubsetbuf(outBuffer2, FILE_BUFFER_SIZE);
           outx->write(header,100);
         }
         partition++;
         firstRecord = true;
      }
      // copy records until end of file or maximum number of records 
      // for a single partition is reached
      double minX=0;
      double minY=0;
      double maxX=0;
      double maxY=0;
      while(    in.good() && (recno < (uint32_t) maxnorec) 
            && (in.tellg()<fileSize)){
         in.seekg(4,ios::cur); // ignore original record number
         uint32_t cl = readBigInt32(in);
         char* record = new char[2*cl];
         in.read(record, 2*cl);
         if(in.good()){
            updateBox(type, record, cl, minX,minY,maxX,maxY,firstRecord);
            uint32_t offset = out->tellp()/2;
            recno++;
            WinUnix::writeBigEndian(*out, recno);
            WinUnix::writeBigEndian(*out,cl);
            out->write(record, 2*cl);
            if(outx){
               WinUnix::writeBigEndian(*outx, offset);
               WinUnix::writeBigEndian(*outx, cl);
            }
         }
         delete[] record;
      }
      uint32_t fs = out->tellp()/2;
      if(fs>50) { // more than header size
         // update file length and bounding box in header
         out->seekp(24, ios::beg);
         WinUnix::writeBigEndian(*out,fs);
         out->seekp(36,ios::beg);
         WinUnix::writeLittle64(*out,minX);
         WinUnix::writeLittle64(*out,minY);
         WinUnix::writeLittle64(*out,maxX);
         WinUnix::writeLittle64(*out,maxY);
         if(outx){
            fs= outx->tellp()/2;
            outx->seekp(24, ios::beg);
            WinUnix::writeBigEndian(*outx,fs);
            outx->seekp(36,ios::beg);
            WinUnix::writeLittle64(*outx,minX);
            WinUnix::writeLittle64(*outx,minY);
            WinUnix::writeLittle64(*outx,maxX);
            WinUnix::writeLittle64(*outx,maxY);
         } 
      }
      out->close();
      delete out;
      out = 0;
      if(outx){
         outx->close();
         delete outx;
         outx=0;
      }
   }
   delete[] inBuffer;
   delete[] outBuffer1;
   delete[] outBuffer2;
   res->Set(true,partition);
   return 0;
}


ValueMapping splitShpVM[] = {
   splitShpVMT<CcString>,
   splitShpVMT<FText>
};

int splitShpSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec splitShpSpec(
  "{string, text} x int [ x bool] -> int ",
  "spitShp(source, maxRecNo, createIndex) ",
  "Split a single shape file into a serie of "
  "shape files each having atmost maxRecNo entries. "
  "Of course, the last partition may have less entries."
  "The partition names come from the original name with appended "
  "_<partition_numer>, where numbering starts with zero. "
  "If the optional boolean argument is given and true, "
  "for each created partition a corresponding shx file is created too.",
  "query splitShp('buildings.shp', 600000, TRUE)"
);

Operator splitShpOp(
  "splitShp",
  splitShpSpec.getStr(),
  2,
  splitShpVM,
  splitShpSelect,
  splitShpTM
);


/*
24.29 Operator ~splitDB3~

This operator splits a dbase-III file into a series of dbase-III 
files having at most a certain number of elements stored.

*/
ListExpr splitDB3TM(ListExpr args){
  string err = "{string,text} x int [x bool] expected";
  if(!nl->HasLength(args,2) ){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(   !CcString::checkType(nl->First(args))
     && !FText::checkType(nl->First(args))){
   return listutils::typeError(err + " (first arg is neither a "
                               "string nor a text)");
  }
  if(!CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err + " (second arg in not an int)");
  }
  return listutils::basicSymbol<CcInt>();
}


template<class T>
int splitDB3VMT(Word* args, Word& result,
                int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  T* fn  = (T*) args[0].addr;
  CcInt* rn = (CcInt*) args[1].addr;
  if(!fn->IsDefined() || !rn->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  string f = fn->GetValue();
  int r = rn->GetValue();
  if(r<1){
    res->SetDefined(false);
    return 0;
  }
  if(!stringutils::endsWith(f,".dbf")){
    cerr << "file name does not end with .dbf" << endl;
    res->SetDefined(false);
    return 0;
  }

  ifstream in(f.c_str(), ios::binary);
  if(!in.good()){
    cerr << "could not open file " << f << endl;
    res->SetDefined(false);
    return 0;
  }
  unsigned char version;
  in.read((char*)(&version),1);

  if(!in.good() || ((version!=0x02) && (version!=0x03) && (version!=0x83))){
    cerr << "file " << f << " is not a supported d-base file" << endl;
    res->SetDefined(false);
    return 0;
  }
  if(version==0x83){
    cerr << "Db-III file with Memofields are not supported yet" << endl;
    res->Set(true,0);
    return 0;
  } 
  char* inBuffer = new char[FILE_BUFFER_SIZE];
  in.rdbuf()->pubsetbuf(inBuffer, FILE_BUFFER_SIZE);
  in.seekg(4);
  uint32_t noRecords = readLittleInt32(in);
  uint16_t headerLength = readLittleInt16(in);
  uint16_t recordLength = readLittleInt16(in);
  char* header = new char[headerLength];
  in.seekg(0);
  in.read(header, headerLength);
  if(!in.good()){
    cerr << "error in reading header from file " << f << endl;
    res->Set(true,0);
    delete[] header;
    delete[] inBuffer;
    return 0;
  }  
  
  char* outBuffer = new char[FILE_BUFFER_SIZE];

  uint32_t partition = 0;
  ofstream* out = 0;
  string base = f.substr(0,f.length()-4);
  size_t bytes2copy;

  uint32_t bs = 16*1024;
  char buf[bs];


  while(in.good() && !in.eof() && noRecords>0){
      out = new ofstream((  base + "_"
                          + stringutils::int2str(partition)
                          + ".dbf").c_str(),
                          ios::binary);
     out->rdbuf()->pubsetbuf(outBuffer, FILE_BUFFER_SIZE);
     uint32_t records2Copy = (uint32_t)r < noRecords?r: noRecords;
     bytes2copy = records2Copy*recordLength;
     noRecords -= records2Copy;
     // change noRecords within header
     if(!WinUnix::isLittleEndian()){
       WinUnix::convertEndian(records2Copy);
     }
     memcpy(header+4,&records2Copy,4);
     out->write(header, headerLength);
     partition++;
     while(bytes2copy>0){
        size_t b = bs<bytes2copy?bs:bytes2copy;
        in.read(buf,b);
        out->write(buf,b);
        bytes2copy -= b;
     }
     unsigned char dataEndMarker=0x1a;
     out->write((char*)(&dataEndMarker),1);
     out->close();
     delete out; 
  }
  delete[] inBuffer;
  delete[] outBuffer;
  delete[] header;
  res->Set(true,partition);
  return 0;
}

ValueMapping splitDB3VM[] = {
  splitDB3VMT<CcString>,
  splitDB3VMT<FText>
};

int splitDB3Select(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

OperatorSpec splitDB3Spec(
  "{string, text} x int -> int",
  "splitDB3(fileName, noRecords) ",
  "Splits a d-base III file into a serie of such files "
  "echa containing at most noRecords entries (the last file "
  "may have less entries. "
  "The filenames are taken from the input file name, extended by "
  "an underscore and the partition number. "
  "The operator returns the number of created partitions.",
  "query splitDB3('ten.dbf',2)"
);

Operator splitDB3Op(
  "splitDB3",
  splitDB3Spec.getStr(),
  2,
  splitDB3VM,
  splitDB3Select,
  splitDB3TM

);

/*
24.99 Operator ~aisimport~

24.99.1 Type Mapping

*/
ListExpr importaisTM(ListExpr args){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("two Arguments expected");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(!nl->HasLength(arg1,2) || !nl->HasLength(arg2,2)){
    return listutils::typeError("internal error");
  }
  arg1 = nl->First(arg1); // extract type

  if(!CcString::checkType(arg1) && !FText::checkType(arg1)){
    return listutils::typeError("expected string or text as first argument");
  }
  if(!CcString::checkType(nl->First(arg2))){
    return listutils::typeError("expected string as second argument");
  }

  arg2 = nl->Second(arg2);
  if(nl->AtomType(arg2)!=StringType){
     return listutils::typeError("second argument must be a constant string");
  }
  string prefix = nl->StringValue(arg2);
  if(!stringutils::isIdent(prefix)){
     return listutils::typeError("second argument is not an valid identifier");
  }

  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  // check for the 27 message types whether there is already a relation
  for(int mt  = 1 ; mt <28; mt++){
     string rname = prefix + "_" + stringutils::int2str(mt);
     if(ctlg->IsObjectName(rname)){
       return listutils::typeError(rname + " is already present");
     }
  }
  return listutils::basicSymbol<CcInt>();
}


/*
24.99.2 Info class

*/
struct ClusterInfo{


  bool operator<(const ClusterInfo& ci) const{
    if(minTime < ci.minTime) return true;
    if(minTime > ci.minTime) return false;
    if(maxTime < ci.maxTime) return true;
    if(maxTime > ci.maxTime) return false;
    return count < ci.count;
  }


  datetime::DateTime minTime;
  datetime::DateTime maxTime;
  size_t count;

  ostream& print(ostream& out)const{
    out << "[ " << minTime << " , " << maxTime << "] : " << count;
    return out;
  }

  bool contains(const datetime::DateTime& time) const{
    return (time>=minTime) && (time<=maxTime);
  }


  ClusterInfo(datetime::DateTime& dt): minTime(dt), maxTime(dt), count(1){}

  void add(const ClusterInfo& cl){
     if(minTime > cl.minTime){
        minTime = cl.minTime;
     }
     if(maxTime < cl.maxTime){
        maxTime = cl.maxTime;
     }
     count += cl.count;
  }

  void add(const datetime::DateTime& dt){
     count++;
     if(dt < minTime){
        minTime = dt;
        return;
     }
     if(dt > maxTime) {
        maxTime = dt;
     }
  }

  datetime::DateTime distance(const datetime::DateTime& dt)const{
     if(dt<minTime){
       return minTime - dt;
     }
     if(dt > maxTime){
       return dt - maxTime;
     }
     datetime::DateTime r(datetime::durationtype, 0);
     return r;
  }
   
  datetime::DateTime distance(const ClusterInfo& ci) const{
     if(minTime > ci.maxTime){
         return minTime - ci.maxTime;
     }
     if(maxTime < ci.minTime){
         return ci.minTime - maxTime;
     }
     return datetime::DateTime(datetime::durationtype,0);
  }

};


ostream& operator<<(ostream& o, const ClusterInfo& ci){
   return ci.print(o);
}


struct Type4Info{

Type4Info():mmsi(0),count(0),failed(0),clusters(){
  uint64_t t = 1000*60*60;
  threshold =datetime::DateTime(datetime::durationtype,t);
}

Type4Info(aisdecode::Message4* msg): mmsi(msg->mmsi), count(0), failed(0), 
                                     clusters(){
  uint64_t t = 1000*60*60;
  threshold =datetime::DateTime(datetime::durationtype,t);
  add(msg);
}

Type4Info(const Type4Info& ti): mmsi(ti.mmsi), count(ti.count), 
                               failed(ti.failed), clusters(ti.clusters){
  uint64_t t = 1000*60*60;
  threshold =datetime::DateTime(datetime::durationtype,t);
}

bool contains(const datetime::DateTime& time){
   for(size_t i=0;i<clusters.size();i++){
      if(clusters[i].contains(time)){
          return true;
      }
   }
   return false;
}


Type4Info& operator=(const Type4Info& ti){
  mmsi = ti.mmsi;
  count = ti.count;
  failed = ti.failed;
  clusters = ti.clusters;
  return *this;
}



void add(aisdecode::Message4* msg) {
   assert(msg->mmsi==this->mmsi);
   count++;
   if(msg->year<1 || msg->month<1 || msg->day<1){
      failed++;
      return;
   }
   if(msg->year>9999 || msg->month>12 || msg->day>31){
      failed++;
      return;   
   }
   if(msg->hour > 23 || msg->minute>59 || msg->second>59){
      failed++;
      return;
   }

   datetime::DateTime time(datetime::instanttype);
   if(!time.IsValid(msg->year, msg->month, msg->day)){
      failed++;
      return;
   }
   time.Set(msg->year, msg->month, msg->day, msg->hour, msg->minute, 
            msg->second);
      
   insert(time); 
}

/*
If the is a cluster containing time, the count of the cluster is increased..
otherwise the nearest cluster is determined. if the distance is smaller than a 
threshold, the clustr

*/
void insert(datetime::DateTime& time){
  if(clusters.empty()){
    ClusterInfo ci(time);
    clusters.push_back(ci);
    return;
  }
  int minIndex = 0;
  datetime::DateTime minDist = clusters[0].distance(time);
  for(size_t i=1;i<clusters.size();i++){
    datetime::DateTime dist = clusters[i].distance(time);
    if(dist < minDist){
        minIndex = i;
        minDist = dist;
    }
  }
  // threshold 1 hour
  if(minDist < threshold){
    clusters[minIndex].add(time);
  } else {
    clusters.push_back(ClusterInfo(time));
  }
}

ostream& print(ostream& out)const{
  out << "---------" << endl;
  out << "MMSI : " << mmsi << endl;
  out << "#messages : " << count << endl;
  out << "invalid messages : " << failed << endl;
  out << "clusters" << endl;
  for(size_t i=0;i<clusters.size();i++){
     out << "   " << clusters[i] << endl;
  }
  out << "---------" << endl;
  return out;
};

void mergeCluster(){
   if(clusters.size() < 2) return;
   sort(clusters.begin(),clusters.end());
   vector<ClusterInfo> merged;
   merged.push_back(clusters[0]);
   int pos = 0;
   for(size_t i=1;i<clusters.size();i++){
      datetime::DateTime dist = merged[pos].distance(clusters[i]);
      if(dist<threshold){
         merged[pos].add(clusters[i]);
      } else {
         merged.push_back(clusters[i]);
         pos++;
      }
   }
   swap(clusters, merged);
}

size_t largestClusterSize() const{
  size_t res = 0;
  for(size_t i=0;i<clusters.size();i++){
     if(clusters[i].count > res){
        res = clusters[i].count;
     }
  }
  return res;
} 


void add(const Type4Info& t4i){
   count += t4i.count;
   failed += t4i.failed;
   for(size_t i=0;i<t4i.clusters.size();i++){
      clusters.push_back(t4i.clusters[i]);
   }
}


void purge(){
   double minClusterSize = largestClusterSize()* 0.01;
   vector<ClusterInfo> purged;
   for(size_t i=0;i<clusters.size();i++){
      if(clusters[i].count >= minClusterSize){
         purged.push_back(clusters[i]);
      }
   }
   swap(purged,clusters);
}

void clear(){
  mmsi = 0;
  count = 0;
  failed = 0;
  clusters.clear();
}  

int mmsi;
size_t count;
size_t failed;
vector<ClusterInfo> clusters;
datetime::DateTime threshold;

};


ostream& operator<<(ostream& o, const Type4Info& ti){
  return ti.print(o);
}


class aisimportInfo{

  public:
    aisimportInfo(const string& _filename,
                  const string&  _prefix): filename(_filename),
                                           prefix(_prefix),
                                           time(datetime::instanttype,0)
                                            {
      cout << "decode file " << _filename << endl;
      time.SetDefined(false);
      for(int i=0;i<27;i++){
        relations.push_back(0);
        tupleTypes.push_back(0);
      }
      ctlg = SecondoSystem::GetCatalog();
      noUsedReason=0;
    }

    ~aisimportInfo(){
      for(int i=0;i<27;i++){
         if(relations[i]){
           // relation deletion is handled by catalog
           tupleTypes[i]->DeleteIfAllowed();
         }
      }
    }

    int operator()(){

       count =0;
       aisdecode::MessageBase* msg;
       aisdecode::aisdecoder* dec = new aisdecode::aisdecoder(filename);
       // proprocessing of type 4 messages
       map<int,Type4Info> mmsicount;
       map<int,Type4Info>::iterator it;
       while( (msg = dec->getNextMessage(4))){
         count++;
         aisdecode::Message4* msg4 = (aisdecode::Message4*) msg;
         it = mmsicount.find(msg4->mmsi);
         if(it==mmsicount.end()){
            mmsicount[msg4->mmsi] = Type4Info(msg4);
         } else {
            it->second.add(msg4);
         }
         delete msg;
       }
       // connect cluster for each mmsi
       for(it=mmsicount.begin(); it!=mmsicount.end();it++){
         it->second.mergeCluster();
       }

       // build a set of forbidden mmsi
       // an mmsi is forbidden if
       //  number of messages < 2 
       //  number of messages < 10% than the average per mmsi
       //  number of invalid messages > 10% for this mmsi
       //  the largest cluster has less than 5% messages in average
       forbidden.clear();

       double average =   count *1.0 / mmsicount.size();
       double min = average * 0.1;
       if(min < 2) min = 2.5;
       double minCluster = average * 0.05;

       for(it=mmsicount.begin(); it!=mmsicount.end();it++){
           if(it->second.count < average){
              forbidden.insert(it->first);
           } else {
              if( (it->second.failed*1.0 / it->second.count) > 0.1){
                 forbidden.insert(it->first);
              } else {
                  if(it->second.largestClusterSize() < minCluster){
                      forbidden.insert(it->first);
                  }
              }
           }
       }



       // for the remaining mmsi
       // union all clusters
       // remove clusters with too less entries
       complete.clear();
       for(it=mmsicount.begin(); it!=mmsicount.end();it++){
          if(forbidden.find(it->first)==forbidden.end()){
              complete.add(it->second);
          }
       }
       complete.mergeCluster();
       complete.purge();

       cout << "ignore type 4 messages of " << forbidden.size() 
            << " mmsis" << endl;
       cout << "allow only time in the intervals " << complete << endl;


       count =0;
       delete dec;

       posMessages.clear();

       dec = new aisdecode::aisdecoder(filename); 
       while( (msg = dec->getNextMessage())){
          count++;
          processMessage1(msg);
       }
       delete dec;
       if(time.IsDefined()){
          processPosMessages(time,time);
       } else {
           assert(posMessages.empty());
       }

       return count;
    }



  private:
    string filename;
    string prefix;
    Instant time;
    vector<Relation*> relations;
    vector<TupleType*> tupleTypes;
    SecondoCatalog* ctlg;
    size_t count; // total number of messages
    map<int, datetime::DateTime> lastTimes;
    set<int> forbidden; // excluded mmsi for type 4 messages
    Type4Info complete;
    int noUsedReason; // integer coded reason why a type 4 message has
                      //  been ignored
    vector<aisdecode::MessageBase*> posMessages;



  void createRelationForType(int type, ListExpr attrList){
        ListExpr tupleList  = nl->TwoElemList(listutils::basicSymbol<Tuple>(),
                                          attrList);
        ListExpr relType = nl->TwoElemList(listutils::basicSymbol<Relation>(),
                                           tupleList);
        TupleType* tt = new TupleType(ctlg->NumericType(nl->Second(relType)));
        tupleTypes[type-1] = tt;
        relations[type-1] = new Relation(tt);
        Word relWord;
        relWord.setAddr(relations[type-1]);
        string name = prefix+"_" + stringutils::int2str(type);
        if(!ctlg->InsertObject( name, "", 
                                relType, relWord,true)){
           cerr << "prolem in inserting relation " << name << "_" << endl;
           assert(false);
        }
  }



/*
Message types 1-3

*/
   void writeMessage(aisdecode::Message1_3* msg){
      if(relations[0] == 0){ // relation not present
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Status"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Rot"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SOG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Accuracy"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Longitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Latitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("COG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Heading"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Time"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Maneuver"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Raim"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Radio"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
        createRelationForType(1,attrList);
      }
      Tuple* tuple = new Tuple(tupleTypes[0]);
      tuple->PutAttribute(0, new CcInt(true,msg->messageType)); 
      tuple->PutAttribute(1, new CcInt(true,msg->repeatIndicator)); 
      tuple->PutAttribute(2, new CcInt(true,msg->mmsi)); 
      tuple->PutAttribute(3, new CcInt(true,msg->status)); 
      tuple->PutAttribute(4, new CcInt(true,msg->rot)); 
      tuple->PutAttribute(5, new CcInt(true,msg->sog)); 
      tuple->PutAttribute(6, new CcInt(true,msg->accuracy)); 
      tuple->PutAttribute(7, new CcReal(true,msg->longitude)); 
      tuple->PutAttribute(8, new CcReal(true,msg->latitude)); 
      tuple->PutAttribute(9, new CcInt(true,msg->cog)); 
      tuple->PutAttribute(10, new CcInt(true,msg->heading)); 
      tuple->PutAttribute(11, new CcInt(true,msg->second)); 
      tuple->PutAttribute(12, new CcInt(true,msg->maneuver)); 
      tuple->PutAttribute(13, new CcInt(true,msg->raim)); 
      tuple->PutAttribute(14, new CcInt(true,msg->rstatus)); 
      tuple->PutAttribute(15, new datetime::DateTime(time)); 
      tuple->PutAttribute(16, new CcInt(true,count)); 
      relations[0]->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
   }

/*
Message type 4

*/
   void writeMessage(aisdecode::Message4* msg){
      if(relations[3]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Year"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Month"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Day"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Hour"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Minute"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Second"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Fix"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Longitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Latitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("EPFD"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Raim"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SotDMA"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                listutils::basicSymbol<datetime::DateTime>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("NoUsedReason"),
                                     listutils::basicSymbol<CcInt>()));
        createRelationForType(4,attrList);
      }
      Tuple* tuple = new Tuple(tupleTypes[3]);
      tuple->PutAttribute(0, new CcInt(true,msg->type));      
      tuple->PutAttribute(1, new CcInt(true,msg->repeat));      
      tuple->PutAttribute(2, new CcInt(true,msg->mmsi));      
      tuple->PutAttribute(3, new CcInt(true,msg->year));      
      tuple->PutAttribute(4, new CcInt(true,msg->month));      
      tuple->PutAttribute(5, new CcInt(true,msg->day));      
      tuple->PutAttribute(6, new CcInt(true,msg->hour));      
      tuple->PutAttribute(7, new CcInt(true,msg->minute));      
      tuple->PutAttribute(8, new CcInt(true,msg->second));      
      tuple->PutAttribute(9, new CcInt(true,msg->fix));      
      tuple->PutAttribute(10, new CcReal(true,msg->longitude));      
      tuple->PutAttribute(11, new CcReal(true,msg->latitude));      
      tuple->PutAttribute(12, new CcInt(true,msg->epfd));      
      tuple->PutAttribute(13, new CcInt(true,msg->raim));      
      tuple->PutAttribute(14, new CcInt(true,msg->sotdma));      
      tuple->PutAttribute(15, new Instant(time));      
      tuple->PutAttribute(16, new CcInt(true,count)); 
      tuple->PutAttribute(17, new CcInt(true,noUsedReason)); 
      relations[3]->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
   }

/*
Message type 5

*/
   void writeMessage(aisdecode::Message5* msg){
     if(relations[4] == 0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("AisVersion"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("IMO"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("CallSign"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("VesselName"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ShipType"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToBow"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToStern"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToPort"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(
                                     nl->SymbolAtom("DimToStarboard"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Epfd"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ETA_Month"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ETA_Day"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ETA_HOUR"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ETA_Minute"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Draught"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Destination"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DTE"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));

        createRelationForType(5,attrList);
     }
     Tuple* tuple = new Tuple(tupleTypes[4]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->ais_version));
     tuple->PutAttribute(4, new CcInt(true,msg->imo));
     tuple->PutAttribute(5, new CcString(true,msg->callSign));
     tuple->PutAttribute(6, new CcString(true,msg->vesselName));
     tuple->PutAttribute(7, new CcInt(true,msg->shipType));
     tuple->PutAttribute(8, new CcInt(true,msg->dimToBow));
     tuple->PutAttribute(9, new CcInt(true,msg->dimToStern));
     tuple->PutAttribute(10, new CcInt(true,msg->dimToPort));
     tuple->PutAttribute(11, new CcInt(true,msg->dimToStarboard));
     tuple->PutAttribute(12, new CcInt(true,msg->epfd));
     tuple->PutAttribute(13, new CcInt(true,msg->month));
     tuple->PutAttribute(14, new CcInt(true,msg->day));
     tuple->PutAttribute(15, new CcInt(true,msg->hour));
     tuple->PutAttribute(16, new CcInt(true,msg->minute));
     tuple->PutAttribute(17, new CcInt(true,msg->draught));
     tuple->PutAttribute(18, new CcString(true,msg->destination));
     tuple->PutAttribute(19, new CcInt(true,msg->dte));
     tuple->PutAttribute(20, new Instant(time));
     tuple->PutAttribute(21, new CcInt(true,count));
     relations[4]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
   }


/*
Message type 9

*/
   void writeMessage(aisdecode::Message9* msg){
      if(relations[8]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Alt"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SOG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Accuracy"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Longitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Latitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("COG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Second"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DTE"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Assigned"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Raim"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Radio"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));

        createRelationForType(9,attrList);
      }
     Tuple* tuple = new Tuple(tupleTypes[8]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->alt));
     tuple->PutAttribute(4, new CcInt(true,msg->sog));
     tuple->PutAttribute(5, new CcInt(true,msg->accuracy));
     tuple->PutAttribute(6, new CcReal(true,msg->longitude));
     tuple->PutAttribute(7, new CcReal(true,msg->latitude));
     tuple->PutAttribute(8, new CcInt(true,msg->cog));
     tuple->PutAttribute(9, new CcInt(true,msg->second));
     tuple->PutAttribute(10, new CcInt(true,msg->dte));
     tuple->PutAttribute(11, new CcInt(true,msg->assigned));
     tuple->PutAttribute(12, new CcInt(true,msg->raim));
     tuple->PutAttribute(13, new CcInt(true,msg->radio));
     tuple->PutAttribute(14, new Instant(time));
     tuple->PutAttribute(15, new CcInt(true,count));
     relations[8]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
   }

/*
Message type 12

*/
   void writeMessage(aisdecode::Message12* msg){
      if(relations[11]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SeqNo"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Dest_MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Retransmit"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Text"),
                                     listutils::basicSymbol<FText>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
        createRelationForType(12,attrList);
      }
     Tuple* tuple = new Tuple(tupleTypes[11]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->source_mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->sequence_number));
     tuple->PutAttribute(4, new CcInt(true,msg->dest_mmsi));
     tuple->PutAttribute(5, new CcInt(true,msg->retransmit));
     tuple->PutAttribute(6, new FText(true,msg->text));
     tuple->PutAttribute(7, new Instant(time));
     tuple->PutAttribute(8, new CcInt(true,count));
     relations[11]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
   }

/*
Message type 14

*/
   void writeMessage(aisdecode::Message14* msg){
     if(relations[13]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Text"),
                                     listutils::basicSymbol<FText>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
        createRelationForType(14,attrList);
     }
     Tuple* tuple = new Tuple(tupleTypes[13]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new FText(true,msg->text));
     tuple->PutAttribute(4, new Instant(time));
     tuple->PutAttribute(5, new CcInt(true,count));

     relations[13]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
   }


/*
Message type 18

*/
   void writeMessage(aisdecode::Message18* msg){
      if(relations[17]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SOG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Accuracy"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Longitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Latitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("COG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Heading"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Second"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("CS"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Display"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DSC"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Band"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Msg22"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Assigned"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Raim"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Radio"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
 
        createRelationForType(18,attrList);
      }
     Tuple* tuple = new Tuple(tupleTypes[17]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->sog));
     tuple->PutAttribute(4, new CcInt(true,msg->accuracy));
     tuple->PutAttribute(5, new CcReal(true,msg->longitude));
     tuple->PutAttribute(6, new CcReal(true,msg->latitude));
     tuple->PutAttribute(7, new CcInt(true,msg->cog));
     tuple->PutAttribute(8, new CcInt(true,msg->heading));
     tuple->PutAttribute(9, new CcInt(true,msg->second));
     tuple->PutAttribute(10, new CcInt(true,msg->cs));
     tuple->PutAttribute(11, new CcInt(true,msg->display));
     tuple->PutAttribute(12, new CcInt(true,msg->dsc));
     tuple->PutAttribute(13, new CcInt(true,msg->band));
     tuple->PutAttribute(14, new CcInt(true,msg->msg22));
     tuple->PutAttribute(15, new CcInt(true,msg->assigned));
     tuple->PutAttribute(16, new CcInt(true,msg->raim));
     tuple->PutAttribute(17, new CcInt(true,msg->radio));
     tuple->PutAttribute(18, new Instant(time));
     tuple->PutAttribute(19, new CcInt(true,count));
     relations[17]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
   }


/*
Message type 19

*/
    void writeMessage(aisdecode::Message19* msg){
      if(relations[18]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("SOG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Accuracy"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Longitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Latitude"),
                                     listutils::basicSymbol<CcReal>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("COG"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Heading"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Second"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Name"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ShipType"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToBow"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToStern"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToPort"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(
                                     nl->SymbolAtom("DimToStarboard"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("EPFD"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Raim"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DTE"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Assigned"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));

        createRelationForType(19,attrList);
      }
     Tuple* tuple = new Tuple(tupleTypes[18]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->sog));
     tuple->PutAttribute(4, new CcInt(true,msg->accuracy));
     tuple->PutAttribute(5, new CcReal(true,msg->longitude));
     tuple->PutAttribute(6, new CcReal(true,msg->latitude));
     tuple->PutAttribute(7, new CcInt(true,msg->cog));
     tuple->PutAttribute(8, new CcInt(true,msg->heading));
     tuple->PutAttribute(9, new CcInt(true,msg->second));
     tuple->PutAttribute(10, new CcString(true,msg->name));
     tuple->PutAttribute(11, new CcInt(true,msg->shiptype));
     tuple->PutAttribute(12, new CcInt(true,msg->dimToBow));
     tuple->PutAttribute(13, new CcInt(true,msg->dimToStern));
     tuple->PutAttribute(14, new CcInt(true,msg->dimToPort));
     tuple->PutAttribute(15, new CcInt(true,msg->dimToStarboard));
     tuple->PutAttribute(16, new CcInt(true,msg->epfd));
     tuple->PutAttribute(17, new CcInt(true,msg->raim));
     tuple->PutAttribute(18, new CcInt(true,msg->dte));
     tuple->PutAttribute(19, new CcInt(true,msg->assigned));
     tuple->PutAttribute(20, new Instant(time));
     tuple->PutAttribute(21, new CcInt(true,count));
     relations[18]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
    }

/*
Message type 24

*/
    void writeMessage(aisdecode::Message24* msg){
      if(relations[23]==0){
        ListExpr attrList = nl->OneElemList(
                                nl->TwoElemList(nl->SymbolAtom("Type"), 
                                     listutils::basicSymbol<CcInt>()));
        ListExpr last = attrList;
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Repeat"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("PartNo"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ShipName"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("ShipType"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("VendorId"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Model"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("Serial"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("CallSign"),
                                     listutils::basicSymbol<CcString>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToBow"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToStern"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("DimToPort"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(
                                     nl->SymbolAtom("DimToStarboard"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(
                                     nl->SymbolAtom("Mothership_MMSI"),
                                     listutils::basicSymbol<CcInt>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("TimeStamp"),
                                     listutils::basicSymbol<Instant>()));
        last = nl->Append(last, nl->TwoElemList(nl->SymbolAtom("MessageNo"),
                                     listutils::basicSymbol<CcInt>()));
        createRelationForType(24,attrList);
      }
     Tuple* tuple = new Tuple(tupleTypes[23]);
     tuple->PutAttribute(0, new CcInt(true,msg->type));
     tuple->PutAttribute(1, new CcInt(true,msg->repeat));
     tuple->PutAttribute(2, new CcInt(true,msg->mmsi));
     tuple->PutAttribute(3, new CcInt(true,msg->partno));
     tuple->PutAttribute(4, new CcString(true,msg->shipsname));
     tuple->PutAttribute(5, new CcInt(true,msg->shiptype));
     tuple->PutAttribute(6, new CcInt(true,msg->vendorid));
     tuple->PutAttribute(7, new CcInt(true,msg->model));
     tuple->PutAttribute(8, new CcInt(true,msg->serial));
     tuple->PutAttribute(9, new CcString(true,msg->callsign));
     tuple->PutAttribute(10, new CcInt(true,msg->dimToBow));
     tuple->PutAttribute(11, new CcInt(true,msg->dimToStern));
     tuple->PutAttribute(12, new CcInt(true,msg->dimToPort));
     tuple->PutAttribute(13, new CcInt(true,msg->dimToStarboard));
     tuple->PutAttribute(14, new CcInt(true,msg->mothership_mmsi));
     tuple->PutAttribute(15, new Instant(time));
     tuple->PutAttribute(16, new CcInt(true,count));
     relations[23]->AppendTuple(tuple);
     tuple->DeleteIfAllowed();
    }


/*
General distribution of messages accorsing to their types.

*/

   void processMessage2(aisdecode::MessageBase* msg){
      int t = msg->getType();
       switch(t){
             case 1:
             case 2:
             case 3: writeMessage( (aisdecode::Message1_3*)msg);
                     break;
             case 4: writeMessage((aisdecode::Message4*)msg);
                     break;
             case 9: writeMessage((aisdecode::Message9*)msg);
                     break;
             case 18: writeMessage((aisdecode::Message18*) msg);
                      break;
             case 19: writeMessage((aisdecode::Message19*) msg);
                      break;
             default: cerr << "found message type " << t 
                           << "in processMessages" << endl;
      }
   }

   template<class T>
   int getSeconds(T* msg){
      return msg->second;
   }

   template<class T>
   bool isValidSecond(T* msg){
       int s = getSeconds(msg);
       return s>=0 && s<=60;
   }


    template<class T>
    void processMessage(T* msg){
       processMessage2(msg);
    }

    template<class T>
    void processMessageWS(T* msg){
       if(time.IsDefined()){
          posMessages.push_back(msg);
       } else {
          processMessage2(msg);
          delete msg;
       }
    }

    void processPosMessages(datetime::DateTime & old, datetime::DateTime now){
       if(posMessages.empty()){
          return;
       } 
       size_t numMessages = posMessages.size();
       datetime::DateTime timeDiff = now - old;
       datetime::DateTime timeDiffPerMsg = timeDiff / numMessages;
       for(size_t i=0;i<numMessages;i++){
            time += timeDiffPerMsg;
            assert(time <= now);
            processMessage2(posMessages[i]);
            delete posMessages[i];
       }
       posMessages.clear();

    }



    void processMessage4(aisdecode::Message4* msg){
              
       // invalid date
       if(   msg->year < 1 || msg->day<1 || msg->month<1 || msg->day>31 
          || msg->month>12){
          noUsedReason=1;
          writeMessage(msg);
          return;
       }

       if(forbidden.find(msg->mmsi)!=forbidden.end()){ // mmsi not allowed
         noUsedReason=2;
         writeMessage(msg);
         return;
       }


       if(!time.IsValid(msg->year, msg->month, msg->day)){
          // invalid date
          noUsedReason=3;
          writeMessage(msg);
          return;
       }
       if(msg->hour>23 || msg->minute>59 || msg->second>59){
          // imvalid time
          noUsedReason=4;
          writeMessage(msg);
          return;
       }

       datetime::DateTime t(datetime::instanttype);
       t.Set(msg->year, msg->month, msg->day, msg->hour, msg->minute, 
             msg->second);
       if(!complete.contains(t)){ // time stamp not allowed
         noUsedReason=5;
         writeMessage(msg);
         return;
       }

       noUsedReason=0;
       if(!time.IsDefined()){ 
          // this is the first valid type 4 message
          // this ais will be the master of time
          time=t;
          assert(lastTimes.empty());
          lastTimes[msg->mmsi] =time;
          writeMessage(msg);
          return;
       }       
       assert(!lastTimes.empty());

       datetime::DateTime msgtime(datetime::instanttype);
       msgtime = t;

       map<int, datetime::DateTime>:: iterator it;
       it = lastTimes.find(msg->mmsi);
       if(it==lastTimes.end()){
          // this ist the first message from this mmsi
          //datetime::DateTime timediff = time - msgtime;
          lastTimes[msg->mmsi] = msgtime;
          writeMessage(msg);
          it = lastTimes.find(msg->mmsi);
       } else {
         if(msgtime <= it->second){
            cout << "MMSI " << msg->mmsi << " goes back in time from " 
                 << it->second << " to " << msgtime 
                 <<  "(" << (it->second - msgtime) << ")" << endl;
            noUsedReason=6;
            writeMessage(msg);
            return;
         }       
      }

       // we have already stored a time for this mmsi
       // compute the time according to the message time
      it->second = msgtime;
       if(msgtime >= time){
          datetime::DateTime diff = msgtime - time;
          if(diff>complete.threshold){
             noUsedReason = 7;
             writeMessage(msg);
             return;
          }
          processPosMessages(time, msgtime);
          time = msgtime;
       } else { // msgtime < time
         ;
         //cout << msg->mmsi << ": back in time for " << (time - msgtime) 
         // << endl;
       }
       writeMessage(msg); 
    }



    void processMessage1(aisdecode::MessageBase* msg){
       int type = msg->getType();
       switch(type){
         case 1:
         case 2:
         case 3: processMessageWS((aisdecode::Message1_3*)msg);
                 break;
         case 4: processMessage4((aisdecode::Message4*)msg);
                 delete msg;
                 break;
         case 5: writeMessage((aisdecode::Message5*)msg);
                 delete msg;
                 break;
         case 9: processMessageWS((aisdecode::Message9*)msg);
                 break;
         case 12: writeMessage((aisdecode::Message12*)msg);
                  delete msg;
                  break;
         case 14: writeMessage((aisdecode::Message14*)msg);
                  delete msg;
                  break;
         case 18: processMessageWS((aisdecode::Message18*)msg);
                  break;
         case 19: processMessageWS((aisdecode::Message19*)msg);
                  break;
         case 24: writeMessage((aisdecode::Message24*)msg);
                  delete msg;
                  break;
         default: cerr << "Secondo: message type " << type 
                       << "not implemented yet" << endl;
                   delete msg;
       }
   }
};

/*
24.99.6 Value Mapping

*/
template<class T>
int importaisVMT(Word* args, Word& result,
                int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  T* filename = (T*) args[0].addr;
  if(!filename->IsDefined()){
    res->SetDefined(false);
    return 0;
  }  
  CcString* prefix = (CcString*) args[1].addr;
  if(!prefix->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  aisimportInfo ais(filename->GetValue(), prefix->GetValue());
  res->Set(true,ais());
  return 0;
}

ValueMapping importaisVM[] = {
    importaisVMT<CcString>,
    importaisVMT<FText>
};

/*
24.99.7 Selection Function

*/
int importaisSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

/*
24.99.7  Specifikation

*/
OperatorSpec importaisSpec(
   "{string, text} x string -> int ",
   "importais(filename, prefix)",
   "Creates a set of relations prefixed by prefix according "
   "to ais messages found in the file. Returns the total amount "
   "of messages in the file.",
   "query importais('aisexample.txt',\"AIS\")"
);

/*
24.99.7 Operator instance 

*/
Operator importaisOp(
  "importais",
  importaisSpec.getStr(),
  2,
  importaisVM,
  importaisSelect,
  importaisTM
);


/*
24.100 exportBinCSV

This operator receives a tuple stream, a string or text,
and some attribute names. The second argument is the 
filename of the file to be created.
The mentioned attribute names 
must be in kind CVSEXPORTABLE. This operator creates
a csv file in format  Nr , <ATTR>, <TUPLE>
Where Nr is just a running number, <ATTR> are the
attributes given in the query and <TUPLE> is the 
base64 coded binary tuple representation.

*/

ListExpr exportBinCSVTM(ListExpr args){
   if(!nl->HasMinLength(args,2)){
     return listutils::typeError("at least two arguments required");
   }
   if(!Stream<Tuple>::checkType(nl->First(args))){
     return listutils::typeError("the first argument must be a tuple stream");
   }
   ListExpr fn = nl->Second(args);
   if(! CcString::checkType(fn) && !FText::checkType(fn)){
     return  listutils::typeError("the second argument must be of "
                                  "type string or text");
   }
   ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
   ListExpr attrs = nl->Rest(nl->Rest(args));
   ListExpr appendList = nl->TheEmptyList();
   ListExpr appendLast = nl->TheEmptyList();

   while(!nl->IsEmpty(attrs)){
     ListExpr attr = nl->First(attrs);
     attrs = nl->Rest(attrs);
     if(nl->AtomType(attr) != SymbolType){
       return listutils::typeError("Invalid attribute name");
     }
     string a = nl->SymbolValue(attr);
     ListExpr attrType;
     int index = listutils::findAttribute(attrList, a, attrType); 
     if(!index){
        return listutils::typeError("Attribute " + a 
                                    + " not part of the tuple");
     }
     ListExpr errorInfo = listutils::emptyErrorInfo();
     if(!am->CheckKind("CSVEXPORTABLE", attrType, errorInfo)){
        return listutils::typeError("Attribute " + a 
                                    + " not in kind CSVEXPORTABLE");
     }
      
     if(nl->IsEmpty(appendList)){
       appendList = nl->OneElemList(nl->IntAtom(index-1));
       appendLast = appendList;
     } else {
       appendLast = nl->Append(appendLast, nl->IntAtom(index-1));
     }
     appendLast = nl->Append(appendLast, nl->StringAtom(a));
     appendLast = nl->Append(appendLast, nl->TextAtom(nl->ToString(attrType)));
   }

   return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
                            appendList,
                            nl->First(args));

}


class exportBinCSVInfo{

  public:
      exportBinCSVInfo( Word _stream, string& _file, 
                        vector<pair<pair<string, string>, int> >& _attrs,
                        ListExpr tupleDescr):
      stream(_stream),base64(' ')
      {
         stream.open();
         out.open(_file.c_str());
         // write header if file is good
         if(out){
            out << "# No: int";
            for(size_t i=0;i<_attrs.size();i++){
              pair<pair<string, string>, int> v = _attrs[i];
              out << "," << v.first.first << ":" << v.first.second;
              attrs.push_back(v.second);
           }
           out << ", T : " << nl->ToString(tupleDescr) << endl;
         } 
         no = 0;
      }

      ~exportBinCSVInfo(){
         stream.close();
         out.close();
      }
   
      Tuple* next(){
         if(!out) return 0;
         Tuple* tuple = stream.request();
         if(tuple){
            writeTuple(tuple);
         }
         return tuple;
      }


  private:
      Stream<Tuple> stream;
      Base64 base64;
      ofstream out;
      int no;
      vector<int> attrs;


   void writeTuple(Tuple* t){
     out << no ;
     for(size_t i=0;i<attrs.size();i++){
        Attribute* attr = t->GetAttribute(attrs[i]);
        string s = attr->getCsvStr();
        out << ", " << s;
     }
     out << ", " << getBase64Data(t) << endl;
   }

   string getBase64Data(Tuple* t){
      size_t size;
      char* binData = t->GetBinRep(size);
      string res;
      base64.encode(binData, size,res);
      delete[] binData;
      return res;
   }
   

};


template<class T>
int exportBinCSVVMT(Word* args, Word& result,
                int message, Word& local, Supplier s){

  exportBinCSVInfo* li = (exportBinCSVInfo*) local.addr;
  switch(message){
       case OPEN: {
            if(li){
               delete li;
               local.addr = 0;
            }
            int noAttrs = (qp->GetNoSons(s)-2) / 4;
            T* fn = (T*) args[1].addr;
            if(!fn->IsDefined()){
              return 0;
            }
            string f = fn->GetValue();

            int start = 2+noAttrs;
            int end = qp->GetNoSons(s);
            vector<pair<pair<string,string>,int> > v;
            for(int i=start;i<end;i=i+3){
               int no = ((CcInt*)args[i].addr)->GetValue();
               string name = ((CcString*)args[i+1].addr)->GetValue();
               string type = ((FText*)args[i+2].addr)->GetValue();
               v.push_back(make_pair(make_pair(name,type),no));
            }
            local.addr = new exportBinCSVInfo(args[0],f,v, 
                                              nl->Second(qp->GetType(s)));
            return 0;
       }
      case REQUEST:
             result.addr = li?li->next():0;
             return result.addr?YIELD:CANCEL;
      case CLOSE:
             if(li){
               delete li;
               local.addr = 0;
             }
             return 0;
  }
  return -1;
}

int exportBinCSVSelect(ListExpr args){
  return CcString::checkType(nl->Second(args))?0:1;
}

ValueMapping exportBinCSVVM[] = {
   exportBinCSVVMT<CcString>,
   exportBinCSVVMT<FText>
};


OperatorSpec exportBinCSVSpec(
  "stream(tuple) x {string,text} x INDENT* -> stream(tuple)",
  " _ exportBinCSV[_,_...]",
  " Writes a tuple stream to a csv file. "
  "The first argument is the stream of tuples to be write, "
  "the second argument is the name of the file, the remaining "
  "arguments that represents attribute names to write additionally as "
  "text into the file. The created file cosists of lines of the form "
  "int, (DATA,)*  <TUPLE> where the first entry is a runnning number,"
  "the following entries are the specified attributea and the last "
  "value is the base64 coded binary representation of the whole tuple.",
  "query plz feed exportBinCSV['plz.bincsv', PLZ] count"
);

Operator exportBinCSVOp(
  "exportBinCSV",
  exportBinCSVSpec.getStr(),
  2,
  exportBinCSVVM,
  exportBinCSVSelect,
  exportBinCSVTM
);


/*
24.101 Operator ~importBinCSVSimple~

*/
ListExpr importBinCSVSimpleTM(ListExpr args){

  if(!nl->HasLength(args,3)){
    return listutils::typeError("three arguments expected");
  }
  if(!listutils::checkUsesArgsInTypeMapping(args)){
    return listutils::typeError("internal error");
  }

  ListExpr fn = nl->First(nl->First(args));
  if(!CcString::checkType(fn) && !FText::checkType(fn)){
    return listutils::typeError("first arg must be a string or a text");
  }
  ListExpr te = nl->First(nl->Second(args));
  if(!Relation::checkType(te) && !FText::checkType(te)){
    return listutils::typeError("second arg must be a relation or a text");
  }

  if(!CcInt::checkType(nl->First(nl->Third(args)))){
     return listutils::typeError("third argument must be of type int");
  }
  ListExpr tupleDescr = nl->TheEmptyList();
  if(FText::checkType(te)){
     ListExpr tev = nl->Second(nl->Second(args));
     Word queryResult ;
     string typeString = " " ;
     string errorString = " " ;
     bool correct ;
     bool evaluable ;
     bool defined ;
     bool isFunction ;
     qp->ExecuteQuery(tev, queryResult, typeString, 
                      errorString, correct, evaluable, defined, isFunction );
     if(!correct || !evaluable || !defined || isFunction ){
        return listutils::typeError ( " could not extract filename ( " +
                                       errorString + " ) " );
    }
    FText* fn = (FText*) queryResult.addr;
    if(!fn->IsDefined()){
      fn->DeleteIfAllowed();
      return listutils::typeError("undefined tuple type description");
    }
    string tt = fn->GetValue();
    fn->DeleteIfAllowed();
    if(!nl->ReadFromString(tt,tupleDescr)){
      return listutils::typeError("tuple description is not a valid list");
    }
    if(!Tuple::checkType(tupleDescr)){
       return listutils::typeError("description is not a valid tuple");
    } 
  } else {
     tupleDescr = nl->Second(te);
  }
  return nl->TwoElemList(listutils::basicSymbol<Stream<Tuple> >(),
                         tupleDescr);
}


class importBinCSVSimpleInfo{

  public:
     importBinCSVSimpleInfo(string _fileName, int _pos, TupleType* _tt){
        in.open(_fileName.c_str());
        pos = _pos;
        tt = _tt;
        tt->IncReference(); 
        id = 1;
        getline(in,line); // ignore first line
     }

     ~importBinCSVSimpleInfo(){
        in.close();
        tt->DeleteIfAllowed();
     }

     Tuple* next(){
        while(!in.eof() && in){
          getline(in,line);
          if(line.length()>0){
             Tuple* res =  createTuple(line); 
             if(res){
                return res;
             }
          }
        }
        return 0;
     }

  private:
    ifstream in;
    TupleType* tt;
    int pos;    
    string line;
    Base64 base64;
    TupleId id;

    
    Tuple* createTuple(string & line){
       // extract base 64 rep of the tuple
       stringutils::StringTokenizer st(line,",");
       string byterep;
       for(int i=0; i< pos; i++){
         if(!st.hasNextToken()){
           return 0;
         }
         byterep = st.nextToken();
       }
       stringutils::trim(byterep);
       if(byterep.empty()) return 0;
       
       int size = base64.sizeDecoded(byterep.size());
       char* bytes = new char[size];
       try{
           size = base64.decode(byterep, bytes);
       } catch(...){
          cout << "decoding base64 failed" << endl;
          delete[] bytes;
          return 0;
       }
       Tuple* res = getTuple(bytes);
       delete[] bytes;
       return res;
    }


    Tuple* getTuple(char* bytes){
      Tuple* res = new Tuple(tt);
      res->ReadFromBin(0, bytes );
      res->SetTupleId(id);
      id++;
      return res;
   }
};

template<class T>
int importBinCSVSimpleVMT(Word* args, Word& result,
                int message, Word& local, Supplier s){

   importBinCSVSimpleInfo* li = (importBinCSVSimpleInfo*) local.addr;
   switch(message){
      case INIT:
            qp->GetLocal2(s).addr = 
                     new TupleType(nl->Second(GetTupleResultType(s)));
            return 0;
      case FINISH: {
             TupleType* tt = (TupleType*)qp->GetLocal2(s).addr;
             if(tt){
               tt->DeleteIfAllowed();
               qp->GetLocal2(s).addr =0;
             }
             return 0;
      }
      case OPEN: {
               if(li){
                 delete li; 
                 local.addr = 0;
               }
               T* fn = (T*) args[0].addr;
               if(!fn->IsDefined()){
                 return 0;
               }
               TupleType* tt = (TupleType*)qp->GetLocal2(s).addr;
               CcInt* pos = (CcInt*) args[2].addr;
               if(!pos->IsDefined()){
                 return 0;
               }
               local.addr = new importBinCSVSimpleInfo(fn->GetValue(),
                                                       pos->GetValue(),tt);
               return 0;
      }
      case REQUEST: result.addr = li?li->next():0;
                    return result.addr?YIELD:CANCEL;

      case CLOSE :
                if(li){
                  delete li;
                  local.addr = 0;
                }
                return 0;
      
   }
   return -1; 

}

int importBinCSVSimpleSelect(ListExpr args){
  return CcString::checkType(nl->First(args))?0:1;
}

ValueMapping importBinCSVSimpleVM[] = {
   importBinCSVSimpleVMT<CcString>,
   importBinCSVSimpleVMT<FText>
};

OperatorSpec importBinCSVSimpleSpec(
  "{string,text} x {rel,text} x int -> stream(tuple)",
  "_ importBinCSVSimple[_,_] ",
  "Imports a relation that is write into a CSV format and the "
  "tuples are base64 code binary. The last argument give the  "
  "position of the tuple within a csv line.",
  " query 'plz.bincsv' importBinCSVSimple[plz, 2] count"
);


Operator importBinCSVSimpleOp(
  "importBinCSVSimple",
   importBinCSVSimpleSpec.getStr(),
   2,
   importBinCSVSimpleVM,
   importBinCSVSimpleSelect,
   importBinCSVSimpleTM
);


/*
24.102 Operator ~geojson2line~

*/

/*
24.102.1 Type Mapping function

The signature is text x bool -> line

*/

ListExpr Geojson2lineTypeMap(ListExpr args){

  if(nl->ListLength(args) != 2) {
     return listutils::typeError("two arguments exptected");
  }

  if(FText::checkType(nl->First(args)) &&
     CcBool::checkType(nl->Second(args))) {
      return nl->SymbolAtom(Line::BasicType());
  }

  return listutils::typeError("text expected");
}

/*
24.102.2 Value Mapping function

*/
int Geojson2line(Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   result = qp->ResultStorage(s);

   FText* text = static_cast<FText*>(args[0].addr);
   CcBool* autocloseBool = static_cast<CcBool*>(args[1].addr);

   Line* res = static_cast<Line*>(result.addr);
   vector<double> points;

   if(! text -> IsDefined()) {
      cerr << "Input text is not defined" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   if(! autocloseBool -> IsDefined()) {
      cerr << "Autoclose is not defined" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   string data = text -> GetValue();
   bool autoclose = autocloseBool -> GetValue();

   bool contains = data.find("\"type\":\"Polygon\"") != std::string::npos;

   if(! contains) {
       cerr << "Input does not contain a Polygon" << endl;
       res -> SetDefined(false);
       return 0; 
   }

   const char* dataArray = data.c_str();
   const char* start = strstr(dataArray, "[[");

   if(start == NULL) {
      cerr << "Input is not valid GEOJson" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   string buffer = "";
   char lastChar = '\0';

   for(int i = start - dataArray; dataArray[i] != '\0'; i++) {
      char curChar = data[i];

      if(curChar == ']' && lastChar == ']') {
          if(! buffer.empty()) {
             points.push_back(stod(buffer));
          }
          break;
      } else if(curChar == ',' || curChar == ']') {
         if(! buffer.empty()) {
             points.push_back(stod(buffer));
             buffer = "";
         }
      } else if(curChar == '[') {
         // Ignore
      } else {
         buffer += curChar;
      }

      lastChar = curChar;
   }

   if(points.size() % 2 != 0) {
      cerr << "Got an uneven amount of points" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   vector<SimplePoint> simplepoints;

   // Convert to points
   for(size_t i = 0; i < points.size(); i = i + 2) {
      double x = points[i];
      double y = points[i+1];
      SimplePoint point = SimplePoint(x, y);
      simplepoints.push_back(point);
   }

   // Check for closed path
   if(simplepoints[0] != simplepoints[simplepoints.size() - 1]) {
      if(! autoclose) {
         cerr << "Error: Region is not closed" << endl;
         res -> SetDefined(false);
         return 0; 
      } else {
         // Close line
         simplepoints.push_back(simplepoints[0]);
      }
   }

   res -> SetDefined(true);
   res -> StartBulkLoad();
   res -> Resize(simplepoints.size());
   for(size_t i = 0; i < simplepoints.size() - 1; ++i) {
        SimplePoint p1 = simplepoints[i];
        SimplePoint p2 = simplepoints[i+1];
        HalfSegment hs1(true,p1.getPoint(),p2.getPoint());
        HalfSegment hs2(false,p1.getPoint(),p2.getPoint());
        hs1.attr.edgeno = i;
        hs2.attr.edgeno = i;
        (*res) += hs1;
        (*res) += hs2;
   }

   res -> EndBulkLoad();
   return 0;
}

/*
24.102.3 Specification of the operator ~geojson2line~

*/
const string Geojson2lineSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text x bool -> region</text--->"
                         "<text>geojson2line (_, _)</text--->"
                         "<text> The operator geojson2line extracts"
                         " all points from a GeoJSON polygon and creates"
                         " a line. The first parameter is the GeoJSON"
                         " input. The second parameter determines if the"
                         " line should be automatically closed"
                         " (if close segment is missing in GeoJSON).</text--->"
                         "<text>query geojson2line([...])</text--->"
                         ") )";

/*
24.102.4 Definition of the operator ~geojson2line~

*/
Operator geojson2line (
        "geojson2line",
        Geojson2lineSpec,
        Geojson2line,
        Operator::SimpleSelect,
        Geojson2lineTypeMap );

/*
24.103 Operator ~geojson2point~

*/

/*
24.103.1 Type Mapping function

The signature is text -> point

*/

ListExpr Geojson2pointTypeMap(ListExpr args){

  if(nl->ListLength(args) != 1) {
     return listutils::typeError("one argument exptected");
  }

  if(FText::checkType(nl->First(args))) {
     return nl->SymbolAtom(Point::BasicType());
  }

  return listutils::typeError("text expected");
}

/*
24.103.2 Value Mapping function

*/
int Geojson2point(Word* args, Word& result, int message,
                   Word& local, Supplier s ){

   result = qp->ResultStorage(s);

   FText* text = static_cast<FText*>(args[0].addr);

   Point* res = static_cast<Point*>(result.addr);
   vector<double> points;

   if(! text -> IsDefined()) {
      cerr << "Input text is not defined" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   string data = text -> GetValue();

   bool contains = data.find("\"type\":\"Point\"") != std::string::npos;

   if(! contains) {
       cerr << "Input does not contain a Point" << endl;
       res -> SetDefined(false);
       return 0; 
   }

   const char* dataArray = data.c_str();
   const char* startStr = "coordinates\":[";
   const char* start = strstr(dataArray, startStr);

   if(start == NULL) {
      cerr << "Input is not valid GEOJson" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   string buffer = "";
   const char* startPos = start + strlen(startStr);

   for(int i = startPos - dataArray; dataArray[i] != '\0'; i++) {
       char curChar = dataArray[i];

       if(curChar == ']') {
          if(! buffer.empty()) {
             points.push_back(stod(buffer));
          }
          break;
       } else if(curChar == ',' || curChar == ']') {
          if(! buffer.empty()) {
             cout << "Data: " << buffer << endl;
             points.push_back(stod(buffer));
             buffer = "";
          }
        } else {
           buffer += curChar;
        }
   }

   if(points.size() != 2) {
      cerr << "Got an unexpected amount of points" << endl;
      res -> SetDefined(false);
      return 0; 
   }

   res->Set(points[0], points[1]); // also sets to DEFINED
   return 0;
}

/*
24.103.3 Specification of the operator ~geojson2point~

*/
const string Geojson2pointSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( "
                         "<text>text -> point</text--->"
                         "<text>geojson2point (_)</text--->"
                         "<text> The operator geojson2ponint extracts"
                         " a point from a GeoJSON text."
                         " The first parameter is the GeoJSON</text--->"
                         "<text>query geojson2point([...])</text--->"
                         ") )";



/*
24.103.4 Definition of the operator ~geojson2point~

*/
Operator geojson2point (
        "geojson2point",
        Geojson2pointSpec,
        Geojson2point,
        Operator::SimpleSelect,
        Geojson2pointTypeMap );



/*
25 Creating the Algebra

*/

class ImExAlgebra : public Algebra
{
public:
  ImExAlgebra() : Algebra()
  {
    AddOperator( &csvexport );
    AddOperator( &shpexport );
    AddOperator( &db3export );
    AddOperator( &shptype );
    AddOperator( &shpimport );
    AddOperator( &shpimport2 );
    shpimport2.SetUsesArgsInTypeMapping();
    AddOperator( &dbtype );
    AddOperator( &dbimport);
    AddOperator( &dbimport2);
    dbimport2.SetUsesArgsInTypeMapping();
    AddOperator( &saveObject);
    AddOperator( &csvimport);
    AddOperator( &csvimport2);
    csvimport2.SetUsesArgsInTypeMapping();
    AddOperator( &isFile);
    AddOperator( &removeFile);
    AddOperator( &createDirectory);
    AddOperator( &fileSize);
    AddOperator( &isDirectory);
    AddOperator( &writeFile);
    AddOperator( &readFile);
    AddOperator( &moveFile);
    AddOperator( &copyFile);
    AddOperator( &basenameOp);
    AddOperator( &getDirectory);
    AddOperator( &toCSVtext);
    AddOperator( &fromCSVtext);
    AddOperator( &getPID);
    AddOperator( &getSecondoVersion);
    AddOperator( &getBDBVersion);
    AddOperator( &getSecondoPlatform);
    AddOperator( &getPageSize);
    AddOperator( &nmeaimport);
    nmeaimport.SetUsesArgsInTypeMapping();
    AddOperator( &nmeaimport_line);
    nmeaimport_line.SetUsesArgsInTypeMapping();
    AddOperator( &get_lines);
    AddOperator( &sqlExport);
    AddOperator( &importGHT1);
    AddOperator( &removeDirectoryOp);
    #ifndef SECONDO_WIN32
    AddOperator( &rtf2txtfile);
    #endif
    AddOperator(&shpBoxOp);
    AddOperator(&shpCollectOp);
    AddOperator(&db3CollectOp);
    AddOperator(&createShxOp);
    AddOperator(&noShpRecordsOp);
    AddOperator(&noDB3RecordsOp);
    AddOperator(&extractShpPartOp);
    AddOperator(&extractDB3PartOp);

    AddOperator(&splitShpOp);
    AddOperator(&splitDB3Op);

    AddOperator(&importaisOp);
    importaisOp.SetUsesArgsInTypeMapping();

    AddOperator(&exportBinCSVOp);
    AddOperator(&importBinCSVSimpleOp);
    importBinCSVSimpleOp.SetUsesArgsInTypeMapping();
    importBinCSVSimpleOp.enableInitFinishSupport();

    AddOperator(&geojson2line);
    AddOperator(&geojson2point);
  }
  ~ImExAlgebra() {};
};

/*
9 Initialization

*/

extern "C"
Algebra*
InitializeImExAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new ImExAlgebra());
}


