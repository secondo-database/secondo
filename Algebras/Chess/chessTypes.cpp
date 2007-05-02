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


[1] Datatypes for ChessAlgebra (implementation)

*/
#include <iostream>
#include <string>

#include "chessTypes.h"

namespace ChessAlgebra
{

/*
2 auxiliary functions implementation

*/

/*
compares two MetainfoEntry objects using strcmp on key component, which is
used for sorting metainfos DBArray in class chessgame

*/
int MetainfoEntryCmp( const void *v1, const void *v2 )
{
  MetainfoEntry * mi1 = ( MetainfoEntry* ) v1;
  MetainfoEntry* mi2 = ( MetainfoEntry* ) v2;

  return ( strcmp( mi1->key, mi2->key ) );
}

/*
3 Class implementations

3.1 Class Material

*/
Material::Material() {}
Material::~Material() {}

Material::Material( char wPawn, char wKnight, char wRook,
                    char wBishop, char wQueen, char wKing,
                    char bPawn, char bKnight, char bRook,
                    char bBishop, char bQueen, char bKing )
{
  material[ WHITE_PAWN ] = wPawn;
  material[ WHITE_KNIGHT ] = wKnight;
  material[ WHITE_ROOK ] = wRook;
  material[ WHITE_BISHOP ] = wBishop;
  material[ WHITE_QUEEN ] = wQueen;
  material[ WHITE_KING ] = wKing;
  material[ BLACK_PAWN ] = bPawn;
  material[ BLACK_KNIGHT ] = bKnight;
  material[ BLACK_ROOK ] = bRook;
  material[ BLACK_BISHOP ] = bBishop;
  material[ BLACK_QUEEN ] = bQueen;
  material[ BLACK_KING ] = bKing;
  SetDefined( true );

  // calculate white material
  char sum = 0;
  for ( int i = WHITE_PAWN; i <= WHITE_KING; i++ )
    sum += material[ i ];
  material[ WHITE_MATERIAL ] = sum;

  // calculate black material
  sum = 0;
  for ( int i = BLACK_PAWN; i <= BLACK_KING; i++ )
    sum += material[ i ];
  material[ BLACK_MATERIAL ] = sum;
}

Material::Material( char const *mat )
{
  for ( int i = WHITE_PAWN; i <= BLACK_KING; i++ )
    material[ i ] = mat[ i ];
  SetDefined( true );

  // calculate white material
  char sum = 0;
  for ( int i = WHITE_PAWN; i <= WHITE_KING; i++ )
    sum += material[ i ];
  material[ WHITE_MATERIAL ] = sum;

  // calculate black material
  sum = 0;
  for ( int i = BLACK_PAWN; i <= BLACK_KING; i++ )
    sum += material[ i ];
  material[ BLACK_MATERIAL ] = sum;
}

int Material::Compare( const Attribute* arg ) const
{
  const Material * mat = ( Material* ) arg;
  if ( !defined && !mat->defined )
    return 0;
  if ( !defined && mat->defined )
    return -1;
  if ( defined && !mat->defined )
    return 1;

  // both objects defined, compare material positions
  int cmp;

  // compare white pawns
  cmp = material[ WHITE_PAWN ] - mat->material[ WHITE_PAWN ];
  if (cmp != 0)
    return cmp;

  // compare black pawns
  cmp = material[ BLACK_PAWN ] - mat->material[ BLACK_PAWN ];
  if (cmp != 0)
    return cmp;

  // compare white knights
  cmp = material[ WHITE_KNIGHT ] - mat->material[ WHITE_KNIGHT ];
  if (cmp != 0)
    return cmp;

  // compare black knights
  cmp = material[ BLACK_KNIGHT ] - mat->material[ BLACK_KNIGHT ];
  if (cmp != 0)
    return cmp;

  // compare white bishops
  cmp = material[ WHITE_BISHOP ] - mat->material[ WHITE_BISHOP ];
  if (cmp != 0)
    return cmp;

  // compare black bishops
  cmp = material[ BLACK_BISHOP ] - mat->material[ BLACK_BISHOP ];
  if (cmp != 0)
    return cmp;

  // compare white rooks
  cmp = material[ WHITE_ROOK ] - mat->material[ WHITE_ROOK ];
  if (cmp != 0)
    return cmp;

  // compare black rooks
  cmp = material[ BLACK_ROOK ] - mat->material[ BLACK_ROOK ];
  if (cmp != 0)
    return cmp;

  // compare white queens
  cmp = material[ WHITE_QUEEN ] - mat->material[ WHITE_QUEEN ];
  if (cmp != 0)
    return cmp;

  // compare black queens
  cmp = material[ BLACK_QUEEN ] - mat->material[ BLACK_QUEEN ];
  if (cmp != 0)
    return cmp;

  // Kings should be always 1, so they are not compared.
  return 0;
}

bool Material::Adjacent( const Attribute *arg ) const
{
  //TODO ?
  return false;
}

bool Material::IsDefined() const
{
  return defined;
}

void Material::SetDefined( bool defined )
{
  this->defined = defined;
}

size_t Material::Sizeof() const
{
  return sizeof( Material );
}

size_t Material::HashValue() const
{

    return material[ BLACK_PAWN ] +
                 material[ BLACK_KNIGHT ] * 9 +
                 material[ BLACK_BISHOP ] * 27 +
                 material[ BLACK_ROOK ] * 81 +
                 material[ BLACK_QUEEN ] * 243 +
                 material[ WHITE_PAWN ] * 729 +
                 material[ WHITE_KNIGHT ] * 2187 +
                 material[ WHITE_BISHOP ] * 6561 +
                 material[ WHITE_ROOK ] * 19683 +
                 material[ WHITE_QUEEN ] * 59049;

}

void Material::CopyFrom( const StandardAttribute* arg )
{
  const Material* mat2 = (Material*) arg;
  defined = mat2->defined;
  for ( int i=0; i<14; i++ )
  {
    material[ i ] = mat2->material[ i ];
  }
}

Material* Material::Clone() const
{
  return new Material( *this );
}

void Material::WriteTo( char *dest ) const
{
  *dest++ = defined ? '1' : '0';
  for (int i=0; i < 14; i++)
    *dest++ = material[ i ];
}

void Material::ReadFrom( const char *src )
{
  defined = (*src++ == '1') ? true : false;
  for (int i=0; i < 14; i++)
    material[ i ] = *src++;
}

SmiSize Material::SizeOfChars() const
{
  return 14;
}

int Material::Count ( string type ) const
{
  // count all figures
  if ( type == "all" )
    return ( material[ WHITE_MATERIAL ] + material[ BLACK_MATERIAL ] );

  // count all white figures
  if ( type == "white" )
    return material[ WHITE_MATERIAL ];

  // count all black figures
  if ( type == "black" )
    return material[ BLACK_MATERIAL ];

  // search type string and return respective value
  for ( int i = WHITE_PAWN; i <= BLACK_KING; i++ )
    if ( type == DecodeAgent( i ) )
      return material[ i ];

  // unknown type string
  return -1;
}

bool Material::IsEqual ( const Material* mat )
{
  for ( int i = WHITE_PAWN; i <= BLACK_KING; i++ )
    if ( ( material[ i ] != mat->material[ i ] ) )
      return false;

  return true;
}

/*
3.2 Class Move

*/
Move::Move() {}
Move::~Move() {}

Move::Move( int moveNumber,
            string agent, string captured,
            string startFile, char startRow,
            string endFile, char endRow,
            bool check )
{
  char agentID = EncodeAgent( agent );
  char capturedID = EncodeAgent( captured );
  CheckBounds( startFile[ 0 ], startRow );
  CheckBounds( endFile[ 0 ], endRow );

  this->moveNumber = moveNumber;
  this->agentID = agentID;
  this->capturedID = capturedID;
  this->startFile = startFile[ 0 ];
  this->startRow = startRow;
  this->endFile = endFile[ 0 ];
  this->endRow = endRow;
  this->check = check;
  SetDefined ( true );
}

Move::Move( int moveNumber,
            char agentID, char capturedID,
            char startFile, char startRow,
            char endFile, char endRow,
            bool check )
{
  CheckBounds( startFile, startRow );
  CheckBounds( endFile, endRow );

  if ( ( agentID < 0 ) || ( agentID > 13 ) )
    agentID = UNDEF;

  if ( ( capturedID < 0 ) || ( capturedID > 13 ) )
    capturedID = UNDEF;

  this->moveNumber = moveNumber;
  this->agentID = agentID;
  this->capturedID = capturedID;
  this->startFile = startFile;
  this->startRow = startRow;
  this->endFile = endFile;
  this->endRow = endRow;
  this->check = check;
  SetDefined ( true );
}

int Move::Compare( const Attribute* arg ) const
{
  const Move * move = ( Move* ) arg;
  if ( !defined && !move->defined )
    return 0;
  if ( !defined && move->defined )
    return -1;
  if ( defined && !move->defined )
    return 1;

  // both objects defined
  int cmp;

  // compare startfile
  cmp = startFile - move->startFile;
  if (cmp != 0)
    return cmp;

  // compare startrow
  cmp = startRow - move->startRow;
  if (cmp != 0)
    return cmp;

  // compare endfile
  cmp = endFile - move->endFile;
  if (cmp != 0)
    return cmp;

  // compare endrow
  cmp = endRow - move->endRow;
  if (cmp != 0)
    return cmp;

  // compare agentID
  cmp = agentID - move->agentID;
  if (cmp != 0)
    return cmp;

  // compare check state
  if ( check && !move->check )
    return 1;
  if ( !check && move->check )
    return -1;

  // compare move numbers
  return ( moveNumber - move->moveNumber );
}

bool Move::Adjacent( const Attribute *arg ) const
{
  return false;
}

bool Move::IsDefined() const
{
  return defined;
}

void Move::SetDefined( bool defined )
{
  this->defined = defined;
}

size_t Move::Sizeof() const
{
  return sizeof( Move );
}

size_t Move::HashValue() const
{
  if (defined) {
      return moveNumber + check +
                 agentID + capturedID +
                 startFile + startRow +
                 endFile + endRow;
  } else {
    return 0;
  }
}

void Move::CopyFrom( const StandardAttribute* arg )
{
  const Move* move2 = (Move*) arg;
  defined = move2->defined;
  moveNumber = move2->moveNumber;
  agentID = move2->agentID;
  capturedID = move2->capturedID;
  startFile = move2->startFile;
  startRow = move2->startRow;
  endFile = move2->endFile;
  endRow = move2->endRow;
  check = move2->check;
}

Move* Move::Clone() const
{
  return new Move( *this );
}

void Move::WriteTo( char *dest ) const
{
  *dest++ = defined ? '1' : '0';
  *dest++ = startFile;
  *dest++ = startRow;
  *dest++ = endFile;
  *dest++ = endRow;
  *dest++ = agentID;
  *dest++ = check ? '1' : '0';
  *dest++ = capturedID;

  ostringstream tmp;
  tmp << moveNumber;
  strcpy(dest, tmp.str().c_str() );
}

void Move::ReadFrom( const char *src )
{
  defined = (*src++ == '1') ? true : false;
  startFile = *src++;
  startRow = *src++;
  endFile = *src++;
  endRow = *src++;
  agentID = *src++;
  check = (*src++ == '1') ? true : false;
  capturedID = *src++;

  istringstream tmp(src);
  tmp >> moveNumber;
}

SmiSize Move::SizeOfChars() const
{
  return ( 8 + sizeof(int) );
}

/*
3.2 Class Position

*/
Position::Position() {}
Position::~Position() {}

Position::Position( int moveNumber )
{
  this->moveNumber = moveNumber;
  for ( int i = 0; i < 64; i++ )
    gamefield[ i ] = NONE;

  for ( int i = WHITE_PAWN; i <= BLACK_KING; i++ )
    material[ i ] = 0;

  defined = true;
}

int Position::Compare( const Attribute* arg ) const
{
  const Position* pos = ( Position* ) arg;
  if ( !defined && !pos->defined )
    return 0;
  if ( !defined && pos->defined )
    return -1;
  if ( defined && !pos->defined )
    return 1;

  // both objects defined, compare material positions
  int cmp;

  // compare white pawns
  cmp = material[ WHITE_PAWN ] - pos->material[ WHITE_PAWN ];
  if (cmp != 0)
    return cmp;

  // compare black pawns
  cmp = material[ BLACK_PAWN ] - pos->material[ BLACK_PAWN ];
  if (cmp != 0)
    return cmp;

  // compare white knights
  cmp = material[ WHITE_KNIGHT ] - pos->material[ WHITE_KNIGHT ];
  if (cmp != 0)
    return cmp;

  // compare black knights
  cmp = material[ BLACK_KNIGHT ] - pos->material[ BLACK_KNIGHT ];
  if (cmp != 0)
    return cmp;

  // compare white bishops
  cmp = material[ WHITE_BISHOP ] - pos->material[ WHITE_BISHOP ];
  if (cmp != 0)
    return cmp;

  // compare black bishops
  cmp = material[ BLACK_BISHOP ] - pos->material[ BLACK_BISHOP ];
  if (cmp != 0)
    return cmp;

  // compare white rooks
  cmp = material[ WHITE_ROOK ] - pos->material[ WHITE_ROOK ];
  if (cmp != 0)
    return cmp;

  // compare black rooks
  cmp = material[ BLACK_ROOK ] - pos->material[ BLACK_ROOK ];
  if (cmp != 0)
    return cmp;

  // compare white queens
  cmp = material[ WHITE_QUEEN ] - pos->material[ WHITE_QUEEN ];
  if (cmp != 0)
    return cmp;

  // compare black queens
  cmp = material[ BLACK_QUEEN ] - pos->material[ BLACK_QUEEN ];
  if (cmp != 0)
    return cmp;

  // material is equal in both positions, compare gamefield positions
  for ( int i = 0; i < 64; i++ )
  {
    if ((gamefield[ i ] != NONE) && (pos->gamefield[ i ] == NONE))
      return 1;
    if ((gamefield[ i ] == NONE) && (pos->gamefield[ i ] != NONE))
      return -1;

    cmp = (gamefield[ i ] - pos->gamefield[ i ]);
    if (cmp != 0)
      return cmp;
  }

  // compare move numbers
  return ( moveNumber - pos->moveNumber );
}

bool Position::Adjacent( const Attribute *arg ) const
{
  //TODO ?
  return false;
}

bool Position::IsDefined() const
{
  return defined;
}

void Position::SetDefined( bool defined )
{
  this->defined = defined;
}

size_t Position::Sizeof() const
{
  return sizeof( Position );
}

size_t Position::HashValue() const
{
    size_t retval = 0;
    if (defined) {
        for(int i = 0; i < 64; i++){
            retval += gamefield[ i ];
        }
    }
    return retval;
}

void Position::CopyFrom( const StandardAttribute* arg )
{
  const Position* pos2 = (Position*) arg;
  defined = pos2->defined;
  moveNumber = pos2->moveNumber;
  for ( int i=0; i<12; i++ )
  {
    material[ i ] = pos2->material[ i ];
  }
  for ( int i=0; i<64; i++ )
  {
    gamefield[ i ] = pos2->gamefield[ i ];
  }
}

Position* Position::Clone() const
{
  return new Position ( *this );
}

void Position::WriteTo( char *dest ) const
{
  *dest++ = defined ? '1' : '0';

  for (int i=0; i < 64; i++)
    *dest++ = gamefield[ i ];

  for (int i=0; i < 12; i++)
    *dest++ = material[ i ];

  ostringstream tmp;
  tmp << moveNumber;
  strcpy(dest, tmp.str().c_str() );
}

void Position::ReadFrom( const char *src )
{
  defined = (*src++ == '1') ? true : false;

  for (int i=0; i < 64; i++)
    gamefield[ i ] = *src++;

  for (int i=0; i < 12; i++)
    material[ i ] = *src++;

  istringstream tmp(src);
  tmp >> moveNumber;
}

SmiSize Position::SizeOfChars() const
{
  return ( 77 + sizeof(int) );
}

ostream& Position::Print( ostream& os ) const
{
  return ( os << *this );
}

ostream& operator <<( ostream& os, const Position& pos )
{
  os << "Move number : " << pos.moveNumber << endl;
  for ( int i = 7; i >= 0; i-- )
  {
    for ( int j = 0; j < 8; j++ )
    {
      os << ( DecodeAgent( pos.gamefield[ ( i * 8 ) + j ] ) ) << " ";
    }
    os << endl;
  }
  os << endl << endl;
  return os;
}

void Position::Reset()
{
  for ( int i = 0; i < 64; i++ )
    gamefield[ i ] = NONE;

  for ( int i = 0; i < 12; i++ )
    material[ i ] = 0;
}

void Position::StartPos()
{
  // set empty positions
  for ( int i = 16; i < 48; i++ )
    gamefield[ i ] = NONE;

  // set white and black pawns
  for ( int i = 0; i < 8; i++ )
  {
    gamefield[ 8 + i ] = WHITE_PAWN;
    gamefield[ 48 + i ] = BLACK_PAWN;
  }

  // set white rooks, knights, bishop, king and queen
  gamefield[ 0 ] = WHITE_ROOK;
  gamefield[ 1 ] = WHITE_KNIGHT;
  gamefield[ 2 ] = WHITE_BISHOP;
  gamefield[ 3 ] = WHITE_QUEEN;
  gamefield[ 4 ] = WHITE_KING;
  gamefield[ 5 ] = WHITE_BISHOP;
  gamefield[ 6 ] = WHITE_KNIGHT;
  gamefield[ 7 ] = WHITE_ROOK;

  // set black rooks, knights, bishop, king and queen
  gamefield[ 56 ] = BLACK_ROOK;
  gamefield[ 57 ] = BLACK_KNIGHT;
  gamefield[ 58 ] = BLACK_BISHOP;
  gamefield[ 59 ] = BLACK_QUEEN;
  gamefield[ 60 ] = BLACK_KING;
  gamefield[ 61 ] = BLACK_BISHOP;
  gamefield[ 62 ] = BLACK_KNIGHT;
  gamefield[ 63 ] = BLACK_ROOK;

  // set white material
  material[ WHITE_PAWN ] = 8;
  material[ WHITE_KNIGHT ] = 2;
  material[ WHITE_BISHOP ] = 2;
  material[ WHITE_ROOK ] = 2;
  material[ WHITE_QUEEN ] = 1;
  material[ WHITE_KING ] = 1;

  // set black material
  material[ BLACK_PAWN ] = 8;
  material[ BLACK_KNIGHT ] = 2;
  material[ BLACK_BISHOP ] = 2;
  material[ BLACK_ROOK ] = 2;
  material[ BLACK_QUEEN ] = 1;
  material[ BLACK_KING ] = 1;
}

int Position::AddAgent( string agentStr, char file, char row )
{
  // encode agent position (file and row will be bound to allowed values)
  char pos = EncodePosition( file, row );

  // get agentID on agent position
  char oldAgentID = ( gamefield[ ( int ) pos ] );

  // check, if agent position is empty and return errorcode -1 otherwhise
  if ( oldAgentID == NONE )
  {
    // store agent, if agentStr is valid or return errorcode -2 otherwhise
    char agentID = ( ( EncodeAgent( agentStr ) ) );
    if ( ( agentID != NONE ) || ( agentID != UNDEF ) )
    {
      gamefield[ ( int ) pos ] = EncodeAgent( agentStr );
      material[ ( int ) EncodeAgent( agentStr ) ] ++;
      defined = true;
      return 0;
    }
    else
    {
      defined = false;
      return -2;
    }
  }
  else
  {
    defined = false;
    return -1;
  }
}

string Position::GetAgent( char file, char row, bool shortname )
{
  char pos = EncodePosition( file, row );
  if ( !shortname )
  {
    return DecodeAgent( ( int ) gamefield[ ( int ) pos ] );
  }
  else
  {
    return DecodeAgentShort( ( int ) gamefield[ ( int ) pos ] );
  }
}

string Position::GetAgentShort( char file, char row )
{
  return GetAgent( file, row, true );
}

void Position::GetMaterial( Material* result )
{
  for ( int i = WHITE_PAWN; i <= BLACK_KING; i++ )
  {
    result->material[ i ] = material[ i ];
  }

  // calculate Count("White")
  char sum = 0;
  for ( int i = WHITE_PAWN; i <= WHITE_KING; i++ )
    sum += material[ i ];
  result->material[ WHITE_MATERIAL ] = sum;

  // calculate Count("Black")
  sum = 0;
  for ( int i = BLACK_PAWN; i <= BLACK_KING; i++ )
    sum += material[ i ];
  result->material[ BLACK_MATERIAL ] = sum;

  // test of Material::WriteTo() and Material::ReadTo()
  /*
  char matStr[result->SizeOfChars()];
  result->WriteTo(&matStr[0]);
  for ( int i = 0; i < 14; i++ )
  {
    result->material[ i ] = 0;
  }
  result->defined = ( false );
  result->ReadFrom(&matStr[0]);
  */
}

bool Position::TestField ( string agentStr, char file, int row )
{
  char agentPosID = gamefield[ ( int ) EncodePosition( file, row ) ];
  char agentID = EncodeAgent( agentStr );

  if ( agentID == agentPosID )
    return true;
  else
    return false;
}

void Position::Range( Position* result, char startfile, char startrow,
                      char endfile, char endrow )
{
  // check gamefields and set them to allowed values, if they are out of bound
  CheckBounds( startfile, startrow );
  CheckBounds( endfile, endrow );
  result->Reset();

  for ( int row = startrow; row <= endrow; row++ )
  {
    for ( char file = startfile; file <= endfile; file++ )
    {
      char pos = EncodePosition( file, row );
      result->gamefield[ ( int ) pos ] = gamefield[ ( int ) pos ];
    }
  }

  result->moveNumber = moveNumber;
  result->SetDefined( true );
}

bool Position::IsIncluded( Position * pos )
{
  for ( int i = 0; i < 64; i++ )
    if ( ( gamefield[ i ] != NONE ) && 
         ( gamefield[ i ] != pos->gamefield[ i ] ) )
      return false;

  return true;
}
/*
3.4 Class Chessgame

*/
Chessgame::Chessgame() {}
Chessgame::~Chessgame() {}

Chessgame::Chessgame( int metainfoCnt, int movesCnt ) :
    metainfo( metainfoCnt ), moves( movesCnt ), positions( 0 )
{
  White[ 0 ] = '\0';
  Black[ 0 ] = '\0';
  WhiteElo[ 0 ] = '\0';
  BlackElo[ 0 ] = '\0';
  Event[ 0 ] = '\0';
  Site[ 0 ] = '\0';
  Date[ 0 ] = '\0';
  ECO[ 0 ] = '\0';
  Result[ 0 ] = '\0';
  defined = true;
}

int Chessgame::Compare( const Attribute* arg ) const
{
  const Chessgame * game= ( Chessgame* ) arg;
  if ( !defined && !game->defined )
    return 0;
  if ( !defined && game->defined )
    return -1;
  if ( defined && !game->defined )
    return 1;

  // both objects defined, compare metainfos
  int cmp;

  // compare Date
  cmp = strcmp( Date, game->Date );
  if (cmp != 0)
    return cmp;

  // compare Event
  cmp = strcmp( Event, game->Event );
  if (cmp != 0)
    return cmp;

  // compare White
  cmp = strcmp( White, game->White );
  if (cmp != 0)
    return cmp;

  // compare Black
  cmp = strcmp( Black, game->Black );
  if (cmp != 0)
    return cmp;

  // compare ECO
  cmp = strcmp( ECO, game->ECO );
  if (cmp != 0)
    return cmp;

  // compare result
  cmp = strcmp( Result, game->Result );
  if (cmp != 0)
    return cmp;

  // compare Site
  cmp = strcmp( Site, game->Site );
  if (cmp != 0)
    return cmp;

  // compare WhiteElo
  cmp = strcmp( WhiteElo, game->WhiteElo );
  if (cmp != 0)
    return cmp;

  // compare BlackElo
  cmp = strcmp( BlackElo, game->BlackElo );
  if (cmp != 0)
    return cmp;

  cmp = ( metainfo.Size() - game->metainfo.Size() );
  if (cmp != 0)
    return cmp;

  // both metainfo DBArrays are of the same size and because they are sorted,
  // equal entrys must be on the same position
  for (int i = 0; i < metainfo.Size(); i++)
  {
    const MetainfoEntry *m1 = GetMetainfoEntry( i );
    const MetainfoEntry *m2 = game->GetMetainfoEntry( i );

    // compare key
    cmp = strcmp( m1->key, m2->key );
    if (cmp != 0)
      return cmp;

    // compare value
    cmp = strcmp( m1->value, m2->value );
    if (cmp != 0)
      return cmp;
  }

  // all metainfos are eqaul at this point
  // compare move count
  cmp = ( moves.Size() - game->moves.Size() );
  if (cmp != 0)
    return cmp;

  for (int i = 0; i < moves.Size(); i++)
  {
    const MoveData *m1 = GetMoveData( i );
    const MoveData *m2 = GetMoveData( i );

    // compare start-position
    cmp = ( m1->GetStartPos() - m2->GetStartPos() );
    if (cmp != 0)
      return cmp;

    // compare end-position
    cmp = ( m1->GetEndPos() - m2->GetEndPos() );
    if (cmp != 0)
      return cmp;

    // compare new agent
    cmp = ( m1->GetNewAgentID() - m2->GetNewAgentID() );
    if (cmp != 0)
      return cmp;
  }

  // both games are equal
  return ( 0 );
}

bool Chessgame::Adjacent( const Attribute *arg ) const
{
  return false;
}

bool Chessgame::IsDefined() const
{
  return defined;
}

void Chessgame::SetDefined( bool defined )
{
  this->defined = defined;
}

size_t Chessgame::Sizeof() const
{
  return sizeof( Chessgame );
}

size_t Chessgame::HashValue() const
{
    size_t retval = 0;
    const MoveData* movedata = new MoveData();
    for(int i = 0; i < moves.Size(); i++){
        moves.Get(i, movedata);
        retval += movedata->IsCheck() +
                            movedata->GetStartFile() +
                            movedata->GetStartRow() +
                            movedata->GetEndFile() +
                            movedata->GetEndRow();
        retval %= 0x8000000;
    }
    return retval;
}

void Chessgame::CopyFrom( const StandardAttribute* arg )
{
  const Chessgame* game2 = (Chessgame*) arg;
  defined = game2->defined;
  strcpy( White, game2->White );
  strcpy( Black, game2->Black );
  strcpy( WhiteElo, game2->WhiteElo );
  strcpy( BlackElo, game2->BlackElo );
  strcpy( Event, game2->Event );
  strcpy( Site, game2->Site );
  strcpy( Date, game2->Date );
  strcpy( ECO, game2->ECO );
  strcpy( Result, game2->Result );

  metainfo.Clear();
  for ( int i = 0; i < game2->metainfo.Size(); i++ )
  {
    MetainfoEntry* newentry = new MetainfoEntry();
    const MetainfoEntry* entry = game2->GetMetainfoEntry( i + 9 );
    //   added 9 because 9 metainfo fields are stored seperate datafields
    strcpy( newentry->key, entry->key );
    strcpy( newentry->value, entry->value );
    metainfo.Append( *newentry );
  }

  moves.Clear();
  for ( int i = 0; i < game2->moves.Size(); i++ )
  {
    MoveData* newmove = new MoveData();
    const MoveData* move = game2->GetMoveData( i );
    newmove->startpos = move->startpos;
    newmove->endpos = move->endpos;
    newmove->newag = move->newag;
    newmove->data = move->data;
    moves.Append( *newmove );
  }

  positions.Clear();
  for ( int i = 0; i < game2->positions.Size(); i++ )
  {
    Position* newpos = new Position();
    newpos->CopyFrom( game2->GetPositionData( i ) );
    positions.Append( *newpos );
  }
}

Chessgame* Chessgame::Clone() const
{
  Chessgame * chessgame = new Chessgame( 0, 0 );

  strcpy( chessgame->White, White );
  strcpy( chessgame->Black, Black );
  strcpy( chessgame->WhiteElo, WhiteElo );
  strcpy( chessgame->BlackElo, BlackElo );
  strcpy( chessgame->Event, Event );
  strcpy( chessgame->Site, Site );
  strcpy( chessgame->Date, Date );
  strcpy( chessgame->ECO, ECO );
  strcpy( chessgame->Result, Result );

  for ( int i = 0; i < metainfo.Size(); i++ )
    chessgame->metainfo.Append( *this->GetMetainfoEntry( i + 9 ) );
  // added 9 because 9 metainfo fields are stored seperate datafields

  for ( int i = 0; i < moves.Size(); i++ )
    chessgame->moves.Append( *this->GetMoveData( i ) );

  for ( int i = 0; i < positions.Size(); i++ )
    chessgame->positions.Append( *this->GetPositionData( i ) );

  return chessgame;
}

void Chessgame::WriteTo( char *dest ) const
{
  *dest++ = defined ? '1' : '0';
  strncpy(dest, &White[0], 49);
  strncpy(dest, &Black[0], 49);
  strncpy(dest, &WhiteElo[0], 6);
  strncpy(dest, &BlackElo[0], 6);
  strncpy(dest, &Event[0], 49);
  strncpy(dest, &Site[0], 49);
  strncpy(dest, &Date[0], 11);
  strncpy(dest, &ECO[0], 7);
  strncpy(dest, &Result[0], 8);

  for (int i = 0; i < metainfo.Size(); i++)
  {
    const MetainfoEntry * entry;
    metainfo.Get( i , entry );
    strncpy( dest, entry->key, 13 );
    strncpy( dest, entry->value, 49 );
  }

  for (int i = 0; i < moves.Size(); i++)
  {
    const MoveData * move;
    moves.Get( i , move );
    *dest++ = move->GetStartFile();
    *dest++ = move->GetStartRow();
    *dest++ = move->GetEndFile();
    *dest++ = move->GetEndRow();
    const string pgn = this->GetPGN(i + 1);
    strncpy(dest, pgn.c_str(), 10);
  }
}

void Chessgame::ReadFrom( const char *src )
{
  defined = (*src++ == '1') ? true : false;
  strncpy(&White[0], src, 49);
  strncpy(&Black[0], src, 49);
  strncpy(&WhiteElo[0], src, 6);
  strncpy(&BlackElo[0], src, 6);
  strncpy(&Event[0], src, 49);
  strncpy(&Site[0], src, 49);
  strncpy(&Date[0], src, 11);
  strncpy(&ECO[0], src, 7);
  strncpy(&Result[0], src, 8);

  metainfo.Clear();
  for (int i = 0; i < metainfo.Size(); i++)
  {
    MetainfoEntry * entry = new MetainfoEntry();
    strncpy( entry->key, src, 13 );
    strncpy( entry->value, src, 49 );
    metainfo.Append( *entry );
  }

  positions.Clear();
  moves.Clear();
  for (int i = 0; i < moves.Size(); i++)
  {
    char startfile = *src++;
    char startrow = *src++;
    char endfile = *src++;
    char endrow = *src++;
    char pgn[10];
    strncpy(&pgn[0], src, 10);
    AddMove(startfile, startrow, endfile, endrow, string(pgn));
  }
}

SmiSize Chessgame::SizeOfChars() const
{
  return ( 234 + (metainfo.Size() * 62) * (moves.Size() * 14));
}

int Chessgame::NumOfFLOBs() const
{
  return 3;
}

FLOB * Chessgame::GetFLOB( const int i )
{
  switch ( i )
  {
  case 1:
    return & metainfo;

  case 2:
    return &moves;

  default:
    return &positions;
  }
}

void Chessgame::AddMetainfoEntry( string key, string value )
{
  int len;

  // do not store metainfos with empty keys
  if ( key.size() == 0 )
    return ;

  switch ( key[ 0 ] )
  {
  case 'W' :
    if ( key == "White" )
    {
      len = value.copy( White, 48, 0 );
      White[ len ] = '\0';
      return ;
    }
    else if ( key == "WhiteElo" )
    {
      len = value.copy( WhiteElo, 5, 0 );
      WhiteElo[ len ] = '\0';
      return ;
    }
    break;

  case 'B' :
    if ( key == "Black" )
    {
      len = value.copy( Black, 48, 0 );
      Black[ len ] = '\0';
      return ;
    }
    else if ( key == "BlackElo" )
    {
      len = value.copy( BlackElo, 5, 0 );
      BlackElo[ len ] = '\0';
      return ;
    }
    break;

  case 'E' :
    if ( key == "Event" )
    {
      len = value.copy( Event, 48, 0 );
      Event[ len ] = '\0';
      return ;
    }
    else if ( key == "ECO" )
    {
      len = value.copy( ECO, 6, 0 );
      ECO[ len ] = '\0';
      return ;
    }
    break;

  case 'R' :
    if ( key == "Result" )
    {
      len = value.copy( Result, 7, 0 );
      Result[ len ] = '\0';
      return ;
    }
    break;

  case 'S' :
    if ( key == "Site" )
    {
      len = value.copy( Site, 48, 0 );
      Site[ len ] = '\0';
      return ;
    }
    break;

  case 'D' :
    if ( key == "Date" )
    {
      len = value.copy( Date, 10, 0 );
      Date[ len ] = '\0';
      return ;
    }
    break;

  }
  // key not found, thus tag pair will be stored in DBArrays metainfo
  MetainfoEntry * entry = new MetainfoEntry();

  // store key
  len = key.copy( entry->key, 12, 0 );
  entry->key[ len ] = '\0';

  // store value
  len = value.copy( entry->value, 48, 0 );
  entry->value[ len ] = '\0';

  // store metainfo entry
  metainfo.Append( *entry );
}

void Chessgame::SortMetainfos()
{
  metainfo.Sort( &MetainfoEntryCmp );
}

int Chessgame::AddMove( char startfile, char startrow,
                        char endfile, char endrow,
                        string pgn )
{
  //   cout << endl << "pgn Notation: " << pgn << endl;
  Position * curpos = GetLastPosition( false );
  MoveData * newmove = new MoveData();
  newmove->SetStartFile( startfile );
  newmove->SetStartRow( startrow );
  newmove->SetEndFile( endfile );
  newmove->SetEndRow( endrow );
  newmove->SetCheck( pgn[ pgn.size() - 1 ] == '+' );

  // calculate castelling state
  int startpos = newmove->GetStartPos();
  int endpos = newmove->GetEndPos();
  if ( ( startpos == 4 )
       && ( endpos == 6 )
       && ( curpos->gamefield[ startpos ] == WHITE_KING ) )
  {
    newmove->SetCastelling( KINGSIDE_CASTELLING );
  }
  else if ( ( startpos == 4 )
            && ( endpos == 2 )
            && ( curpos->gamefield[ startpos ] == WHITE_KING ) )
  {
    newmove->SetCastelling( QUEENSIDE_CASTELLING );
  }
  else if ( ( startpos == 60 )
            && ( endpos == 62 )
            && ( curpos->gamefield[ startpos ] == BLACK_KING ) )
  {
    newmove->SetCastelling( KINGSIDE_CASTELLING );
  }
  else if ( ( startpos == 60 )
            && ( endpos == 58 )
            && ( curpos->gamefield[ startpos ] == BLACK_KING ) )
  {
    newmove->SetCastelling( QUEENSIDE_CASTELLING );
  }

  // White en passant capture
  if ( startpos >= 32 && startpos <= 39 &&
       ( endpos == startpos + 7 || endpos == startpos + 9 ) &&
       curpos->gamefield[ startpos ] == WHITE_PAWN && 
       curpos->gamefield [ endpos ] == NONE &&
       curpos->gamefield [ endpos - 8 ] == BLACK_PAWN )
  {
    newmove->SetEnPassant( true );
  }

  // Black en passant capture
  else if ( startpos >= 24 && startpos <= 31 &&
            ( endpos == startpos - 7 || endpos == startpos - 9 ) &&
            curpos->gamefield[ startpos ] == BLACK_PAWN && 
            curpos->gamefield [ endpos ] == NONE &&
            curpos->gamefield [ endpos + 8 ] == WHITE_PAWN )
  {
    newmove->SetEnPassant( true );
  }
  else
  {
    newmove->SetEnPassant( false );
  }

  string agent = DecodeAgentShort( curpos->gamefield[ startpos ] );
  agent[ 0 ] = toupper( agent[ 0 ] );

  if ( pgn[ 0 ] == agent[ 0 ] )
    newmove->SetOutputAgent( true );
  else
    newmove->SetOutputAgent( false );

  ostringstream endposstr;
  endposstr << endfile << ( ( int ) endrow );
  unsigned int pos = pgn.find( endposstr.str() );
  pos--;
  if ( pgn[ pos ] == 'x' )
    pos--;

  // check, if start row is contained in pgn notation
  if ( pos >= 0 )
  {
    if ( ( pgn[ pos ] >= '1' ) && ( pgn[ pos ] <= '8' ) )
    {
      newmove->SetOutputRow( true );
      pos--;
    }
    else
    {
      newmove->SetOutputRow( false );
    }
  }

  // check, if start file is contained in pgn notation
  if ( pos >= 0 )
  {
    if ( ( pgn[ pos ] >= 'a' ) && ( pgn[ pos ] <= 'h' ) )
    {
      newmove->SetOutputFile( true );
      pos--;
    }
    else
    {
      newmove->SetOutputFile( false );
    }
  }

  string newAgent;
  pos = pgn.find( "=" );
  if ( pos != string::npos )
  {
    newAgent += ( endrow == 8 ) ? pgn[ pos + 1 ] : tolower( pgn[ pos + 1 ] );
  }
  else
    newAgent = DecodeAgent( curpos->gamefield[ startpos ] );
  //cout << pgn << "," << pos << "," << newAgent << "\n";
  newmove->SetNewAgent( newAgent );

  // save move
  moves.Append( *newmove );

  // save current position after POS_STORE_INTERVALL moves
  if ( ( moves.Size() > 0 ) && ( ( moves.Size() % POS_STORE_INTERVALL ) == 0 ) )
    positions.Append( *curpos );
  return true;
}

void Chessgame::GetMove( Move * result, int moveNumber )
{
  if ( moveNumber < 1 )
    moveNumber = 1;
  else if ( moveNumber > moves.Size() + 1 )
    moveNumber = moves.Size() + 1;

  const MoveData * move;
  moves.Get( moveNumber - 1, move );

  Position* curpos = new Position();
  GetPosition( curpos, moveNumber - 1, true, false );

  // get captured id
  char captured;
  if ( !move->EnPassant() )
  {
    captured = curpos->gamefield [ (int)move->GetEndPos() ];
  }
  else
  {
    char agentID = curpos->gamefield [ (int)move->GetStartPos() ];
    if ( agentID <= WHITE_KING )
      captured = BLACK_PAWN;
    else
      captured = WHITE_PAWN;
  }

  // write data to result
  result->moveNumber = moveNumber;
  result->capturedID = ( captured );
  result->agentID = curpos->gamefield [ (int)move->GetStartPos() ];
  result->startFile = ( move->GetStartFile() );
  result->startRow = ( move->GetStartRow() );
  result->endFile = ( move->GetEndFile() );
  result->endRow = ( move->GetEndRow() );
  result->check = ( move->IsCheck() );
  result->defined = ( true );

  // test of Move::WriteTo() and Move::ReadTo()
  /*
  char moveStr[result->SizeOfChars()];
  result->WriteTo(&moveStr[0]);
  result->moveNumber = 0;
  result->agentID = 0;
  result->capturedID = 0;
  result->startFile = 'a';
  result->startRow = 0;
  result->endFile = 'a';
  result->endRow = 0;
  result->check = false;
  result->defined = ( false );
  result->ReadFrom(&moveStr[0]);
  */
}

Move* Chessgame::GetMove( int moveNumber )
{
  Move * result = new Move();
  GetMove( result, moveNumber );
  return result;
}

void Chessgame::GetPosition( Position* result, int moveNumber,
                             bool loadGamefield, bool getMaterial ) const
{
  // assertion: ( ( moveNumber >= 0 ) && ( moveNumber <= moves.Size() ) )

  result->moveNumber = moveNumber;
  int curmove;

  // load last stored gamefield bevore moveNumber or initialize a new one
  int posnum = ( int ) ( moveNumber / POS_STORE_INTERVALL );
  if ( ( posnum == 0 ) || !loadGamefield )
  {
    result->StartPos();
    curmove = 0;
  }
  else
  {
    const Position * pos = GetPositionData( posnum - 1 );
    for ( int i = 0; i < 64; i++ )
      result->gamefield[ i ] = pos->gamefield[ i ];
    curmove = pos->moveNumber;

    if ( getMaterial )
      for ( int i = 0; i < 12; i++ )
        result->material[ i ] = pos->material[ i ];
  }

  // calculate moves
  while ( curmove < moveNumber )
  {
    const MoveData * currentMove = GetMoveData( curmove++ );
    result->MakeMove(
      currentMove->GetStartPos(),
      currentMove->GetEndPos(),
      currentMove->GetNewAgentID(),
      currentMove->GetCasteling() );
  }

  result->defined = defined;

  // test of Position::WriteTo() and Position::ReadTo()
  /*
  char posStr[result->SizeOfChars()];
  result->WriteTo(&posStr[0]);
  for ( int i = 0; i < 64; i++ )
  {
    result->gamefield[ i ] = 0;
  }
  for ( int i = 0; i < 12; i++ )
  {
    result->material[ i ] = 0;
  }
  result->defined = ( false );
  result->ReadFrom(&posStr[0]);
  */
}

Position* Chessgame::GetPosition( int moveNumber )
{
  Position * result = new Position();
  GetPosition( result, moveNumber, false );
  return result;
}

Position* Chessgame::GetLastPosition( bool loadGamefield )
{
  Position * result = new Position();
  GetPosition( result, moves.Size(), loadGamefield );
  return result;
}

const string Chessgame::GetPGN( int moveNumber ) const
{
  if ( moveNumber < 1 )
    moveNumber = 1;
  else if ( moveNumber > moves.Size() + 1 )
    moveNumber = moves.Size() + 1;


  string result;
  Position* curpos = new Position();
  GetPosition( curpos, moveNumber - 1 );


  const MoveData * move;
  moves.Get( moveNumber - 1, move );

  int startpos = move->GetStartPos();
  int endpos = move->GetEndPos();

  if ( move->GetCasteling() )
  {
    if ( move->GetCasteling() == QUEENSIDE_CASTELLING )
      result = "O-O-O";
    else
      result = "O-O";
  }
  else
  {
    if ( move->OutputAgent() )
    {
      string agstr = DecodeAgentShort( curpos->gamefield[ startpos ] );
      agstr[ 0 ] = toupper( agstr[ 0 ] );
      result += agstr;
    }

    if ( move->OutputFile() )
    {
      result += move->GetStartFile();
    }

    if ( move->OutputRow() )
    {
      ostringstream row;
      row << ( int ) ( move->GetStartRow() );
      result += row.str();
    }

    if ( ( curpos->gamefield[ endpos ] & 0x07 ) != NONE )
    {
      result += 'x';
    }

    // append endfile and endrow
    result += move->GetEndFile();
    ostringstream row;
    row << ( int ) ( move->GetEndRow() );
    result += row.str();

    if ( DecodeAgentShort( curpos->gamefield[ startpos ] ) !=  
         move->GetNewAgent() )
    {
      result += '=';
      string agstr = move->GetNewAgent();
      agstr[ 0 ] = toupper( agstr[ 0 ] );
      result += agstr;
    }
  }

  if ( move->IsCheck() )
    result += '+';

  return result;
}

void Chessgame::GetMetainfoValue( string key, STRING* result )
{
  if ( key.size() == 0 )
  {
    string err = "Missing key!";
    strcpy ( *result, err.c_str() );
  }
  if ( isupper( key[ 0 ] ) )
  { // search for pgn-notation key values
    switch ( key[ 0 ] )
    {
    case 'W' :
      if ( key == "White" )
      {
        strcpy ( *result, White );
        return ;
      }
      else if ( key == "WhiteElo" )
      {
        strcpy ( *result, WhiteElo );
        return ;
      }
      break;

    case 'B' :
      if ( key == "Black" )
      {
        strcpy ( *result, Black );
        return ;
      }
      else if ( key == "BlackElo" )
      {
        strcpy ( *result, BlackElo );
        return ;
      }
      break;

    case 'E' :
      if ( key == "Event" )
      {
        strcpy ( *result, Event );
        return ;
      }
      else if ( key == "ECO" )
      {
        strcpy ( *result, ECO );
        return ;
      }
      break;

    case 'S' :
      if ( key == "Site" )
      {
        strcpy ( *result, Site );
        return ;
      }
      break;

    case 'D' :
      if ( key == "Date" )
      {
        strcpy ( *result, Date );
        return ;
      }
      break;

    case 'R' :
      if ( key == "Result" )
      {
        strcpy ( *result, Result );
        return ;
      }
      break;

    }
  }
  else
  { // search for chessgameAlgebra-notation key values
    switch ( key[ 0 ] )
    {
    case 'n' :
      if ( key == "name_w" )
      {
        strcpy ( *result, White );
        return ;
      }
      else if ( key == "name_b" )
      {
        strcpy ( *result, Black );
        return ;
      }
      break;

    case 'd' :
      if ( key == "date" )
      {
        strcpy ( *result, Date );
        return ;
      }
      break;

    case 's' :
      if ( key == "site" )
      {
        strcpy ( *result, Site );
        return ;
      }
      break;

    case 'e' :
      if ( key == "event" )
      {
        strcpy ( *result, Event );
        return ;
      }
      else if ( key == "eco_code" )
      {
        strcpy ( *result, ECO );
        return ;
      }
      break;

    case 'r' :
      if ( key == "result" )
      {
        strcpy ( *result, Result );
        return ;
      }
      else if ( key == "rating_w" )
      {
        strcpy ( *result, WhiteElo );
        return ;
      }
      else if ( key == "rating_b" )
      {
        strcpy ( *result, BlackElo );
        return ;
      }
      break;

    case 'm' :
      if ( key == "moves" )
      {
        sprintf( *result, "%d", moves.Size() );
        return ;
      }
      break;

    }
  }

  // key not found, thus tag pair will be searched in DBArrays metainfo
  int len, pos;

  // create new MetainfoEntry object as search key
  MetainfoEntry * mi_key = new MetainfoEntry();
  len = key.copy( mi_key->key, 12, 0 );
  mi_key->key[ len ] = '\0';

  // search mi_key and copy value to result STRING, if found
  if ( metainfo.Find( mi_key, &MetainfoEntryCmp, pos ) )
  {
    const MetainfoEntry * entry;
    metainfo.Get( pos , entry );
    strcpy( *result, ( entry->value ) );
  }
  else
  {
    string err = "key " + key + " not found";
    strcpy ( *result, err.c_str() );
  }
}

const MetainfoEntry* Chessgame::GetMetainfoEntry( int i ) const
{
  int len;
  string key, value;

  if ( i > GetMetainfoCount() )
  {
    MetainfoEntry * entry;
    key = "undef";
    value = "undef";
    len = key.copy( entry->key, 12, 0 );
    entry->key[ len ] = '\0';
    len = value.copy( entry->value, 48, 0 );
    entry->value[ len ] = '\0';
    return entry;
  }
  else if ( i < 9 )
  {
    MetainfoEntry * entry = new MetainfoEntry();
    switch ( i )
    {
    case 0:
      key = "White";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, White );
      break;
    case 1:
      key = "Black";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, Black );
      break;
    case 2:
      key = "WhiteElo";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, WhiteElo );
      break;
    case 3:
      key = "BlackElo";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, BlackElo );
      break;
    case 4:
      key = "Event";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, Event );
      break;
    case 5:
      key = "Site";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, Site );
      break;
    case 6:
      key = "Date";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, Date );
      break;
    case 7:
      key = "ECO";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, ECO );
      break;
    case 8:
      key = "Result";
      len = key.copy( entry->key, 12, 0 );
      entry->key[ len ] = '\0';
      strcpy ( entry->value, Result );
      break;
    }
    return entry;
  }
  else
  {
    const MetainfoEntry * entry;
    metainfo.Get( i - 9, entry );
    return entry;
  }
}

