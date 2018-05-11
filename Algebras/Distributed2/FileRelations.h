
/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]

*/

#ifndef FILERELATIONS_H
#define FILERELATIONS_H


#include <iostream>
#include <string>

#include "NestedList.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

#define FILE_BUFFER_SIZE 1048576

/*
1 Class ~BinRelWriter~

This class provides implementation for saving a tuple stream into a
file. All tuples muat have the same type. For saving a complete relation
the ~writeRelationToFile~ function  can be used. In all other cases,
the first function to call is ~writeHeader~, followed by a set of 
~writeNextTuple~ calls, and a final ~finish~ call.

*/
class BinRelWriter{
  public:
  /*
   1.4 ~writeRelationToFile~

   This function write a complete relation iclusive header and end marker 
   to a single file.

  */
   static bool writeRelationToFile(Relation* rel, ListExpr relType, 
                      const std::string& fileName,
                      bool writeToDFS = false);

   BinRelWriter(const std::string& filename, ListExpr type, 
                size_t bufferSite = 0,
                bool writeToDFS = false );

   ~BinRelWriter();

   inline bool ok(){
     return  out->good();
   }

   inline bool writeNextTuple(Tuple* tuple){
      return writeNextTuple(*out,tuple);
   }

   inline std::string getFileName(){
     return filename;
   }


  private:
     std::string filename;
     std::ofstream* out;
     char* buffer;
     bool writeToDFS;


/*
1.1 ~writeHeader~

Writes header information into an output stream. The ~type~ argument specifies the
relation type in nested list format.

*/
     static bool writeHeader(std::ostream& out, ListExpr type);
/*
1.2 ~writeNextTuple~

This function write a single tuple to an output stream in binary format.

*/
     static bool writeNextTuple(std::ostream& out,Tuple* tuple);

/*
1.3 ~finish~

The ~finish~ functions writes an end marker to the output stream.

*/
     static bool finish(std::ostream& out);

};


/*
2 Class ~feed5Info~

This class can be used for extracting a tuple stream from a file containing
a relation in binary format. 

*/

class ffeed5Info{

  public:
/*
1.1 Constructor

This constructor opens the file with name __filename__ and extracts 
its header information. If the relation type stored in the file is 
in conflict with the relation type specified in the argument, no tuples
will be produced.

*/
    ffeed5Info(const std::string& filename, const ListExpr _tt);

/*
1.2 Constructor

This constructors works as the previous constructor with the difference that
the relation scheme is given by the tuple type instead of a nested list
description.

*/

    ffeed5Info(const std::string& filename, TupleType* _tt);

/*
1.3 Constructor

This constructor open a binary relation without checking for a given 
relation scheme.

*/
    ffeed5Info(const std::string& filename);

/*
1.4 Destructor

*/
    ~ffeed5Info();

/*
1.5 ~getRelType~

Returns the relation scheme coded as a nested list.

*/
    ListExpr getRelType();

/*
1.6 ~isOK~

Checks whether in input stream is ok and the relation scheme within the file
fits to a given relation scheme.

*/
    bool isOK();

/*
1.7 ~changePosition~

This operation changes the position of the internal file position. This may 
be useful for using indexes on file relations.

*/
    void changePosition(size_t pos);

/*
1.8 ~next~

Returns the next tuple stored in the file or 0 if the file does not contain
a further tuple.

*/
    Tuple* next();

  private:

/*
1.9 Private Members

*/
     std::ifstream in;       // file input stream
     char* inBuffer;    // for buffering the input stream
     TupleType* tt;     // the used tuple type
     bool ok;           // internal variable about state
     ListExpr fileTypeList;  // relation scheme stored within the file

/*
1.10 ~readHeader~

This function extracts header information from the file.

*/
     void readHeader(TupleType* tt);

     bool openFile(const std::string& fileName);

};

#endif


