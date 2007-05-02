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

[1] Datatypes for ChessAlgebra

Following classes are defined in this file:\\

1 Defines and includes

*/

#ifndef CHESSTYPES_H
#define CHESSTYPES_H

#include "FTextAlgebra.h"
#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "DBArray.h"
#include "Attribute.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"
#include <iostream>
#include <string>
#include <sstream>

namespace ChessAlgebra
{
/*
2 auxiliary functions declaration

*/

/*
this constant contains the number of moves, after which the whole position is
stored - this feature is used to regard faster move calculations (see class
chessgame for details)

*/
const char POS_STORE_INTERVALL = 20;

/*
these arrays are used in EncodeAgent() and DecodeAgent()

*/
const string AGENT_NAMES[ 14 ] =
  {
    "Pawn", "Knight", "Bishop", "Rook", "Queen", "King",
    "pawn", "knight", "bishop", "rook", "queen", "king",
    "none", "undef"
  };
const string AGENT_SHORTNAMES[ 14 ] =
  {
    "P", "N", "B", "R", "Q", "K",
    "p", "n", "b", "r", "q", "k",
    "-", "x"
  };

/*
these constants correspondent to the agent id's,
which are delivered from ~EncodeAgent()~

*/
enum
{
  WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
  BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
  NONE, UNDEF
};

/*
casteling state constants

*/
enum
{
  NO_CASTELLING, KINGSIDE_CASTELLING, QUEENSIDE_CASTELLING
};

/*
constants for material array access

*/
const int WHITE_MATERIAL = 12;
const int BLACK_MATERIAL = 13;

/*
used to transfer agent strings ('Pawn', 'Knight', etc.) into char values,
which are used for internal representation

*/
inline char EncodeAgent( string agent )
{
  for ( int i = WHITE_PAWN; i < NONE; i++ )
    if ( ( agent == AGENT_NAMES[ i ] ) ||
         ( agent == AGENT_SHORTNAMES[ i ] ) )
      return i;

  // value not defined
  return UNDEF;
}

/*
calculates respective agent string from an agent-id

*/
inline string DecodeAgent( char agent )
{
  if ( ( agent < WHITE_PAWN ) || ( agent > NONE ) )
    return AGENT_NAMES[ UNDEF ];
  else
    return AGENT_NAMES[ ( int ) agent ];
}

/*
works like ~DecodeAgent~, but whith short names, e.g. 'Q' instead of 'Queen'

*/
inline string DecodeAgentShort( char agent )
{
  if ( ( agent < WHITE_PAWN ) || ( agent > NONE ) )
    return AGENT_SHORTNAMES[ UNDEF ];
  else
    return AGENT_SHORTNAMES[ ( int ) agent ];
}

/*
bounds row and file to allowed values

*/
inline void CheckBounds( char &file, char &row )
{
  if ( file < 'a' )
    file = 'a';

  if ( file > 'h' )
    file = 'h';

  if ( row < 1 )
    row = 1;

  if ( row > 8 )
    row = 8;
}

/*
encodes the position given by file and row into a char value, which contains
the file in bits 0-2 and the row in bits 3-5 (--rrrfff)

This is equivalent to values 0 (lower left corner) to 63 (upper right corner).

*/
inline char EncodePosition( char file, char row )
{
  CheckBounds( file, row );
  return ( ( row - 1 ) << 3 ) + ( file - 'a' );
}


/*
3 Class declarations

3.1 Class Material

*/
class Material : public IndexableStandardAttribute
{
public:
  Material();
  Material( char const *mat );
  Material( char wPawn, char wKnight, char wRook,
            char wBishop, char wQueen, char wKing,
            char bPawn, char bKnight, char bRook,
            char bBishop, char bQueen, char bKing );
  ~Material();

  int Compare( const Attribute* arg ) const;
  bool Adjacent( const Attribute *arg ) const;
  bool IsDefined() const;
  void SetDefined( bool defined );
  size_t Sizeof() const;
  size_t HashValue() const;
  void CopyFrom( const StandardAttribute* arg );
  Material* Clone() const;
  void WriteTo( char *dest ) const;
  void ReadFrom( const char *src );
  SmiSize SizeOfChars() const;

