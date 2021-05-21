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