/*
3.5 Class MovingChessPiece

*/
MovingChessPiece::~MovingChessPiece()
{
  //delete moveDuration;
  //(&mpoint)->Destroy(); // calls Destroy member function of class Mapping
}
/*
this is the standard constructor used to create each piece's startposition

*/
MovingChessPiece::MovingChessPiece( string k, string file, 
                                    int row , const double CreationTime, 
                                    const DateTime* movedur ) :
    kind( k ),
    movedInLastInterval( false ),
    startrow( row ),
    moveDuration( durationtype ),
    startInstant( instanttype ),
    endInstant( instanttype ),
    iv( startInstant, endInstant, true, true ),
    mpoint( 10 ),
    upoint( iv, 0, 0, 0, 0 )
{
  isWhite = ( kind[ 0 ] == toupper( kind[ 0 ] ) );
  startfile = tolower( file[ 0 ] );
  actfile = startfile;
  actrow = row;
  mpoint.Clear();
  mpoint.StartBulkLoad();
  moveDuration.Equalize(movedur);
  startInstant.ReadFrom( CreationTime );
  endInstant = startInstant;
  upoint.timeInterval.start = startInstant;
  upoint.timeInterval.end = endInstant;
  x0 = ( double ) ( file[ 0 ] - 'a' + 1 );
  y0 = ( double ) row;
  if ( (isWhite && (startrow==8)) || 
       (!isWhite && (startrow==1)) ) // this was a pawn's promotion
  {
    x0 += -0.25;
    y0 += -0.25;
  }
  upoint.p0.Set( x0, y0 );
  x1 = x0;
  y1 = y0;
  upoint.p1.Set( x1, y1 );
  mpoint.Add( upoint );
  upoint.timeInterval.lc = false;
}

