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

//paragraph [1] title: [{\Large \bf ]   [}]

[1] SemanticTrajectory Algebra
Author: Catherine Higgins



*/

#include "Algebra.h"
#include "NestedList.h"
// Used for importing and exporting the objects
#include "QueryProcessor.h"
#include "StandardTypes.h"
// This is needed for CcInt, CcReal
#include "Attribute.h"
#include "Symbols.h"
#include "ListUtils.h"
// Needed for ListExpr functions
#include "Algebras/Relation-C++/RelationAlgebra.h"
// use of tuples
#include "../../Tools/Flob/DbArray.h"
// Needed for Flob Array
#include "Algebras/Rectangle/RectangleAlgebra.h"
// for bounding box
#include "../../Tools/Flob/Flob.h"
// Needed for flob
#include "Algebras/Rectangle/CellGrid.h"
// Used for operator stcellnumber
#include "Stream.h"
// Needed to process streams
#include <iostream>
#include <string>
#include <cmath>
#include <float.h>
// This is needed for DBL_MAX
#include "StringUtils.h"
//This is needed to tokenized strings
#include "Algebras/Geoid/Geoid.h"
#include <vector>

extern NestedList* nl;
extern QueryProcessor *qp;


using std::cout;
using std::endl;
namespace semtraj {

/*

  1 Data structures for semantictrajectory datatype

  1.1 Struct for a coordinate point <x, y> of type real

*/

struct Coordinate
{
  Coordinate() {}
/*
Do not use this constructor.

*/

  Coordinate( double xcoord, double ycoord ):
    x( xcoord ), y( ycoord )
    {}

  Coordinate(const Coordinate& c) : x(c.x), y(c.y) {}

  Coordinate& operator=(const Coordinate& c){
    x = c.x;
    y = c.y;
    return *this;
  }

  ~Coordinate(){}

  double x;
  double y;
};


/*

1.2 Struct for holding a Text data relating to each coordinate point
Used to hold position of where each string begins in a Flob
Flob contains each string associated to each coordinate point

*/

struct TextData
{
  SmiSize Offset;
  SmiSize Length;
};


/*
1.3 Holds the coordinates of data points in a gridcell2D
Used for the trajectory pruning (divide and conquer)

*/
struct Cell
{
  Cell() {}
/*
Do not use this constructor.

*/

  Cell( int32_t id, int32_t x, int32_t y ):
    tripId( id), origx( x ), origy( y )
    {}

  Cell(const Cell& c) :
  tripId( c.tripId),
  origx( c.origx ),
  origy( c.origy ) {}

  Cell& operator=(const Cell& c){
    tripId = c.tripId;
    origx = c.origx;
    origy = c.origy;
    return *this;
  }

  int32_t GetX() { return origx; }
  int32_t GetY() { return origy; }
  int32_t GetId() { return tripId; }
  ~Cell(){}

  int32_t tripId;
  int32_t origx;
  int32_t origy;
};


/*
1.4
Struct for textual summary
Holds the index number of a word and it's frequency

*/

struct WordST
{
  WordST() {}
/*
Do not use this constructor.

*/

  WordST( int32_t id, int32_t count ):
    indexId( id), count( count )
    {}

  WordST(const WordST& w) :
  indexId( w.indexId),
  count( w.count )
  {}

  WordST& operator=(const WordST& w){
    indexId = w.indexId;
    count = w.count;
    return *this;
  }

  int32_t GetIdxId() { return indexId; }
  int32_t GetCount() { return count; }
  ~WordST(){}

  int32_t indexId;
  int32_t count;
  SmiSize Offset;
  SmiSize Length;

};

/*
1.5
Struct for batch grouping

*/

struct BatchGroup
{
  BatchGroup() {}
/*
Do not use this constructor.

*/

  BatchGroup(Rectangle<2> mbr2):
    mbr(mbr2)
    {
    }

  BatchGroup(const BatchGroup& b) :
  mbr(b.mbr)
  {}

  BatchGroup& operator=(const BatchGroup& b){
    mbr = b.mbr;
    return *this;
  }
  bool addnewMBR(Rectangle<2> mbr2)
  {
    mbr = mbr.Union(mbr2);
    return true;
  }

  Rectangle<2> GetMBR()
  {
    return mbr;
  }
  // Search for ID

  ~BatchGroup(){}

  Rectangle<2> mbr;
};



/*

2 Class SemanticTrajectory

*/
class SemanticTrajectory : public Attribute
{

  public:

    SemanticTrajectory(int dummy);
    //must initialize the attribute and the DbArray
    // using non-standard constructor
    ~SemanticTrajectory();


    SemanticTrajectory(const SemanticTrajectory& st);
    SemanticTrajectory& operator=(const SemanticTrajectory& st);

    void InitializeST()
    {
      SetDefined(true);
    }

    bool EndST(const SemanticTrajectory& st)
    {
      if (!IsDefined())
      {
        Clear();
        SetDefined(false);
        return false;
      }
      coordinates.copyFrom(st.coordinates);
      st_TextData.copyFrom(st.st_TextData);
      st_TextFlob.copyFrom(st.st_TextFlob);
      bbox = st.bbox;
      return true;
    }

    void Clear()
    {
        coordinates.clean();
        st_TextData.clean();
        st_TextFlob.clean();
        cells.clean();
        words.clean();
        sum_TextFlob.clean();
        bbox.SetDefined(false);
    }
    /*
    Functions for Attribute type
    */
    int NumOfFLOBs() const {
      return 6; }
    Flob *GetFLOB(const int i);
    int Compare(const Attribute*) const {
      return 0;}
    bool Adjacent(const Attribute*) const {
      return false;}
    SemanticTrajectory *Clone() const;
    size_t Sizeof() const {
      return sizeof(*this);}
    size_t HashValue() const {
      return 0;}
    void CopyFrom(const Attribute* right);

    /* Functions for semantic datatype */
    bool AddString(const std::string& stString);
    bool AddStringSum(const std::string& stString, int id, int count);
    void AddCoordinate( const Coordinate& p);
    void Destroy();
    Coordinate GetCoordinate( int i ) const;
    bool GetString(int index, std::string& rString) const;
    bool GetStringSum(int index, std::string& rString) const;
    int GetNumCoordinates() const {
      return coordinates.Size(); }
    int GetNumTextData() const {
      return  st_TextData.Size(); }
    const bool IsEmpty() const {
      return GetNumCoordinates() == 0
      && GetNumTextData() == 0; }
    const Rectangle<2> GetBoundingBox( ) const {
      return bbox; }
    void SetBoundingBox(const bool defined,
      const double *min,
      const double *max)
    {
      bbox.Set(true, min, max);
    }
    std::list<std::string> GetStringArray() const;

    /* Functions for spatial summary */
    void AddCell( const Cell& c);
    Cell GetCell( int i ) const {
      assert( 0 <= i && i < GetNumCells() );
      Cell c;
      cells.Get( i, &c );
      return c;
    }

    int GetNumCells() const { return cells.Size(); }
    const bool IsEmptySpatialSum() const {
      return GetNumCells() == 0;}
    DbArray<Cell> GetCellList() const;
    static int CompareX(const void* a, const void* b)
    {
      const double ax = ((Cell*)a)->GetX();
      const double bx = ((Cell*)b)->GetX();
      if (ax < bx)
      {
        return -1;
      }
      else
      {
        return (ax == bx) ? 0 : 1;
      }
    }

    static int CompareY(const void* a, const void* b)
    {
      const double ax = ((Cell*)a)->GetY();
      const double bx = ((Cell*)b)->GetY();
      if (ax < bx)
      {
        return -1;
      }
      else
      {
        return (ax == bx) ? 0 : 1;
      }
    }
    static int CompareString(const void* a, const void* b)
    {
      std::string str1 = ((CcString*)a)->GetValue();
      std::string str2 = ((CcString*)b)->GetValue();
      int res = str1.compare(str2);
      return res;

    }
    static bool
    compare_nocase (const std::string& first,
      const std::string& second)
    {
      unsigned int i=0;
      while ( (i<first.length()) && (i<second.length()) )
      {
        if (first[i] < second[i]) return true;
        else if (first[i]>second[i]) return false;
        ++i;
      }
      return ( first.length() < second.length() );
    }

    WordST GetWord( int i) const {
      assert( 0 <= i && i < GetNumWords() );
      WordST w;
      words.Get( i, &w );
      return w;
    }
    int GetNumWords() const { return words.Size(); }
    const bool IsEmptyTextualSum() const {
      return GetNumWords() == 0;
    }
    static int CompareIndex(const void* a, const void* b)
    {
      const int32_t w1 = ((WordST*)a)->GetIdxId();
      const int32_t w2 = ((WordST*)b)->GetIdxId();
      if (w1 < w2)
      {
        return -1;
      }
      else
      {
        return (w1 == w2) ? 0 : 1;
      }
    }
    bool FindIdx(const int& key, int& pos)
    {
        return words.Find(&key, CompareIndex, pos);
    }
    bool replaceWord(WordST& w, int i)
    {
      return words.Put(i, w);
    }
    DbArray<WordST> GetTextSumList() const;
    /*
    OPERATOR HELPER FUNCTIONS
    */

    double Similarity(SemanticTrajectory& st, double diag,
      double alpha);
    double Relevance(int index,SemanticTrajectory& st,
      double diag, double alpha);
    double Sim(int i, int y, SemanticTrajectory& st,
      double diag, double alpha);
    double SpatialDist(int i, int y,
      SemanticTrajectory& st);
    double TextualScore(int i, int y,
      SemanticTrajectory& st);
    double GetDiagonal(Rectangle<2> &rec);

    /*
    OPERATOR HELPER FUNCTIONS
    FOR BBSim
    */


    /* The usual functions */
    static Word In( const ListExpr typeInfo,
      const ListExpr instance,
      const int errorPos,
      ListExpr& errorInfo,
      bool& correct );

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word Create( const ListExpr typeInfo );

    static void Delete(
      const ListExpr typeInfo,
      Word& w );

    static void Close(
      const ListExpr typeInfo,
      Word& w );

    static bool Save( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );

    static bool Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );

    static Word Clone( const ListExpr typeInfo,
      const Word& w );

    static bool KindCheck( ListExpr type,
      ListExpr& errorInfo );

    static int SizeOfObj();

    static ListExpr Property();

    static void* Cast(void* addr);

    static const std::
    string BasicType() { return "semantictrajectory"; }
    static const bool checkType(const ListExpr type)
    {
      return listutils::isSymbol(type, BasicType());
    }

  private:
    /*
    Constructor should never be used
    */
    SemanticTrajectory() {}
    /*
    DbArray to hold coordinate points
    */
    DbArray<Coordinate> coordinates;

    /*
    Holds position of each Text at each coordinate
    */
    DbArray<TextData> st_TextData;
    /*
    Holds all characters for one semantic trajectory
    */
    Flob st_TextFlob;
    /*
    DbArray to hold Cell data
    */
    DbArray<Cell> cells;
    /*
    DbArray to hold word data
    */
    DbArray<WordST> words;
    /*

    */
    Flob sum_TextFlob;
    /*
    The bounding box that encloses the SemanticTrajectory
    */
    Rectangle<2> bbox;
};

/*

Non-stardard constructor must be used when using Flobs or DbArrays
Initialize each type of Flob to 0

*/

SemanticTrajectory::SemanticTrajectory(int dummy):
  Attribute(true),
  coordinates(0),
  st_TextData(0),
  st_TextFlob(0),
  cells(0),
  words(0),
  sum_TextFlob(0),
  bbox(false)
{

}

SemanticTrajectory::
SemanticTrajectory(const SemanticTrajectory& st) :
  Attribute(st.IsDefined()),
  coordinates(st.coordinates.Size()),
  st_TextData(st.st_TextData.Size()),
  st_TextFlob(st.st_TextFlob.getSize()),
  cells(st.cells.Size()),
  words(st.words.Size()),
  sum_TextFlob(st.sum_TextFlob.getSize()),
  bbox(st.bbox)
{

  bool success = false;
  success = st_TextData.Append(st.st_TextData);
  assert(success);
  success = coordinates.Append(st.coordinates);
  assert(success);
  success = st_TextFlob.copyFrom(st.st_TextFlob);
  assert(success);
  success = cells.Append(st.cells);
  assert(success);
  success = words.Append(st.words);
  success = sum_TextFlob.copyFrom(st.sum_TextFlob);
  assert(success);
  bbox = st.bbox;
  // Maybe add here bbox is defined ?
  // Is that part of the rectangle class

}

SemanticTrajectory::~SemanticTrajectory()
{

}

SemanticTrajectory& SemanticTrajectory::
operator=(const SemanticTrajectory& st)
{
  SetDefined(st.IsDefined());

  bool success = false;
  success = st_TextData.clean();
  assert(success);
  success = st_TextData.copyFrom(st.st_TextData);
  assert(success);
  success = coordinates.clean();
  assert(success);
  success = coordinates.copyFrom(st.coordinates);
  assert(success);
  success = st_TextFlob.clean();
  assert(success);
  success = st_TextFlob.copyFrom(st.st_TextFlob);
  bbox = st.bbox;
  success = cells.clean();
  assert(success);
  success = cells.Append(st.cells);
  assert(success);
  success = words.clean();
  assert(success);
  success = words.Append(st.words);
  success = sum_TextFlob.clean();
  assert(success);
  success = sum_TextFlob.copyFrom(st.sum_TextFlob);
  return *this;
}


/*

i == 0 return coordinates
i == 1 return st\_TextData
i == 2 return st\_TextFlob
i == 3 returns cells
i == 4 returns words

*/

Flob *SemanticTrajectory::GetFLOB(const int i)
{
  Flob* stFlob = 0;
  if (i == 0) {
    stFlob = &coordinates;
  } else if (i == 1) {
    stFlob = &st_TextData;
  } else if (i == 2) {
    stFlob = &st_TextFlob;
  } else if (i == 3) {
    stFlob = &cells;
  } else if (i == 4) {
    stFlob = &words;
  } else if (i == 5)
  {
    stFlob = &sum_TextFlob;
  }
  return stFlob;
}

