/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]


[1] Header File of type Record

December 2009,  B. Franzkowiak

[TOC]

1 Overview

This header file essentially contains the definition of the Record class.

With this class a new structured data type named Record is implemented into
SECONDO. Elements of already existing attributes of kind DATA can be summerized
to a new data type likewise a structure.
Apparently a Record can contain one or more FLOBs or other Records.

For detailed information refer to ~Record.cpp~.

*/

/*

3 Defines and includes


*/

#ifndef __RECORD_H__
#define __RECORD_H__

#include "Attribute.h"
#include "Symbols.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"
#include "NestedList.h"
#include "ListUtils.h"


using namespace std;

/*

4 class Record

As Record should be of kind DATA it has to erben von Attribute.

*/


class Record : public Attribute
{

/*
4.1 Public area of class Record

*/

public:

/*

4.1.1 Constructor and destructor area

The constructor of the record passing the number of elements to initialize it.

*/

    Record(int noElements);

/*
The destructor

*/

    ~Record();

/*

4.1.2 Public record method area


*/


/*
~GetNoElements~ returns the number of elements in this record.

*/
    int GetNoElements() const;

/*

~GetElement~ returns the element at position pos

*/
    Attribute* GetElement(int pos) const;

/*
~GetElementName~ returns the element name at position pos.

*/

    const string GetElementName(int pos) const;

/*
~GetElementTypeName~ returns the element type name at position pos.

*/

    const string GetElementTypeName(int pos) const;

/*
~SetElement~ stores the given element at the given position
in this record.

*/

    bool SetElement(int pos, Attribute* elem,
                    const string& typeName, const string& elemName);

/*
~AppendElement~ appends the given element at the end of this record.

*/

    bool AppendElement(Attribute* elem,
                       const string& typeName, const string& elemName);

/*

4.1.3 Area with abstract(virtual) ~Attribute~ methods that mandatory
have to be implemented.


*/

    virtual size_t     Sizeof() const;
    virtual int        Compare(const Attribute *rhs) const;
    virtual bool       Adjacent(const Attribute* attr) const;
    virtual Attribute* Clone() const;
    virtual size_t     HashValue() const;
    virtual void       CopyFrom(const Attribute* right);

/*

4.1.4 Area with virtual ~Attribute~ methods that optionally have to
be implemented for this data type because a record owns FLOB.


*/
    virtual int      NumOfFLOBs() const;
    virtual Flob*    GetFLOB(const int i);
    virtual ostream& Print(ostream& os) const;

/*

4.1.5 Area with only static ~SECONDO~ methods


*/
    static Word       In(const ListExpr typeInfo, const ListExpr instance,
                         const int errorPos, ListExpr& errorInfo,
                         bool& correct);
    static ListExpr   Out(ListExpr typeInfo, Word value);
    static Word       Create(const ListExpr typeInfo);
    static void       Delete(const ListExpr typeInfo, Word& w);
    static void       Close(const ListExpr typeInfo, Word& w);
    static Word       Clone(const ListExpr typeInfo, const Word& w);
    static void*      Cast(void* addr );
    static bool       KindCheck(ListExpr type, ListExpr& errorInfo);
    static int        SizeOfObj();
    static ListExpr   Property();

    static const string BasicType() { return "record"; }
    static const bool checkType(ListExpr type){
      ListExpr errorInfo = listutils::emptyErrorInfo();
      return KindCheck(type, errorInfo);
    }
/*
 ~Clear~

Removes all content from this Record.

*/   
  void Clear();
/*
4.2 Private area of class Record

*/

  private:
/*
4.1.1 Constructor area

This constructor should not be used.

*/
    Record(); // Do not use this constructor directly


/*
4.1.2 Private structure and methods

Private data type ~ElemInfo~ to hold element infos

*/

    struct ElemInfo
    {
      char   elemName[MAX_STRINGSIZE + 1];
      char   typeName[MAX_STRINGSIZE + 1];
      int    algebraId;
      int    typeId;
      bool   hasData;
      size_t dataOffset;
      size_t extDataOffset;
    };


/*
~IsRecordTypeList~ checks if the given type list is a record type list.

*/
    static bool IsRecordTypeList(ListExpr typeInfo);

/*
~GetElementValueList~ gets the element value as a list for the given position.

*/
    ListExpr GetElementValueList(int pos) const;


/*
4.1.3 Attributes of Record Type.

~hashValue~ contains the hash value for the whole record.

~noElements~ contains the number of elements in the record

~elemInfoArray~ contains all structured meta information about the elements

~elemData~ a FLOB to save the elements

~elemExtData~ a FLOB to save the element FLOBs

*/
    size_t            hashValue;
    int               noElements;
    DbArray<ElemInfo> elemInfoArray;
    Flob              elemData;
    Flob              elemExtData;
};

#endif // __RECORD_H__