/*
extendInterval extends the interval of the newest upoint by the as exttime specified duration.
If the piece wasn't moved in the last half move, the upoint's interval is extended,
otherwise a new upoint is created

*/

void MovingChessPiece::extendInterval(const DateTime* exttime)
{
  upoint.timeInterval.start = upoint.timeInterval.end;    
  // new interval end is specified time later
  upoint.timeInterval.end.Add( exttime );
  if ( movedInLastInterval )     // if piece was moved in last interval
  {
    upoint.p0.Set( upoint.p1.GetX(), upoint.p1.GetY() );  // create new UPoint
    mpoint.Add( upoint );
  }
  else mpoint.MergeAdd( upoint ); // otherwise try to extend last UPoint
  movedInLastInterval = false;
}

/*

adjustTime extends the interval of the newest upoint to the as parameter specified instant.
If the piece wasn't moved in the last half move, the upoint's interval is extended,
otherwise a new upoint is created

*/

void MovingChessPiece::adjustTime(const Instant newEndInstant)
{
  if (upoint.timeInterval.end==newEndInstant) return;
  // former interval end is new start
  upoint.timeInterval.start = upoint.timeInterval.end;   
  // new interval end is specified time
  upoint.timeInterval.end   = newEndInstant;   
  if ( movedInLastInterval ) // if piece was moved in last interval
  {
    upoint.p0.Set( upoint.p1.GetX(), upoint.p1.GetY() ); // create new UPoint
    mpoint.Add( upoint );
  }
  else mpoint.MergeAdd( upoint );  // otherwise try to extend last UPoint
  movedInLastInterval = false;
}