/*
Add a coordinate to the DbArray

*/
void SemanticTrajectory::
AddCoordinate(
  const Coordinate& c)
{
  coordinates.Append(c);

}

Coordinate SemanticTrajectory::
GetCoordinate( int i ) const {
  assert( 0 <= i && i < GetNumCoordinates() );
  Coordinate c;
  coordinates.Get( i, &c );
  return c;
}

/*
This function is responsible of adding a string to the Text

*/

bool SemanticTrajectory::
AddString(const std::string& stString)
{

  if (!stString.empty()) {
    TextData td;
    td.Offset = st_TextFlob.getSize();
    td.Length = stString.length();
    bool success = st_TextData.Append(td);
    assert(success);
    st_TextFlob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
}

bool SemanticTrajectory::AddStringSum(
  const std::string& stString, int id, int count)
{
  if (!stString.empty()) {
    WordST td;
    td.Offset = sum_TextFlob.getSize();
    td.Length = stString.length();
    td.indexId = id;
    td.count = count;
    bool success = words.Append(td);
    assert(success);
    sum_TextFlob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
}


std::list<std::string> SemanticTrajectory::GetStringArray() const
{


  std::list<std::string> stringArray;

  int nStrings = st_TextData.Size();
  bool bOK = false;
  std::string holdString;

  for(int i = 0; i < nStrings; i++)
  {
    bOK = GetString(i, holdString);
    stringutils::StringTokenizer parse(holdString, " ");
    if (bOK == true)
    {
      while(parse.hasNextToken())
      {
        std::string eval = parse.nextToken();
        stringutils::trim(eval);
        stringArray.push_back(eval);
       }
    }
  }
  stringArray.sort(SemanticTrajectory::compare_nocase);
  return stringArray;
}



bool SemanticTrajectory::
GetString(int index, std::string& rString) const
{

  bool success = false;
  int numString = st_TextData.Size();
  if (index < numString)
  {
      TextData textData;
      success = st_TextData.Get(index, &textData);
      if (success == true)
      {
        SmiSize bufferSize = textData.Length + 1;
        char* pBuffer = new char[bufferSize];
        if (pBuffer != 0)
        {
          memset(
            pBuffer,
            0,
            bufferSize * sizeof(char));
          success = st_TextFlob.read(
            pBuffer,
            textData.Length,
            textData.Offset);
        }
        if(success == true)
        {
          rString = pBuffer;
        }
        delete [] pBuffer;
      }
  }
  return success;
}

bool SemanticTrajectory::
GetStringSum(int index, std::string& rString) const
{

  bool success = false;
  int numString = words.Size();
  if (index < numString)
  {
      WordST textData;
      success = words.Get(index, &textData);
      if (success == true)
      {
        SmiSize bufferSize = textData.Length + 1;
        char* pBuffer = new char[bufferSize];
        if (pBuffer != 0)
        {
          memset(
            pBuffer,
            0,
            bufferSize * sizeof(char));
            success = sum_TextFlob.read(
            pBuffer,
            textData.Length,
            textData.Offset);
        }
        if(success == true)
        {
          rString = pBuffer;
        }
        delete [] pBuffer;
      }
  }
  return success;
}


/*
This takes the attribute we wish to CopyFrom

*/
void SemanticTrajectory::CopyFrom(
  const Attribute* right)
{

  if(right != 0)
  {
    const SemanticTrajectory* st =
    static_cast<
    const SemanticTrajectory*>(right);
    if(st != 0)
    {
      *this = *st;
    }
  }
}

SemanticTrajectory*
SemanticTrajectory::Clone() const
{

  SemanticTrajectory *st =
  new SemanticTrajectory(*this);
  assert(st != 0);
  return st;

}



/*
Add a cell to the DbArray

*/
void SemanticTrajectory::
AddCell(const Cell& c)
{
  cells.Append(c);
}

DbArray<Cell> SemanticTrajectory::
GetCellList() const
{
  assert(IsDefined());
  return cells;
}

DbArray<WordST> SemanticTrajectory::
GetTextSumList() const
{
  assert(IsDefined());
  return words;
}


/*
  List represenation
  TODO UPDATE THIS INFO AS IT IS NOT UP TO DATE
   ((id xl xr yl yr)...(id xl xr yl yr))
  Nested list of two: first is a 4 coordinates to represent a rectangle

*/


ListExpr
SemanticTrajectory::Out(
  ListExpr typeInfo,
  Word value )
{

  // the addr pointer of value
  SemanticTrajectory* st =
  (SemanticTrajectory*)(value.addr);
  if (!st->IsDefined()) {
      return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if( st->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {
    std::string holdValue;
    bool success = st->GetString(0, holdValue);

    if (success == false) {
      holdValue = "";
    }
    ListExpr result =
      nl->OneElemList(
        nl->ThreeElemList(
          nl->RealAtom(st->GetCoordinate(0).x),
          nl->RealAtom(st->GetCoordinate(0).y),
          nl->TextAtom(holdValue)));
    ListExpr last = result;

    for( int i = 1; i < st->GetNumCoordinates(); i++ )
    {
      bool success = st->GetString(i, holdValue);
      if (success == false) {
        holdValue = "";
      }

      last = nl->Append(last,
               nl->ThreeElemList(
               nl->RealAtom(st->GetCoordinate(i).x),
               nl->RealAtom(st->GetCoordinate(i).y),
               nl->TextAtom(holdValue)));

    }

    ListExpr textual = nl->Empty();
    ListExpr spatial = nl->Empty();

    /* Need to retrieve spatial information if any */
    if (!st->IsEmptySpatialSum())
    {

      spatial =
          nl->OneElemList(
            nl->ThreeElemList(
              nl->IntAtom(st->GetCell(0).tripId),
              nl->IntAtom(st->GetCell(0).origx),
              nl->IntAtom(st->GetCell(0).origy)
            )
          );
       ListExpr spatiallast = spatial;

       for( int i = 1; i < st->GetNumCells(); i++ )
       {
         spatiallast = nl->Append(spatiallast,
               nl->ThreeElemList(
                 nl->IntAtom(st->GetCell(i).tripId),
                 nl->IntAtom(st->GetCell(i).origx),
                 nl->IntAtom(st->GetCell(i).origy)
               )
           );
        }

    }

    if (!st->IsEmptyTextualSum())
    {

      std::string holdValue;
      bool success = st->GetStringSum(0, holdValue);

      if (success == false) {
        holdValue = "";
      }
      textual =
          nl->OneElemList(
            nl->ThreeElemList(
              nl->IntAtom(st->GetWord(0).indexId),
              nl->IntAtom(st->GetWord(0).count),
              nl->TextAtom(holdValue)
            )
          );
       ListExpr textuallast = textual;
       for( int i = 1; i < st->GetNumWords(); i++ )
       {

         bool success = st->GetStringSum(i, holdValue);

         if (success == false) {
           holdValue = "";
         }
         textuallast = nl->Append(textuallast,
               nl->ThreeElemList(
                 nl->IntAtom(st->GetWord(i).indexId),
                 nl->IntAtom(st->GetWord(i).count),
                 nl->TextAtom(holdValue)
               )
           );
        }

    }

    ListExpr final =
      nl->FourElemList(
        nl->FourElemList(
          nl->RealAtom(st->bbox.getMinX()),
          nl->RealAtom(st->bbox.getMinY()),
          nl->RealAtom(st->bbox.getMaxX()),
          nl->RealAtom(st->bbox.getMaxY())
        ),
        spatial,
        textual,
        result
        );

    return final;
  }
}

/*
  List represenation
  Nested list of four:
    first nested list: Represents bounding box which is a 4 coordinates
     to represent a rectangle
    second nested list: Represents the spatial summary
    (this can be expected to be empty until operator is makespatialsum
     is called)
    third nested list: Represents the textual summary
    (this can be expected to be empty until operator is called)
    fourth nested list: Represents the trajectory data

  TODO UPDATE LIST REPRESENTATION HERE ~ This is no longer accurate


*/

/* The bouding box get's calculated everytime */

Word
SemanticTrajectory::In( const ListExpr typeInfo,
const ListExpr instance,
const int errorPos,
ListExpr& errorInfo,
bool& correct )
{

  Word word;
  correct = false;
  SemanticTrajectory* st = new SemanticTrajectory(0);
  st->SetDefined(true);

  // This is if list is undefined
  //  ~ setDefined to false
  if(listutils::isSymbolUndefined( instance ))
  {
    st->SetDefined(false);
    correct = true;
    word.addr = st;
    return word;
  }

  if (nl != 0)
  {

    int nListLength = nl->ListLength(instance);
    // If list has 4 arguments
    // and undefined content
    //~ setDefined to false
    if (nListLength == 4 &&
    nl->IsAtom(nl->Fourth(instance)) &&
    nl->AtomType(nl->Fourth(instance)) == SymbolType &&
    listutils::isSymbolUndefined(nl->Fourth(instance)))
    {
      correct = true;
      st->SetDefined(false);
      correct = true;
      word.addr = st;
      return word;
    }

    if (nListLength == 4)
    {
      ListExpr first = nl->Empty();
      ListExpr rest = nl->Fourth(instance);
      double mind[2];
      double maxd[2];
      double c_x1;
      double c_y1;
      double c_x2;
      double c_y2;
      /* TODO Add check here to make sure
      list rest is at least two points
       to be valid semantic trajectory (maybe)*/
      while (nl->IsEmpty(rest) == false)
      {
        first = nl->First( rest );
        rest = nl->Rest( rest );

        if( nl->ListLength(first) == 3 &&
            nl->IsAtom(nl->First(first)) &&
            nl->AtomType(nl->First(first))
            == RealType &&
            nl->IsAtom(nl->Second(first)) &&
            nl->AtomType(nl->Second(first))
            == RealType &&
            nl->IsAtom(nl->Third(first)))
        {
          double x1 = nl->RealValue(nl->First(first));
          double y1 = nl->RealValue(nl->Second(first));
          Coordinate c(x1, y1);
          st->AddCoordinate(c);
          bool typeString =
          nl->AtomType(nl->Third(first)) == StringType;
          bool typeText =
          nl->AtomType(nl->Third(first)) == TextType;
          std::string stringValue;
          if (typeString == true)
          {
            stringValue =
            nl->StringValue(nl->Third(first));
          }
          if (typeText == true)
          {
            stringValue =
            nl->TextValue(nl->Third(first));
          }

          st->AddString(stringValue);

          /* Set bounding box here */
          if (st->GetNumCoordinates() == 1)
          {
            c_x1 = x1;
            c_y1 = y1;
            c_x2 = x1;
            c_y2 = y1;
          }
          else
          {
            c_x1 = std::min(c_x1, x1);
            c_y1 = std::min(c_y1, y1);
            c_x2 = std::max(c_x2, x1);
            c_y2 = std::max(c_y2, y1);

          }
        }
        else
        {
          correct = false;
          delete st;
          return SetWord( Address(0) );
        }
        if (c_x1 < c_x2 && c_y1 < c_y2)
        {
          mind[0] = c_x1;
          mind[1] = c_y1;
          maxd[0] = c_x2;
          maxd[1] = c_y2;
          st->bbox.Set(true, mind, maxd);
        }
      }
      /* TODO add extra checks and conditions */
      /* Extract info for spatial summary if any */
      if (!listutils::isSymbolUndefined(
        nl->Second(instance)))
      {

        ListExpr first = nl->Empty();
        ListExpr rest = nl->Second(instance);

        while (nl->IsEmpty(rest) == false)
        {
          first = nl->First( rest );
          rest = nl->Rest( rest );
          if( nl->ListLength(first) == 3 &&
              nl->IsAtom(nl->First(first)) &&
              nl->AtomType(nl->First(first))
              == IntType &&
              nl->IsAtom(nl->Second(first)) &&
              nl->AtomType(nl->Second(first))
              == IntType &&
              nl->IsAtom(nl->Third(first)) &&
              nl->AtomType(nl->Third(first))
              == IntType
            )
          {
            Cell c(nl->IntValue(nl->First(first)),
                   nl->IntValue(nl->Second(first)),
                   nl->IntValue(nl->Third(first))
                 );
            st->AddCell(c);

          }
          else
          {
            correct = false;
            delete st;
            return SetWord( Address(0) );
          }


        } // end of while
      } // end of if
      /* TODO add extra checks and conditions */
      /* Extract info for textual summary if any */
      if (!listutils::isSymbolUndefined(nl->Third(instance)))
      {
        ListExpr first = nl->Empty();
        ListExpr rest = nl->Third(instance);

        while (nl->IsEmpty(rest) == false)
        {
          first = nl->First( rest );
          rest = nl->Rest( rest );
          if( nl->ListLength(first) == 3 &&
              nl->IsAtom(nl->First(first)) &&
              nl->AtomType(nl->First(first))
              == IntType &&
              nl->IsAtom(nl->Second(first)) &&
              nl->AtomType(nl->Second(first))
              == IntType && nl->IsAtom(nl->Third(first))
            )
          {

            bool typeString =
            nl->AtomType(nl->Third(first)) == StringType;

            std::string stringValue;
            if (typeString == true)
            {
              stringValue =
              nl->StringValue(nl->Third(first));
            } else {
              stringValue =
              nl->TextValue(nl->Third(first));
            }

            st->AddStringSum(stringValue,
              nl->IntValue(nl->First(first)),
              nl->IntValue(nl->Second(first)));

          }
          else
          {
            correct = false;
            delete st;
            return SetWord( Address(0) );
          }
        }
      }

    } // end of if n ===4
    correct = true;
    return SetWord(st);
  } // end of n != 0

  return word;
}


/*
3.3
Function Describing the Signature
of the Type Constructor
TODO update when textual sum is added

*/
ListExpr
SemanticTrajectory::Property()
{
  return
  (nl->TwoElemList(
     nl->FiveElemList(
        nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List"),
        nl->StringAtom("Remarks")),
     nl->FiveElemList(nl->StringAtom(
        "->" + Kind::DATA()
        ),
        nl->StringAtom(
        SemanticTrajectory::BasicType()
        ),
        nl->TextAtom(
          "((minx miny maxx maxy)"
        "((tripid xl1 xr1 yl1 yr1)"
        "(tripid xl xr yl yr))"
        "()(<x1> <y1> <Text1>)..."
        "(<xn> <yn> <Text2>))"),
        nl->TextAtom(
        "((3.2 15.4 6.34 15.4)"
        "()()"
        "((3.2 15.4 text1)"
        "(6.34 20.8 text2)))"),
        nl->TextAtom(
        "x- and y-coordinates"
        " must be "
        "of type real."))));
}

/*
3.4 Kind Checking Function

This function checks whether the type constructor is applied correctly.
Since type constructor ~semanticTrajectory~ does
not have arguments, this is trivial.

*/
bool
SemanticTrajectory::KindCheck(
  ListExpr type,
  ListExpr& errorInfo )
{
  return (nl->IsEqual(
    type,
    SemanticTrajectory::BasicType() ));
}

/*

3.5 ~Create~-function

*/
Word SemanticTrajectory::Create(
  const ListExpr typeInfo)
{
  SemanticTrajectory* st = new SemanticTrajectory(0);
  return (SetWord(st));
}




void SemanticTrajectory::Destroy()
{

  st_TextData.Destroy();
  st_TextFlob.destroy();
  coordinates.Destroy();
  cells.Destroy();
  words.Destroy();
  sum_TextFlob.destroy();
}

/*
3.6 ~Delete~-function

*/
void SemanticTrajectory::Delete(
  const ListExpr typeInfo,
  Word& w)
{

  SemanticTrajectory* st =
  (SemanticTrajectory*)w.addr;
  st->Destroy();
  delete st;
}

/*
3.6 ~Open~-function

*/
bool
SemanticTrajectory::Open(
             SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{

  SemanticTrajectory *st =
  (SemanticTrajectory*)Attribute::Open(
    valueRecord, offset, typeInfo );
  value.setAddr( st );

  return true;
}

/*
3.7 ~Save~-function

*/
bool
SemanticTrajectory::Save( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{

  SemanticTrajectory *st =
  (SemanticTrajectory *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, st );
  return true;
}

/*
3.8 ~Close~-function

*/
void SemanticTrajectory::Close(
  const ListExpr typeInfo,
  Word& w)
{

  SemanticTrajectory* st = (SemanticTrajectory*)w.addr;
  delete st;
}

Word SemanticTrajectory::Clone(
  const ListExpr typeInfo,
  const Word& rWord)
{

  Word word;
  SemanticTrajectory* pSemanticTrajectory =
  static_cast<SemanticTrajectory*>(rWord.addr);
  if (pSemanticTrajectory != 0)
  {
    word.addr =
    new SemanticTrajectory(*pSemanticTrajectory);
    assert(word.addr != 0);
  }
  return word;
}


/*
3.9 ~SizeOf~-function

*/
int SemanticTrajectory::SizeOfObj()
{
  return sizeof(SemanticTrajectory);
}

/*
3.10 ~Cast~-function

*/
void* SemanticTrajectory::Cast(void* addr)
{

  return (new (addr) SemanticTrajectory);
}


/*

Operator helper functions

*/

double SemanticTrajectory::Relevance(int i,
  SemanticTrajectory& st,
  double diag, double alpha)
{
  // Return the max of all return similarity equation

  double max = -999999; /*TODO use a max varialble */
  int numOfCoor2 = st.GetNumCoordinates();
  for (int y = 0; y <numOfCoor2; y++)
  {
    double temp = Sim(i,y,st, diag, alpha);

    if (temp > max)
    {
      max = temp;
    }
  }

  return max;
}


double SemanticTrajectory::Sim(int i, int y,
  SemanticTrajectory& st,
  double diag, double alpha)
{
  double dist = SpatialDist(i, y, st);

  double normalizedscore = 1 - (double)(dist/diag);
  // It's inversly proportional to the
  //  distance so we need to substract from 1
  double result = alpha * normalizedscore
  + (1 - alpha) * TextualScore(i, y, st);
  return result;
}

double SemanticTrajectory::GetDiagonal(
  Rectangle<2>& rec){
  double x1 = rec.getMinX();
  double y1 = rec.getMinY();
  double x2 = rec.getMaxX();
  double y2 = rec.getMaxY();
  double result = sqrt(pow((x1 - x2),2)
  + pow((y1 - y2),2));
  return result;
}

double GetDiag(
  Rectangle<2>& rec){
  double x1 = rec.getMinX();
  double y1 = rec.getMinY();
  double x2 = rec.getMaxX();
  double y2 = rec.getMaxY();
  double result = sqrt(pow((x1 - x2),2)
  + pow((y1 - y2),2));
  return result;
}

double SemanticTrajectory::SpatialDist(int i, int y,
  SemanticTrajectory& st)
{
  double x1 = GetCoordinate(i).x;
  double y1 = GetCoordinate(i).y;
  double x2 = st.GetCoordinate(y).x;
  double y2 = st.GetCoordinate(y).y;

  double result =
  sqrt(pow((x1 - x2),2)
  + pow((y1 - y2),2));
  return result;
}


double SemanticTrajectory::TextualScore(int i, int y,
  SemanticTrajectory& st)
{

    bool success = false;
    std::string s1;
    success = GetString(i, s1);
    assert(success);
    std::string s2;
    success = st.GetString(y, s2);
    assert(success);
    if (s1.length() > 0 && s2.length() > 0)
    {

      int numMatches = 0;
      stringutils::StringTokenizer st1(s1, " ");
      stringutils::StringTokenizer st2(s2, " ");
      int numToken1 = 0;
      int numToken2 = 0;
      std::string eval = "";
      std::string eval2 = "";
      bool done1 = false;
      bool done2 = false;
      if (st1.hasNextToken())
      {
        eval = st1.nextToken();
        numToken1++;
      } else {
        done1 = true;
      }
      if (st2.hasNextToken())
      {
        eval2 = st2.nextToken();
        numToken2++;
      } else {
        done2 = true;
      }
      std::string prev_str = "";
      int duplicate = 0;
      while(!done1 || !done2)
      {
        if((eval).compare(eval2) == 0)
        {
          if ((prev_str).compare(eval) != 0)
          {
            numMatches++;
          } else {
            duplicate = duplicate + 2;
          }
          prev_str = eval;

          if (st1.hasNextToken())
          {
            eval = st1.nextToken();
            numToken1++;
          } else {
            done1 = true;
          }
          if (st2.hasNextToken())
          {
            eval2 = st2.nextToken();
            numToken2++;
          } else {
            done2 = true;
          }
        }

        else if ((eval).compare(eval2) < 0) {
          if ((prev_str).compare(eval) == 0 || (prev_str).compare(eval2) == 0)
          {
            duplicate++;
          } else {
            prev_str = "";
          }
          if (st1.hasNextToken())
          {
            eval = st1.nextToken();
            numToken1++;
          } else
          {
            done1 = true;
            if (st2.hasNextToken())
            {
              eval2 = st2.nextToken();
              numToken2++;
            } else { done2 = true;}
          }

        } else {
          if ((prev_str).compare(eval) == 0 || (prev_str).compare(eval2) == 0)
          {
            duplicate++;
          } else {
            prev_str = "";
          }
          if (st2.hasNextToken())
          {
            eval2 = st2.nextToken();
            numToken2++;
          }
          else
          {
            done2 = true;
            if (st1.hasNextToken())
            {
              eval = st1.nextToken();
              numToken1++;
            } else { done1 = true;}
          }
        }
      }

      if (numMatches == 0.0)
      {
        return 0.0;
      }
      double uniquewords = (double)
      (numToken2 + numToken1 - duplicate - numMatches);
      double result = (double) numMatches / uniquewords;
      return result;
    }


    return 0.0;
}

double SemanticTrajectory::Similarity(
  SemanticTrajectory& st,
  double diag,
  double alpha)
{

  int numCoordinates1 = GetNumCoordinates();
  double sumOfRelevance1 = 0;
  double leftTotal = 0;

  for (int i = 0; i < numCoordinates1; i++)
  {
    sumOfRelevance1 =
    sumOfRelevance1 + Relevance(i, st, diag, alpha);
  }

  leftTotal =
  sumOfRelevance1 / (double) numCoordinates1;

  int numCoordinates2 = st.GetNumCoordinates();
  double sumOfRelevance2 = 0;
  double rightTotal = 0;
  for (int i = 0; i < numCoordinates2; i++)
  {
    sumOfRelevance2 =
    sumOfRelevance2 + st.Relevance(
    i, *this, diag, alpha);
  }
  rightTotal =
  sumOfRelevance2 / (double) numCoordinates2;


  double result  = rightTotal + leftTotal;
  cout << result << endl;
  return result;
}


/*
OPERATOR Functions

*/



/*
Operator ~makesemtraj~

*/

ListExpr TypeMapMakeSemtraj(ListExpr args){

  // Check to see if it's the right number of arguments
  if (nl->HasLength(args, 4))
  {
    // Make sure each param is of right type
    std::string err = "stream(tuple) x attr_1 x"
    " attr_2 x attr_3 expected";

    ListExpr stream = nl->First(args);
    ListExpr attrname_longitude = nl->Second(args);
    ListExpr attrname_latitude = nl->Third(args);
    ListExpr attrname_semantics = nl->Fourth(args);

    if(!listutils::isTupleStream(stream)){
      return
      listutils::typeError(
        "first parameter must be a tuple stream");
    }

    if(!listutils::isSymbol(attrname_longitude)){
      return
      listutils::typeError("second parameter must"
      "be an attribute name");
    }

    if(!listutils::isSymbol(attrname_latitude)){
      return
      listutils::typeError("third parameter must"
      " be an attribute name");
    }

    if(!listutils::isSymbol(attrname_semantics)){
      return
      listutils::typeError("fourth parameter must"
      "be an attribute name");
    }

    ListExpr type;
    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(stream));

    // Get the index for the longitude
    std::string name =
    nl->SymbolValue(attrname_longitude);
    int index1 =
    listutils::findAttribute(attrList, name, type);
    if(index1==0){
      return
      listutils::typeError(
        "attribute name " + name +
       " unknown in tuple"
       " stream");
    }
    if(!CcReal::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'real'");
    }

    name = nl->SymbolValue(attrname_latitude);

    int index2 =
    listutils::findAttribute(attrList, name, type);

    if(index2==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }
    if(!CcReal::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'real'");
    }

    name = nl->SymbolValue(attrname_semantics);
    int index3 =
    listutils::findAttribute(attrList, name, type);
    if(index3==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }
    if(!CcString::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'string'");
    }

    std::string restype =
    SemanticTrajectory::BasicType();


    ListExpr indexes = nl->ThreeElemList(
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->IntAtom(index3-1));

    return
    nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             indexes,
                             nl->SymbolAtom(restype));
  }
  return
  listutils::typeError("Wrong number of arguments");

}




int MakeSemTrajMV(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){

  result = qp->ResultStorage(s);
  SemanticTrajectory* res =
   static_cast<SemanticTrajectory*>(result.addr);

  int noargs = qp->GetNoSons(s);

  int idx1 = ((CcInt*)args[noargs-3].addr)->GetValue();
  int idx2 = ((CcInt*)args[noargs-2].addr)->GetValue();
  int idx3 = ((CcInt*)args[noargs-1].addr)->GetValue();

  double c_x1;
  double c_y1;
  double c_x2;
  double c_y2;
  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();
  while((tuple = stream.request()))
  {
    CcReal* x = (CcReal*) tuple->GetAttribute(idx1);
    CcReal* y = (CcReal*) tuple->GetAttribute(idx2);
    CcString* sem = (CcString*)
    tuple->GetAttribute(idx3);
    double x1 = x->GetValue();
    double y1 = y->GetValue();

    Coordinate c(x1, y1);
    res->AddCoordinate(c);
    std::string str = sem->GetValue();
    std::string resu = "";
    for (size_t i = 0; i < str.size(); ++i)
    {
      if ((str[i] >= 'a' && str[i] <= 'z')
      || (str[i] >= 'A' && str[i] <= 'Z') || str[i] == ' ') {
        resu = resu + str[i];
      }
    }

    std::list<std::string> tokenlist;
    stringutils::StringTokenizer st1(resu, " ");
    while(st1.hasNextToken())
    {
      std::string eval = st1.nextToken();
      stringutils::trim(eval);
      stringutils::toLower(eval);
      tokenlist.push_back(eval);
    }
    std::string finalstr = "";
    tokenlist.sort(SemanticTrajectory::compare_nocase);
    int size = tokenlist.size();
    int i = 0;
    for(const auto &word : tokenlist)
    {
     if (i != size - 1)
     {
        finalstr += word + " ";
     }
     else
     {
        finalstr += word;
     }
     i++;
    }

    res->AddString(finalstr);

    if (res->GetNumCoordinates() == 1)
    {
      c_x1 = x1;
      c_y1 = y1;
      c_x2 = x1;
      c_y2 = y1;
    }
    else
    {
      c_x1 = std::min(c_x1, x1);
      c_y1 = std::min(c_y1, y1);
      c_x2 = std::max(c_x2, x1);
      c_y2 = std::max(c_y2, y1);

    }

    tuple->DeleteIfAllowed();
  }
  if (c_x1 < c_x2 && c_y1 < c_y2)
  {
    double mind[2];
    double maxd[2];
    mind[0] = c_x1;
    mind[1] = c_y1;
    maxd[0] = c_x2;
    maxd[1] = c_y2;
    res->SetBoundingBox(true, mind, maxd);
  }
  stream.close();
  return 0;
}

ListExpr TypeMapMakeSemtraj2(ListExpr args){
  // Check to see if it's the right number of arguments
  if (nl->HasLength(args, 6))
  {
    // Make sure each param is of right type
    std::string err = "stream(tuple) x stream(tuple) x a1 x"
    " a2 x a3 x a4 expected";

    ListExpr stream = nl->First(args);
    ListExpr stream2 = nl->Second(args);
    ListExpr attrname_longitude = nl->Third(args);
    ListExpr attrname_latitude = nl->Fourth(args);
    ListExpr attrname_semantics = nl->Fifth(args);
    ListExpr attrname_elem = nl->Sixth(args);

    if(!listutils::isTupleStream(stream)){
      return
      listutils::typeError(
        "first parameter must be a tuple stream");
    }
    if(!listutils::isTupleStream(stream2)){
      return
      listutils::typeError(
        "first parameter must be a tuple stream");
    }
    if(!listutils::isSymbol(attrname_longitude)){
      return
      listutils::typeError("second parameter must"
      "be an attribute name");
    }

    if(!listutils::isSymbol(attrname_latitude)){
      return
      listutils::typeError("third parameter must"
      " be an attribute name");
    }

    if(!listutils::isSymbol(attrname_semantics)){
      return
      listutils::typeError("fourth parameter must"
      "be an attribute name");
    }
    if(!listutils::isSymbol(attrname_elem)){
      return
      listutils::typeError("fourth parameter must"
      "be an attribute name");
    }
    ListExpr type;
    // extract the attribute list
    ListExpr attrList = nl->Second(nl->Second(stream));
    ListExpr attrList2 = nl->Second(nl->Second(stream2));
    // Get the index for the longitude
    std::string name =
    nl->SymbolValue(attrname_longitude);
    int index1 =
    listutils::findAttribute(attrList, name, type);
    if(index1==0){
      return
      listutils::typeError(
        "attribute name " + name +
       " unknown in tuple"
       " stream");
    }
    if(!CcReal::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'real'");
    }

    name = nl->SymbolValue(attrname_latitude);

    int index2 =
    listutils::findAttribute(attrList, name, type);

    if(index2==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }
    if(!CcReal::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'real'");
    }

    name = nl->SymbolValue(attrname_semantics);
    int index3 =
    listutils::findAttribute(attrList, name, type);
    if(index3==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }
    if(!CcString::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'string'");
    }

    name = nl->SymbolValue(attrname_elem);
    int index4 =
    listutils::findAttribute(attrList2, name, type);
    if(index4==0){
      return
      listutils::typeError("attribute elem " + name +
      " unknown in tuple stream");
    }
    if(!CcString::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'string'");
    }
    std::string restype =
    SemanticTrajectory::BasicType();


    ListExpr indexes = nl->FourElemList(
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->IntAtom(index3-1),
                         nl->IntAtom(index4-1));

    return
    nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             indexes,
                             nl->SymbolAtom(restype));
  }
  return
  listutils::typeError("Wrong number of arguments");

}




int MakeSemTrajMV2(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){


  result = qp->ResultStorage(s);
  SemanticTrajectory* res =
   static_cast<SemanticTrajectory*>(result.addr);

  int noargs = qp->GetNoSons(s);

  int idx1 = ((CcInt*)args[noargs-4].addr)->GetValue();
  int idx2 = ((CcInt*)args[noargs-3].addr)->GetValue();
  int idx3 = ((CcInt*)args[noargs-2].addr)->GetValue();
  int idx4 = ((CcInt*)args[noargs-1].addr)->GetValue();

  //Prepare stop list
  Stream<Tuple> stream2(args[1]);
  unsigned int bucketnum = 800;
  std::hash<std::string> str_hash;
  std::vector<std::string*>** hashTable;
  hashTable = new std::vector<std::string*>*[bucketnum];
  for (unsigned int i = 0; i < bucketnum; i++)
  {
    hashTable[i] = 0;
  }
  Tuple* tuple2;
  stream2.open();
  while((tuple2 = stream2.request()))
  {
    CcString* stopw = (CcString*)
    tuple2->GetAttribute(idx4);
    std::string stw = stopw->GetValue();
    std::string resu = "";
    for (size_t i = 0; i < stw.size(); ++i)
    {
      if ((stw[i] >= 'a' && stw[i] <= 'z') ||
       (stw[i] >= 'A' && stw[i] <= 'Z')) {
        resu = resu + stw[i];
      }
    }
    stringutils::trim(resu);
    stringutils::toLower(resu);
    std::string* stn = new std::string(resu);
    size_t hash = str_hash(resu) % bucketnum;

    if (!hashTable[hash])
    {
      hashTable[hash] = new std::vector<std::string*>();
    }
    hashTable[hash]->push_back(stn);
    tuple2->DeleteIfAllowed();
  }
  stream2.close();

  double c_x1;
  double c_y1;
  double c_x2;
  double c_y2;
  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();
  unsigned int bucketpos = 0;
  while((tuple = stream.request()))
  {
    CcReal* x = (CcReal*) tuple->GetAttribute(idx1);
    CcReal* y = (CcReal*) tuple->GetAttribute(idx2);
    CcString* sem = (CcString*)
    tuple->GetAttribute(idx3);
    if ((sem->GetValue()).length() > 0)
    {
      std::string str = sem->GetValue();
      std::string resu = "";
      for (size_t i = 0; i < str.size(); ++i)
      {
        if ((str[i] >= 'a' && str[i] <= 'z') ||
         (str[i] >= 'A' && str[i] <= 'Z') || str[i] == ' ') {
          resu = resu + str[i];
        }
      }

      std::list<std::string> tokenlist;
      stringutils::StringTokenizer st1(resu, " ");
      const std::vector<std::string*>* bucket;
      size_t hash = 0;
      while(st1.hasNextToken())
      {
        std::string eval = st1.nextToken();
        stringutils::trim(eval);
        stringutils::toLower(eval);

        bucket = 0;
        hash = str_hash(eval) % bucketnum;

        bucket = hashTable[hash];
        bucketpos =0;
        if(bucket)
        {
            bool flag = true;
            while(bucketpos < bucket->size())
            {
              std::string* e = (*bucket)[bucketpos];
              if ((e)->compare(eval) == 0)
              {
                flag = false;
                break;
              }
              bucketpos++;
            }
            if (flag)
            {
              // hash function is not good
              // Good words having same hash as stopwords
              tokenlist.push_back(eval);
            }

        }
        else
        {

          tokenlist.push_back(eval);
        }

      }
      std::string finalstr = "";
      tokenlist.sort(SemanticTrajectory::compare_nocase);
      int size = tokenlist.size();
      int i = 0;
      for(const auto &word : tokenlist)
      {
       if (i != size - 1)
       {
          finalstr += word + " ";
       }
       else
       {
          finalstr += word;
       }
       i++;
      }

      res->AddString(finalstr);
    }
    else
    {
      res->AddString("");
    }
    double x1 = x->GetValue();
    double y1 = y->GetValue();

    Coordinate c(x1, y1);
    res->AddCoordinate(c);


    if (res->GetNumCoordinates() == 1)
    {
      c_x1 = x1;
      c_y1 = y1;
      c_x2 = x1;
      c_y2 = y1;
    }
    else
    {
      c_x1 = std::min(c_x1, x1);
      c_y1 = std::min(c_y1, y1);
      c_x2 = std::max(c_x2, x1);
      c_y2 = std::max(c_y2, y1);

    }

    tuple->DeleteIfAllowed();
  }
  if (c_x1 < c_x2 && c_y1 < c_y2)
  {
    double mind[2];
    double maxd[2];
    mind[0] = c_x1;
    mind[1] = c_y1;
    maxd[0] = c_x2;
    maxd[1] = c_y2;
    res->SetBoundingBox(true, mind, maxd);
  }

  for (unsigned int i = 0; i< bucketnum; i++)
  {
    std::vector<std::string*>* v = hashTable[i];
    if (v)
    {
      for(unsigned int j = 0; j < v->size(); j++)
      {
        std::string* stn = (*v)[j];
        delete stn;

      }
      (v)->clear();
      delete hashTable[i];
      hashTable[i] = 0;
    }
  }
  delete hashTable;
  stream.close();
  return 0;
}


/*
Operator ~"stbox"~

*/


ListExpr STboxTM(ListExpr args)
{
  std::string errmsg =
  "Expected datatype semanticTrajectory";

  if(!SemanticTrajectory::checkType(nl->First(args)))
  {
    return listutils::typeError(errmsg);
  }

  return (nl->SymbolAtom( Rectangle<2>::BasicType()));
}

int STbboxMapValue(Word* args, Word& result,
   int message,
   Word& local,
   Supplier s)
{
    result = qp->ResultStorage( s );
    Rectangle<2> *box =
    static_cast<Rectangle<2>*>(result.addr);
    const SemanticTrajectory* st =
    static_cast<SemanticTrajectory*>(args[0].addr);
    (*box) = st->GetBoundingBox();
    box->SetDefined(true);
    return 0;
}



/*
Operator ~ uniquekeywords

*/

ListExpr
extractkeywordsTM( ListExpr args ){

  std::string err =
  "must be of type uniquestringarray or semantic";

  if ( nl->ListLength(args) == 1
  && (SemanticTrajectory::checkType(nl->First(args))))
  {
    return nl->TwoElemList( nl->SymbolAtom(Symbol::STREAM()),
                              nl->SymbolAtom(CcString::BasicType()));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


int extractkeywordMapV(Word* args, Word& result,
  int message,
  Word& local,
  Supplier s)
{

  // An auxiliary type which keeps of
  // which coordinate is being
  // examine between requests

  struct TheWords {

    int visitedwords = 0;
    std::list<std::string>::iterator it;
    int numOfWords = 0;
    std::list<std::string> wordList;
    TheWords(std::list<std::string>& list)
    {
      wordList = list;
      numOfWords = wordList.size();
      it = wordList.begin();
    }

    bool getNextWord(std::string& el)
    {
      el = *it;
      stringutils::trim(el);
      it++;
      visitedwords++;
      return true;
    }

  };
  TheWords* localword =
  static_cast<TheWords*>(local.addr);

  std::list<std::string> list;
  switch(message)
  {

    case OPEN: // Initialize the local storage
    {
      // If localword already exists
      // need to delete and restart

      if(localword)
      {
        delete localword;
        localword = 0;
      }

      SemanticTrajectory* strarr =
      static_cast<SemanticTrajectory*>(args[0].addr);
      list = strarr->GetStringArray();
      localword =
      new TheWords(list);
      local.addr = localword;
      return 0;
    }
    case REQUEST: // returnthe next stream element
    {
      if (localword->visitedwords <
        localword->numOfWords)
      {

        std::string str;
        bool success = localword->getNextWord(str);
        if (success == true)
        {
          CcString* elem = new CcString(true, str);
          result.addr = elem;
        }
        else
        {
          result.addr = 0;
        }

        return YIELD;
      }
      else
      {

        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      if (localword != 0)
      {
        delete localword;
        local.addr = 0;
        list.clear();
      }
      return 0;
    }
    default:
    {
      // This should never happen
      return -1;
    }
  }
return 0;

}



//TypeMapping for operator ~ makesummaries ~

ListExpr MakesummariesTM( ListExpr args )
{
  if( (nl->ListLength(args) == 7)
    && listutils::isTupleStream(nl->First(args))
    && CcInt::checkType(nl->Second(args))
    && SemanticTrajectory::checkType(nl->Third(args))
    && CellGrid2D::checkType(nl->Fourth(args))
    && listutils::isSymbol(nl->Fifth(args))
    && listutils::isSymbol(nl->Sixth(args))
    && listutils::isSymbol(nl->Seventh(args))
    )
  {

    ListExpr stream = nl->First(args);
    ListExpr attrname_WiD = nl->Fifth(args);
    ListExpr attrname_Ctn = nl->Sixth(args);
    ListExpr attrname_word = nl->Seventh(args);
    ListExpr attrList = nl->Second(nl->Second(stream));
    ListExpr type;
    std::string name =
    nl->SymbolValue(attrname_WiD);
    int index1 =
    listutils::findAttribute(attrList, name, type);

    if(index1==0){
      return
      listutils::typeError(
        "attribute name " + name +
       " unknown in tuple"
       " stream");
    }
    if(!CcInt::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'real'");
    }

    name = nl->SymbolValue(attrname_Ctn);

    int index2 =
    listutils::findAttribute(attrList, name, type);

    if(index2==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }

    name = nl->SymbolValue(attrname_word);
    int index3 =
    listutils::findAttribute(attrList, name, type);

    if(index3==0){
      return
      listutils::typeError("attribute name " + name +
      " unknown in tuple stream");
    }

    ListExpr indexes = nl->ThreeElemList(
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1),
                         nl->IntAtom(index3-1));

    return
    nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                      indexes,
    nl->SymbolAtom(SemanticTrajectory::BasicType()));
  }
  return
  listutils::typeError("Expected "
  "(stream(tuple) x int x semantictrajectory x "
  "cellgrid2d x attr_WordId x attr_Ctn x attr_Word).");
}


//ValueMapping for operator ~ makespatialsum ~


int MakesummariesMV(Word* args, Word& result,
   int message,
   Word& local,
   Supplier s)
{

  result = qp->ResultStorage(s);
  int noargs = qp->GetNoSons(s);
  int idx1 = ((CcInt*)args[noargs-3].addr)->GetValue();
  int idx2 = ((CcInt*)args[noargs-2].addr)->GetValue();
  int idx3 = ((CcInt*)args[noargs-1].addr)->GetValue();
  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();


  CcInt* id = static_cast<CcInt*>( args[1].addr );

  SemanticTrajectory* st =
  static_cast<SemanticTrajectory*>(args[2].addr);
  CellGrid2D* grid =
  static_cast<CellGrid2D*>(args[3].addr);
  SemanticTrajectory* res =
  static_cast<SemanticTrajectory*>(result.addr);


  res->Clear();
  res->InitializeST();

  /*
  Create vector to hold already found cell numbers

  */
  std::vector<std::pair<int, int>> cellsresult;

  /*
  Iterate over each coordinate in semantictrajectory
  Find the cell and return origin of cell
  See if that cell origin is already present
  in if not add to spatialsummary

  */

  for(int i = 0; i < st->GetNumCoordinates(); i++)
  {
    Coordinate c = st->GetCoordinate(i);
    if(!grid->onGrid(c.x, c.y))
    {

      return listutils::typeError("Coordinate outside of grid"
      ", please enter a new grid dimension in operator");
    }
    int32_t cellIndexX = grid->getXIndex(c.x);
    int32_t cellIndexY = grid->getYIndex(c.y);

    if(std::find(
      cellsresult.begin(),
      cellsresult.end(),
      std::make_pair(cellIndexX,cellIndexY))
      == cellsresult.end())
    {
      Cell cell(id->GetValue(),
       cellIndexX,
       cellIndexY);

      res->AddCell(cell);
      cellsresult.push_back(std::make_pair(
        cellIndexX,
        cellIndexY));
    }
  }


  while((tuple = stream.request()))
  {
    CcInt* id = (CcInt*) tuple->GetAttribute(idx1);
    CcInt* ctn = (CcInt*) tuple->GetAttribute(idx2);
    CcString* str = (CcString*) tuple->GetAttribute(idx3);
    res->AddStringSum(str->GetValue(),id->GetIntval(), ctn->GetIntval());
    tuple->DeleteIfAllowed();
  }

  res->EndST(*st);

  res->Finalize();
  stream.close();
  return 0;
}

ListExpr SimilarityTypeMap(ListExpr args)
{

  std::string err = "stream(tuple) x attr1 x "
               "attr2 x real x real x rect";
  if(!nl->HasLength(args, 6))
  {
    return listutils::typeError(err);
  }
  ListExpr stream1 = nl->First(args);
  ListExpr attr1 = nl->Second(args); // semtraj1
  ListExpr attr2 = nl->Third(args); //  semtraj2
  ListExpr attr4 = nl->Fourth(args); // real
  ListExpr attr5 = nl->Fifth(args); // Real
  ListExpr attr6 = nl->Sixth(args); // Rect
  if (!Stream<Tuple>::checkType(stream1))
  {
    return listutils::typeError(err + "(first arg is not a tuple sream)");
  }

  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  if (!CcReal::checkType(attr4))
  {
    return listutils::typeError(err + "4th arg must be a real");
  }
  if (!CcReal::checkType(attr5))
  {
    return listutils::typeError(err + "5th arg must be a real");
  }
  if (!Rectangle<2>::checkType(attr6))
  {
    return listutils::typeError(err + "6th arg must be a rectangle");
  }
  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  ListExpr attrList1 = nl->Second(nl->Second(stream1));
  std::string attrname1 = nl->SymbolValue(attr1);
  std::string attrname2 = nl->SymbolValue(attr2);
  ListExpr attrType1;


  int index1 = listutils::findAttribute(attrList1,attrname1,attrType1);
  if(index1==0){
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }

  int index2 = listutils::findAttribute(attrList1,attrname2,attrType1);
  if(index2==0){
    return listutils::typeError(attrname2+
                     " is not an attribute of the second stream");
  }




  ListExpr indexList = nl->TwoElemList(
                        nl->IntAtom(index1-1),
                        nl->IntAtom(index2-1));

  /*
  Last argument to this ThreeElemList
  Return types Stream<Tuple>
  Tuple Type and attribute List
  */
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                nl->TwoElemList(
                                  nl->SymbolAtom(Tuple::BasicType()),
                                  attrList1)));
}

class SIMInfo
{
  public:

    SIMInfo(Word& _stream1,
      const ListExpr _resType,
      const int _index1, const int _index2,
      double _alpha, double _threshold, double _diag) :
      stream1(_stream1),
      tt(0),
       index1(_index1), index2(_index2),
       alpha(_alpha), threshold(_threshold),
       diag(_diag)
      {
        tt = new TupleType(_resType);
        stream1.open();
      }
    ~SIMInfo()
    {
      stream1.close();
      tt->DeleteIfAllowed();
    }
    Tuple* nextTuple()
    {

      Tuple* res = new Tuple(tt);
      while((res = stream1.request()))
      {
        SemanticTrajectory* st1 =
        (SemanticTrajectory*) res->GetAttribute(index1);
        SemanticTrajectory* st2 =
        (SemanticTrajectory*) res->GetAttribute(index2);
        double result = st1->Similarity(*st2, diag, alpha);

        if (result > threshold)
        {
          return res;

        }
      }
      return 0;
    }
  private:
    Stream<Tuple> stream1;
    TupleType* tt;
    int index1;
    int index2;
    double alpha;
    double threshold;
    double diag;


};
int SimilarityMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s ){


   SIMInfo* li = (SIMInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                    delete li;
                  }
                  ListExpr ttype = nl->Second(GetTupleResultType(s));
                  int noargs = qp->GetNoSons(s);
                  int idx1 = ((CcInt*)args[noargs-2].addr)->GetValue();
                  int idx2 = ((CcInt*)args[noargs-1].addr)->GetValue();
                  CcReal * alpha = static_cast<CcReal*>(args[3].addr);
                  CcReal * thresh = static_cast<CcReal*>(args[4].addr);
                  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[5].addr);
                  double diagonal = GetDiag(*rec);

                  local.addr = new SIMInfo(
                               args[0],
                               ttype,
                               idx1,
                               idx2,
                               alpha->GetValue(),
                               thresh->GetValue(),
                               diagonal);
                  return 0;
                }
     case REQUEST: { if(!li){
                       return CANCEL;
                   }
                     result.addr = li->nextTuple();;
                     return result.addr ? YIELD:CANCEL;
                   }
     case CLOSE : {
                    if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                  }
   }
   return 0;

}


