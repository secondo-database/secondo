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


#include "SemanticTrajectoryAlgebra.h"
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
// use of tuple
#include "Algebras/Rectangle/CellGrid.h"
#include "Stream.h"
// Needed to process streams
extern NestedList* nl;
extern QueryProcessor *qp;


using std::cout;
using std::endl;
namespace semtraj {

  /*

  Non-stardard constructor must be used when using Flobs or DbArrays
  Initialize each type of Flob to 0

  */

  SemanticTrajectory::SemanticTrajectory(int dummy):
    Attribute(true),
    coordinates(0),
    semantics(0),
    semantics_Flob(0),
    cells(0),
    words(0),
    words_Flob(0),
    bbox(false)
  {

  }

  SemanticTrajectory::
  SemanticTrajectory(const SemanticTrajectory& st) :
    Attribute(st.IsDefined()),
    coordinates(st.coordinates.Size()),
    semantics(st.semantics.Size()),
    semantics_Flob(st.semantics_Flob.getSize()),
    cells(st.cells.Size()),
    words(st.words.Size()),
    words_Flob(st.words_Flob.getSize()),
    bbox(st.bbox)
  {

    bool success = false;
    success = semantics.Append(st.semantics);
    assert(success);
    success = coordinates.Append(st.coordinates);
    assert(success);
    success = semantics_Flob.copyFrom(st.semantics_Flob);
    assert(success);
    success = cells.Append(st.cells);
    assert(success);
    success = words.Append(st.words);
    success = words_Flob.copyFrom(st.words_Flob);
    assert(success);
    bbox = st.bbox;
  }

  SemanticTrajectory::~SemanticTrajectory()
  {

  }

  SemanticTrajectory& SemanticTrajectory::
  operator=(const SemanticTrajectory& st)
  {
    SetDefined(st.IsDefined());
    bool success = false;
    success = semantics.clean();
    assert(success);
    success = semantics.copyFrom(st.semantics);
    assert(success);
    success = coordinates.clean();
    assert(success);
    success = coordinates.copyFrom(st.coordinates);
    assert(success);
    success = semantics_Flob.clean();
    assert(success);
    success = semantics_Flob.copyFrom(st.semantics_Flob);
    bbox = st.bbox;
    success = cells.clean();
    assert(success);
    success = cells.Append(st.cells);
    assert(success);
    success = words.clean();
    assert(success);
    success = words.Append(st.words);
    success = words_Flob.clean();
    assert(success);
    success = words_Flob.copyFrom(st.words_Flob);
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
      stFlob = &semantics;
    } else if (i == 2) {
      stFlob = &semantics_Flob;
    } else if (i == 3) {
      stFlob = &cells;
    } else if (i == 4) {
      stFlob = &words;
    } else if (i == 5)
    {
      stFlob = &words_Flob;
    }
    return stFlob;
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
    List represenation
    TODO UPDATE THIS INFO AS IT IS NOT UP TO DATE
     ((id xl xr yl yr)...(id xl xr yl yr))
    Nested list of two: first is a 4 coordinates
    to represent a rectangle

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
      bool success = st->GetSemString(0, holdValue);

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
        bool success = st->GetSemString(i, holdValue);
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

  */

  /* The bouding box get's calculated everytime */