/*
appendMove is the normal function that moves a piece after the normal delay time

*/

void MovingChessPiece::appendMove ( string newfile, 
                                    int newrow, bool TargetOffset )
{
   // last interval end is new interval start
  upoint.timeInterval.start = upoint.timeInterval.end;   
  upoint.timeInterval.end.Add( &moveDuration );
  upoint.p0.Set( upoint.p1.GetX(), upoint.p1.GetY() );
  upoint.p1.Set( (double) ( tolower( newfile[0] ) - 'a' + 1 ),
                            ( double ) newrow );
  if (TargetOffset) upoint.p1.Set( 0.25+upoint.p1.GetX(), 
                                   0.25+upoint.p1.GetY() );
  //cout << kind << actfile << actrow << newfile << newrow;
  mpoint.Add( upoint );
  actfile = tolower( newfile[ 0 ] );
  actrow = newrow;
  movedInLastInterval = true;
}

/*
applyCastelling moves the rook besides the king after this has moved more than one field away

*/

void MovingChessPiece::applyCastelling ()
{
  if ( startfile == "a" ) appendMove( "d", actrow , false);
  else appendMove( "f", actrow , false);
}

/*
removePiece removes a captured piece after waiting in place for
the normale delay time
and the duration of the capturing piece's move

*/