  int Count ( string ) const;
  /*
  ~Count~ returns number of figures for a given agent, e.g. 'K' or 'Knight' -
  lowercase equals white player, uppercase equals black player,
  'white', 'black' or 'all' counts all respective agents,
  return value of -1 indicates invalid input value

  */

  bool IsEqual ( const Material* mat );
  /*
  ~IsEqual~ returns true, if local material array contains the same values as in
  the mat array

  */

private:
  friend class Position;
  bool defined;

  char material[ 14 ];
  /*
  stores agents material (fields 0..11) and material sum for all white and
  black angents (fields 12 and 13)

  */
};

/*
3.2 Class Move

*/
class Move : public IndexableStandardAttribute
{
public:
  Move();
  Move( int moveNumber,
        string agent, string captured,
        string startFile, char startRow,
        string endFile, char endRow,
        bool check );
  Move( int moveNumber,
        char agentID, char capturedID,
        char startFile, char startRow,
        char endFile, char endRow,
        bool check );
  ~Move();

  int Compare( const Attribute* arg ) const;
  bool Adjacent( const Attribute *arg ) const;
  bool IsDefined() const;
  void SetDefined( bool defined );
  size_t Sizeof() const;
  size_t HashValue() const;
  void CopyFrom( const StandardAttribute* arg );
  Move* Clone() const;
  void WriteTo( char *dest ) const;
  void ReadFrom( const char *src );
  SmiSize SizeOfChars() const;

  int GetMoveNumber();
  /*
  returns the halfmove number of current move

  */

  string GetAgent();
  /*
  returns agent of current move

  */

  string GetCaptured();
  /*
  returns the agent, which was captured in currend move (=NONE, if no agent was
  captured)

  */

  int GetStartRow();
  /*
  returns the start row of the current move

  */

  int GetEndRow();
  /*
  returns the end row of the current move


  */

  string GetStartFile();
  /*
  returns the start file of the current move


  */

  string GetEndFile();
  /*
  returns the end file of the current move


  */

  bool IsCheck();
  /*
  returns true, if agent has offered check


  */

  bool GetCaptures();
  /*
  returns true, if agent has captured another agent

  */

private:
  friend class Chessgame;
  bool defined;
  int moveNumber;
  char agentID;
  char capturedID;
  char startFile;
  char startRow;
  char endFile;
  char endRow;
  bool check;
};

/*
3.3 Class Position

*/
class Position : public IndexableStandardAttribute
{
public:
  Position();
  Position( int moveNumber );
  ~Position();

  int Compare( const Attribute* arg ) const;
  bool Adjacent( const Attribute *arg ) const;
  bool IsDefined() const;
  void SetDefined( bool defined );
  size_t Sizeof() const;
  size_t HashValue() const;
  void CopyFrom( const StandardAttribute* arg );
  Position* Clone() const;
  void WriteTo( char *dest ) const;
  void ReadFrom( const char *src );
  SmiSize SizeOfChars() const;
  ostream& Print( ostream& os ) const;
  friend ostream& operator <<( ostream& os, const Position& pos );

  void Reset();
  /*
  creates an empty position

  */

  void StartPos();
  /*
  creates the start position

  */

  int AddAgent( string agent, char file, char row );
  /*
  adds a new agent into the gamefield

  */
  string GetAgent( char file, char row, bool shortname = false );
  /*
  returns the agent (short)name on the given position

  */

  string GetAgentShort( char file, char row );
  /*
  returns the agent shortname on the given position

  */

  bool TestField( string agent, char file, int row );
  /*
  checks if the given agent is at the specified field.

  */

  void GetMaterial( Material* result );

  void Range( Position* result, char startfile, char startrow,
              char endfile, char endrow );
  /*
  returns a partial gamefield of current position-object in result parameter

  */

  bool IsIncluded( Position * pos );
  /*
  returns true, if all figures of current position are on the same place in
  pos parameter (pos could contain more figures)

  */

  int GetMoveNumber() { return moveNumber; }
  /*
  returns the halfmove number of current move

  */