  Word
  SemanticTrajectory::In(
  const ListExpr typeInfo,
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

            st->AddSemString(stringValue);

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

    semantics.Destroy();
    semantics_Flob.destroy();
    coordinates.Destroy();
    cells.Destroy();
    words.Destroy();
    words_Flob.destroy();
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
Add a coordinate to the DbArray

*/
void SemanticTrajectory::
AddCoordinate(
  const Coordinate& c)
{
  coordinates.Append(c);

}

Coordinate SemanticTrajectory::
GetCoordinate( int i ) const
{
  assert( 0 <= i && i < GetNumCoordinates() );
  Coordinate c;
  coordinates.Get( i, &c );
  return c;
}

/*
This function is responsible of adding a string to the Text

*/

bool SemanticTrajectory::
AddSemString(const std::string& stString)
{

  if (!stString.empty())
  {
    TextData td;
    td.Offset = semantics_Flob.getSize();
    td.Length = stString.length();
    bool success = semantics.Append(td);
    assert(success);
    semantics_Flob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
}

bool SemanticTrajectory::
GetSemString(int index, std::string& rString) const
{

  bool success = false;
  int numString = semantics.Size();
  if (index < numString)
  {
      TextData textData;
      success = semantics.Get(index, &textData);
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
          success = semantics_Flob.read(
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

bool SemanticTrajectory::AddStringSum(
  const std::string& stString, int id, int count)
{
  if (!stString.empty())
  {
    WordST td;
    td.Offset = words_Flob.getSize();
    td.Length = stString.length();
    td.indexId = id;
    td.count = count;
    bool success = words.Append(td);
    assert(success);
    words_Flob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
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
            success = words_Flob.read(
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

std::list<std::string> SemanticTrajectory::GetStringArray() const
{


  std::list<std::string> stringArray;

  int nStrings = semantics.Size();
  bool bOK = false;
  std::string holdString;

  for(int i = 0; i < nStrings; i++)
  {
    bOK = GetSemString(i, holdString);
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
  + double (1 - alpha) * TextualScore(i, y, st);
  return result;
}

double SemanticTrajectory::GetDiagonal(
  Rectangle<2>& rec)
{
  double x1 = rec.getMinX();
  double y1 = rec.getMinY();
  double x2 = rec.getMaxX();
  double y2 = rec.getMaxY();
  double result = sqrt(pow((x1 - x2),2)
  + pow((y1 - y2),2));
  return result;
}

double GetDiag(
  Rectangle<2>& rec)
{
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
    success = GetSemString(i, s1);
    assert(success);
    std::string s2;
    success = st.GetSemString(y, s2);
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

  return result;
}




/*
Batch Functions

*/
Batch::Batch(int dummy):
  Attribute(true),
  batchwords(0),
  batchwords_Flob(0),
  trips(0),
  bcells(0),
  bwords(0),
  bwords_Flob(0),
  bcoordinates(0),
  bsemantics(0),
  bsemantics_Flob(0),
  bbox(false),
  batchId(0)
{

}

Batch::
Batch(const Batch& b) :
  Attribute(b.IsDefined()),
  batchwords(b.batchwords.Size()),
  batchwords_Flob(b.batchwords_Flob.getSize()),
  trips(b.trips.Size()),
  bcells(b.bcells.Size()),
  bwords(b.bwords.Size()),
  bwords_Flob(b.bwords_Flob.getSize()),
  bcoordinates(b.bcoordinates.Size()),
  bsemantics(b.bsemantics.Size()),
  bsemantics_Flob(b.bsemantics_Flob.getSize()),
  bbox(b.bbox),
  batchId(b.batchId)
{

  bool success = false;
  success =
  batchwords.Append(b.batchwords);
  assert(success);
  success =
  batchwords_Flob.copyFrom(b.batchwords_Flob);
  assert(success);
  success = trips.Append(b.trips);
  assert(success);
  success = bcells.Append(b.bcells);
  assert(success);
  success = bwords.Append(b.bwords);
  assert(success);
  success =
  bwords_Flob.copyFrom(b.bwords_Flob);
  assert(success);
  success =
  bcoordinates.Append(b.bcoordinates);
  assert(success);
  success = bsemantics.Append(b.bsemantics);
  assert(success);
  success =
  bsemantics_Flob.copyFrom(b.bsemantics_Flob);
  bbox = b.bbox;
}

Batch::~Batch()
{

}

Batch& Batch::
operator=(const Batch& b)
{
  SetDefined(b.IsDefined());

  bool success = false;
  success = batchwords.clean();
  assert(success);
  success =
  batchwords.copyFrom(b.batchwords);
  assert(success);
  success = batchwords_Flob.clean();
  assert(success);
  success =
  batchwords_Flob.copyFrom(b.batchwords_Flob);
  assert(success);
  success = trips.clean();
  assert(success);
  success = trips.copyFrom(b.trips);
  assert(success);
  success = bcells.clean();
  assert(success);
  success = bcells.copyFrom(b.bcells);
  assert(success);
  success = bwords.clean();
  assert(success);
  success = bwords.copyFrom(b.bwords);
  assert(success);
  success = bwords_Flob.clean();
  assert(success);
  success =
  bwords_Flob.copyFrom(b.bwords_Flob);
  assert(success);
  success = bcoordinates.clean();
  assert(success);
  success =
  bcoordinates.copyFrom(b.bcoordinates);
  assert(success);
  success = bsemantics.clean();
  assert(success);
  success =
  bsemantics.copyFrom(b.bsemantics);
  assert(success);
  success = bsemantics_Flob.clean();
  assert(success);
  success =
  bsemantics_Flob.copyFrom(b.bsemantics_Flob);
  bbox = b.bbox;
  batchId = b.batchId;
  return *this;
}



/*

TODO Update explain what each are for

*/

Flob *Batch::GetFLOB(const int i)
{
  Flob* stFlob = 0;
  if (i == 0) {
    stFlob = &batchwords;
  } else if (i == 1) {
    stFlob = &batchwords_Flob;
  } else if (i == 2) {
    stFlob = &trips;
  } else if (i == 3) {
    stFlob = &bcells;
  } else if (i == 4) {
    stFlob = &bwords;
  } else if (i == 5) {
    stFlob = &bwords_Flob;
  } else if (i == 6) {
    stFlob = &bcoordinates;
  } else if (i ==7) {
    stFlob = &bsemantics;
  } else if (i == 8) {
    stFlob = &bsemantics_Flob;
  }
  return stFlob;
}

void Batch::CopyFrom(
  const Attribute* right)
{

  if(right != 0)
  {
    const Batch* b =
    static_cast<
    const Batch*>(right);
    if(b != 0)
    {
      *this = *b;
    }
  }
}

Batch*
Batch::Clone() const
{

  Batch *b =
  new Batch(*this);
  assert(b != 0);
  return b;

}
/*
3.3
Function Describing the Signature
of the Type Constructor
TODO update when textual sum is added

*/
ListExpr
Batch::Property()
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
        Batch::BasicType()
        ),
        nl->TextAtom(
          "((batchId)(minx miny maxx maxy)"
        "((1 word1 ctn)"
        "(2 word2 ctn)...)"
        "((st1) (st2)...))"),
        nl->TextAtom(
        "((1)(3.2 15.4 6.34 15.4)"
        "(1 diversity 2)(2 bear 3)"
        "((st data type)"
        "(st datatype)))"),
        nl->TextAtom(
        "batchid, mbr,"
        " wordlist, "
        "triplist.")
      )));
}

/*
3.4 Kind Checking Function

*/
bool
Batch::KindCheck(
  ListExpr type,
  ListExpr& errorInfo )
{
  return (nl->IsEqual(
    type,
    Batch::BasicType() ));
}

/*

3.5 ~Create~-function

*/
Word Batch::Create(
  const ListExpr typeInfo)
{
  Batch* b = new Batch(0);
  return (SetWord(b));
}

void Batch::Destroy()
{
  batchwords.Destroy();
  batchwords_Flob.destroy();
  trips.Destroy();
  bcells.Destroy();
  bwords.Destroy();
  bwords_Flob.destroy();
  bcoordinates.Destroy(),
  bsemantics.Destroy(),
  bsemantics_Flob.destroy();
}

/*
3.6 ~Delete~-function

*/
void Batch::Delete(
  const ListExpr typeInfo,
  Word& w)
{

  Batch* b =
  (Batch*)w.addr;
  b->Destroy();
  delete b;
}

/*
3.6 ~Open~-function

*/
bool
Batch::Open(
             SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{

  Batch *b =
  (Batch*)Attribute::Open(
    valueRecord, offset, typeInfo );
  value.setAddr( b );

  return true;
}

/*
3.7 ~Save~-function

*/
bool
Batch::Save( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{

  Batch *b =
  (Batch *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, b );
  return true;
}

/*
3.8 ~Close~-function

*/
void Batch::Close(
  const ListExpr typeInfo,
  Word& w)
{

  Batch* b = (Batch*)w.addr;
  delete b;
}

Word Batch::Clone(
  const ListExpr typeInfo,
  const Word& rWord)
{

  Word word;
  Batch* p =
  static_cast<Batch*>(rWord.addr);
  if (p != 0)
  {
    word.addr =
    new Batch(*p);
    assert(word.addr != 0);
  }
  return word;
}


/*
3.9 ~SizeOf~-function

*/
int Batch::SizeOfObj()
{
  return sizeof(Batch);
}

/*
3.10 ~Cast~-function

*/
void* Batch::Cast(void* addr)
{
  return (new (addr) Batch);
}

ListExpr
Batch::Out(
  ListExpr typeInfo,
  Word value )
{

  // the addr pointer of value
  Batch* b =
  (Batch*)(value.addr);
  if ( !b->IsDefined() ) {
      return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  if( b->IsEmpty() )
  {
    return (nl->TheEmptyList());
  }
  else
  {

    ListExpr textual = nl->Empty();
    // ListExpr trips = nl->Empty();

    /* Need to retrieve equivalent of WordList */
    if (!b->IsEmptyWordList())
    {

      std::string holdValue;
      bool success = b->GetBSumString(0, holdValue);
      if (success == false) {
        holdValue = "";
      }
      textual =
        nl->OneElemList(
          nl->ThreeElemList(
            nl->IntAtom(b->GetBSumWord(0).indexId),
            nl->IntAtom(b->GetBSumWord(0).count),
            nl->TextAtom(holdValue)
          )
        );
         ListExpr textuallast = textual;
         for( int i = 1; i < b->GetWordListSize(); i++ )
         {

           bool success = b->GetBSumString(i, holdValue);

           if (success == false) {
             holdValue = "";
           }
           textuallast = nl->Append(textuallast,
                 nl->ThreeElemList(
                   nl->IntAtom(b->GetBSumWord(i).indexId),
                   nl->IntAtom(b->GetBSumWord(i).count),
                   nl->TextAtom(holdValue)
                 )
             );
          }

        }

    /* Retrive the tripList */
    ListExpr result;
    if (!b->IsEmptyTripList())
    {

      Trip t = b->GetTrip(0);


      /* Retrieve cell information */
      int getStartIdx = t.GetStartCellsIdx();
      int count = getStartIdx;
      int getEndIdx = t.GetEndCellsIdx();
      ListExpr spatial = nl->Empty();
      if (getStartIdx < getEndIdx)
      {
        spatial =
            nl->OneElemList(
              nl->ThreeElemList(
                nl->IntAtom(b->GetBCell(count).tripId),
                nl->IntAtom(b->GetBCell(count).origx),
                nl->IntAtom(b->GetBCell(count).origy)
              )
            );

        // #endif
         ListExpr spatiallast = spatial;
         count++;
         for( int i = count; i < getEndIdx; i++ )
         {
           spatiallast = nl->Append(spatiallast,
                 nl->ThreeElemList(
                   nl->IntAtom(b->GetBCell(i).tripId),
                   nl->IntAtom(b->GetBCell(i).origx),
                   nl->IntAtom(b->GetBCell(i).origy)
                 )
             );

          }
      } // end of cell information
      /* Retrieve Trip textual summary */
      int getStartIdx1 = t.GetStartSumWordsIdx();
      int getEndIdx1 = t.GetEndSumWordsIdx();
      ListExpr tripsum = nl->Empty();

      if (getStartIdx1 < getEndIdx1)
      {
        std::string holdValue;
        // Equivalent of GetStingSum
        bool success = b->GetBWord(getStartIdx1, holdValue);

        if (success == false) {
          holdValue = "";
        }


        tripsum =
            nl->OneElemList(
              nl->ThreeElemList(
                nl->IntAtom(b->GetBWordInfo(getStartIdx1).indexId),
                nl->IntAtom(b->GetBWordInfo(getStartIdx1).count),
                nl->TextAtom(holdValue)
              )
            );

         ListExpr tripsumlast = tripsum;
         for( int i = getStartIdx1 + 1; i < getEndIdx1; i++ )
         {
           std::string holdValue;
           // Equivalent of GetStingSum
           bool success = b->GetBWord(i, holdValue);
           if (success == false) {
             holdValue = "";
           }

           tripsumlast = nl->Append(tripsumlast,
                 nl->ThreeElemList(
                   nl->IntAtom(b->GetBWordInfo(i).indexId),
                   nl->IntAtom(b->GetBWordInfo(i).count),
                   nl->TextAtom(holdValue)
                 )
             );
          }
      } // End of Trip  textsummary information

      /* Retrive Trip coordinates and Semantics */
      int getStartIdx2 = t.GetStartCoordsIdx();
      int count2 = getStartIdx2;
      int getEndIdx2 = t.GetEndCoordsIdx();
      ListExpr tripdata = nl->Empty();
      if (getStartIdx2 < getEndIdx2)
      {
        std::string holdValue;
        bool success = b->GetBSemString(count2, holdValue);

        if (success == false) {
          holdValue = "";
        }
        tripdata = nl->OneElemList(
            nl->ThreeElemList(
              nl->RealAtom(b->GetBCoordinate(count2).x),
              nl->RealAtom(b->GetBCoordinate(count2).y),
              nl->TextAtom(holdValue)));
        ListExpr tlast = tripdata;

        count2++;
        for( int i = count2; i < getEndIdx2; i++ )
        {
          std::string holdValue;
          bool success = b->GetBSemString(i, holdValue);

          if (success == false) {
            holdValue = "";
          }
          tlast = nl->Append(tlast,
            nl->ThreeElemList(
            nl->RealAtom(b->GetBCoordinate(i).x),
            nl->RealAtom(b->GetBCoordinate(i).y),
            nl->TextAtom(holdValue)));

        }
      } // End of Trip data
      result = nl->OneElemList(
        nl->TwoElemList(
          nl->OneElemList(
          nl->IntAtom(t.GetId())),
            nl->FourElemList(
              nl->FourElemList(
                nl->RealAtom(t.bbox.getMinX()),
                nl->RealAtom(t.bbox.getMinY()),
                nl->RealAtom(t.bbox.getMaxX()),
                nl->RealAtom(t.bbox.getMaxY())
              ),
              spatial,
              tripsum,
              tripdata
            )
        )
      );
      ListExpr last = result;
      for (int i = 1; i < b->GetNumTrips(); i++)
      {
        Trip t = b->GetTrip(i);

        /* Get Cell Information */
        int getStartIdx = t.GetStartCellsIdx();
        int count3 = getStartIdx;
        int getEndIdx = t.GetEndCellsIdx();
        ListExpr spatial = nl->Empty();
        if (getStartIdx < getEndIdx)
        {
          spatial =
              nl->OneElemList(
                nl->ThreeElemList(
                  nl->IntAtom(b->GetBCell(count3).tripId),
                  nl->IntAtom(b->GetBCell(count3).origx),
                  nl->IntAtom(b->GetBCell(count3).origy)
                )
              );
           ListExpr spatiallast = spatial;
           count3++;
           for( int i = count3; i < getEndIdx; i++ )
           {
             spatiallast = nl->Append(spatiallast,
                   nl->ThreeElemList(
                     nl->IntAtom(b->GetBCell(i).tripId),
                     nl->IntAtom(b->GetBCell(i).origx),
                     nl->IntAtom(b->GetBCell(i).origy)
                   )
               );
            }
        } // End of getting cell informtion

        int getStartIdx1 = t.GetStartSumWordsIdx();
        int count1 = getStartIdx1;
        int getEndIdx1 = t.GetEndSumWordsIdx();
        ListExpr tripsum = nl->Empty();
        if (getStartIdx1 < getEndIdx1)
        {
          std::string holdValue;
          bool success = b->GetBWord(count1, holdValue);

          if (success == false) {
            holdValue = "";
          }

          tripsum =
              nl->OneElemList(
                nl->ThreeElemList(
                  nl->IntAtom(b->GetBWordInfo(count1).indexId),
                  nl->IntAtom(b->GetBWordInfo(count1).count),
                  nl->TextAtom(holdValue)
                )
              );
           ListExpr tripsumlast = tripsum;
           count1++;
           for( int i = count1; i < getEndIdx1; i++ )
           {

             std::string holdValue;
             // Equivalent of GetStingSum
             bool success = b->GetBWord(i, holdValue);
             if (success == false) {
               holdValue = "";
             }

             tripsumlast = nl->Append(tripsumlast,
                   nl->ThreeElemList(
                     nl->IntAtom(b->GetBWordInfo(i).indexId),
                     nl->IntAtom(b->GetBWordInfo(i).count),
                     nl->TextAtom(holdValue)
                   )
               );
            }
        } // End of Trip information
        /* Retrive Trip coordinates and Semantics */
        int getStartIdx2 = t.GetStartCoordsIdx();
        int count2 = getStartIdx2;
        int getEndIdx2 = t.GetEndCoordsIdx();
        ListExpr tripdata = nl->Empty();
        if (getStartIdx2 < getEndIdx2)
        {
          std::string holdValue;
          bool success = b->GetBSemString(count2, holdValue);

          if (success == false) {
            holdValue = "";
          }
          tripdata =
          nl->OneElemList(
              nl->ThreeElemList(
                nl->RealAtom(b->GetBCoordinate(count2).x),
                nl->RealAtom(b->GetBCoordinate(count2).y),
                nl->TextAtom(holdValue)));

          ListExpr tlast = tripdata;
          count2++;
          for( int i = count2; i < getEndIdx2; i++ )
          {
            std::string holdValue;
            bool success = b->GetBSemString(i, holdValue);

            if (success == false) {
              holdValue = "";
            }
            tlast = nl->Append(tlast,
              nl->ThreeElemList(
              nl->RealAtom(b->GetBCoordinate(i).x),
              nl->RealAtom(b->GetBCoordinate(i).y),
              nl->TextAtom(holdValue)));

          }
        } // End of Trip data
        last = nl->Append(last,
          nl->TwoElemList(
            nl->OneElemList(
            nl->IntAtom(t.GetId())),
              nl->FourElemList(
                nl->FourElemList(
                  nl->RealAtom(t.bbox.getMinX()),
                  nl->RealAtom(t.bbox.getMinY()),
                  nl->RealAtom(t.bbox.getMaxX()),
                  nl->RealAtom(t.bbox.getMaxY())
                ),
                spatial,
                tripsum,
                tripdata
              )
          )
        );
      }
    }

    ListExpr final = nl->FourElemList(
        nl->OneElemList(
        nl->IntAtom(b->GetBatchId())),
        nl->FourElemList(
          nl->RealAtom(b->bbox.getMinX()),
          nl->RealAtom(b->bbox.getMinY()),
          nl->RealAtom(b->bbox.getMaxX()),
          nl->RealAtom(b->bbox.getMaxY())
        ),
        textual,
        result
        );
    return final;
  }
}

/*
  List represenation
  Nested list of 4:
    first nl: Represents the batchId
    second nested list:
    Represents bounding box which is a 4 coordinates
     to represent a rectangle
    third nested list: Represents the wordlist
    fourth nested list: Represents the triplist

*/

Word
Batch::In( const ListExpr typeInfo,
const ListExpr instance,
const int errorPos,
ListExpr& errorInfo,
bool& correct )
{

  Word word;
  correct = false;
  Batch* b = new Batch(0);
  b->SetDefined(true);

  // This is if list is undefined
  //  ~ setDefined to false
  if(listutils::isSymbolUndefined( instance ))
  {
    b->SetDefined(false);
    correct = true;
    word.addr = b;
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
      b->SetDefined(false);
      correct = true;
      word.addr = b;
      return word;
    }

    if (nListLength == 4)
    {


      /* Collect BatchId */
      if (!listutils::isSymbolUndefined(nl->First(instance)))
      {
        ListExpr first = nl->First(instance);
        if( nl->ListLength(first) == 1 &&
            nl->IsAtom(nl->First(first)) &&
            nl->AtomType(nl->First(first))
            == IntType)
        {
            b->batchId = nl->IntValue(nl->First(first));

        }
      }
      /* Extract info for bbox */
      if (!listutils::isSymbolUndefined(nl->Second(instance)))
      {
        ListExpr first = nl->Second(instance);
        double mind[2];
        double maxd[2];
        if( nl->ListLength(first) == 4 &&
            nl->IsAtom(nl->First(first)) &&
            nl->AtomType(nl->First(first))
            == RealType &&
            nl->IsAtom(nl->Second(first)) &&
            nl->AtomType(nl->Second(first))
            == RealType &&
            nl->IsAtom(nl->Third(first)) &&
            nl->AtomType(nl->Third(first))
            == RealType &&
            nl->IsAtom(nl->Fourth(first)) &&
            nl->AtomType(nl->Fourth(first))
            == RealType
          )
        {
          mind[0] = nl->RealValue(nl->First(first));
          mind[1] = nl->RealValue(nl->Second(first));
          maxd[0] = nl->RealValue(nl->Third(first));
          maxd[1] = nl->RealValue(nl->Fourth(first));
          b->bbox.Set(true, mind, maxd);
        }
      }

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

            b->AddBSumString(stringValue,
              nl->IntValue(nl->First(first)),
              nl->IntValue(nl->Second(first)));

          }
          else
          {
            correct = false;
            delete b;
            return SetWord( Address(0) );
          }
        }
      }

      /* Extract info for TripList */
      if (!listutils::isSymbolUndefined(nl->Fourth(instance)))
      {


        ListExpr first = nl->Empty();
        ListExpr rest = nl->Fourth(instance);
        int cellidxcounter = 0;
        int tripsumidxcounter = 0;
        int tripdataidxcounter = 0;
        /* This is the while loop for each trip */

        while (nl->IsEmpty(rest) == false)
        {


          first = nl->First( rest );
          rest = nl->Rest( rest );
          Trip t(0);
          if(!listutils::isSymbolUndefined(nl->First(first)))
          {
            /* Get first part of two element list */

            ListExpr ok = nl->First(first);

            t.SetId(nl->IntValue(nl->First(ok)));

          }
          /*
          Extract second part of two element list
          Second part consists of 4 parts: BBox, Cell, TextSum, DayTrip

          */
          if(!listutils::isSymbolUndefined(nl->Second(first)))
          {
            ListExpr ok = nl->Second(first);
            if( nl->ListLength(ok) == 4)
            {

              /* Extract Bbox information */
              if (!listutils::isSymbolUndefined(nl->First(ok)))
              {
                ListExpr box = nl->First(ok);
                double mind[2];
                double maxd[2];
                if( nl->ListLength(box) == 4 &&
                    nl->IsAtom(nl->First(box)) &&
                    nl->AtomType(nl->First(box))
                    == RealType &&
                    nl->IsAtom(nl->Second(box)) &&
                    nl->AtomType(nl->Second(box))
                    == RealType &&
                    nl->IsAtom(nl->Third(box)) &&
                    nl->AtomType(nl->Third(box))
                    == RealType &&
                    nl->IsAtom(nl->Fourth(box)) &&
                    nl->AtomType(nl->Fourth(box))
                    == RealType
                  )
                {
                  mind[0] = nl->RealValue(nl->First(box));
                  mind[1] = nl->RealValue(nl->Second(box));
                  maxd[0] = nl->RealValue(nl->Third(box));
                  maxd[1] = nl->RealValue(nl->Fourth(box));

                  t.SetBoundingBox(true, mind, maxd);
                }
              }
              /*  Extract the Cell info */
              if (!listutils::isSymbolUndefined(
                nl->Second(ok)))
              {

                ListExpr first1 = nl->Empty();
                ListExpr rest1 = nl->Second(ok);
                t.SetStartCellsIdx(cellidxcounter);
                while (nl->IsEmpty(rest1) == false)
                {
                  first1 = nl->First( rest1 );
                  rest1 = nl->Rest( rest1 );
                  if( nl->ListLength(first1) == 3 &&
                      nl->IsAtom(nl->First(first1)) &&
                      nl->AtomType(nl->First(first1))
                      == IntType &&
                      nl->IsAtom(nl->Second(first1)) &&
                      nl->AtomType(nl->Second(first1))
                      == IntType &&
                      nl->IsAtom(nl->Third(first1)) &&
                      nl->AtomType(nl->Third(first1))
                      == IntType
                    )
                  {
                    Cell c(nl->IntValue(nl->First(first1)),
                           nl->IntValue(nl->Second(first1)),
                           nl->IntValue(nl->Third(first1))
                         );
                    b->AddBCell(c);
                    cellidxcounter++;
                  }
                  else
                  {
                    correct = false;
                    delete b;
                    return SetWord( Address(0) );
                  }


                } // end of while
                t.SetEndCellsIdx(cellidxcounter);
              } // end of if
              /*  Extract the Textual */
              if (!listutils::isSymbolUndefined(
                nl->Third(ok)))
              {

                ListExpr f1 = nl->Empty();
                ListExpr r1 = nl->Third(ok);
                t.SetStartSumWordsIdx(tripsumidxcounter);
                while (nl->IsEmpty(r1) == false) {
                  f1 = nl->First( r1 );
                  r1 = nl->Rest( r1 );

                  if( nl->ListLength(f1) == 3 &&
                      nl->IsAtom(nl->First(f1)) &&
                      nl->AtomType(nl->First(f1))
                      == IntType &&
                      nl->IsAtom(nl->Second(f1)) &&
                      nl->AtomType(nl->Second(f1))
                      == IntType && nl->IsAtom(nl->Third(f1))
                    )
                  {
                    bool typeString =
                    nl->AtomType(nl->Third(f1)) == StringType;

                    std::string stringValue;
                    if (typeString == true)
                    {
                      stringValue =
                      nl->StringValue(nl->Third(f1));
                    } else {
                      stringValue =
                      nl->TextValue(nl->Third(f1));
                    }

                    b->AddBWord(stringValue,
                      nl->IntValue(nl->First(f1)),
                      nl->IntValue(nl->Second(f1)));
                    tripsumidxcounter++;
                  }
                  else
                  {
                    correct = false;
                    delete b;
                    return SetWord( Address(0) );
                  }

                }
                t.SetEndSumWordsIdx(tripsumidxcounter);
              }
              // /* Extract the DayTrip */
              if (!listutils::isSymbolUndefined(
                nl->Fourth(ok)))
              {
                ListExpr f2 = nl->Empty();
                ListExpr r2 = nl->Fourth(ok);
                t.SetStartCoordsIdx(tripdataidxcounter);
                while (nl->IsEmpty(r2) == false)
                {
                  f2 = nl->First( r2 );
                  r2 = nl->Rest( r2 );

                  if( nl->ListLength(f2) == 3 &&
                      nl->IsAtom(nl->First(f2)) &&
                      nl->AtomType(nl->First(f2))
                      == RealType &&
                      nl->IsAtom(nl->Second(f2)) &&
                      nl->AtomType(nl->Second(f2))
                      == RealType &&
                      nl->IsAtom(nl->Third(f2)))
                  {

                    double x1 = nl->RealValue(nl->First(f2));
                    double y1 = nl->RealValue(nl->Second(f2));
                    Coordinate c(x1, y1);
                    b->AddBCoordinate(c);
                    bool typeString =
                    nl->AtomType(nl->Third(f2)) == StringType;
                    bool typeText =
                    nl->AtomType(nl->Third(f2)) == TextType;
                    std::string stringValue;
                    if (typeString == true)
                    {
                      stringValue =
                      nl->StringValue(nl->Third(f2));
                    }
                    if (typeText == true)
                    {
                      stringValue =
                      nl->TextValue(nl->Third(f2));
                    }

                    b->AddBSemString(stringValue);
                    tripdataidxcounter++;

                  }
                  else
                  {
                    correct = false;
                    delete b;
                    return SetWord( Address(0) );
                  }
                }
                t.SetEndCoordsIdx(tripdataidxcounter);
              }
            }
            b->AddTrip(t);

          }
        }
      }


    } // end of if n ===4
    correct = true;
    return SetWord(b);
  } // end of n != 0

  return word;
}



/*  3.11 ~ Helpher functions */

/* Functions for Batch WordList */

bool Batch::
GetBSumString(int index, std::string& rString) const
{

  bool success = false;
  int numString = batchwords.Size();
  if (index < numString)
  {
      BatchWord textData;
      success = batchwords.Get(index, &textData);
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
            success = batchwords_Flob.read(
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

bool Batch::AddBSumString(
  const std::string& stString, int id, int count)
{
  if (!stString.empty()) {
    BatchWord td;
    td.Offset = batchwords_Flob.getSize();
    td.Length = stString.length();
    td.indexId = id;
    td.count = count;

    bool success = batchwords.Append(td);
    assert(success);
    success = batchwords_Flob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    assert(success);
    return true;
  }
  return false;
}

/*
Functions for Trip WordSummary

*/

bool Batch::AddBWord(
  const std::string& stString, int id, int count)
{
  if (!stString.empty()) {
    WordST td;
    td.Offset = bwords_Flob.getSize();
    td.Length = stString.length();
    td.indexId = id;
    td.count = count;
    bool success = bwords.Append(td);
    assert(success);
    bwords_Flob.write(
      stString.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
}

bool Batch::
GetBWord(int index, std::string& rString) const
{

  bool success = false;
  int numString = bwords.Size();
  if (index < numString)
  {
      WordST textData;
      success = bwords.Get(index, &textData);

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
            success = bwords_Flob.read(
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
Functions for Trip bSemantics

*/

bool Batch::AddBSemString(const std::string& str)
{
  if (!str.empty()) {
    TextData td;
    td.Offset = bsemantics_Flob.getSize();
    td.Length = str.length();
    bool success = bsemantics.Append(td);
    assert(success);
    bsemantics_Flob.write(
      str.c_str(),
      td.Length,
      td.Offset);
    return true;
  }
  return false;
}
bool Batch::GetBSemString(int index, std::string& str) const
{
  bool success = false;
  int numString = bsemantics.Size();
  if (index < numString)
  {
      TextData textData;
      success = bsemantics.Get(index, &textData);
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
          success = bsemantics_Flob.read(
            pBuffer,
            textData.Length,
            textData.Offset);
        }
        if(success == true)
        {
          str = pBuffer;
        }
        delete [] pBuffer;
      }
  }
  return success;
}

void Batch::AddBCoordinate(const Coordinate& c)
{
  bcoordinates.Append(c);
}

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
                      Supplier s)
{

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
    for(
      const auto &word :
      tokenlist)
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

    res->AddSemString(finalstr);

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

ListExpr TypeMapMakeSemtraj2(ListExpr args)
{
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
                      Supplier s)
{


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

      res->AddSemString(finalstr);
    }
    else
    {
      res->AddSemString("");
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
extractkeywordsTM( ListExpr args )
{

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

ListExpr FilterSimTypeMap(ListExpr args)
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

class SIMData
{
  public:

    SIMData(Word& _stream1,
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
    ~SIMData()
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

int FilterSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{


   SIMData* li = (SIMData*) local.addr;
   switch(message) {
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

                  local.addr = new SIMData(
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

ListExpr FilterTTSimTypeMap(ListExpr args)
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
  ListExpr attr7 = nl->Seventh(args); // Grid
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
    return listutils::typeError(err + "(7th arg must be CellGrid2D");
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
        if (double (1-alpha) > 0)
        {
            InitializeTables();
        }
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


      double result1 = 0.0;
      double result2 = 0.0;
      double result = 0.0;
      double ts = 0.0;
      double textalpha = double (1 - alpha);
      if (textalpha > 0)
      {
        getTablesReady(*st1, *st2);
        ts = textualScore(*st1, *st2);
        result2 = double (1 - alpha ) * ts;
      }


      double dist = 0.0;
      if (alpha > 0)
      {
        dist =
        MinDistAux(*st1, *st2, wx, wy);
      }


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
      SemanticTrajectory& st2, double wx, double wy)
    {

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
      result = MinDistUtils(*Ax, *Ay, m, wx, wy);
      return result;
    }

    double MinDistUtils(
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

            minDL = MinDistUtils(*AxL, *AyL, n, wx, wy);

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

            minDR = MinDistUtils(*AxR, *AyR, m-n, wx, wy);

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
            st1.GetSemString(i, holdvalue);

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
            st2.GetSemString(i, holdvalue);
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

int FilterTTSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{

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

ListExpr FilterBBSimTypeMap(ListExpr args)
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
            double distance = 0.0;
            if (alpha > 0)
            {
              distance = r1->Distance(*r2, geoid);
            }
            double normalizedScore = 0.0;
            if (distance == 0.0) {
              normalizedScore = 1;
            }
            else
            {
              normalizedScore = 1 - (double)(distance/diag);
            }

            double result = alpha * normalizedScore * 2 + double (1-alpha) * 2;

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

int FilterBBSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{

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
                   int message, Word& local, Supplier s )
{

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

*/


ListExpr BTSimTypeMap(ListExpr args)
{

  std::string err = "stream(tuple) x  x "
               "";
  if(!nl->HasLength(args, 4))
  {
    return listutils::typeError(err);
  }
  if(!Batch::checkType(nl->First(args)))
  {
    return listutils::typeError("expecting one batch type");
  }
  if(!SemanticTrajectory::checkType(nl->Second(args)))
  {
    return listutils::typeError("expecting semanticTrajectory type");
  }
  if (!CcReal::checkType(nl->Third(args)) ||
  !Rectangle<2>::checkType(nl->Fourth(args)))
  {
      return listutils::typeError("third and fourth argument"
      "of real and rectangle");
  }

  return NList(CcReal::BasicType()).listExpr();

}

double Batch::RelevanceSumBT(Rectangle<2>& mbr,
   SemanticTrajectory& st, double alpha,
    double diag)
{

  double relevancesum = 0.0;
  double finalres = 0.0;
  int sumWords = 0;
  std::vector<std::string*>** hashTable;
  const std::vector<std::string*>* bucket;
  std::hash<std::string> str_hash;
  unsigned int bucketPos = 0;
  unsigned int bucketnum = 900;

  hashTable = new std::vector<std::string*>*[bucketnum];
  for (unsigned int i = 0; i < bucketnum; i++)
  {
    hashTable[i] = 0;
  }


  if (!IsEmptyWordList())
  {
    for (int i = 0; i < GetWordListSize(); i++)
    {

      std::string holdValue;
      bool success = GetBSumString(i, holdValue);
      assert(success);
      std::string* stn = new std::string(holdValue);
      size_t hash = str_hash(holdValue) % bucketnum;

      if (!hashTable[hash])
      {
        hashTable[hash] = new std::vector<std::string*>();
      }
      hashTable[hash]->push_back(stn);

      sumWords++;
    }
  }

  for (int i = 0; i < st.GetNumCoordinates(); i++)
  {

    double LH = 0;
    double RH = 0;
    std::string holdvalue;
    st.GetSemString(i, holdvalue);
    if (holdvalue.length() > 0 && double (1-alpha) > 0)
    {
      // Do the textual stuff;
      int numMatches = 0, count = 0, duplicate = 0;
      std::string prev_str = "";
      stringutils::StringTokenizer tokenizer(holdvalue, " ");

      while(tokenizer.hasNextToken())
      {
        std::string eval = tokenizer.nextToken();
        size_t hash = str_hash(eval) % bucketnum;
        bucket = 0;
        bucket = hashTable[hash];
        bucketPos = 0;
        // There is a match
        if (bucket)
        {
          // Find the match
          while(bucketPos < bucket->size())
          {
            std::string* p = (*bucket)[bucketPos];
            if((prev_str).compare(eval) == 0)
            {
                //This match seen before
                duplicate++;
                break;
            }
            else if ((*p).compare(eval) == 0)
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
        count++;
      }
      double textualscore = 0;
      if (!numMatches == 0)
      {
        double uniquewords = (double)
        (sumWords + count - (duplicate + numMatches));
        textualscore = (double) numMatches / uniquewords;
      }

      RH = double (1 - alpha) * textualscore;
    }
    if (alpha > 0)
    {
      double dist = getDistanceBT(mbr, st.GetCoordinate(i).x,
       st.GetCoordinate(i).y);
      double normalizedScore = 0.0;
      if (dist == 0.0)
      {
        normalizedScore  = 1;
      }
      else
      {
        normalizedScore = 1 - (double) (dist/diag);
      }
      LH = alpha * normalizedScore;
    }
    double localrel = LH + RH;

    relevancesum = relevancesum + localrel;
  }
  if (relevancesum > 0)
  {
    finalres = relevancesum / st.GetNumCoordinates();
  }

  for (unsigned int i = 0; i< bucketnum; i++)
  {
    std::vector<std::string*>* v = hashTable[i];
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

  return finalres;

}

double Batch::getDistanceBT(Rectangle<2>& mbr, double x, double y)
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

double Batch::EuclidDistRT(double x1,
    double y1,
    double x2,
    double y2)
{
  return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2));
}

int BTSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{


  Batch* b = static_cast<Batch*>( args[0].addr );
  SemanticTrajectory* st = static_cast<SemanticTrajectory*>( args[1].addr );
  CcReal * alpha = static_cast<CcReal*>(args[2].addr);
  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[3].addr);

  double alpha1 = alpha->GetValue();

  double diag = GetDiag(*rec);
  const Geoid* geoid = 0;
  Rectangle<2> bmbr = b->GetBoundingBox();
  double distance = bmbr.Distance(st->GetBoundingBox(), geoid);
  double normalizedScore = 1 - (double)(distance/diag);
  double RH = alpha1 * normalizedScore + double (1-alpha1);
  double LH = b->RelevanceSumBT(bmbr, *st, alpha1, diag);
  result = qp->ResultStorage(s);
  double answer = LH + RH;
  ((CcReal*) result.addr)->Set(answer);
  return 0;
}


ListExpr FilterBTSimTypeMap(ListExpr args)
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
  if(index1==0)
  {
    return listutils::typeError(attrname1+
                     " is not an attribute of the first stream");
  }
  int index2 = listutils::findAttribute(attrList2, attrname2,attrType1);
  if(index2==0)
  {
    return listutils::typeError(attrname2+
                     " is not an attribute of the first stream");
  }
  int index3 = listutils::findAttribute(attrList2, attrname3,attrType1);
  if(index3==0)
  {
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

          double result1 = alpha * normalizedScore + double (1-alpha);

          double result2 = 0.0;
          double sum = 0.0;
          for (int i = 0; i < st->GetNumCoordinates(); i++)
          {
            std::string holdvalue;
            st->GetSemString(i, holdvalue);

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
      result2 = double (1 - alpha) * textscore;
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

int FilterBTSimMapValue( Word* args, Word& result,
                   int message, Word& local, Supplier s )
{

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

ListExpr SimTypeMap(ListExpr args)
{

  NList type(args);
  if(type != NList(SemanticTrajectory::BasicType(),
  SemanticTrajectory::BasicType(),
  Rectangle<2>::BasicType(),
  CcReal::BasicType()))
  {
    return NList::typeError("Expecting two semantic trajectories,"
    "a rectangle<2> and a real value for the alpha");
  }
  return NList(CcReal::BasicType()).listExpr();
}

int SimMapValue(Word* args, Word& result,
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
  CcReal * alpha =
  static_cast<CcReal*>(args[3].addr);

  result = qp->ResultStorage(s);
  double diag = st1->GetDiagonal(*rec);

  double answer = st1->Similarity(*st2, diag, alpha->GetValue());
  ((CcReal*) result.addr)->Set(answer);

  return 0;
}

ListExpr BBSimTypeMap(ListExpr args)
{

  if(!Batch::checkType(nl->First(args))
  && !Batch::checkType(nl->Second(args)))
  {
    return listutils::typeError("expecting two tuples"
    "for first and second arguments");
  }
  if (!CcReal::checkType(nl->Third(args)) ||
  !Rectangle<2>::checkType(nl->Fourth(args))){
      return listutils::typeError("third and fourth argument of"
      "real and rectangle");
  }

  return NList(CcReal::BasicType()).listExpr();
}

int BBSimMapValue(Word* args, Word& result,
  int message,
  Word& local,
  Supplier s)
{

  Batch* t1 = static_cast<Batch*>( args[0].addr );
  Batch* t2 = static_cast<Batch*>( args[1].addr );
  CcReal * alpha = static_cast<CcReal*>(args[2].addr);
  Rectangle<2>* rec = static_cast<Rectangle<2>*>(args[3].addr);


  result = qp->ResultStorage(s);
  double diag = GetDiag(*rec);
  const Rectangle<2> r1 = t1->GetBoundingBox();
  const Rectangle<2> r2 = t2->GetBoundingBox();
  const Geoid* geoid = 0;
  double distance = 0.0;
  double alpha1 = alpha->GetValue();
  if (alpha1 > 0)
  {
    distance = r1.Distance(r2, geoid);
  }
  double normalizedScore = 0.0;
  if (distance == 0.0) {
    normalizedScore = 1;
  }
  else
  {
    normalizedScore = 1 - (double)(distance/diag);
  }

  double answer = alpha1 * normalizedScore * 2 + double (1-alpha1) * 2;
  ((CcReal*) result.addr)->Set(answer);

  return 0;
}

ListExpr TTSimTypeMap(ListExpr args)
{

  std::string err = "Should have 5 arguments";
  if(!nl->HasLength(args, 5))
  {
    return listutils::typeError(err);
  }

  if(!SemanticTrajectory::checkType(nl->First(args))
  && !SemanticTrajectory::checkType(nl->Second(args)))
  {
    return listutils::typeError("expecting two first arguments"
     "to first arguments to be semantictrajectory");
  }
  if (!CcReal::checkType(nl->Third(args))){
      return listutils::typeError("third argument should be a real");
  }
  if (!Rectangle<2>::checkType(nl->Fourth(args)))
  {
    return listutils::typeError("fourth argument should be a rectangle");
  }
  if(!CellGrid2D::checkType(nl->Fifth(args)))
  {
    return listutils::typeError(err + "(5th arg must be CellGrid2D");
  }

  return NList(CcReal::BasicType()).listExpr();
}

double SemanticTrajectory::textualScoreTT(SemanticTrajectory& st2)
{
  std::vector<std::string*>** hashTable;
  std::vector<std::string*>** hashTable2;
  const std::vector<std::string*>* bucket;
  std::hash<std::string> str_hash;
  unsigned int bucketnum = 100;


  hashTable = new std::vector<std::string*>*[bucketnum];
  for (unsigned int i = 0; i < bucketnum; i++)
  {
    hashTable[i] = 0;
  }

  hashTable2 = new std::vector<std::string*>*[bucketnum];
  for (unsigned int i = 0; i < bucketnum; i++)
  {
    hashTable2[i] = 0;
  }
  for (int i = 0; i < GetNumWords(); i++)
  {
   std::string holdValue = "";
   bool success = GetStringSum(i, holdValue);
   if(success)
   {

     size_t hash = str_hash(holdValue) % bucketnum;

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

     size_t hash = str_hash(holdValue) % bucketnum;

     if (!hashTable2[hash])
     {
       hashTable2[hash] = new std::vector<std::string*>();
     }
     std::string* stn = new std::string(holdValue);
     hashTable2[hash]->push_back(stn);
   }

  }
  double TSim = 0.0;
  unsigned int bucketpos = 0;
  for(int i = 0; i < GetNumCoordinates(); i++)
  {
    std::string holdvalue;
    GetSemString(i, holdvalue);

    if (holdvalue.length() > 0)
    {
      stringutils::StringTokenizer parse_st1(holdvalue, " ");
      while(parse_st1.hasNextToken())
      {
        std::string eval = parse_st1.nextToken();
        stringutils::trim(eval);
        size_t hash = str_hash(eval) % bucketnum;;
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
              TSim = TSim + ((double) 1/GetNumCoordinates());
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
    st2.GetSemString(i, holdvalue);
    if (holdvalue.length() > 0)
    {
      stringutils::StringTokenizer parse_st2(holdvalue, " ");

      while(parse_st2.hasNextToken())
      {

        std::string eval = parse_st2.nextToken();
        stringutils::trim(eval);
        size_t hash = str_hash(eval) % bucketnum;
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
  delete hashTable; delete hashTable2;
  return TSim;
}




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

  TypeConstructor batch (
    Batch::BasicType(),
    //name
    Batch::Property,
    //property function
    Batch::Out,
    Batch::In,
    //Out and In functions
    0,
    0,
    //SaveTo and RestoreFrom functions
    Batch::Create,
    Batch::Delete,
    //object creation and deletion
    Batch::Open,
    Batch::Save,
    //object open and save
    Batch::Close,
    Batch::Clone,
    //object close and clone
    Batch::Cast,
    //cast function
    Batch::SizeOfObj,
    //sizeof function
    Batch::KindCheck);
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

      AddTypeConstructor(&batch);
      batch.AssociateKind(Kind::DATA());

      // AddOperator(
      // BuildBatchInfo(),
      // BuildBatchMapValue,
      // BuildBatchTypeMap);
      //
      // AddOperator(
      //   LargerBatchInfo(),
      //   LargerBatchValueMap,
      //   LargerBatchTypeMap
      // );
      //
      // AddOperator(
      //   GetTripsInfo(),
      //   GetTripsValueMap,
      //   GetTripsTypeMap
      // );
      //
      // AddOperator(
      //   FilterBBSimInfo(),
      //   FilterBBSimMapValue,
      //   FilterBBSimTypeMap
      // );
      //
      // AddOperator(
      //   BBSimInfo(),
      //   BBSimMapValue,
      //   BBSimTypeMap
      // );
      //
      // AddOperator(
      //   FilterBTSimInfo(),
      //   FilterBTSimMapValue,
      //   FilterBTSimTypeMap
      // );
      //
      // AddOperator(
      //   BTSimInfo(),
      //   BTSimMapValue,
      //   BTSimTypeMap
      // );
      //
      // AddOperator(
      //   FilterTTSimInfo(),
      //   FilterTTSimMapValue,
      //   FilterTTSimTypeMap
      // );
      //
      // AddOperator(
      //   TTSimInfo(),
      //   TTSimMapValue,
      //   TTSimTypeMap
      // );
      //
      // AddOperator(
      //   FilterSimInfo(),
      //   FilterSimMapValue,
      //   FilterSimTypeMap);
      //
      // AddOperator(
      //   SimInfo(),
      //   SimMapValue,
      //   SimTypeMap);
      //
      // AddOperator(
      //  MakeSemTrajInfo(),
      //  MakeSemTrajMV,
      //  TypeMapMakeSemtraj);
      //
      //  AddOperator(
      //   MakeSemTrajInfo2(),
      //   MakeSemTrajMV2,
      //   TypeMapMakeSemtraj2);
      //
      // AddOperator(
      //   STBboxInfo(),
      //   STbboxMapValue,
      //   STboxTM);
      //
      // AddOperator(
      //   MakeSummariesInfo(),
      //   MakesummariesMV,
      //   MakesummariesTM );
      //
      // AddOperator(
      //   ExtractKeywordsInfo(),
      //   extractkeywordMapV,
      //   extractkeywordsTM);
      //
      // AddOperator (
      //   BatchesInfo(),
      //   BatchesVM,
      //   BatchesTM);

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