void MovingChessPiece::removePiece ()
{
  // last interval end is new interval start
  upoint.timeInterval.start = upoint.timeInterval.end;
  upoint.timeInterval.end.Add( &moveDuration );
  upoint.p0.Set( upoint.p1.GetX(), upoint.p1.GetY() );
  if ( isWhite ) y1 = -2.0;
  else y1 = 2.0;
  upoint.p1.Set( ( double ) ( startfile[ 0 ] - 'a' + 1 ),
                 ( double ) startrow + y1 );
  mpoint.Add( upoint );
  actfile = "";
  actrow = 0;
  movedInLastInterval = true;
  //cout << " x ";
}

/*
centerPiece positions the piece in the center of the field, i.e. to the
nearest integer coordinates; use this function e.g. after the removal of a captured piece

*/
void MovingChessPiece::centerPiece ()
{
  upoint.timeInterval.start = upoint.timeInterval.end; 
  upoint.timeInterval.end.Add( &moveDuration );
  upoint.p0.Set( upoint.p1.GetX(), upoint.p1.GetY() );
  upoint.p1.Set( floor(upoint.p0.GetX() + 0.5), 
                 floor(upoint.p1.GetY() + 0.5) );
  mpoint.Add( upoint );
  movedInLastInterval = true;
}