  void MakeMove( const int &startpos, const int &endpos,
                 const char &newag, const int &castelling );
  /*
  applies a move to the current position - the agent on startpos moves to
  endpos, newag is equal to agent on startpos, except a pawn was transfered into
  another agent, castelling contains the castelling state

  */

private:
  friend class Chessgame;
  char gamefield[ 64 ];
  char material[ 12 ];
  int moveNumber;
  bool defined;
};

/*
3.4 Class Chessgame

*/
struct MetainfoEntry;
struct MoveData;
class Chessgame : public IndexableStandardAttribute
{
public:
  Chessgame();
  Chessgame( int metainfoCnt, int movesCnt );
  ~Chessgame();

  int Compare( const Attribute* arg ) const;
  bool Adjacent( const Attribute *arg ) const;
  bool IsDefined() const;
  void SetDefined( bool defined );
  size_t Sizeof() const;
  size_t HashValue() const;
  void CopyFrom( const StandardAttribute* arg );
  Chessgame* Clone() const;
  void WriteTo( char *dest ) const;
  void ReadFrom( const char *src );
  SmiSize SizeOfChars() const;
  int NumOfFLOBs() const;
  FLOB *GetFLOB( const int i );

  /*
  in the following functions moveNumber starts counting from 1

  */

  void AddMetainfoEntry( string key, string value );
  /*
  adds the metainfo pair (key/value) to current game, where key should be used
  only once per game (otherwhise the pairs with key White, Black, WhiteElo,
  BlackElo, Event, Site, Date, ECO or Result will be overwritten, all other
  pairs will be stored everytime, but only the first value will be returned

  If all metainfo pairs have been added the function SortMetainfos has to be
  called, otherwhise GetMetainfo will not work correct!

  */

  void SortMetainfos();
  /*
  sorts the metainfo array using metainfo.Sort

  */

  int AddMove( char startfile, char startrow, char targetfile, char targetrow,
               string pgn );
  /*
  adds a new move to the game - pgn is the pgn notaition of the actual halfmove,
  e.g. "c4", "Qxh3" or "O-O-O"

  */

  void GetMove( Move * result, int moveNumber );
  /*
  returns the halfmove moveNumber in result

  */

  Move* GetMove( int moveNumber );
  /*
  returns the halfmove moveNumber

  */

  Position* GetPosition( int moveNumber );
  /*
  returns the positions after halfmove moveNumber

  */

  void GetPosition( Position* result, int moveNumber,
                    bool loadGamefield = true, bool getMaterial = true ) const;
  /*
  returns the position after halmove moveNumber in result - if loadGamefiled is
  false, the position will be calculated from the start position, otherwhise
  the last stored position before moveNumber will be loaded as starting point of
  the calculation (see also POS_STORE_INTERVALL) - getMaterial should be false,
  if the material is not of interest, e.g. in function getMove()

  */

  Position* GetLastPosition( bool loadGamefield = true );
  /*
  returns the position after the last added halfmove

  */

  const string GetPGN( int moveNumber ) const;
  /*
  returns the pgn notation of the halfmove moveNumber

  */

  void GetMetainfoValue( string key, STRING* result );
  /*
  searches key in metainfos and returns respective value in result, if the
  key was found - otherwhise it returns "key ... not found"

  */

  const MetainfoEntry* GetMetainfoEntry( int i ) const;
  /*
  retunrns the MetainfoEntry object number i, where 0-8 are the metainfos which
  are stored in seperate datafields (see below) - for i > 8 the (i-9)th entry
  in metainfo DBArray will be returned - if i>GetMetainfoCount() the pair
  (undef/undef) will be returned

  */


  int GetMetainfoCount() const;
  /*
  rerturns the number of stored metainfos (= metainfo.Size() + 9)

  */

  int GetMovesCount() const;
  /*
  retunrs the number of stored moves

  */

  const MoveData* GetMoveData( int i ) const;
  /*
  returns object i from movedata (= movedata for halfmove (i+1))

  */

private:
  const Position* GetPositionData( int i ) const;
  /*
  returns object i from positions DBArray

  */