ListExpr TTSimTypeMap(ListExpr args)
{


  std::string err = "stream(tuple) x attr1 x "
               "attr2 x real x real x rect x Grid";
  if(!nl->HasLength(args, 7))
  {
    return listutils::typeError(err);
  }
  ListExpr stream1 = nl->First(args);
  ListExpr attr1 = nl->Second(args); // semtraj1
  ListExpr attr2 = nl->Third(args); //  semtraj2
  ListExpr attr4 = nl->Fourth(args); // real
  ListExpr attr5 = nl->Fifth(args); // Real
  ListExpr attr6 = nl->Sixth(args); // Rect
  ListExpr attr7 = nl->Seventh(args); // Rect
  if (!Stream<Tuple>::checkType(stream1))
  {
    return listutils::typeError(err + "(first arg is not a tuple sream)");
  }

  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  if (!CcReal::checkType(attr4))
  {
    return listutils::typeError(err + "4th arg must be a real");
  }
  if (!CcReal::checkType(attr5))
  {
    return listutils::typeError(err + "5th arg must be a real");
  }
  if (!Rectangle<2>::checkType(attr6))
  {
    return listutils::typeError(err + "6th arg must be a rectangle");
  }
  if(!CellGrid2D::checkType(attr7))
  {
    return listutils::typeError(err + "(third arg must be CellGrid2D");
  }
  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  ListExpr attrList1 = nl->Second(nl->Second(stream1));
  std::string attrname1 = nl->SymbolValue(attr1);
  std::string attrname2 = nl->SymbolValue(attr2);
  ListExpr attrType1;


  int index1 = listutils::findAttribute(attrList1,attrname1,attrType1);
  if(index1==0){
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }

  int index2 = listutils::findAttribute(attrList1,attrname2,attrType1);
  if(index2==0){
    return listutils::typeError(attrname2+
                     " is not an attribute of the second stream");
  }




  ListExpr indexList = nl->TwoElemList(
                        nl->IntAtom(index1-1),
                        nl->IntAtom(index2-1));

  /*
  Last argument to this ThreeElemList
  Return types Stream<Tuple>
  Tuple Type and attribute List
  */
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                nl->TwoElemList(
                                  nl->SymbolAtom(Tuple::BasicType()),
                                  attrList1)));
}