const string MovingChessPiece::getActFile()
{ return actfile;}

const int MovingChessPiece::getActRow()
{ return actrow;}

const string* MovingChessPiece::getKind()
{ return & kind;}

bool* MovingChessPiece::trueIsWhite()
{ return & isWhite;}

MPoint* MovingChessPiece::getMPoint()
{ return & mpoint;}

const double MovingChessPiece::getCurrentTime()
{ return upoint.timeInterval.end.ToDouble();}

void MovingChessPiece::closeMPoint()
{
  if ( kind == AGENT_NAMES[ UNDEF ] ) return ;
  /*
           bool result = true;
       const UPoint *lastunit, *unit;
       int k = 0, j = 0;
       for (k=0;k<i;k++)
       {
         mpoint.Get(k, unit);
         if (unit->IsValid()) coutNewInterval(unit);
         else cout << "unit " << k << " in mpoint is invalid";
       }
       if( i == 1 )
       {
         mpoint.Get( 0, unit );
         result = ( unit->IsValid() );
       }
       else for( k = 1; k < i; k++ )
       {
         mpoint.Get( k-1, lastunit );
         if( !lastunit->IsValid() )
         {
           result = false;
           j=-1;
           break;
             }
         mpoint.Get( k, unit );
         if( !unit->IsValid() )
         {
           result = false;
           break;
         }
         if( (!lastunit->Disjoint( *unit )) && 
             (!lastunit->TU_Adjacent( *unit )) )
         {
           result = false;
           j=1;
           break;
             }
          }
  */
  mpoint.EndBulkLoad( true );
}

