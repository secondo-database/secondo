
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

#ifndef __SEMANTIC_TRAJECTORY_ALGEBRA_H__
#define __SEMANTIC_TRAJECTORY_ALGEBRA_H__


#include "../../Tools/Flob/DbArray.h"
// Needed for Flob Array
#include "../../Tools/Flob/Flob.h"
// Needed for flob
#include "Algebras/Rectangle/RectangleAlgebra.h"
// for bounding box
#include <iostream>
#include <string>
#include <cmath>
#include <float.h>
// This is needed for DBL_MAX
#include "StringUtils.h"
//This is needed to tokenized strings
#include "Algebras/Geoid/Geoid.h"
#include <vector>
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
  Struct for batch grouping operator batche

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
  1.6
  Struct to hold the batch summary for words
  Works in conjunction with batchwords_Flob flob
  Part of the ST component

  */



  struct BatchWord
  {
    BatchWord() {}
  /*
  Do not use this constructor.

  */

    BatchWord( int32_t id, int32_t count ):
      indexId( id), count( count )
      {}

    BatchWord(const BatchWord& w) :
    indexId( w.indexId),
    count( w.count )
    {}

    BatchWord& operator=(const BatchWord& w){
      indexId = w.indexId;
      count = w.count;
      return *this;
    }

    int32_t GetIdxId() { return indexId; }
    int32_t GetCount() { return count; }
    ~BatchWord(){}

    int32_t indexId;
    int32_t count;
    SmiSize Offset;
    SmiSize Length;

  };

  /*
  Structs used with the Batch Class

  */

  /*
  1.7
  Struct to hold the trip summary information

  */

  struct Trip
  {
    Trip() {}

    Trip( int dummy
      ):
      id(0),
      begin_idxCoords(0),
      end_idxCoords(0),
      begin_idxSumWords(0),
      end_idxSumWords(0),
      begin_idxCells(0),
      end_idxCells(0),
      bbox(0)
      {}

    Trip(const Trip& t) :
    id( t.id),
    begin_idxCoords(t.begin_idxCoords),
    end_idxCoords(t.end_idxCoords),
    begin_idxSumWords(t.begin_idxSumWords),
    end_idxSumWords(t.end_idxSumWords),
    begin_idxCells(t.begin_idxCells),
    end_idxCells(t.end_idxCells),
    bbox(t.bbox)
    {}

    Trip& operator=(const Trip& t){
      id = t.id;
      begin_idxCoords = t.begin_idxCoords;
      end_idxCoords = t.end_idxCoords;
      begin_idxSumWords = t.begin_idxSumWords;
      end_idxSumWords = t.end_idxSumWords;
      begin_idxCells = t.begin_idxCells;
      end_idxCells = t.end_idxCells;
      bbox = t.bbox;
      return *this;
    }

    int GetId() { return id; }
    void SetId(int newid) { id = newid; }

    int GetStartCoordsIdx() { return begin_idxCoords; }
    int GetStartSumWordsIdx() { return begin_idxSumWords; }
    int GetStartCellsIdx() { return begin_idxCells; }
    int GetEndCoordsIdx() { return end_idxCoords; }
    int GetEndSumWordsIdx() { return end_idxSumWords; }
    int GetEndCellsIdx() { return end_idxCells; }

    void SetStartCoordsIdx(int val) { begin_idxCoords = val; }
    void SetStartSumWordsIdx(int val) { begin_idxSumWords = val; }
    void SetStartCellsIdx(int val) { begin_idxCells = val; }
    void SetEndCoordsIdx(int val) { end_idxCoords = val; }
    void SetEndSumWordsIdx(int val) { end_idxSumWords = val; }
    void SetEndCellsIdx(int val) { end_idxCells = val;}

    const Rectangle<2> GetBoundingBox() const {
      return bbox;
    }

    void SetBoundingBox(const bool defined,
    const double *min,
    const double *max)
    {
      bbox.Set(true, min, max);
    }

    ~Trip(){}

    int id;
    int begin_idxCoords;
    int end_idxCoords;
    /* Assumed that Semantics associated to num_coords
    have the same index range */
    int begin_idxSumWords;
    int end_idxSumWords;
    int begin_idxCells;
    int end_idxCells;
    Rectangle<2> bbox;

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
        semantics.copyFrom(st.semantics);
        semantics_Flob.copyFrom(st.semantics_Flob);
        bbox = st.bbox;
        return true;
      }

      void Clear()
      {
          coordinates.clean();
          semantics.clean();
          semantics_Flob.clean();
          cells.clean();
          words.clean();
          words_Flob.clean();
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
      bool AddSemString(const std::string& stString);
      bool GetSemString(int index, std::string& rString) const;
      bool AddStringSum(const std::string& stString, int id, int count);
      void AddCoordinate( const Coordinate& p);
      void Destroy();
      Coordinate GetCoordinate( int i ) const;
      bool GetStringSum(int index, std::string& rString) const;
      int GetNumCoordinates() const {
        return coordinates.Size(); }
      int GetNumTextData() const {
        return  semantics.Size(); }
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
      /* Functions for textual summary */
      WordST GetWord( int i) const
      {
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
      FOR TTSim
      */
      double MinDistAux( SemanticTrajectory& st2, double wx, double wy);
      double MinDistUtils(DbArray<Cell>& Ax,
        DbArray<Cell>& Ay, int32_t m, double wx, double wy);
      double EuclidDist(int32_t x1,
          int32_t y1,
          int32_t x2,
          int32_t y2, double wx, double wy) const;
      double BruteForce(
      DbArray<Cell>& Ax,
      int32_t m, double wx, double wy);
      double GetCellDist(
      Cell& c1, Cell& c2, double wx, double wy);
      double textualScoreTT(SemanticTrajectory& st2);


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
      DbArray<TextData> semantics;
      /*
      Holds all characters for one semantic trajectory
      */
      Flob semantics_Flob;
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
      Flob words_Flob;
      /*
      The bounding box that encloses the SemanticTrajectory
      */
      Rectangle<2> bbox;
  };




  /*

  3 Class Batch

  */

  class Batch : public Attribute
  {

    public:

      Batch(int dummy);
      ~Batch();

      Batch(const Batch& b);
      Batch& operator=(const Batch& b);

      /*
      Functions for Attribute type
      */
      int NumOfFLOBs() const {
        return 9; }
      Flob *GetFLOB(const int i);
      int Compare(const Attribute*) const {
        return 0;}
      bool Adjacent(const Attribute*) const {
        return false;}
      Batch *Clone() const;
      size_t Sizeof() const {
        return sizeof(*this);}
      size_t HashValue() const {
        return 0;}
      void CopyFrom(const Attribute* right);
      void Destroy();

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
      string BasicType() { return "batch"; }
      static const bool checkType(const ListExpr type)
      {
        return listutils::isSymbol(type, BasicType());
      }


      void Clear()
      {
          batchwords.clean();
          batchwords_Flob.clean();
          trips.clean();
          bcells.clean();
          bwords.clean();
          bcoordinates.clean();
          bsemantics.clean();
          bsemantics_Flob.clean();
          bwords_Flob.clean();
          bcoordinates.clean();
          bsemantics.clean();
          bsemantics_Flob.clean();
          bbox.SetDefined(false);
          // batchId.SetDefined(false);
      }
      /* For Batch WordSummary */
      bool GetBSumString(int index, std::string& rString) const;
      bool AddBSumString(const std::string& stString,
        int id, int count);
      int GetWordListSize() const { return batchwords.Size(); }

      const bool IsEmptyWordList() const {
        return GetWordListSize() == 0;}

      BatchWord GetBSumWord( int i) const {
        assert( 0 <= i && i < GetWordListSize() );
        BatchWord w;
        batchwords.Get( i, &w );
        return w;
      }

      /*
      For Trip Information

      */
      void AddTrip(const Trip& t) { trips.Append(t); }

      Trip GetTrip(int i) const {
        assert(0 <= i && i < GetNumTrips());
        Trip t;
        trips.Get(i, &t);
        return t;
      }

      int GetNumTrips() const { return trips.Size(); }

      const bool IsEmptyTripList() const {
        return GetNumTrips() == 0;
      }

      /*
      For Trip Spatial Information

      */
      void AddBCell(const Cell& c) { bcells.Append(c);}
      Cell GetBCell (int i) const {
        assert(0 <= i && i < GetNumBCells());
        Cell c;
        bcells.Get(i, &c);
        return c;
      }
      int GetNumBCells() const { return bcells.Size(); }
      const bool IsEmptySpatialSum() const {
        return GetNumBCells() == 0; }

      /*
      For Trip Word Summary information

      */

      bool AddBWord(const std::string& stString,
        int id, int count);
      bool GetBWord(int index,
        std::string& rString) const;


      WordST GetBWordInfo( int i) const
      {
        assert( 0 <= i && i < GetNumBWords() );
        WordST w;
        bwords.Get( i, &w );
        return w;
      }
      int GetNumBWords() const { return bwords.Size(); }



      /* For Trip Semantics information */
      bool AddBSemString(const std::string& stString);
      bool GetBSemString(int index, std::string& rString) const;

      /* For Trip Coordinates information */
      int GetNumBCoordinates() const {
        return bcoordinates.Size(); }
      const bool IsEmptyBCoordinates() const {
        return bcoordinates.Size() == 0; }
      void AddBCoordinate(const Coordinate& c);


      Coordinate GetBCoordinate( int i) const
      {
        assert( 0 <= i && i < GetNumBCoordinates() );
        Coordinate c;
        bcoordinates.Get( i, &c );
        return c;
      }
      /* For Batch Bounding box */
      const Rectangle<2> GetBoundingBox( ) const {
        return bbox; }
      void SetBoundingBox(const bool defined,
        const double *min,
        const double *max)
      {
        bbox.Set(true, min, max);
      }

      void SetBatchId(const int val)
      {
        batchId = val;
      }
      int GetBatchId()
      {
        return batchId;
      }


      /* TODO this needs to be improved */
      const bool IsEmpty() const {
        return GetWordListSize() == 0; }
      /* TODO need to add batch word function */

      /*
      Helper functions for btsim

      */
      double RelevanceSumBT(Rectangle<2>& mbr,
        SemanticTrajectory& st,
          double alpha,
          double diag);
      double getDistanceBT(Rectangle<2>& mbr,
        double x, double y);
      double EuclidDistRT(double x1,
          double y1,
          double x2,
          double y2);


    private:
      /*
      Constructor should never be used
      */
      Batch() {}

      /*
      DbArray to hold word data
      */
      DbArray<BatchWord> batchwords;

      /*
      Flob to hold the words
      */

      Flob batchwords_Flob;

      /* DbArray to hold Trip info */

      DbArray<Trip> trips;

      /* DbArray to hold Trip Spatial Information */

      DbArray<Cell> bcells;

      /* DbArray to hold ST Trip summary */
      DbArray<WordST> bwords;

      /* Flob to hold actual string information for bwords */
      Flob bwords_Flob;

      /*
      DbArray to hold coordinate points
      */
      DbArray<Coordinate> bcoordinates;

      /*
      Holds position of each Text at each coordinate
      */
      DbArray<TextData> bsemantics;
      /*
      Holds all characters for one semantic trajectory
      */
      Flob bsemantics_Flob;

      /*
      The bounding box that encloses the batch
      */
      Rectangle<2> bbox;

      int batchId;
  };



} // end of namespace
#endif