class TTInfo
{
  public:

    TTInfo(unsigned int bucknum, Word& _stream1,
      const ListExpr _resType,
      const int _index1, const int _index2,
      double _alpha, double _threshold, double _diag, double _wx, double _wy) :
      hashTable(0), hashTable2(0), bucket(0),
      bucketnum(bucknum),
      stream1(_stream1),
      tt(0),
      index1(_index1), index2(_index2),
      alpha(_alpha), threshold(_threshold),
      diag(_diag), wx(_wx), wy(_wy)
      {
        tt = new TupleType(_resType);
        stream1.open();
        InitializeTables();
      }
    ~TTInfo()
    {
      stream1.close();

      tt->DeleteIfAllowed();
    }
    size_t getBucket(std::string& str)
    {
      return str_hash(str) % bucketnum;
    }
    void InitializeTables()
    {
      if(!hashTable)
      {

        hashTable = new std::vector<std::string*>*[bucketnum];
        for (unsigned int i = 0; i < bucketnum; i++)
        {
          hashTable[i] = 0;
        }
      }

      if(!hashTable2)
      {
        hashTable2 = new std::vector<std::string*>*[bucketnum];
        for (unsigned int i = 0; i < bucketnum; i++)
        {
          hashTable2[i] = 0;
        }
      }
    }
    void getTablesReady(SemanticTrajectory& st1, SemanticTrajectory& st2)
    {

      for (int i = 0; i < st1.GetNumWords(); i++)
      {
       std::string holdValue = "";
       bool success = st1.GetStringSum(i, holdValue);
       if(success)
       {

         size_t hash = getBucket(holdValue);

         if (!hashTable[hash])
         {
           hashTable[hash] = new std::vector<std::string*>();
         }
         std::string* stn = new std::string(holdValue);
         hashTable[hash]->push_back(stn);
       }

      }

      for (int i = 0; i < st2.GetNumWords(); i++)
      {
       std::string holdValue = "";
       bool success = st2.GetStringSum(i, holdValue);
       if(success)
       {

         size_t hash = getBucket(holdValue);

         if (!hashTable2[hash])
         {
           hashTable2[hash] = new std::vector<std::string*>();
         }
         std::string* stn = new std::string(holdValue);
         hashTable2[hash]->push_back(stn);
       }

      }


    }
    Tuple* nextTuple()
    {

      Tuple* res = new Tuple(tt);
      while((res = stream1.request()))
      {

      SemanticTrajectory* st1 =
      (SemanticTrajectory*) res->GetAttribute(index1);
      SemanticTrajectory* st2 =
      (SemanticTrajectory*) res->GetAttribute(index2);

      getTablesReady(*st1, *st2);
      double result1 = 0.0;
      double result2 = 0.0;
      double result = 0.0;
      double ts = 0.0;
      ts = textualScore(*st1, *st2);

      result2 = (1 - alpha ) * ts;
      double dist = 0.0;
      dist =
      MinDistAux(*st1, *st2, wx, wy);

      double normalizedScore = 0.0;
      if (dist != 0.0)
      {
        normalizedScore = 1 - (double)(dist/diag);
      }
      else {
        normalizedScore = 1;
      }
      result1 = alpha * normalizedScore;
      result = result1 + result2;
      clearContent();
      cout << result1 << " r2 " << result2 << " " << threshold << endl;
      if (result > threshold)
      {
        return res;
      }
    }
      return 0;
    }
    void clearContent()
    {

      for (unsigned int i = 0; i< bucketnum; i++)
      {
        std::vector<std::string*>* v = hashTable[i];
        if (v)
        {
          for(unsigned int j = 0; j < v->size(); j++)
          {
            std::string* stn = (*v)[j];
            delete stn;

          }
          (v)->clear();
          delete hashTable[i];
          hashTable[i] = 0;
        }
      }

      for (unsigned int i = 0; i< bucketnum; i++)
      {
        std::vector<std::string*>* v = hashTable2[i];
        if (v)
        {
          for(unsigned int j = 0; j < v->size(); j++)
          {
               std::string* stn = (*v)[j];
               delete stn;

          }
          (v)->clear();
          delete hashTable2[i];
          hashTable2[i] = 0;
        }
      }

    }