  DBArray<MetainfoEntry> metainfo;
  /*
  stores the metainfos, which are not stored directly

  */

  DBArray<MoveData> moves;
  /*
  stores the moves of the actual game

  */

  DBArray<Position> positions;
  /*
  stores positions of actual game - only positions after every
  POS_STORE_INTERVALL halfmoves will be stored

  */

  /*
  the following datafields store the mostly used metainfos directly

  */
  char White[ 49 ];
  char Black[ 49 ];
  char WhiteElo[ 6 ];
  char BlackElo[ 6 ];
  char Event[ 49 ];
  char Site[ 49 ];
  char Date[ 11 ];
  char ECO[ 7 ];
  char Result[ 8 ];
  bool defined;
  /*
  Metainfo numbering:
  0 : White
  1 : Black
  2 : WhiteElo
  3 : BlackElo
  4 : Event
  5 : Site
  6 : Date
  7 : ECO
  8 : Result

  */
};

/*
3.5 Struct MetainfoEntry

*/
struct MetainfoEntry
{
  char key[ 13 ];
  char value[ 49 ];
};

/*
3.6 Class MoveData

*/
class MoveData
{
public:
  const char GetStartFile() const;
  const char GetStartRow() const;
  const char GetStartPos() const;
  char GetEndFile() const;
  const char GetEndRow() const;
  const char GetEndPos() const;
  const string GetNewAgent() const;
  const char GetNewAgentID() const;
  const bool IsCheck() const;
  const int GetCasteling() const;
  const bool OutputAgent() const;
  const bool OutputFile() const;
  const bool OutputRow() const;
  const bool EnPassant() const;

  void SetStartFile( const char &value );
  void SetStartRow( const char &value );
  void SetEndFile( const char &value );
  void SetEndRow( const char &value );
  void SetNewAgent( string &agent );
  void SetCheck( const bool &value );
  void SetCastelling( const int &value );
  void SetOutputAgent( const bool &value );
  void SetOutputFile( const bool &value );
  void SetOutputRow( const bool &value );
  void SetEnPassant( const bool &value );

private:
  friend class Chessgame;
  char startpos, endpos, newag, data;
};

/*
3.7 Class MovingChessPiece

*/

class MovingChessPiece
{
public:
  MovingChessPiece() : kind( AGENT_NAMES[ UNDEF ] ) {} // Standardconstructor
  ~MovingChessPiece();

  MovingChessPiece( string k, string file, int row, 
                    const double CreationTime , 
                    const DateTime* moveDuration );

  void extendInterval (const DateTime* exttime);
  void adjustTime (const Instant adjtime);
  void appendMove ( string newfile, int newrow, bool targetOffset );
  void applyCastelling ();
  void removePiece ();
  void centerPiece ();
  const string getActFile();
  const int getActRow();
  const string* getKind();
  bool* trueIsWhite();
  MPoint* getMPoint();
  const double getCurrentTime();
  void closeMPoint();
  void coutNewInterval( UPoint* up );
  void coutNewInterval( const UPoint* up );

private:
  string kind;
  bool isWhite, movedInLastInterval;
  string startfile;
  int startrow;
  string actfile;
  double x0, y0, x1, y1; // coordinates for the creation of UPoints
  int actrow, i, j;
  DateTime moveDuration;
  Instant startInstant;
  Instant endInstant;
  Interval<Instant> iv;
  MPoint mpoint;
  /*
  This variable contains the last UPoint added to mpoint. The Add-Method of
  MPoint (=Mapping) adds a clone of this UPoint, not the object itself. UPoint
  inherits from SpatialTemporalUnit, this from TemporalUnit, this has a local
  variable Interval<Instant> timeInterval, this Interval has two locals start
  and end of type Instant. UPoint has also two locals Point p0, p1
  */
  UPoint upoint;
};

/*
3.8 Class MovingChessPieces

*/

class MovingChessPieces
{
public:
  MovingChessPieces();
  ~MovingChessPieces();

