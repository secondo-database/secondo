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
#include "Algebras/Raster2/UniqueStringArray.h"
// for operator stopword
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

  int32_t GetoriginX() { return origx; }
  int32_t GetoriginY() { return origy; }
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
        bbox.SetDefined(false);
    }
    /*
    Functions for Attribute type
    */
    int NumOfFLOBs() const {
      return 5; }
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
    bool AddStringst(const std::string& stString);
    void AddCoordinate( const Coordinate& p);
    void Destroy();
    Coordinate GetCoordinate( int i ) const;
    bool GetString(int index, std::string& rString) const;
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
    // double GetCellDist(Cell& c1, Cell& c2);
    // double EuclidDist(double x1, double y1,
    // double x2, double y2) const;
    // double MinDistAux(SemanticTrajectory& sp);
    // double MinDistUtils(SemanticTrajectory& sp,
    //  DbArray<Cell>& Ax,
    // DbArray<Cell>& Ay, int32_t m);
    // DbArray<Cell> GetCellList() const;
    // double BruteForce(DbArray<Cell>& Ax, int32_t m);
    /* TODO move the implementation outside the class definition */
    static int CompareX(const void* a, const void* b)
    {
      const double ax = ((Cell*)a)->GetoriginX();
      const double bx = ((Cell*)b)->GetoriginX();
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
      const double ax = ((Cell*)a)->GetoriginY();
      const double bx = ((Cell*)b)->GetoriginY();
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
    static bool compare_nocase (const std::string& first,
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
    /* Functions for textual summary */
    void AddWord( const WordST& w);
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
      return listutils::
      isSymbol(type, BasicType());
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
TODO change name of function

*/

bool SemanticTrajectory::
AddStringst(const std::string& stString)
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

*/
void SemanticTrajectory::AddWord( const WordST& w)
{
  words.Append(w);
}

/*
Add a cell to the DbArray

*/
void SemanticTrajectory::
AddCell(const Cell& c)
{
  cells.Append(c);
}

// DbArray<Cell> SemanticTrajectory::
//GetCellList() const
// {
//   assert(IsDefined());
//   return cells;
// }

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

      textual =
          nl->OneElemList(
            nl->TwoElemList(
              nl->IntAtom(st->GetWord(0).indexId),
              nl->IntAtom(st->GetWord(0).count)
            )
          );
       ListExpr textuallast = textual;
       for( int i = 1; i < st->GetNumWords(); i++ )
       {
         textuallast = nl->Append(textuallast,
               nl->TwoElemList(
                 nl->IntAtom(st->GetWord(i).indexId),
                 nl->IntAtom(st->GetWord(i).count)
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

          st->AddStringst(stringValue);

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
          if( nl->ListLength(first) == 2 &&
              nl->IsAtom(nl->First(first)) &&
              nl->AtomType(nl->First(first))
              == IntType &&
              nl->IsAtom(nl->Second(first)) &&
              nl->AtomType(nl->Second(first))
              == IntType
            )
          {
            WordST w(nl->IntValue(nl->First(first)),
                   nl->IntValue(nl->Second(first))
                 );
            st->AddWord(w);
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

  double max = -99; /*TODO use a max varialble */
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

    /*
    GetStrings
    Tokenize into two list
    Compare for numMatches
    */
    bool success = false;
    std::string s1;
    success = GetString(i, s1);
    assert(success);
    std::string s2;
    success = st.GetString(y, s2);
    assert(success);
    std::list<std::string> tokenlist;
    std::list<std::string> tokenlist2;

    stringutils::StringTokenizer st1(s1, " ");
    while(st1.hasNextToken())
    {
      std::string eval = st1.nextToken();
      tokenlist.push_back(eval);
    }

    /*Retrieve values of second string*/
    stringutils::StringTokenizer st2(s2, " ");
    while(st2.hasNextToken())
    {
      std::string eval = st2.nextToken();
      tokenlist2.push_back(eval);
    }

    int numMatches= 0;
    int sumOfBoth =
    tokenlist.size() + tokenlist2.size();
    std::list<std::string>::iterator it;
    for(it = tokenlist.begin();
     it != tokenlist.end();
     it++)
    {
      bool flag = false;
      std::list<std::string>::iterator it2;
      for(it2 = tokenlist2.begin();
      it2 != tokenlist2.end();
      it2++)
      {
          if((*it).compare(*it2) == 0)
          {
            if (flag == false)
            {
                numMatches = numMatches + 1;
                flag = true;
            }
            it2 =tokenlist2.erase(it2);
          }
      }
    }


    double uniquewords =
    (double) (sumOfBoth - numMatches);

    double result = (double) numMatches / uniquewords;

    return result;
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
  return floor (result * 100000) / 100000;
}


/*
OPERATOR Functions

*/

/*
@author Catherine Higgins
Operator ~makeuniquelistwords~

*/

ListExpr TypeMapMakeUniqueListWords(ListExpr args)
{

  // Check to see if it's the right number of arguments
  if (!nl->HasLength(args, 2))
  {
    return
    listutils::typeError("Wrong number of arguments");
  }

  // Make sure each param is of right type
  std::string err = "stream(tuple) x attr_1 expected";

  ListExpr stream = nl->First(args);
  ListExpr attrname_word = nl->Second(args);

  if(!listutils::isTupleStream(stream)){
    return  listutils::typeError(
    "first parameter must be"
    " a tuple stream");
  }

  if(!listutils::isSymbol(attrname_word)){
    return
    listutils::typeError("second parameter must"
    "be an attribute name");
  }

  ListExpr type;
  // extract the attribute list
  ListExpr attrList = nl->Second(nl->Second(stream));

  // Get the index for the longitude
  std::string name =
  nl->SymbolValue(attrname_word);
  int index1 = listutils::findAttribute(
    attrList, name, type);
  if(index1==0){
    return
    listutils::typeError(
      "attribute name " + name +
      " unknown in tuple stream");
  }


  std::string restype =
  raster2::UniqueStringArray::BasicType();


  ListExpr indexes = nl->OneElemList(
                       nl->IntAtom(index1-1)
                     );

  return
  nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                           indexes,
                           nl->SymbolAtom(restype));
}




int MapValueMakeUniqueListWords(
  Word* args,
  Word& result,
  int message,
  Word& local,
  Supplier s)
{

  result = qp->ResultStorage(s);
  raster2::UniqueStringArray* res =
  static_cast<
  raster2::UniqueStringArray*>(result.addr);

  int noargs = qp->GetNoSons(s);
  int idx = ((CcInt*)args[noargs-1].addr)->GetValue();

  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();
  while((tuple = stream.request()))
  {

    CcString* str =
    (CcString*) tuple->GetAttribute(idx);
    res->AddString(str->GetValue());
    tuple->DeleteIfAllowed();
  }

  stream.close();
  res->Finalize();
  return 0;
}


/*
Operator ~similarity ~

*/
ListExpr SimilarityTypeMap(ListExpr args)
{

  NList type(args);
  if(type != NList(SemanticTrajectory::BasicType(),
  SemanticTrajectory::BasicType(),
  Rectangle<2>::BasicType(),
   CcReal::BasicType()))
  {
    return
    NList::typeError(
      "Expecting two semantic trajectories,"
    " a rectangle<2> and a real value for the alpha");
  }
  return
  NList(CcReal::BasicType()).listExpr();
}

int SimilarityMapValue(Word* args,
  Word& result,
  int message,
  Word& local,
  Supplier s)
{

  SemanticTrajectory* st1 =
  static_cast<SemanticTrajectory*>(args[0].addr);
  SemanticTrajectory* st2 =
  static_cast<SemanticTrajectory*>(args[1].addr);
  Rectangle<2>* rec =
  static_cast<Rectangle<2>*>(args[2].addr);
  CcReal * alpha = static_cast<CcReal*>(args[3].addr);

  result = qp->ResultStorage(s);
  double diag = st1->GetDiagonal(*rec);
  double answer = st1->Similarity(
    *st2,
    diag,
    alpha->GetValue());
  ((CcReal*) result.addr)->Set(answer);

  return 0;
}

/*
Operator ~makesemtraj~

*/

ListExpr TypeMapMakeSemtraj(ListExpr args){

  // Check to see if it's the right number of arguments
  if (nl->HasLength(args, 4) || nl->HasLength(args, 5))
  {
    // Make sure each param is of right type
    std::string err = "stream(tuple) x attr_1 x"
    " attr_2 x attr_3 expected";

    ListExpr stream = nl->First(args);
    ListExpr attrname_longitude = nl->Second(args);
    ListExpr attrname_latitude = nl->Third(args);
    ListExpr attrname_semantics = nl->Fourth(args);
    if (nl->HasLength(args, 5))
    {
      if(!raster2::UniqueStringArray::checkType(
        nl->Fifth(args)))
      {
          return
          listutils::typeError(
            "Fifth args must be of"
            "type UniqueStringArray");
      }
    }
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


int makesemtrajSelect( ListExpr args )
{
  NList type(args);
  if (nl->HasLength(args, 5) &&
  type.fifth().isSymbol(
  raster2::UniqueStringArray::BasicType()))
  {
    return 1;
  }
  else
  {
    return 0;
  }
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
    res->AddStringst(sem->GetValue());
    // if (!res->bbox.IsDefined()){
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
    // } // end of if

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

int MakeSemTrajMVwStop(Word* args, Word& result,
                      int message, Word& local,
                      Supplier s){


  result = qp->ResultStorage(s);
  SemanticTrajectory* res =
  static_cast<SemanticTrajectory*>(result.addr);
  raster2::UniqueStringArray* stopwords =
  static_cast<
  raster2::UniqueStringArray*>(args[4].addr);

  int noargs = qp->GetNoSons(s);
  int idx1 = ((CcInt*)args[noargs-3].addr)->GetValue();
  int idx2 = ((CcInt*)args[noargs-2].addr)->GetValue();
  int idx3 = ((CcInt*)args[noargs-1].addr)->GetValue();

  double c_x1;
  double c_y1;
  double c_x2;
  double c_y2;
  // Get StopWords once
  std::list<std::string> sw;
  sw = stopwords->GetUniqueStringArray();
  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();
  while((tuple = stream.request()))
  {
    std::string holdresult = "";
    CcReal* x = (CcReal*) tuple->GetAttribute(idx1);
    CcReal* y = (CcReal*) tuple->GetAttribute(idx2);
    CcString* sem = (CcString*)
     tuple->GetAttribute(idx3);
    double x1 = x->GetValue();
    double y1 = y->GetValue();
    Coordinate c(x1, y1);
    res->AddCoordinate(c);

    /* TO DO handle the apostrophes */
    /* Tokenize string first and
    filter out any stop words */
    stringutils::StringTokenizer st(
      sem->GetValue(), " ");

    while(st.hasNextToken())
    {
      std::string eval = st.nextToken();
      stringutils::trim(eval);
      stringutils::toLower(eval);
      if(std::find(
        sw.begin(),
        sw.end(),
        eval) == sw.end())
      {
        if(holdresult.empty())
        {
            holdresult = eval;
        }
        else
        {
          holdresult = holdresult + " " + eval;
        }
      }
    }

    res->AddStringst(holdresult);

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
  res->Finalize();
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



/*
Operator ~stcellnumber~

*/

ListExpr
STcellNumberTM( ListExpr args )
{

  std::string err =
  "stcellnumber expects a semantic"
  "trajectory and a girdcell2d";

  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError(err);
  }
  if (!SemanticTrajectory::checkType(nl->First(args))
  || !CellGrid2D::checkType(nl->Second(args)))
  {
    return listutils::typeError(err);
  }

  return NList(Symbol::STREAM(),
  CcInt::BasicType()).listExpr();
}


int STCellNumberMapValue(Word* args, Word& result,
  int message,
  Word& local,
  Supplier s)
{
  // An auxiliary type which keeps of
  // which coordinate is being
  // examine between requests

  struct InGrid {
    CellGrid2D* grid;
    std::set<int> outputCells;
    int visitCoordinates = 0;
    int numCoordinates= 0;
    SemanticTrajectory* st;
    InGrid(const double &x0, const double &y0,
      const double &wx, const double &wy,
       const int32_t &nox, void* addr)
    {
      grid = new CellGrid2D(x0, y0, wx, wy, nox);
      st = static_cast<SemanticTrajectory*>(addr);
      numCoordinates = st->GetNumCoordinates();
    }

    bool getNextCell(int& cell)
    {

      double x =
      st->GetCoordinate(visitCoordinates).x;
      double y =
      st->GetCoordinate(visitCoordinates).y;
      int tempcell = grid->getCellNo(x,y);
      visitCoordinates++;
      if (outputCells.find(tempcell)
      != outputCells.end())
      {
        return false;
      }
      outputCells.insert(tempcell);
      cell = tempcell;
      return true;
    }
    void deleteInnerGrid()
    {
      //Not sure if this is needed
      // or happens automatically
      // delete grid;
    }

  };
  InGrid* localgrid =
  static_cast<InGrid*>(local.addr);


  switch(message)
  {

    case OPEN: // Initialize the local storage
    {
      // If localgrid already exists
      // need to delete and restart
      if(localgrid)
      {
        delete localgrid;
        localgrid = 0;
      }

      // CellGrid2D* grid =
      // static_cast<CellGrid2D*>(args[1].addr);
      //TODO extract the values
      // and not hardcode TheEmptyList

      localgrid =
      new InGrid(0.0, 0.0,1.1, 1.1, 10, args[0].addr);
      local.addr = localgrid;
      return 0;
    }
    case REQUEST: // returnthe next stream element
    {

      if (localgrid->visitCoordinates <
        localgrid->numCoordinates)
      {
        int cell;
        bool success = localgrid->getNextCell(cell);

        if (success == true)
        {
          CcInt* elem = new CcInt(true, cell);
          cout << "what is the answer " << elem << endl;
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
        localgrid->deleteInnerGrid();
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      if (localgrid != 0)
      {
        delete localgrid;
        local.addr = 0;
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

int extractkeywordMapV(Word* args, Word& result,
  int message,
  Word& local,
  Supplier s)
{
  // An auxiliary type which keeps of
  // which coordinate is being
  // examine between requests
  // cout << "Does it enter VM" << endl;
  struct TheWords {

    int visitedwords = 0;
    std::list<std::string>::iterator it;
    int numOfWords = 0;
    // raster2::UniqueStringArray* strArr;
    std::list<std::string> wordList;
    TheWords(std::list<std::string>& list)
    {
      wordList = list;
      numOfWords = wordList.size();
      it = wordList.begin();
      // strArr = static_cast<raster2::UniqueStringArray*>(addr);
      // numCoordinates = strArr->Get();
    }

    bool getNextWord(std::string& el)
    {
      el = *it;
      it++;
      visitedwords++;
      return true;
    }

  };
  TheWords* localword =
  static_cast<TheWords*>(local.addr);


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
      // cout << "Open" << endl;
      SemanticTrajectory* strarr =
      static_cast<SemanticTrajectory*>(args[0].addr);
      std::list<std::string> list = strarr->GetStringArray();
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
        // localgrid->deleteInnerGrid();

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

/*
Type Mapping for Operator ~mindist~

*/
//  ListExpr MinDistTM(ListExpr args)
//  {
//    std::string err =
// "Must pass in two SemanticTrajectory"
// " and one rectangle";
//    if(!SemanticTrajectory::checkType(
      // nl->First(args))
//       ||
//       !SemanticTrajectory::checkType(
      // nl->Second(args))
//       ||
//       !Rectangle<2>::checkType(nl->Third(args))
//     )
//    {
//      return listutils::typeError(err);
//    }
//    return NList(CcReal::BasicType()).listExpr();
// }
//
// int MinDistMV(Word* args, Word& result,
// int message,
// Word& local,
// Supplier s)
// {
//
//   result = qp->ResultStorage(s);
//   /*
//   Collect all the arguments
//   Create new Real value to hold return result
//   */
//   SemanticTrajectory* s1 =
// static_cast<SemanticTrajectory*>( args[0].addr );
//   SemanticTrajectory* s2 =
// static_cast<SemanticTrajectory*>( args[1].addr );
//   // Rectangle<2>* rect =
// static_cast<Rectangle<2>*>(args[2].addr);
//   double answer = s1->MinDistAux(*s2);
//   CcReal* res =
// static_cast<CcReal*>(result.addr);
//   res->Set(true, answer);
//
//
//
//   return 0;
// }
//
// double SemanticTrajectory::MinDistAux(
// SemanticTrajectory& sp) {
//     DbArray<Cell>* Ax = new DbArray<Cell>(0);
//     Ax->Append(GetCellList());
//     Ax->Append(sp.GetCellList());
//     DbArray<Cell>* Ay = Ax;
// //Using copy construtor here
//     Ax->Sort(SemanticTrajectory::CompareX);
//     Ay->Sort(SemanticTrajectory::CompareY);
//     int32_t m = Ax->Size();
//     double result = MinDistUtils(sp, *Ax, *Ay, m);
//   return result;
// }
//
// double SemanticTrajectory::MinDistUtils(
// SemanticTrajectory& sp,
// DbArray<Cell>& Ax,
// DbArray<Cell>& Ay, int32_t m)
// {
//   double minD = DBL_MAX;
//   if (m > 3)
//   {
//
//     int n = m/2;
//     DbArray<Cell>* AxL = new DbArray<Cell>(0);
//     Cell cell1;
//     Ax.Get(0,cell1);
//     Ax.copyTo(*AxL, 0, n ,0);
//     DbArray<Cell>* AxR = new DbArray<Cell>(0);
//     Ax.copyTo(*AxR, n, m-n, 0);
//     DbArray<Cell>* AyL = new DbArray<Cell>(0);
//     DbArray<Cell>* AyR = new DbArray<Cell>(0);

//     double minDL;
//     double minDR;
//     for (int i = 0; i < m; i++)
//     {
//       Cell celly, cellx;
//       Ay.Get(i, celly);
//       Ax.Get(i, cellx);
//       if(celly.GetXL() <= cellx.GetXL())
//       {
//         AyL->Append(celly);
//       }
//       else
//       {
//         AyR->Append(celly);
//       }
//     }
//
//     /* Check to see if AxL has
        // cells from both C1 and C2 */
//     Cell cellAxL;
//     AxL->Get(0, cellAxL);
//     int32_t firstId = cellAxL.GetId();
//     bool both = false;
//     for (int i = 1; i < AxL->Size(); i++)
//     {
//       AxL->Get(i, cellAxL);
//       if (cellAxL.GetId() != firstId)
//       {
//         both = true;
//         break;
//       }
//     }
//     if (both == true)
//     {
//       minDL = MinDistUtils(sp, *AxL, *AyL, n);
//     }
//     else
//     {
//       minDR = DBL_MAX;
//     }
//     Cell cellAxR;
//     AxR->Get(0, cellAxR);
//     firstId = cellAxR.GetId();
//     both = false;
//     for (int i = 1; i < AxR->Size(); i++)
//     {
//       AxR->Get(i, cellAxR);
//       if (cellAxR.GetId() != firstId)
//       {
//         both = true;
//         break;
//       }
//     }
//     if (both == true)
//     {
//       minDL = MinDistUtils(sp, *AxR, *AyR, m-n);
//     }
//     else
//     {
//       minDL = DBL_MAX;
//     }
//     minD = minDL > minDR ? minDR : minDL;
//     DbArray<Cell>* Am = new DbArray<Cell>(0);
//     for (int i = 0; i < m; i++)
//     {
//       Cell x;
//       Cell y;
//       Ax.Get(i, x);
//       Ay.Get(i, y);
//       if (fabs(y.GetXL() - x.GetXL()) < minD)
//       {
//         Am->Append(y);
//       }
//     }
//     for (int i = 0; i < Am->Size(); i++)
//     {
//       for (int j = i + 1; j < Am->Size(); j++)
//       {
//         Cell c1;
//         Cell c2;
//         Am->Get(i, c1);
//         Am->Get(j, c2);
//         if (c1.GetId() != c2.GetId())
//         {
//           double tempmin = GetCellDist(c1,c2);
//           if (tempmin < minD)
//           {
//             minD = tempmin;
//           }
//         }
//       }
//     }
//
//     //Don't forget to delete the DbArrays
// that were initialized here
//     delete Am;
//     delete AyR;
//     delete AyL;
//     delete AxR;
//     delete AxL;
//   }
//   else
//   {
//     minD = BruteForce(Ax, m);
//   }
//   return minD;
// }

// double SemanticTrajectory::GetCellDist(
// Cell& c1, Cell& c2)
// {
//   double result;
//   //Are they the same cell
//   if (c1.GetXL() == c2.GetXL()
// && c1.GetYL() == c2.GetYL())
//   {
//     return 0.0;
//   }
//
//   //c1 and c2 on the same x Axis
//   if (c1.GetXL() == c2.GetXL())
//   {
//     // if c1 is above c2
//     // Take bottom corner of c1 and top corner of c2
//     if(c1.GetYL() > c2.GetYL())
//     {
//       return
// EuclidDist(c1.GetXL(), c1.GetYL(),
// c2.GetXL(), c2.GetYR());
//     }
//     // if c1 is below c2
//     // Take top corner of c1 and bottom corner of c2
//     else
//     {
//       return
// EuclidDist(c1.GetXL(),
// c1.GetYR(),
// c2.GetXL(),
// c2.GetYL());
//     }
//   }
//   //c1 and c2 on the same y axis
//   if (c1.GetYL() == c2.GetYL())
//   {
//     // if c1 is to the right of c2
//     if(c1.GetXL() > c2.GetXL())
//     {
//       return
 // EuclidDist(c1.GetXL(), c1.GetYL(),
 // c2.GetXR(), c2.GetYL());
//     }
//     // if c1 is to the left of c2
//     else
//     {
//       return
// EuclidDist(c1.GetXR(),
// c1.GetYL(),
// c2.GetXL(), c2.GetYL());
//     }
//   }
//   // if c1 is above c2
//   if (c1.GetYL() > c2.GetYL())
//   {
//     // if c1 is to the right of c2
//     if (c1.GetXL() > c2.GetXL())
//     {
//       return
// EuclidDist(c1.GetXL(),
// c1.GetYL(),
// c2.GetXR(),
// c2.GetYR());
//     }
//     // if c1 is to the left c2
//     //
//     else
//     {
//       return
 // EuclidDist(c1.GetXR(), c1.GetYL(),
 // c2.GetXL(), c2.GetYR());
//     }
//   }
//   // if c1 is below c2
//   else
//   {
//     // if c1 is to the right of c2
//     if (c1.GetXL() > c2.GetXL())
//     {
//       return
 //      EuclidDist(
 // c1.GetXR(),
 // c1.GetYL(),
 // c2.GetXR(),
 // c2.GetYL());
//     }
//     // if c1 is to the left c2
//     else
//     {
//       return
 // EuclidDist(
 // c1.GetXR(),
 // c1.GetYR(),
 // c2.GetXL(),
  // c2.GetYL());
//     }
//   }
//   return result;
// }
//
// double SemanticTrajectory::BruteForce(
// DbArray<Cell>& Ax,
// int32_t m)
// {
//   double minD = DBL_MAX;
//   for (int i = 0; i < Ax.Size(); i++)
//   {
//     for (int j = i + 1; j < Ax.Size(); j++)
//     {
//       Cell c1;
//       Cell c2;
//       Ax.Get(i, c1);
//       Ax.Get(j, c2);
//       if (c1.GetId() != c2.GetId())
//       {
//         double tempmin = GetCellDist(c1,c2);
//         if (tempmin < minD)
//         {
//           minD = tempmin;
//         }
//       }
//     }
//   }
//   return minD;
// }
//
//
// double SemanticTrajectory::EuclidDist(double x1,
    // double y1,
  // double x2,
// double y2) const
// {
//   return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2));
// }




//TypeMapping for operator ~ makesummaries ~

ListExpr MakesummariesTM( ListExpr args )
{
  if( (nl->ListLength(args) == 6)
    && listutils::isTupleStream(nl->First(args))
    && listutils::isNumericType(nl->Second(args))
    && SemanticTrajectory::checkType(nl->Third(args))
    && CellGrid2D::checkType(nl->Fourth(args))
    && listutils::isSymbol(nl->Fifth(args))
    && listutils::isSymbol(nl->Sixth(args))
    )
  {
    ListExpr stream = nl->First(args);
    ListExpr attrname_WiD = nl->Fifth(args);
    ListExpr attrname_Ctn = nl->Sixth(args);
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
    if(!CcInt::checkType(type)){
      return
      listutils::typeError("attribute '" + name +
      "' must be of type 'int'");
    }
    ListExpr indexes = nl->TwoElemList(
                         nl->IntAtom(index1-1),
                         nl->IntAtom(index2-1));
    return
    nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                      indexes,
    nl->SymbolAtom(SemanticTrajectory::BasicType()));
  }
  return
  listutils::typeError("Expected "
  "(stream(tuple) x int x semantictrajectory x "
  "cellgrid2d x attr_WordId x attr_Ctn).");
}


//ValueMapping for operator ~ makespatialsum ~


int MakesummariesMV(Word* args, Word& result,
   int message,
   Word& local,
   Supplier s)
{

  // cout << "Enters MakeSum MV" << endl;
  result = qp->ResultStorage(s);
  int noargs = qp->GetNoSons(s);
  int idx1 = ((CcInt*)args[noargs-2].addr)->GetValue();
  int idx2 = ((CcInt*)args[noargs-1].addr)->GetValue();
  // cout << "What are the idx " << idx1 << " " << idx2 << endl;
  Stream<Tuple> stream(args[0]);
  Tuple* tuple;
  stream.open();
  //Collect all the arguments
  //Create new spatial summary to hold results

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
  /*
  TODO should add error check to
  make sure grid is right dimension
  ie data point may be oustide
  if wrong grid is given

  */
  for(int i = 0; i < st->GetNumCoordinates(); i++)
  {
    Coordinate c = st->GetCoordinate(i);

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
    // cout << "WordID " << id->GetIntval() << endl;
    WordST wt(id->GetIntval(), ctn->GetIntval());
    res->AddWord(wt);
    tuple->DeleteIfAllowed();
  }

  res->EndST(*st);
  // for (int i = 0 ; i < res->GetNumWords(); i++)
  // {
  //   cout << "End print" << endl;
  //   cout << res->GetWord(i).indexId << endl;
  // }
  res->Finalize();
  stream.close();
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


  // cout << "What are the args position" << index1 << " " << index2 << endl;
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
            // cout << "dgbox " << dgbox << endl;
            // cout << "diagmin " << diagmin << endl;
            if(dgbox < diagmin)
            {
              diagmin = dgbox;
              // cout << "Yes BMIN is " << i << endl;
              bmin = i; // hold the index of
            }
            i++;
          }
          if (diagmin <= diathreshold)
          {
            // cout << "Assigned to bmin" << endl;
            it = currentBatches.begin();
            std::advance(it, bmin);
            (*it).addnewMBR(st->GetBoundingBox());
            assignBatchId = bmin;
          }
          else
          {
            // cout << "New Group" << endl;
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

struct STCellNumberInfo : OperatorInfo
{
  STCellNumberInfo()
  {
    name = "stcellnumber";
    signature = SemanticTrajectory::BasicType()
    + " X GridCell2D -> Stream (int)";
    syntax = "stcellnumber(_,_)";
    meaning =
    "Returns all cell number of "
    "the coordinates of a semantic trajectory";
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


struct SimilarityInfo : OperatorInfo
{
  SimilarityInfo()
  {
    name = "similarity";
    signature = SemanticTrajectory::BasicType() + " x "
    + SemanticTrajectory::BasicType()
    + " x " + Rectangle<2>::BasicType()
    + " x " + CcReal::BasicType()
    + " -> " + CcReal::BasicType();
    syntax = "similarity (_,_,_,_)";
    meaning =
    "Get similarity score of"
    " two semantic Trajectories";
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
    name = "makesummaries";
    signature =
    "stream(tuple) x int x semantictrajectory x cellgrid2d x attr x attr"
    " -> semantictrajectory";
    syntax = "_ makesummaries [_,_,_,_,_]";
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


struct GetWordIdInfo : OperatorInfo
{
  GetWordIdInfo()
  {
    name = "getwordid";
    signature =
    "semantictrajectory -> stream(int)";
    syntax = "getwordid(_)";
    meaning =
    "Retrieve wordId from a semantictrajectory";
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

      AddOperator(SimilarityInfo(),
      SimilarityMapValue,
      SimilarityTypeMap);

      ValueMapping makesemMV[] = {
        MakeSemTrajMV,
        MakeSemTrajMVwStop,
        0};
      AddOperator(MakeSemTrajInfo(),
       makesemMV,
       makesemtrajSelect,
       TypeMapMakeSemtraj);
      AddOperator(
        STBboxInfo(),
        STbboxMapValue,
        STboxTM);
      // AddOperator(
      // STCellNumberInfo(),
      // STCellNumberMapValue,
      // STcellNumberTM);
      AddOperator( MakeSummariesInfo(),
      MakesummariesMV,
      MakesummariesTM );
      // AddOperator(
      // MinDistInfo(),
      // MinDistMV,
      // MinDistTM );

      AddOperator( ExtractKeywordsInfo(),
      extractkeywordMapV,
      extractkeywordsTM);

      AddOperator( MakeUniqueListWordsInfo(),
      MapValueMakeUniqueListWords,
      TypeMapMakeUniqueListWords );

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