    double MinDistAux(SemanticTrajectory& st1,
      SemanticTrajectory& st2, double wx, double wy) {

      DbArray<Cell>* Ax = new DbArray<Cell>(0);
      DbArray<Cell> valuesSt1 = st1.GetCellList();
      DbArray<Cell> valuesSt2 = st2.GetCellList();
      Ax->Append(valuesSt1);
      Ax->Append(valuesSt2);
      DbArray<Cell>* Ay = new DbArray<Cell>(0);
      Ay->Append(valuesSt1);
      Ay->Append(valuesSt2);

      Ax->Sort(SemanticTrajectory::CompareX);
      Ay->Sort(SemanticTrajectory::CompareY);
      int32_t m = Ax->Size();
      double result = 0.0;
      for (int i = 0; i < Ax->Size(); i++)
      {
        Cell c;
        Ax->Get(i, &c);

      }
      for (int i = 0; i < Ay->Size(); i++)
      {
        Cell c;
        Ay->Get(i, &c);

      }
      result = MinDistUtils(st1 ,st2, *Ax, *Ay, m, wx, wy);
      return result;
    }

    double MinDistUtils(SemanticTrajectory& st1,
        SemanticTrajectory&st2,
        DbArray<Cell>& Ax,
        DbArray<Cell>& Ay, int32_t m, double wx, double wy)
    {

      double minD = DBL_MAX;

      if (m > 3)
      {

        int n = m/2;

        DbArray<Cell>* AxL = new DbArray<Cell>(0);
        Cell cell1;
        Ax.Get(0,cell1);
        Ax.copyTo(*AxL, 0, n, 0);
        DbArray<Cell>* AxR = new DbArray<Cell>(0);
        Ax.copyTo(*AxR, n, m-n, 0);
        DbArray<Cell>* AyL = new DbArray<Cell>(0);
        DbArray<Cell>* AyR = new DbArray<Cell>(0);

        double minDL = 0.0;
        double minDR = 0.0;


        for (int i = 0; i < Ay.Size(); i++)
        {
          Cell celly, cellx;
          Ay.Get(i, celly);
          Ax.Get(n, cellx);
          if(celly.GetX() <= cellx.GetX())
          {
            AyL->Append(celly);
          }
          else
          {
            AyR->Append(celly);
          }
        }

        /* Check to see if AxL has
            cells from both C1 and C2 */
        if (AyL->Size() != 0){
          Cell cellAyL;
          AyL->Get(0, cellAyL);
          int32_t firstId = cellAyL.GetId();
          bool both = false;
          for (int i = 1; i < AyL->Size(); i++)
          {
            AyL->Get(i, cellAyL);
            if (cellAyL.GetId() != firstId)
            {
              both = true;
              break;
            }
          }
          if (both == true)
          {

            minDL = MinDistUtils(st1, st2, *AxL, *AyL, n, wx, wy);

          }
          else
          {
            minDL = DBL_MAX;
          }
        } else {
          minDL = DBL_MAX;
        }
        if (AyR->Size() != 0)
        {
          Cell cellAyR;
          AyR->Get(0, cellAyR);
          int32_t firstId = cellAyR.GetId();
          bool both = false;
          for (int i = 1; i < AyR->Size(); i++)
          {
            AyR->Get(i, cellAyR);
            if (cellAyR.GetId() != firstId)
            {
              both = true;
              break;
            }
          }
          if (both == true)
          {

            minDR = MinDistUtils(st1, st2, *AxR, *AyR, m-n, wx, wy);

          }
          else
          {
            minDR = DBL_MAX;
          }
        }
        else {
          minDR = DBL_MAX;
        }

        minD = minDL > minDR ? minDR : minDL;


        DbArray<Cell>* Am = new DbArray<Cell>(0);
        for (int i = 0; i < Ay.Size(); i++)
        {
          Cell x;
          Cell y;

          Ax.Get(n, x);
          Ay.Get(i, y);
          if (fabs(y.GetX() - x.GetX()) < minD)
          {
            Am->Append(y);
          }
        }


        for (int i = 0; i < Am->Size(); i++)
        {
          for (int j = i + 1; j < Am->Size(); j++)
          {
            Cell c1;
            Cell c2;
            Am->Get(i, c1);
            Am->Get(j, c2);
            if (c1.GetId() != c2.GetId())
            {

              double tempmin = GetCellDist(c1,c2, wx, wy);

              if (tempmin < minD)
              {
                minD = tempmin;
              }
            }
          }
        }

        //Don't forget to delete the DbArrays
        // that were initialized here
        delete Am;
        delete AyR;
        delete AyL;
        delete AxR;
        delete AxL;
      }
      else
      {
          minD = BruteForce(Ax, m, wx, wy);

      }
      return minD;
    }