  void realizeMove( Move* mv, const MoveData* mvdata );
  const bool isValid( int i );
  MovingChessPiece* getMovingChessPiece( int i );
  void incMPointWriteNextCount();
  int getMPointWriteNextCount();
  void setTupleTypeRemember( TupleType* tt );
  TupleType* getTupleTypeRemember();
  void closeMPoints();

private:
  MovingChessPiece* mcp [ 48 ];
  int MPointWriteNextCount, i, j;
  bool TargetOffset;
  TupleType* TupleTypeRemember;
  const DateTime *waitBeforeMoveDuration, *totalMoveDuration,
                 *waitAfterMoveDuration, *moveDuration;
  Instant startOfGame;
  Interval<Instant> iv;
  UPoint upref;  
};

/*
4 inline functions

4.1 Class Move

*/
inline int Move::GetMoveNumber()
{ return moveNumber; }

inline string Move::GetAgent()
{ return DecodeAgent( agentID ); }

inline string Move::GetCaptured()
{ return DecodeAgent( capturedID ); }

inline int Move::GetStartRow()
{ return startRow; }

inline int Move::GetEndRow()
{ return endRow; }

inline string Move::GetStartFile()
{
  return string( 1, startFile);
}

inline string Move::GetEndFile()
{
  return string( 1, endFile);
}

inline bool Move::IsCheck()
{ return check; }

inline bool Move::GetCaptures()
{ return ( capturedID != NONE ); }

// function only called once in Chessgame::GetMove(), thus the definition
// as inline function doesn't matter, although this function is relative big
inline void Position::MakeMove( const int &startpos, const int &endpos,
                                const char &newag, const int &castelling )
{
  if ( castelling )
  {
    // white king castelling?
    if ( startpos == 4 )
    {
      if ( castelling == KINGSIDE_CASTELLING )
      {
        gamefield[ 7 ] = NONE;
        gamefield[ 5 ] = WHITE_ROOK;
      }
      else
      {
        gamefield[ 0 ] = NONE;
        gamefield[ 3 ] = WHITE_ROOK;
      }
    }
    // black king castelling
    else
    {
      if ( castelling == KINGSIDE_CASTELLING )
      {
        gamefield[ 63 ] = NONE;
        gamefield[ 61 ] = BLACK_ROOK;
      }
      else
      {
        gamefield[ 56 ] = NONE;
        gamefield[ 59 ] = BLACK_ROOK;
      }
    }
  }
  else if ( ( gamefield[ endpos ] ) != NONE )
  { // an agent was chatched, adjust material array
    material[ ( int ) gamefield[ ( int ) endpos ] ] --;
  }
  else // check for en passant capture
  {
    // white en passant capture
    if ( gamefield[ startpos ] == WHITE_PAWN &&
         gamefield [ endpos - 8 ] == BLACK_PAWN &&
         startpos >= 32 && startpos <= 39 &&
         ( endpos == startpos + 7 || endpos == startpos + 9 ) )
    {
      gamefield[ endpos - 8 ] = NONE;
      material[ BLACK_PAWN ] --;
    }

    // black en passant capture
    if ( gamefield[ startpos ] == BLACK_PAWN &&
         gamefield [ endpos + 8 ] == WHITE_PAWN &&
         startpos >= 24 && startpos <= 31 &&
         ( endpos == startpos - 7 || endpos == startpos - 9 ) )
    {
      gamefield[ endpos + 8 ] = NONE;
      material[ WHITE_PAWN ] --;
    }
  }

  gamefield[ startpos ] = NONE;
  gamefield[ endpos ] = newag;
}

/*
4.2 Class Position

*/

/*
4.3 Class Chessgame

*/

inline int Chessgame::GetMetainfoCount() const
{
  // added 9 because 9 metainfo fields are stored seperate datafields
  return metainfo.Size() + 9;
}

inline int Chessgame::GetMovesCount() const
  { return moves.Size(); }

inline const MoveData* Chessgame::GetMoveData( int i ) const
{
  const MoveData * movedata;
  moves.Get( i, movedata );
  return movedata;
}

inline const Position* Chessgame::GetPositionData( int i ) const
{
  const Position * pos;
  positions.Get( i, pos );
  return pos;
}

/*
4.4 Struct MoveData

*/
inline const char MoveData::GetStartFile() const
  { return ( ( startpos & 0x07 ) + 'a' ); }

inline const char MoveData::GetStartRow() const
  { return ( ( ( startpos >> 3 ) & 0x07 ) + 1 ); }

inline const char MoveData::GetStartPos() const
  { return startpos; }

inline char MoveData::GetEndFile() const
  { return ( ( endpos & 0x07 ) + 'a' ); }

inline const char MoveData::GetEndRow() const
  { return ( ( ( endpos >> 3 ) & 0x07 ) + 1 ); }

inline const char MoveData::GetEndPos() const
  { return endpos; }

inline const string MoveData::GetNewAgent() const
  { return DecodeAgentShort( newag ); }

inline const char MoveData::GetNewAgentID() const
  { return newag; }

inline const bool MoveData::IsCheck() const
  { return ( data & 0x01 ); }

inline const int MoveData::GetCasteling() const
{
  if ( data & 0x02 )
    return KINGSIDE_CASTELLING;
  else if ( data & 0x04 )
    return QUEENSIDE_CASTELLING;
  else
    return NO_CASTELLING;
}

inline const bool MoveData::OutputAgent() const
{ return ( data & 0x08 ); }

inline const bool MoveData::OutputFile() const
  { return ( data & 0x10 ); }

inline const bool MoveData::OutputRow() const
  { return ( data & 0x20 ); }

inline const bool MoveData::EnPassant() const
  { return ( data & 0x40 ); }

inline void MoveData::SetStartFile( const char &value )
{
  char file = ( value - 'a' );
  if ( file < 0 )
    file = 0;
  if ( file > 7 )
    file = 7;

  startpos &= 0x38;
  startpos |= file;
}

inline void MoveData::SetStartRow( const char &value )
{
  char row = ( value - 1 );
  if ( row < 0 )
    row = 0;
  if ( row > 7 )
    row = 7;

  startpos &= 0x07;
  startpos |= ( row << 3 );
}

inline void MoveData::SetEndFile( const char &value )
{
  char file = ( value - 'a' );
  if ( file < 0 )
    file = 0;
  if ( file > 7 )
    file = 7;

  endpos &= 0x38;
  endpos |= file;
}

inline void MoveData::SetEndRow( const char &value )
{
  char row = ( value - 1 );
  if ( row < 0 )
    row = 0;
  if ( row > 7 )
    row = 7;

  endpos &= 0x07;
  endpos |= ( row << 3 );
}

inline void MoveData::SetNewAgent( string &agent )
{
  newag = EncodeAgent( agent );
}

inline void MoveData::SetCheck( const bool &value )
{
  if ( value )
    data |= 0x01; // set bit 0
  else
    data &= 0x1E; // reset bit 0
}

inline void MoveData::SetCastelling( const int &value )
{
  if ( value == KINGSIDE_CASTELLING )
  {
    data |= 0x02; // set bit 1
    data &= 0xFB; // reset bit 2
  }
  else if ( value == QUEENSIDE_CASTELLING )
  {
    data &= 0xFD; // reset bit 1
    data |= 0x04; // set bit 2
  }
  else
  {
    data &= 0xFD; // reset bit 1
    data &= 0xFB; // reset bit 2
  }
}

inline void MoveData::SetOutputAgent( const bool &value )
{
  if ( value )
    data |= 0x08; // set bit 3
  else
    data &= 0xF7; // reset bit 3
}

inline void MoveData::SetOutputFile( const bool &value )
{
  if ( value )
    data |= 0x10; // set bit 4
  else
    data &= 0xEF; // reset bit 4
}

inline void MoveData::SetOutputRow( const bool &value )
{
  if ( value )
    data |= 0x20; // set bit 5
  else
    data &= 0xDF; // reset bit 5
}

inline void MoveData::SetEnPassant( const bool &value )
{
  if ( value )
    data |= 0x40; // set bit 6
  else
    data &= 0xBF;  // reset bit 6
}

} // namespace ChessAlgebra
#endif // PST_TYPES_H