void MovingChessPiece::coutNewInterval( UPoint* up )
{
  cout << "Interval: "
  << up->timeInterval.start.GetHour() << ":"
  << up->timeInterval.start.GetMinute() << ":"
  << up->timeInterval.start.GetSecond() << ":"
  << up->timeInterval.start.GetMillisecond() << " - "
  << up->timeInterval.end.GetHour() << ":"
  << up->timeInterval.end.GetMinute() << ":"
  << up->timeInterval.end.GetSecond() << ":"
  << up->timeInterval.end.GetMillisecond() << endl;
}

void MovingChessPiece::coutNewInterval( const UPoint* up )
{
  cout << "Interval: "
  << up->timeInterval.start.GetHour() << ":"
  << up->timeInterval.start.GetMinute() << ":"
  << up->timeInterval.start.GetSecond() << ":"
  << up->timeInterval.start.GetMillisecond() << " " << up->p0 << " - "
  << up->timeInterval.end.GetHour() << ":"
  << up->timeInterval.end.GetMinute() << ":"
  << up->timeInterval.end.GetSecond() << ":"
  << up->timeInterval.end.GetMillisecond() << " " << up->p1 << " " << endl;
}
/*
3.6 Class MovingChessPieces

*/
MovingChessPieces::MovingChessPieces() :
  startOfGame(instanttype),
  iv(startOfGame, startOfGame, true, true),
  upref(iv, 0, 0, 0, 0)
{
  waitBeforeMoveDuration = new DateTime ( 0, 2000, durationtype );
  moveDuration           = new DateTime ( 0, 1000, durationtype );
  totalMoveDuration      = new DateTime ( 0, 5000, durationtype );
  waitAfterMoveDuration  = new DateTime ( 0, 1000, durationtype );
  startOfGame.Set( 2007, 1, 1, 9, 0, 0, 0 );  // 1.1.2007 9 Uhr
  upref.timeInterval.start = startOfGame;
  upref.timeInterval.end   = startOfGame;

  mcp[ 00 ] = new MovingChessPiece( "King", "e", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration ); 
                                     // King must be first for castelling
  mcp[ 01 ] = new MovingChessPiece( "Rook", "a", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration ); 
                                    // Rooks must be next for castelling
  mcp[ 02 ] = new MovingChessPiece( "Rook", "h", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 03 ] = new MovingChessPiece( "Pawn", "a", 2 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 04 ] = new MovingChessPiece( "Pawn", "b", 2 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 05 ] = new MovingChessPiece( "Pawn", "c", 2 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 06 ] = new MovingChessPiece( "Pawn", "d", 2 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 07 ] = new MovingChessPiece( "Pawn", "e", 2 , 
                                    startOfGame.ToDouble() ,
                                    moveDuration );
  mcp[ 8 ] = new MovingChessPiece( "Pawn", "f", 2 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 9 ] = new MovingChessPiece( "Pawn", "g", 2 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 10 ] = new MovingChessPiece( "Pawn", "h", 2 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 11 ] = new MovingChessPiece( "Knight", "b", 1 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 12 ] = new MovingChessPiece( "Knight", "g", 1 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 13 ] = new MovingChessPiece( "Bishop", "c", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 14 ] = new MovingChessPiece( "Bishop", "f", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 15 ] = new MovingChessPiece( "Queen", "d", 1 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 16 ] = new MovingChessPiece( "king", "e", 8 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration ); 
                                   // king must be first for castelling
  mcp[ 17 ] = new MovingChessPiece( "rook", "a", 8 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration ); 
                                   // rooks must be next for castelling
  mcp[ 18 ] = new MovingChessPiece( "rook", "h", 8 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 19 ] = new MovingChessPiece( "pawn", "a", 7 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 20 ] = new MovingChessPiece( "pawn", "b", 7 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 21 ] = new MovingChessPiece( "pawn", "c", 7 , 
                                     startOfGame.ToDouble() ,
                                     moveDuration );
  mcp[ 22 ] = new MovingChessPiece( "pawn", "d", 7 ,
                                     startOfGame.ToDouble() ,
                                     moveDuration );
  mcp[ 23 ] = new MovingChessPiece( "pawn", "e", 7 ,
                                    startOfGame.ToDouble() ,
                                    moveDuration );
  mcp[ 24 ] = new MovingChessPiece( "pawn", "f", 7 , 
                                     startOfGame.ToDouble() , 
                                     moveDuration );
  mcp[ 25 ] = new MovingChessPiece( "pawn", "g", 7 , 
                                    startOfGame.ToDouble() , 
                                    moveDuration );
  mcp[ 26 ] = new MovingChessPiece( "pawn", "h", 7 , 
                                    startOfGame.ToDouble() ,
                                    moveDuration );
  mcp[ 27 ] = new MovingChessPiece( "knight", "b", 8 ,  
                                    startOfGame.ToDouble() ,
                                    moveDuration );
  mcp[ 28 ] = new MovingChessPiece( "knight", "g", 8 ,
                                     startOfGame.ToDouble() , 
                                      moveDuration );
  mcp[ 29 ] = new MovingChessPiece( "bishop", "c", 8 ,
                                     startOfGame.ToDouble() ,
                                     moveDuration );
  mcp[ 30 ] = new MovingChessPiece( "bishop", "f", 8 ,
                                     startOfGame.ToDouble() ,
                                     moveDuration );
  mcp[ 31 ] = new MovingChessPiece( "queen", "d", 8 , 
                                    startOfGame.ToDouble() ,
                                    moveDuration );
  for ( i = 32;i < 48;i++ ) mcp[ i ] = new MovingChessPiece();
  MPointWriteNextCount = 0;
}