    double GetCellDist(
    Cell& c1, Cell& c2, double wx, double wy)
    {
      double result;
      //Are they the same cell
      if (c1.GetX() == c2.GetX()
    && c1.GetY() == c2.GetY())
      {
        return 0.0;
      }

      //c1 and c2 on the same x Axis
      if (c1.GetX() == c2.GetX())
      {
        // if c1 is above c2
        // Take bottom corner of c1 and top corner of c2
        if(c1.GetY() > c2.GetY())
        {
          int32_t y2 = (int32_t)(c2.GetY() + 1);
          return
          EuclidDist(c1.GetX(), c1.GetY(),
          c2.GetX(), y2, wx, wy);
        }
        // if c1 is below c2
        // Take top corner of c1 and bottom corner of c2
        else
        {
          int32_t y1 = (int32_t)(c1.GetY() + 1);
          return
          EuclidDist(c1.GetX(),
          y1,
          c2.GetX(),
          c2.GetY(), wx, wy);
        }
      }
      //c1 and c2 on the same y axis
      if (c1.GetY() == c2.GetY())
      {
        // if c1 is to the right of c2
        if(c1.GetX() > c2.GetX())
        {
          int32_t x2 = (int32_t)(c2.GetX() + 1);
          return
          EuclidDist(c1.GetX(), c1.GetY(),
          x2, c2.GetY(), wx, wy);
        }
        // if c1 is to the left of c2
        else
        {
          int32_t x1 = (int32_t)(c1.GetX() + 1);
          return
          EuclidDist(x1,
          c1.GetY(),
          c2.GetX(), c2.GetY(), wx, wy);
        }
      }
      // if c1 is above c2
      if (c1.GetY() > c2.GetY())
      {
        // if c1 is to the right of c2
        if (c1.GetX() > c2.GetX())
        {
          int32_t x2 = (int32_t)(c2.GetX() + 1);
          int32_t y2 = (int32_t)(c2.GetY() + 1);
          return
          EuclidDist(c1.GetX(),
          c1.GetY(),
          x2,
          y2, wx, wy);
        }
        // if c1 is to the left c2
        //
        else
        {

          int32_t x1 = (int32_t)(c1.GetX() + 1);
          int32_t y2 = (int32_t)(c2.GetY() + 1);
          return
          EuclidDist(x1, c1.GetY(),
          c2.GetX(), y2, wx, wy);
        }
      }
      // if c1 is below c2
      else
      {
        // if c1 is to the right of c2
        if (c1.GetX() > c2.GetX())
        {
          int32_t x1 = (c1.GetX() + 1);
          int32_t x2 = (c2.GetX() + 1);
          return
          EuclidDist(
            x1,
            c1.GetY(),
            x2,
            c2.GetY(), wx, wy);
        }
        // if c1 is to the left c2
        else
        {
          int32_t x1 = (c1.GetX() + 1);
          int32_t y1 = (c1.GetY() + 1);
          return
          EuclidDist(
          x1,y1,
            c2.GetX(),
            c2.GetY(), wx, wy);
        }
      }
      return result;
    }

    double BruteForce(
    DbArray<Cell>& Ax,
    int32_t m, double wx, double wy)
    {

      double minD = DBL_MAX;
      for (int i = 0; i < Ax.Size(); i++)
      {
        for (int j = i + 1; j < Ax.Size(); j++)
        {
          Cell c1;
          Cell c2;
          Ax.Get(i, c1);
          Ax.Get(j, c2);
          if (c1.GetId() != c2.GetId())
          {
            double tempmin = GetCellDist(c1,c2, wx, wy);

            if (tempmin < minD)
            {
              minD = tempmin;
            }
          }
        }
      }
      return minD;
    }
    double EuclidDist(int32_t x1,
        int32_t y1,
        int32_t x2,
        int32_t y2, double wx, double wy) const
    {
        double x11 = (double) x1*wx;
        double x21 = (double) x2*wx;
        double y11 =  (double) y1*wy;
        double y21 = (double) y2*wy;
        return sqrt(pow((x11 - x21),2) + pow((y11 - y21),2));
    }

    double textualScore(SemanticTrajectory& st1, SemanticTrajectory& st2)
    {

          double TSim = 0.0;
          unsigned int bucketpos = 0;
          for(int i = 0; i < st1.GetNumCoordinates(); i++)
           {
            std::string holdvalue;
            st1.GetString(i, holdvalue);

            if (holdvalue.length() > 0)
            {
              stringutils::StringTokenizer parse_st1(holdvalue, " ");
              while(parse_st1.hasNextToken())
              {
                std::string eval = parse_st1.nextToken();
                stringutils::trim(eval);
                size_t hash = getBucket(eval);
                bucket = 0;
                bucket = hashTable2[hash];
                bucketpos = 0;
                if(bucket)
                {
                  while(bucketpos < bucket->size())
                  {
                    std::string* check = (*bucket)[bucketpos];
                    if ((check)->compare(eval) == 0)
                    {
                      TSim = TSim + ((double) 1/st1.GetNumCoordinates());
                      break;
                    }
                    bucketpos++;
                  }
                }
              }
            }
          }
          for(int i = 0; i < st2.GetNumTextData(); i++)
          {

            std::string holdvalue;
            st2.GetString(i, holdvalue);
            if (holdvalue.length() > 0)
            {
              stringutils::StringTokenizer parse_st2(holdvalue, " ");

              while(parse_st2.hasNextToken())
              {

                std::string eval = parse_st2.nextToken();
                stringutils::trim(eval);
                size_t hash = getBucket(eval);
                bucket = 0;
                bucket = hashTable[hash];
                bucketpos = 0;
                if(bucket)
                {
                  while(bucketpos < bucket->size())
                  {

                    std::string* check = (*bucket)[bucketpos];
                    if ((check)->compare(eval) == 0)
                    {

                      TSim = TSim + ((double) 1/st2.GetNumCoordinates());
                      break;
                    }
                    bucketpos++;
                  }

                }
              }
            }
          }
          return TSim;
        }
  private:

    std::vector<std::string*>** hashTable;
    std::vector<std::string*>** hashTable2;
    const std::vector<std::string*>* bucket;
    unsigned int bucketnum;
    Stream<Tuple> stream1;
    TupleType* tt;
    int index1;
    int index2;
    double alpha;
    double threshold;
    double diag;
    std::hash<std::string> str_hash;
    double wx;
    double wy;


};

int TTSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s ){

   TTInfo* li = (TTInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                    delete li;
                  }
                  ListExpr ttype = nl->Second(GetTupleResultType(s));
                  int noargs = qp->GetNoSons(s);
                  int idx1 = ((CcInt*)args[noargs-2].addr)->GetValue();
                  int idx2 = ((CcInt*)args[noargs-1].addr)->GetValue();
                  CellGrid2D* grid = static_cast<CellGrid2D*>(args[6].addr);
                  CcReal * alpha = static_cast<CcReal*>(args[3].addr);
                  CcReal * thresh = static_cast<CcReal*>(args[4].addr);
                  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[5].addr);
                  double diagonal = GetDiag(*rec);
                  double wx = grid->getXw();
                  double wy = grid->getYw();

                  local.addr = new TTInfo(
                               100,
                               args[0],
                               ttype,
                               idx1,
                               idx2,
                               alpha->GetValue(),
                               thresh->GetValue(),
                               diagonal,
                               wx,
                               wy);
                  return 0;
                }
     case REQUEST: { if(!li){
                       return CANCEL;
                   }
                     result.addr = li->nextTuple();;
                     return result.addr ? YIELD:CANCEL;
                   }
     case CLOSE : {
                    if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                  }
   }
   return 0;


}


/*
Operator for BBSim
Purpose: To compare the similarity between to batch MBR's for pruning

*/

ListExpr BBSimTypeMap(ListExpr args)
{
  std::string err = "stream(tuple) x attr1 x "
               "attr2 x attr3 x attr4 x CcReal";
  if(!nl->HasLength(args, 8))
  {
    return listutils::typeError(err);
  }
  ListExpr stream1 = nl->First(args);
  ListExpr attr1 = nl->Second(args); // batch 1 id
  ListExpr attr2 = nl->Third(args); // batch 2 id
  ListExpr attr3 = nl->Fourth(args); // batch mbr 1
  ListExpr attr4 = nl->Fifth(args); // batch mbr2
  if (!Stream<Tuple>::checkType(stream1))
  {
    return listutils::typeError(err + "(first arg is not a tuple sream)");
  }
  if (!CcReal::checkType(nl->Sixth(args)))
  {
    return listutils::typeError(err + "6th arg must be a real");
  }
  if (!CcReal::checkType(nl->Seventh(args)))
  {
    return listutils::typeError(err + "7th arg must be a real");
  }
  if (!Rectangle<2>::checkType(nl->Eigth(args)))
  {
    return listutils::typeError(err + "8th arg must be a rectangle");
  }
  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  if(!listutils::isSymbol(attr3)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr4)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  ListExpr attrList1 = nl->Second(nl->Second(stream1));
  std::string attrname1 = nl->SymbolValue(attr1);
  std::string attrname2 = nl->SymbolValue(attr2);
  std::string attrname3 = nl->SymbolValue(attr3);
  std::string attrname4 = nl->SymbolValue(attr4);
  ListExpr attrType1;


  int index1 = listutils::findAttribute(attrList1,attrname1,attrType1);
  if(index1==0){
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }

  int index2 = listutils::findAttribute(attrList1,attrname2,attrType1);
  if(index2==0){
    return listutils::typeError(attrname2+
                     " is not an attribute of the second stream");
  }

  int index3 = listutils::findAttribute(attrList1,attrname3,attrType1);
  if(index3==0){
    return listutils::typeError(attrname3+
                     " is not an attribute of the first stream");
  }

  int index4 = listutils::findAttribute(attrList1,attrname4,attrType1);
  if(index4==0){
    return listutils::typeError(attrname4+
                     " is not an attribute of the second stream");
  }


  ListExpr indexList = nl->FourElemList(
                        nl->IntAtom(index1-1),
                        nl->IntAtom(index2-1),
                        nl->IntAtom(index3-1),
                        nl->IntAtom(index4-1));

  /*
  Last argument to this ThreeElemList
  Return types Stream<Tuple>
  Tuple Type and attribute List
  */
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                nl->TwoElemList(
                                  nl->SymbolAtom(Tuple::BasicType()),
                                  attrList1)));
}

class BatchBatchInfo
{
  public:

    BatchBatchInfo(Word& _stream1,
      const ListExpr _resType,
      const int _index1, const int _index2,
      const int _index3, const int _index4,
      double _alpha, double _threshold, double _diag) :
      stream1(_stream1),
      tt(0),
       index1(_index1), index2(_index2),
       index3(_index3), index4(_index4),
       alpha(_alpha), threshold(_threshold),
       diag(_diag)
      {
        tt = new TupleType(_resType);
        stream1.open();
      }
    ~BatchBatchInfo()
    {
      stream1.close();
      tt->DeleteIfAllowed();
    }
    Tuple* nextTuple()
    {

        Tuple* res = new Tuple(tt);
        while((res = stream1.request()))
        {
          CcInt* id1 = (CcInt*) res->GetAttribute(index1);
          CcInt* id2 = (CcInt*) res->GetAttribute(index2);
          if(id1->GetIntval() < id2->GetIntval()
          || id1->GetIntval() == id2->GetIntval())
          {

            Rectangle<2>* r1 = (Rectangle<2>*) res->GetAttribute(index3);
            Rectangle<2>* r2 = (Rectangle<2>*) res->GetAttribute(index4);

            const Geoid* geoid = 0;
            double distance = r1->Distance(*r2, geoid);
            double normalizedScore = 0.0;
            if (distance == 0.0) {
              normalizedScore = 1;
            }
            else
            {
              normalizedScore = 1 - (double)(distance/diag);
            }

            double result = alpha * normalizedScore * 2 + (1-alpha) * 2;

            if (result > threshold)
            {
              if (r1->Area() < r2->Area())
              {

                Tuple* t = reverseBatches(res);
                res->DeleteIfAllowed();
                return t;
              } else {
                return res;
              }
            }
          }

        }

        return 0;
    }
  private:
    Stream<Tuple> stream1;
    TupleType* tt;
    int index1;
    int index2;
    int index3;
    int index4;
    double alpha;
    double threshold;
    double diag;



    Tuple* reverseBatches(Tuple* t1)
    {
      Tuple* res = new Tuple(tt);
      int no1 = t1->GetNoAttributes();

      int half = no1 / 2;
      for (int i = 0; i < no1; i++)
      {
        if (i < half)
        {
            res->CopyAttribute(half+i, t1, i);
        }
        else
        {
            res->CopyAttribute(i-half, t1, i);
        }

      }
      return res;
    }
};

int BBSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s ){

   BatchBatchInfo* li = (BatchBatchInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                    delete li;
                  }
                  ListExpr ttype = nl->Second(GetTupleResultType(s));
                  int noargs = qp->GetNoSons(s);
                  int idx1 = ((CcInt*)args[noargs-4].addr)->GetValue();
                  int idx2 = ((CcInt*)args[noargs-3].addr)->GetValue();
                  int idx3 = ((CcInt*)args[noargs-2].addr)->GetValue();
                  int idx4 = ((CcInt*)args[noargs-1].addr)->GetValue();
                  CcReal * alpha = static_cast<CcReal*>(args[5].addr);
                  CcReal * thresh = static_cast<CcReal*>(args[6].addr);
                  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[7].addr);
                  double diagonal = GetDiag(*rec);

                  local.addr = new BatchBatchInfo(args[0], ttype,
                               idx1,
                               idx2,
                               idx3,
                               idx4,
                               alpha->GetValue(),
                               thresh->GetValue(),
                               diagonal);
                  return 0;
                }
     case REQUEST: { if(!li){
                       return CANCEL;
                   }

                     result.addr = li->nextTuple();;
                     return result.addr ? YIELD:CANCEL;
                   }
     case CLOSE : {
                    if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                  }
   }
   return 0;

}


ListExpr BatchesTM(ListExpr args)
{
  std::string err = "stream(tuple) x attr1 x "
               "attr2";
  if(!nl->HasLength(args, 4))
  {
    return listutils::typeError(err);
  }
  ListExpr stream1 = nl->First(args);
  ListExpr attr1 = nl->Second(args);
  ListExpr attr2 = nl->Third(args);
  if (!Stream<Tuple>::checkType(stream1))
  {
    return listutils::typeError(err + "(first arg is not a tuple sream)");
  }
  if (!CcReal::checkType(nl->Fourth(args)))
  {
    return listutils::typeError(err + "4th arg must be a real");
  }
  if(!listutils::isSymbol(attr1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(attr2)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  ListExpr attrList1 = nl->Second(nl->Second(stream1));
  std::string attrname1 = nl->SymbolValue(attr1);
  std::string attrname2 = nl->SymbolValue(attr2);
  ListExpr attrType1;


  int index1 = listutils::findAttribute(attrList1,attrname1,attrType1);
  if(index1==0){
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }

  int index2 = listutils::findAttribute(attrList1,attrname2,attrType1);
  if(index2==0){
    return listutils::typeError(attrname2+
                     " is not an attribute of the second stream");
  }



  ListExpr indexList = nl->TwoElemList(
                        nl->IntAtom(index1-1),
                        nl->IntAtom(index2-1));

  /*
  Last argument to this ThreeElemList
  Return types Stream<Tuple>
  Tuple Type and attribute List
  */
  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                nl->TwoElemList(
                                  nl->SymbolAtom(Tuple::BasicType()),
                                  attrList1)));
}


class HoldBatchesInfo
{
  public:

    HoldBatchesInfo(Word& _stream1,
      const ListExpr _resType,
      const int _index1, const int _index2, double _threshold) :
      stream1(_stream1),
      tt(0), batchidcounter(0), currentBatches(0),
       index1(_index1), index2(_index2), diathreshold(_threshold)
      {
        tt = new TupleType(_resType);
        batchidcounter = 0;
        stream1.open();
      }
    ~HoldBatchesInfo()
    {
      stream1.close();
      tt->DeleteIfAllowed();
    }
    Tuple* nextTuple()
    {

        std::vector<BatchGroup>::iterator it;
        Tuple* res = new Tuple(tt);
        int assignBatchId = -1;
        if ((res = stream1.request()))
        {

          int bmin = -1;
          double diagmin = DBL_MAX;
          int i = 0;
          SemanticTrajectory* st =
          (SemanticTrajectory*) res->GetAttribute(index1);
          for (it=currentBatches.begin(); it!=currentBatches.end(); ++it)
          {

            BatchGroup tempBatch(st->GetBoundingBox());
            tempBatch.addnewMBR((*it).GetMBR());
            Rectangle<2> mbr = tempBatch.GetMBR();
            double x1 = mbr.getMinX();
            double y1 = mbr.getMinY();
            double x2 = mbr.getMaxX();
            double y2 = mbr.getMaxY();

            double dgbox = sqrt(pow((x1 - x2),2)
            + pow((y1 - y2),2));

            if(dgbox < diagmin)
            {
              diagmin = dgbox;

              bmin = i; // hold the index of
            }
            i++;
          }
          if (diagmin <= diathreshold)
          {

            it = currentBatches.begin();
            std::advance(it, bmin);
            (*it).addnewMBR(st->GetBoundingBox());
            assignBatchId = bmin;
          }
          else
          {

            currentBatches.push_back(BatchGroup(st->GetBoundingBox()));
            assignBatchId = batchidcounter;
            batchidcounter++;
          }
          res->PutAttribute(index2, new CcInt(true, assignBatchId));
          return res;
        } else
        {
          return 0;
        }


        return res;
    }
  private:
    Stream<Tuple> stream1;
    TupleType* tt;
    int batchidcounter;
    std::vector<BatchGroup> currentBatches;
    int index1;
    int index2;
    double diathreshold;
};




int BatchesVM( Word* args, Word& result,
                   int message, Word& local, Supplier s ){

   HoldBatchesInfo* li = (HoldBatchesInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                    delete li;
                  }
                  ListExpr ttype = nl->Second(GetTupleResultType(s));
                  int noargs = qp->GetNoSons(s);
                  int idx1 = ((CcInt*)args[noargs-2].addr)->GetValue();
                  int idx2 = ((CcInt*)args[noargs-1].addr)->GetValue();
                  CcReal * thresh = static_cast<CcReal*>(args[3].addr);

                  local.addr = new HoldBatchesInfo(args[0], ttype,
                               idx1,
                               idx2,
                               thresh->GetValue());
                  return 0;
                }
     case REQUEST: { if(!li){
                       return CANCEL;
                   }

                     result.addr = li->nextTuple();;
                     return result.addr ? YIELD:CANCEL;
                   }
     case CLOSE : {
                    if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                  }
   }
   return 0;

}


/*
Operator for BTSim
Purpose: To compare the similarity between to batch MBR's for pruning

*/

ListExpr BTSimTypeMap(ListExpr args)
{

  std::string err = "stream(tuple) x  x "
               "";
  if(!nl->HasLength(args, 9))
  {
    return listutils::typeError(err);
  }
  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);
  ListExpr batchMBR = nl->Sixth(args);
  ListExpr st1 = nl->Third(args);
  ListExpr word = nl->Fourth(args);
  ListExpr Ctn = nl->Fifth(args);

  if (!Stream<Tuple>::checkType(stream1))
  {
    return listutils::typeError(err + "(first arg is not a tuple stream)");
  }
  if (!Stream<Tuple>::checkType(stream2))
  {
    return listutils::typeError(err + "(second arg is not a tuple stream)");
  }
  if(!listutils::isSymbol(st1)){
    return listutils::typeError(err + "(first attrname is not valid)");
  }
  if(!listutils::isSymbol(word)){
    return listutils::typeError(err + "(second attrname is not valid)");
  }
  if(!listutils::isSymbol(Ctn)){
    return listutils::typeError(err + "(third attrname is not valid)");
  }
  if (!Rectangle<2>::checkType(batchMBR))
  {
    return listutils::typeError(err + "6th arg must be a rectangle");
  }
  // THE PARAMS FOR NORMALIZATION ~ ALPHA ~ _threshold
  if (!CcReal::checkType(nl->Seventh(args)))
  {
    return listutils::typeError(err + "6th arg must be a real");
  }
  if (!CcReal::checkType(nl->Eigth(args)))
  {
    return listutils::typeError(err + "7th arg must be a real");
  }
  if (!Rectangle<2>::checkType(nl->Ninth(args)))
  {
    return listutils::typeError(err + "8th arg must be a rectangle");
  }
  std::string attrname1 = nl->SymbolValue(st1);
  std::string attrname2 = nl->SymbolValue(word);
  std::string attrname3 = nl->SymbolValue(Ctn);
  ListExpr attrType1;
  ListExpr attrList1 = nl->Second(nl->Second(stream1));
  ListExpr attrList2 = nl->Second(nl->Second(stream2));

  int index1 = listutils::findAttribute(attrList1, attrname1,attrType1);
  if(index1==0){
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }
  int index2 = listutils::findAttribute(attrList2, attrname2,attrType1);
  if(index2==0){
    return listutils::typeError(attrname2+
                     " is not an attribute of the first stream");
  }
  int index3 = listutils::findAttribute(attrList2, attrname3,attrType1);
  if(index3==0){
    return listutils::typeError(attrname3+
                     " is not an attribute of the first stream");
  }

  // Temporarily here because I will be adding more
  ListExpr indexList = nl->ThreeElemList(
                        nl->IntAtom(index1-1),
                        nl->IntAtom(index2-1),
                        nl->IntAtom(index3-1)
                      );


  /*
  Last argument to this ThreeElemList
  Return types Stream<Tuple>
  Tuple Type and attribute List
  */

  return nl->ThreeElemList( nl->SymbolAtom(Symbols::APPEND()),
                            indexList,
                            nl->TwoElemList(
                                nl->SymbolAtom(Stream<Tuple>::BasicType()),
                                nl->TwoElemList(
                                  nl->SymbolAtom(Tuple::BasicType()),
                                  attrList1)));

}




class BatchTrajInfo
{
  struct WordInfo
  {

    CcString* str;
    CcInt* count;
  };

  public:

    BatchTrajInfo(unsigned int bucknum,
      Rectangle<2>& _rect, Word& _stream1, Word& _stream2,
      const ListExpr _resType,
      const int _index1, const int _index2, const int _index3,
      double _alpha, double _threshold, double _diag) :
      hashTable(0), bucket(0), bucketPos(0), bucketnum(bucknum), bmbr(_rect),
      stream1(_stream1),
      stream2(_stream2),
      tt(0),
       index1(_index1), index2(_index2),
       index3(_index3),
       alpha(_alpha), threshold(_threshold),
       diag(_diag), wordTuple(0), sumOfBoth(0)
      {
        // this->stream2 = stream2;
        wordTuple =0;
        tt = new TupleType(_resType);

        stream1.open();
        qp->Open(stream2.addr);
        readBatchTSum();
      }
    ~BatchTrajInfo()
    {
      stream1.close();
      qp->Close(stream2.addr);

      clearTable();
      tt->DeleteIfAllowed();
    }

    void clearTable(){
      for (unsigned int i = 0; i< bucketnum; i++)
      {
        std::vector<WordInfo*>* v = hashTable[i];
        if (v)
        {
          for(unsigned int j = 0; j < v->size(); j++)
          {
            delete (*v)[j];

          }
          delete hashTable[i];
          hashTable[i] = 0;
        }
      }
    }