MovingChessPieces::~MovingChessPieces()
{
  for ( i = 0;i < 48;i++ ) delete mcp[ i ];
}

void MovingChessPieces::realizeMove( Move* mv, const MoveData* mvdata )
{
  //cout << "MovingChessPieces::realizeMove entered" << endl;
  upref.timeInterval.end.Add(totalMoveDuration);
  for ( i = 0;i < 48;i++ )
  {
    if ( *( mcp[i]->getKind() ) == AGENT_NAMES[ UNDEF ] ) 
          continue; //this mcp wasn't used yet

    if (    mcp[i]->getActRow() == mv->GetStartRow() && 
         (mcp[i]->getActFile())[0] == tolower((mv->GetStartFile())[0]))
    { // this mcp represents the piece being moved,
      // detect if another piece was captured
      for ( j = 0;j < 48;j++ )
      {
        if ( *( mcp[j]->getKind() ) == AGENT_NAMES[ UNDEF ] )
              continue; //this mcp wasn't used yet
    if ( mcp[j]->getActRow() == mv->GetEndRow() && 
        (mcp[j]->getActFile())[0] == tolower((mv->GetEndFile())[0]) )
        {
            mcp[j]->extendInterval(waitBeforeMoveDuration);
            mcp[j]->extendInterval(moveDuration);
            mcp[j]->removePiece(); 
            // this mcp represents the piece being captured
      break;
        }
      }
      if (j<48) TargetOffset = true; 
      // another piece was captured, step besides of it
      else      TargetOffset = false;

     // next lines recognize a pawn's 
     // promotion when he reaches the other baseline
      if (   ( *( mcp[i]->getKind() ) == "Pawn" && mv->GetEndRow() == 8 )
          || ( *( mcp[i]->getKind() ) == "pawn" && mv->GetEndRow() == 1 ) )
      {
        for ( j = 32;j < 48;j++ ) { 
          if ( *(mcp[j]->getKind())==AGENT_NAMES[UNDEF] ) break; } 
          // find unused mcp
        mcp[i]->extendInterval(waitBeforeMoveDuration);
    mcp[i]->appendMove( mv->GetEndFile() , mv->GetEndRow() , TargetOffset );
    delete mcp[j];
        mcp[j] = new MovingChessPiece( 
                        AGENT_NAMES[(int)mvdata->GetNewAgentID()],
                        mv->GetEndFile(), mv->GetEndRow(), 
                        mcp[i]->getCurrentTime() , moveDuration );
        mcp[i]->removePiece(); // remove the pawn
    mcp[j]->centerPiece(); // and put the officer in the fields's center
      }
      else
      {       // move wasn't pawn's promotion, execute move
        mcp[i]->extendInterval(waitBeforeMoveDuration);
    mcp[i]->appendMove( mv->GetEndFile() , mv->GetEndRow() , TargetOffset );
    if (TargetOffset) mcp[i]->centerPiece(); // if another piece was captured

    // next lines recognize castelling 
    //if the king was moved over more than one file
        if ( ( *( mcp[i]->getKind() ) == "King" || 
               *( mcp[i] ->getKind() ) == "king" )
             && ( abs( (mv->GetEndFile())[0] - (mv->GetStartFile())[0] ) > 1 )
           )
        {   // castelling! look which rook has to move
          if ( ( mv->GetEndFile() ) [ 0 ] == 'g' ) j = 2;// white kingside rook
          else j = 1;                                   // white queenside rook
          if ( mv->GetStartRow() == 8 ) j += 16;            // black rooks
          mcp[j]->extendInterval(waitBeforeMoveDuration);
      mcp[j]->extendInterval(moveDuration);
      mcp[j]->applyCastelling();
        }
      }
    }
  }
  for ( int i = 0;i < 48;i++ )
  {
    if (! (*(mcp[i]->getKind()) == AGENT_NAMES[UNDEF]))
      mcp[i]->adjustTime(upref.timeInterval.end); 
      // for mcps representing pieces not moved
  }
}

const bool MovingChessPieces::isValid( int i )
{ return !( *( mcp[ i ] ->getKind() ) == AGENT_NAMES[ UNDEF ] ); }

MovingChessPiece* MovingChessPieces::getMovingChessPiece( int i )
{ return mcp[ i ]; }

void MovingChessPieces::incMPointWriteNextCount()
{MPointWriteNextCount++;}

int MovingChessPieces::getMPointWriteNextCount()
{ return MPointWriteNextCount;}

void MovingChessPieces::setTupleTypeRemember( TupleType* tt )
{TupleTypeRemember = tt;}

TupleType* MovingChessPieces::getTupleTypeRemember() { 
      return TupleTypeRemember;
}

void MovingChessPieces::closeMPoints()
{
  //cout << "MovingChessPieces::closeMPoints entered" << endl;
  for ( i = 0;i < 48;i++ ) mcp[ i ] ->closeMPoint(); }

} // namespace ChessAlgebra