    size_t getBucket(CcString* str)
    {
      return str->HashValue() % bucketnum;
    }
    void readBatchTSum()
    {

      if (!hashTable)
      {
        hashTable = new std::vector<WordInfo*>*[bucketnum];
        for (unsigned int i = 0; i < bucketnum; i++)
        {
          hashTable[i] = 0;
        }
      }


      Word tuple;
      qp->Request(stream2.addr, tuple);
      while (qp->Received(stream2.addr))
      {
        wordTuple = static_cast<Tuple*>(tuple.addr);
        CcString* str1 = (CcString*) wordTuple->GetAttribute(index2);
        CcInt* ctn = (CcInt*) wordTuple->GetAttribute(index3);
        size_t hash = getBucket(str1);

        if (!hashTable[hash])
        {
          hashTable[hash] = new std::vector<WordInfo*>();
        }
        WordInfo* rInfo = new WordInfo;
        rInfo->count = ctn;
        rInfo->str = str1;
        hashTable[hash]->push_back(rInfo);
        sumOfBoth = sumOfBoth + 1;
        qp->Request(stream2.addr, tuple);
      }

    }
    Tuple* nextTuple()
    {


        Tuple* res = new Tuple(tt);
        while ((res = stream1.request()))
        {
          SemanticTrajectory* st =
          (SemanticTrajectory*) res->GetAttribute(index1);
          const Geoid* geoid = 0;

          double distance = bmbr.Distance((st->GetBoundingBox()), geoid);

          double normalizedScore = 1 - (double)(distance/diag);

          double result1 = alpha * normalizedScore + (1-alpha);

          double result2 = 0.0;
          double sum = 0.0;
          for (int i = 0; i < st->GetNumCoordinates(); i++)
          {
            std::string holdvalue;
            st->GetString(i, holdvalue);

            double rel = RelevanceBT(bmbr, st->GetCoordinate(i).x,
            st->GetCoordinate(i).y, holdvalue,alpha, diag);

            sum = sum + rel;
          }
          if (sum > 0)
          {
            result2 = sum / st->GetNumCoordinates();
          }

          double result3 = result1 + result2;

          if (result3 > threshold)
          {
            return res;
          }
        }

        return 0;
    }
  private:
    std::vector<WordInfo*>** hashTable;
    const std::vector<WordInfo*>* bucket;
    unsigned int bucketPos;
    unsigned int bucketnum;
    Rectangle<2> bmbr;
    Stream<Tuple> stream1;
    Word stream2;
    TupleType* tt;
    int index1;
    int index2;
    int index3;
    double alpha;
    double threshold;
    double diag;
    Tuple* wordTuple;
    int sumOfBoth = 0;

    double EuclidDistRT(double x1,
        double y1,
        double x2,
        double y2)
    {
      return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2));
    }

    double RelevanceBT(Rectangle<2>& mbr, double x,
        double y,
        std::string& objectwords, double alpha,
        double diag)
    {
      double result1 = 0.0;
      double result2 = 0.0;
      double textscore = 0.0;
      if (objectwords.length() > 0)
      {
        textscore = getTextualScoreBT(objectwords);
      }
      result2 = (1 - alpha) * textscore;
      double dist = getDistanceBT(mbr, x, y);
      double normalizedScore = 0.0;
      if (dist == 0.0)
      {
        normalizedScore  = 1;
      }
      else
      {
        normalizedScore = 1 - (double) (dist/diag);
      }
      result1 = alpha * normalizedScore;
      return result1 + result2;

    }

    double getTextualScoreBT(std::string& objectwords)
    {

      int numMatches = 0;
      int count = 0;
      int duplicate = 0;
      std::string prev_str = "";
      stringutils::StringTokenizer parse1(objectwords, " ");

      while(parse1.hasNextToken())
      {
        std::string eval = parse1.nextToken();

        CcString* stn = new CcString(true, eval);
        size_t hash = getBucket(stn);
        bucket = 0;
        bucket = hashTable[hash];
        bucketPos = 0;


        // There is a match
        if (bucket)
        {
          // Find the match
          while(bucketPos < bucket->size())
          {
            WordInfo* p = (*bucket)[bucketPos];

            if((prev_str).compare(eval) == 0)
            {
                //This match seen before
                duplicate++;
                break;
            }
            else if (((p->str)->GetValue()).compare(eval) == 0)
            {
              numMatches++;
              prev_str = eval;

              break;
            }
            bucketPos++;
          }
        } else {
          prev_str = "";
        }

        count = count + 1;

      }

      if (numMatches == 0)
      {
        return 0.0;
      }
      double uniquewords = (double)
      (sumOfBoth + count - (duplicate + numMatches));
      return (double) numMatches / uniquewords;
    }

    double getDistanceBT(Rectangle<2>& mbr, double x, double y)
    {
      double xmin = mbr.getMinX();
      double ymin = mbr.getMinY();
      double xmax = mbr.getMaxX();
      double ymax = mbr.getMaxY();
      if (xmin <= x && x <= xmax && ymin <= y && y <= ymax)
      {
        return 0.0;
      }
      // Upper quadrant
      if (ymin > y)
      {
          //left
          if (xmax < x) {
            return EuclidDistRT(x, y, xmax ,ymin);
          }
          //Center
          if (xmin <= x && x <= xmax) {
            return EuclidDistRT(x, y, x ,ymin);
          }
          //Right
          if(xmin > x)
          {
            return EuclidDistRT(x, y, xmin, ymin);
          }
      }
      else
      {
        //left
        if (xmax < x) {
          return EuclidDistRT(x, y, xmax ,ymax);
        }
        //Center
        if (xmin <= x && x <= xmax) {
          return EuclidDistRT(x, y, x ,ymax);
        }
        //Right
        if(xmin > x)
        {
          return EuclidDistRT(x, y, xmin, ymax);
        }
      }

      if (ymin <= y && y <= ymax)
      {
          if (x < xmin)
          {
            return EuclidDistRT(x, y, xmin, y);
          }
          if (x > xmax)
          {
            return EuclidDistRT(x, y, xmax, y);
          }
      }

      return 0.0;
    }
};

int BTSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s ){

   BatchTrajInfo* li = (BatchTrajInfo*) local.addr;
   switch(message){
     case OPEN: { if(li){
                    delete li;
                  }
                  ListExpr ttype = nl->Second(GetTupleResultType(s));

                  int noargs = qp->GetNoSons(s);
                  int idx1 = ((CcInt*)args[noargs-3].addr)->GetValue();
                  int idx2 = ((CcInt*)args[noargs-2].addr)->GetValue();
                  int idx3 = ((CcInt*)args[noargs-1].addr)->GetValue();
                  Rectangle<2> * mbr = static_cast<Rectangle<2>*>(args[5].addr);
                  CcReal * alpha = static_cast<CcReal*>(args[6].addr);
                  CcReal * thresh = static_cast<CcReal*>(args[7].addr);
                  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[8].addr);
                  double diagonal = GetDiag(*rec);

                  local.addr = new BatchTrajInfo(
                    9999,
                    *(mbr),
                    args[0],
                    args[1],
                    ttype,
                    idx1,
                    idx2,
                    idx3,
                    alpha->GetValue(),
                   thresh->GetValue(),
                    diagonal);
                  return 0;
                }
     case REQUEST: { if(!li){
                       return CANCEL;
                   }

                     result.addr = li->nextTuple();

                     return result.addr ? YIELD:CANCEL;
                   }
     case CLOSE : {
                    if(li){
                      delete li;
                      local.addr=0;
                    }
                    return 0;
                  }
   }
   return 0;

}


/*

Operator Descriptions

*/

struct BatchesInfo : OperatorInfo
{
  BatchesInfo()
  {
    name = "batches";
    signature = "stream(tulple(X)) x "
    "a1 x a2 x real -> Stream (tuple(X))";
    syntax = "_ batches[_,_,_]";
    meaning =
    "Assigns batchid to a list of semantictrajectory "
    "~attribute a1 should be the ST"
    "attribute a2 should batchId field"
    " and attribute a3 should be the diagonal threshold value";
  }
};


struct BBSimInfo : OperatorInfo
{
  BBSimInfo()
  {
    name = "bbsim";
    signature = "Stream(Tuple(x)) x id1 x id2"
    "x MBR1 x MBR2 x CcReal x CcReal x Rectangle"
    " -> Stream(Tuple(x))";
    syntax = "_ bbsim[_,_,_,_,_,_,_]";
    meaning = "Filters out batch-batch pair that are below the threshold"
    "and return the batch pair tuple"
    "with the batch with the largest Area 1st in tuple";
  }
};

struct BTSimInfo : OperatorInfo
{
  BTSimInfo()
  {
    name = "btsim";
    signature = "Stream(Tuple(x)) x Stream(Tuple(x)) "
    "x attr x attr x attr x rect x real x real x rect"
    " -> Stream(Tuple(x))";
    syntax = "_ _ btsim[_,_,_,_,_,_,_]";
    meaning = "Filters out trajectory-batch that are below the threshold";
  }
};


struct TTSimInfo : OperatorInfo
{
  TTSimInfo()
  {
    name = "ttsim";
    signature = "Stream(Tuple(x)) x attr x attr"
    " x real x real x rect x CellGrid2D"
    " -> Stream(Tuple(x))";
    syntax = "_ ttsim[_,_,_,_,_,_]";
    meaning = "Filters out trajectory-trajectory pair"
    " that are below the threshold";
  }
};

struct SimilarityInfo : OperatorInfo
{
  SimilarityInfo()
  {
    name = "sim";
    signature = "Stream(Tuple(x)) x attr x attr x "
    " CellGrid2D x real x real x rect"
    " -> Stream(Tuple(x))";
    syntax = "_ sim[_,_,_,_,_]";
    meaning =
    "Filter out trajectory-trajectory pair that are below threshold";
  }
};

struct STBboxInfo : OperatorInfo
{
  STBboxInfo()
  {
    name = "stbox";
    signature = SemanticTrajectory::BasicType()
    + " -> " + Rectangle<2>::BasicType();
    syntax = "stbox(_)";
    meaning = "Returns the bounding box"
    "of a semantic trajectory";
  }
};

struct MakeSemTrajInfo : OperatorInfo
{
  MakeSemTrajInfo()
  {
    name = "makesemtraj";
    signature =
    "stream(tuple(a1 t1) ...(an tn) ) "
    " x ai x aj x ak-> semantictrajectory";
    syntax = "_ makesemtraj [_,_,_]";
    meaning =
    "Convert stream of tuples"
    " into a semantictrajectory datatype";
  }
};

struct MakeSemTrajInfo2 : OperatorInfo
{
  MakeSemTrajInfo2()
  {
    name = "makesemtraj2";
    signature =
    "stream(tuple(a1 t1) ...(an tn) ) "
    " x ai x aj x ak-> semantictrajectory";
    syntax = "_ _ makesemtraj2 [_,_,_,_]";
    meaning =
    "Convert stream of tuples"
    " into a semantictrajectory datatype"
    " + removes stopwords";
  }
};

struct MakeUniqueListWordsInfo : OperatorInfo
{
  MakeUniqueListWordsInfo()
  {
    name = "makeuniquelistwords";
    signature =
    "stream(tuple(a1 t1) ...(an tn) ) x ai"
    " -> uniquestringarray";
    syntax = "_ makeuniquelistwords [_]";
    meaning =
    "Convert a stream of words into uniquestringarray";
  }
};

struct MakeSummariesInfo : OperatorInfo
{
  MakeSummariesInfo()
  {
    name = "makesum";
    signature =
    "stream(tuple) x int x semantictrajectory x "
    "cellgrid2d x attr x attr x attr"
    " -> semantictrajectory";
    syntax = "_ makesum [_,_,_,_,_,_]";
    meaning =
    "Retrieves cell origin coordinates from"
    "a semantictrajectory to create spatial and textual summary";
  }
};


struct MinDistInfo : OperatorInfo
{
  MinDistInfo()
  {
    name = "mindist";
    signature =
    "semantictrajectory x semantictrajectory"
    " x rectangle -> real";
    syntax = "mindist (_,_,_)";
    meaning =
    "Returns the spatial score between two values";
  }
};


struct ExtractKeywordsInfo : OperatorInfo
{
  ExtractKeywordsInfo()
  {
    name = "extractkeywords";
    signature =
    "semantictrajectory -> stream(string)";
    syntax = "_ extractkeywords";
    meaning =
    "Extracts semanticTrajectory string and"
    " converts it into a"
    "a stream of words";
  }
};




/*
3.11 Creation of the Type Constructor Instance

*/
TypeConstructor semantictrajectory (
  SemanticTrajectory::BasicType(),
  //name
  SemanticTrajectory::Property,
  //property function
  SemanticTrajectory::Out,
  SemanticTrajectory::In,
  //Out and In functions
  0,
  0,
  //SaveTo and RestoreFrom functions
  SemanticTrajectory::Create,
  SemanticTrajectory::Delete,
  //object creation and deletion
  SemanticTrajectory::Open,
  SemanticTrajectory::Save,
  //object open and save
  SemanticTrajectory::Close,
  SemanticTrajectory::Clone,
  //object close and clone
  SemanticTrajectory::Cast,
  //cast function
  SemanticTrajectory::SizeOfObj,
  //sizeof function
  SemanticTrajectory::KindCheck);
  //kind checking function




/*
4 SemanticTrajectoryAlgebra

*/
class SemanticTrajectoryAlgebra : public Algebra
{
  public:
    SemanticTrajectoryAlgebra() : Algebra()
    {
      AddTypeConstructor(&semantictrajectory);
      semantictrajectory.AssociateKind(Kind::DATA());

      AddOperator(BBSimInfo(),
      BBSimMapValue,
      BBSimTypeMap
      );

      AddOperator(BTSimInfo(),
      BTSimMapValue,
      BTSimTypeMap
      );

      AddOperator(TTSimInfo(),
      TTSimMapValue,
      TTSimTypeMap
      );

      AddOperator(SimilarityInfo(),
      SimilarityMapValue,
      SimilarityTypeMap);

      AddOperator(MakeSemTrajInfo(),
       MakeSemTrajMV,
       TypeMapMakeSemtraj);

       AddOperator(MakeSemTrajInfo2(),
        MakeSemTrajMV2,
        TypeMapMakeSemtraj2);
      AddOperator(
        STBboxInfo(),
        STbboxMapValue,
        STboxTM);

      AddOperator( MakeSummariesInfo(),
      MakesummariesMV,
      MakesummariesTM );

      AddOperator( ExtractKeywordsInfo(),
      extractkeywordMapV,
      extractkeywordsTM);

      AddOperator ( BatchesInfo(),
      BatchesVM,
      BatchesTM
      );
    }
    ~SemanticTrajectoryAlgebra() {};

};


/*

5 Initialization

*/

extern "C"
Algebra*
InitializeSemanticTrajectoryAlgebra(NestedList *nlRef,
  QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new SemanticTrajectoryAlgebra());
}
} // end of namespace
