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


[1] chess Algebra

20070104 A. Behrendt Operator movingpoints
20070121 (stream(tuple (Piece: string IsWhite: bool Route:mpoint))) wird erzeugt.
   Rochade funktioniert, Bauernwandlung noch nicht eingebaut.
   Testspiel chess1 getestet, korrekt bis auf Bauernwandlung im letzten Zug.

1 Defines and includes

*/
using namespace std;
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <iostream>
#include <string>
#include <sstream>
#include "chessTypes.h"
#include "pgnparser.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace ChessAlgebra
{
#define readIntValue(var, list)\
  if ( nl->IsAtom( list ) && nl->AtomType( list ) == IntType )\
    var = nl->IntValue( list );\
  else\
  {\
    string liststr;\
    nl->WriteToString(liststr, list );\
    ErrorReporter::ReportError( "Error: Expected an "\
                    "integer value at " + liststr + "." );\
    correct = false;\
    return SetWord( Address( 0 ) );\
  }

#define readStringValue(var, list)\
  if ( nl->IsAtom( list ) && nl->AtomType( list ) == StringType )\
    var = nl->StringValue( list );\
  else\
  {\
    string liststr;\
    nl->WriteToString(liststr, list );\
    ErrorReporter::ReportError( "Error: Expected a"\
                           " string value at " + liststr + "." );\
    correct = false;\
    return SetWord( Address( 0 ) );\
  }

#define readBoolValue(var, list)\
  if ( nl->IsAtom( list ) && nl->AtomType( list ) == BoolType )\
    var = nl->BoolValue( list );\
  else\
  {\
    string liststr;\
    nl->WriteToString(liststr, list );\
    ErrorReporter::ReportError( "Error: Expected a"\
                              " bool value at " + liststr + "." );\
    correct = false;\
    return SetWord( Address( 0 ) );\
  }

#define returnError(errstr)\
  ErrorReporter::ReportError( errstr );\
  correct = false;\
  return SetWord( Address( 0 ) )

/*
2 Type Constructors

*/

/*
2.1 Type Constructor material

*/
ListExpr
MaterialProp()
{
  return (
           nl->TwoElemList(
             nl->FiveElemList(
               nl->StringAtom( "Signature" ),
               nl->StringAtom( "Example Type List" ),
               nl->StringAtom( "List Rep" ),
               nl->StringAtom( "Example List" ),
               nl->StringAtom( "Remarks" ) ),
             nl->FiveElemList(
               nl->StringAtom( "-> DATA" ),
               nl->StringAtom( "material" ),
               nl->StringAtom( "(int int ... int) (12 int values)" ),
               nl->StringAtom( "(8 2 2 2 1 1 8 2 2 2 1 1)" ),
               nl->StringAtom( "..." )
             )
           )
         );
}

ListExpr
OutMaterial( ListExpr typeInfo, Word value )
{
  Material * mat = ( Material* ) ( value.addr );
  ListExpr outlist;
  if ( mat->IsDefined() )
  {
    outlist = nl->Cons(
                nl->TwoElemList(
                  nl->StringAtom( DecodeAgent( BLACK_KING ) ),
                  nl->IntAtom( mat->Count( DecodeAgent( BLACK_KING ) ) )
                ),
                nl->TheEmptyList()
              );

    for ( int i = BLACK_QUEEN; i >= WHITE_PAWN; i-- )
    {
      outlist = nl->Cons(
                  nl->TwoElemList(
                    nl->StringAtom( DecodeAgent( i ) ),
                    nl->IntAtom( mat->Count( DecodeAgent( i ) ) )
                  ),
                  outlist );
    }
  }
  else
    return nl->SymbolAtom( "undef" );

  return ( outlist );
}

Word
InMaterial(
  const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr & errorInfo, bool & correct )
{
  if ( nl->ListLength( instance ) == 12 )
  {
    char agents[ 12 ];
    ListExpr current = nl->First( instance );
    ListExpr rest = nl->Rest( instance );

    if ( nl->IsAtom( current ) && nl->AtomType( current ) == IntType )
      agents[ 0 ] = nl->IntValue( current );
    else if ( ( nl->ListLength( current ) == 2 )
              && ( nl->IsAtom( nl->Second( current ) ) )
              && ( nl->AtomType( nl->Second( current ) ) ) == IntType )
      agents[ 0 ] = nl->IntValue( nl->Second( current ) );
    else
    {
      returnError(
              "Error: Wrong list format. Expected list of type\n"
              "(int ... int) or ((string int) ... (string int))." );
    }

    for ( int i = 1; i < 12; i++ )
    {
      current = nl->First( rest );
      rest = nl->Rest( rest );
      if ( nl->IsAtom( current ) && nl->AtomType( current ) == IntType )
        agents[ i ] = nl->IntValue( current );
      else if ( ( nl->ListLength( current ) == 2 )
                && ( nl->IsAtom( nl->Second( current ) ) )
                && ( nl->AtomType( nl->Second( current ) ) ) == IntType )
        agents[ i ] = nl->IntValue( nl->Second( current ) );
      else
      {
        returnError(
                "Error: Wrong list format. Expected list of type\n"
                "(int ... int) or ((string int) ... (string int)).\n"
                "(string value will be ignored)" );
      }
    }
    cout << "OK" << endl;
    correct = true;
    Material * newmat = new Material ( agents );
    return SetWord( newmat );
  }
  else
  {
    returnError(
            "Error: Wrong list format. Expect list with 12 elements of type\n"
            "int or (string int)." );
  }
}

void *
CastMaterial( void * addr )
{
  return ( new ( addr ) Material );
}

Word
CreateMaterial( const ListExpr typeInfo )
{
  return ( SetWord( new Material( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ) ) );
}

void
DeleteMaterial( const ListExpr typeInfo, Word & w )
{
  ( ( Material * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseMaterial( const ListExpr typeInfo, Word & w )
{
  ( ( Material * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneMaterial( const ListExpr typeInfo, const Word & w )
{
  return SetWord( ( ( Material * ) w.addr ) ->Clone() );
}

int
SizeOfMaterial()
{
  return sizeof( Material );
}

bool
MaterialType( ListExpr type, ListExpr & errorInfo )
{
  return ( nl->IsEqual( type, "material" ) );
}



/*
2.2 Type Constructor move

*/
ListExpr
MoveProp()
{
  return (
           nl->TwoElemList(
             nl->FiveElemList(
               nl->StringAtom( "Signature" ),
               nl->StringAtom( "Example Type List" ),
               nl->StringAtom( "List Rep" ),
               nl->StringAtom( "Example List" ),
               nl->StringAtom( "Remarks" ) ),
             nl->FiveElemList(
               nl->StringAtom( "-> DATA" ),
               nl->StringAtom( "(move)" ),
               nl->StringAtom( "(int string string string" 
                                " int string int bool)" ),
               nl->StringAtom( "(3 \"Pawn\" \"none\" \"b\" 2 \"b\" 4 FALSE)" ),
               nl->StringAtom( "move, agent, capt. startpos, endpos, check" )
             )
           )
         );
}

ListExpr
OutMove( ListExpr typeInfo, Word value )
{
  Move * move = ( Move* ) ( value.addr );
  ListExpr outlist;
  if ( move->IsDefined() )
  {
    outlist = nl->Cons( nl->BoolAtom( move->IsCheck() ), nl->TheEmptyList() );
    outlist = nl->Cons( nl->IntAtom( move->GetEndRow() ), outlist );
    outlist = nl->Cons( nl->StringAtom( move->GetEndFile() ), outlist );
    outlist = nl->Cons( nl->IntAtom( move->GetStartRow() ), outlist );
    outlist = nl->Cons( nl->StringAtom( move->GetStartFile() ), outlist );
    outlist = nl->Cons( nl->StringAtom( move->GetCaptured() ), outlist );
    outlist = nl->Cons( nl->StringAtom( move->GetAgent() ), outlist );
    outlist = nl->Cons( nl->IntAtom( move->GetMoveNumber() ), outlist );
  }
  else
    return nl->SymbolAtom( "undef" );

  return ( outlist );
}

Word
InMove(
  const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr & errorInfo, bool & correct )
{
  cout << "InMove..." << endl;
  if ( nl->ListLength( instance ) == 8 )
  {
    int moveNumber, startRow, endRow;
    string agent, captured, startFile, endFile;
    bool check;

    ListExpr current = nl->First( instance );
    ListExpr rest = nl->Rest( instance );
    readIntValue( moveNumber, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readStringValue( agent, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readStringValue( captured, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readStringValue( startFile, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readIntValue( startRow, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readStringValue( endFile, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readIntValue( endRow, current )

    current = nl->First( rest );
    rest = nl->Rest( rest );
    readBoolValue( check, current )

    correct = true;
    Move * newmove = new Move ( moveNumber, agent, captured,
                                startFile, startRow, endFile, endRow,
                                check );
    return SetWord( newmove );
  }
  else
  {
    returnError(
            "Error: Wrong list format. Expected list of type\n"
            "(int string string string int string int bool)." );
  }
}

void *
CastMove( void * addr )
{
  return ( new ( addr ) Move );
}

Word
CreateMove( const ListExpr typeInfo )
{
  return ( SetWord( new Move( 0, "none", "none", "a", 1, "a", 1, false ) ) );
}

void
DeleteMove( const ListExpr typeInfo, Word & w )
{
  ( ( Move * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseMove( const ListExpr typeInfo, Word & w )
{
  ( ( Move * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneMove( const ListExpr typeInfo, const Word & w )
{
  return SetWord( ( ( Move * ) w.addr ) ->Clone() );
}

int
SizeOfMove()
{
  return sizeof( Move );
}

bool
MoveType( ListExpr type, ListExpr & errorInfo )
{
  return ( nl->IsEqual( type, "move" ) );
}



/*
2.3 Type Constructor position

*/
ListExpr
PositionProp()
{
  return (
           nl->TwoElemList(
             nl->FiveElemList(
               nl->StringAtom( "Signature" ),
               nl->StringAtom( "Example Type List" ),
               nl->StringAtom( "List Rep" ),
               nl->StringAtom( "Example List" ),
               nl->StringAtom( "Remarks" ) ),
             nl->FiveElemList(
               nl->StringAtom( "-> DATA" ),
               nl->StringAtom( "position" ),
               nl->StringAtom( "(int ((string string int)...))" ),
               nl->StringAtom( "(43 ((\"King\" \"b\" 5)(\"king\" \"h\" 7)))" ),
               nl->StringAtom( "..." )
             )
           )
         );
}
ListExpr
OutPosition( ListExpr typeInfo, Word value )
{
  Position * pos = ( Position* ) ( value.addr );
  ListExpr outlist;
  if ( pos->IsDefined() )
  {
    int moveNumber = pos->GetMoveNumber();
    ListExpr agentList, agentRow;

    bool firstRow = true;
    agentList = nl->TheEmptyList();
    for ( int row = 1; row <= 8; row++ )
    {
      bool firstAgent = true;
      agentRow = nl->TheEmptyList();
      for ( char file = 'h'; file >= 'a'; file-- )
      {
        if ( firstAgent )
        {
          agentRow = nl->Cons(
                       nl->StringAtom( pos->GetAgentShort( file, row ) ) ,
                       nl->TheEmptyList() );
          firstAgent = false;
        }
        else
        {
          agentRow = nl->Cons(
                       nl->StringAtom( pos->GetAgentShort( file, row ) ) ,
                       agentRow );
        }
      }
      if ( firstRow )
      {
        agentList = nl->Cons( agentRow , nl->TheEmptyList() );
        firstRow = false;
      }
      else
      {
        agentList = nl->Cons( agentRow , agentList );
      }
    }
    outlist = nl->TwoElemList( nl->IntAtom( moveNumber ), agentList );

  }
  else
    return nl->SymbolAtom( "undef" );

  return ( outlist );
}

Word
InPosition(
  const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr & errorInfo, bool & correct )
{
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first, second, current;
    string agent;
    int moveNumber, row;

    first = nl->First( instance );
    second = nl->Second( instance );

    readIntValue( moveNumber, first )
    Position * newpos = new Position ( moveNumber );

    ListExpr rowList;
    if ( nl->ListLength( second ) == 8 )
    {
      int row = 8;
      while ( !nl->IsEmpty( second ) )
      {
        char file = 'a';
        rowList = nl->First( second );
        second = nl->Rest( second );
        if ( nl->ListLength( rowList ) == 8 )
        {
          while ( !nl->IsEmpty( rowList ) )
          {
            current = nl->First( rowList );
            rowList = nl->Rest( rowList );
            readStringValue( agent, current )
            int err = 0;
            if ( agent != DecodeAgentShort( NONE ) )
              err = newpos->AddAgent( agent, file, row );
            if ( err == -2 )
            {
              ostringstream errmsg;
              errmsg << "Error: Agent type '" << agent << "' not found.";
              returnError( errmsg.str() );
            }
            file++;
          }
          row--;
        }
        else
        {
          returnError(
                  "Error: Wrong list format. Expected list of type\n"
                  "(int ((string ... string)...(string ... string))),\n"
                  "which defines the agents on all 64 squares." );
        }
      }
    }
    else
    {
      string file;
      while ( !nl->IsEmpty( second ) )
      {
        current = nl->First( second );
        second = nl->Rest( second );
        if ( nl->ListLength( current ) == 3 )
        {
          readStringValue( agent, nl->First( current ) )
          readStringValue( file, nl->Second( current ) )
          readIntValue( row, nl->Third( current ) )
          int err = newpos->AddAgent( agent, file[ 0 ], row );
          if ( err )
          {
            ostringstream errmsg;
            if ( err == -1 )
            {
              errmsg << "Error: Can't store agent '"
              << agent
              << "' at square " << file << row
              << ", square already contains an agent of type '"
              << newpos->GetAgent( file[ 0 ], row )
              << "'.";
            }
            else if ( err == -2 )
            {
              errmsg << "Error: Agent type '" << agent << "' not found.";
            }
            returnError( errmsg.str() );
          }
        }
        else
        {
          returnError(
                  "Error: Wrong list format. Expected list of type\n"
                  "(int ((string string int)...(string string int)))." );
        }
      }
    }

    correct = true;
    return SetWord( newpos );
  }
  else
  {
    returnError(
            "Error: Wrong list format. Expected list of type\n"
            "(int ((string string int)...(string string int))) or\n"
            "(int ((string ... string)...(string ... string)))." );
  }
}

void *
CastPosition( void * addr )
{
  return ( new ( addr ) Position );
}

Word
CreatePosition( const ListExpr typeInfo )
{
  return ( SetWord( new Position( 0 ) ) );
}

void
DeletePosition( const ListExpr typeInfo, Word & w )
{
  ( ( Position * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

void
ClosePosition( const ListExpr typeInfo, Word & w )
{
  ( ( Position * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

Word
ClonePosition( const ListExpr typeInfo, const Word & w )
{
  return SetWord( ( ( Position * ) w.addr ) ->Clone() );
}

int
SizeOfPosition()
{
  return sizeof( Position );
}

bool
PositionType( ListExpr type, ListExpr & errorInfo )
{
  return ( nl->IsEqual( type, "position" ) );
}



/*
2.4 Type Constructor Chessgame

*/
ListExpr
ChessgameProp()
{
  return (
           nl->TwoElemList(
             nl->FiveElemList(
               nl->StringAtom( "Signature" ),
               nl->StringAtom( "Example Type List" ),
               nl->StringAtom( "List Rep" ),
               nl->StringAtom( "Example List" ),
               nl->StringAtom( "Remarks" ) ),
             nl->FiveElemList(
               nl->StringAtom( "-> DATA" ),
               nl->StringAtom( "( chessgame ) " ),
               nl->StringAtom( "() " ),
               nl->StringAtom( "() " ),
               nl->StringAtom( "" )
             )
           )
         );
}

ListExpr
OutChessgame( ListExpr typeInfo, Word value )
{
  Chessgame * game = ( Chessgame* ) ( value.addr );
  ListExpr outlist;

  if ( game->IsDefined() )
  {
    ListExpr metainfoList, movesList;

    // build metainfoList;
    if ( !game->GetMetainfoCount() )
      metainfoList = nl->TheEmptyList();
    else
    {
      bool firstListEntry = true;
      for ( int i = ( ( game->GetMetainfoCount() ) - 1 ); i >= 0; i-- )
      {
        const MetainfoEntry* metainfo = game->GetMetainfoEntry( i );

        /* do not print EventDate and ECO if respective values are empty. Date,
           Event, White, Black, Site, Round and Result (STR-Tags) will always be
           printed, even if they are originaly not contained in the pgn-file.
           All other tags will only be printed, if they are contained in the
           respective pgn file
        */
        string key ( metainfo->key );
        if ( ( metainfo->value[ 0 ] != '\0' ) ||
             ( ( key != "EventDate" ) &&
               ( key != "ECO" ) )
           )
        {
          if ( firstListEntry )
          {
            metainfoList = nl->Cons( nl->TwoElemList(
                                       nl->StringAtom( metainfo->key ),
                                       nl->StringAtom( metainfo->value )
                                     ), nl->TheEmptyList() );
            firstListEntry = false;
          }
          else
          {
            metainfoList = nl->Cons( nl->TwoElemList(
                                       nl->StringAtom( metainfo->key ),
                                       nl->StringAtom( metainfo->value )
                                     ), metainfoList );
          }
        }
      }
    }

    // build movesList;
    if ( game->GetMovesCount() == 0 )
      movesList = nl->TheEmptyList();
    else
    {
      bool firstListEntry = true;
      for ( int i = ( game->GetMovesCount() ); i > 0; i-- )
      {
        ListExpr movelist;
        Move* move = new Move();
        game->GetMove( move, i );

        // build movelist
        movelist = nl->Cons( nl->StringAtom( game->GetPGN( i ) ), 
                             nl->TheEmptyList() );
        movelist = nl->Cons( nl->BoolAtom( move->IsCheck() ), movelist );
        movelist = nl->Cons( nl->IntAtom( move->GetEndRow() ), movelist );
        movelist = nl->Cons( nl->StringAtom( move->GetEndFile() ), movelist );
        movelist = nl->Cons( nl->IntAtom( move->GetStartRow() ), movelist );
        movelist = nl->Cons( nl->StringAtom( move->GetStartFile() ), movelist);
        movelist = nl->Cons( nl->StringAtom( move->GetCaptured() ), movelist );
        movelist = nl->Cons( nl->StringAtom( move->GetAgent() ), movelist );

        if ( firstListEntry )
        {
          movesList = nl->Cons( movelist, nl->TheEmptyList() );
          firstListEntry = false;
        }
        else
        {
          movesList = nl->Cons( movelist, movesList );
        }
      }
    }

    outlist = nl->TwoElemList( metainfoList, movesList );
  }
  else
    return nl->SymbolAtom( "undef" );

  return ( outlist );
}

Word
InChessgame(
  const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr & errorInfo, bool & correct )
{
  Chessgame * game = new Chessgame( 0, 0 );
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr current, rest;

    // read metainfos
    rest = nl->First( instance );
    while ( !nl->IsEmpty( rest ) )
    {
      current = nl->First( rest );
      rest = nl->Rest( rest );

      if ( nl->ListLength( current ) == 2 )
      {
        string key, value;
        readStringValue( key, nl->First( current ) )
        readStringValue( value, nl->Second( current ) )

        // add metainfo entry
        game->AddMetainfoEntry( key, value );
      }
      else
      {
        returnError(
                "Error: Wrong list format. Expected list of type\n"
                "((string string)...(string string))." );
      }
    }

    // read moves
    rest = nl->Second( instance );
    while ( !nl->IsEmpty( rest ) )
    {
      current = nl->First( rest );
      rest = nl->Rest( rest );

      if ( nl->ListLength( current ) == 8 )
      {
        string startFile, endFile, pgn;
        int startRow, endRow;

        // first value is ignored
        ListExpr current2 = nl->First( current );
        ListExpr rest2 = nl->Rest( current );

        // second value is ignored
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );

        // read startfile
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );
        readStringValue( startFile, current2 )

        // read startrow
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );
        readIntValue( startRow, current2 )

        // read endfile
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );
        readStringValue( endFile, current2 )

        // read endrow
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );
        readIntValue( endRow, current2 )

        // check state value is ignored
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );

        // read pgn-notation
        current2 = nl->First( rest2 );
        rest2 = nl->Rest( rest2 );
        readStringValue( pgn, current2 )

        // add move
        game->AddMove( startFile[ 0 ], startRow, endFile[ 0 ], endRow, pgn );

      }
      else
      {
        returnError(
                "Error: Wrong list format. Expected list of type\n"
                "((string string string int string "
                "int bool string string)...(...))." );
      }
    }
    game->SortMetainfos();
    correct = true;
    return SetWord( game );
  }
  else if ( ( nl->ListLength( instance ) == 1 )
            && ( nl->IsAtom( nl->First( instance ) )
                 && nl->AtomType( nl->First( instance ) ) == TextType ) )
  {
    string filename;
    nl->Text2String( nl->First( instance ), filename );
    ifstream * file = new ifstream( filename.c_str() );
    if ( file )
    {
      game = ParseFile( file );
      if ( game )
      {
        correct = true;
        return SetWord( game );
      }
      else
      {
        returnError( "Error while parsing file " + filename + "." );
      }
    }
    else
    {
      returnError( "Error: File " + filename + " not found." );
    }
  }
  else
  {
    returnError(
            "Error: Wrong list format. Expected list of type\n"
            "(metainfo_list move_list)." );
  }
}

void *
CastChessgame( void * addr )
{
  return ( new ( addr ) Chessgame );
}

Word
CreateChessgame( const ListExpr typeInfo )
{
  return ( SetWord( new Chessgame( 0, 0 ) ) );
}

void
DeleteChessgame( const ListExpr typeInfo, Word & w )
{
  ( ( Chessgame * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseChessgame( const ListExpr typeInfo, Word & w )
{
  ( ( Chessgame * ) w.addr ) ->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneChessgame( const ListExpr typeInfo, const Word & w )
{
  return SetWord( ( ( Chessgame * ) w.addr ) ->Clone() );
}

int
SizeOfChessgame()
{
  return sizeof( Chessgame );
}

bool
ChessgameType( ListExpr type, ListExpr & errorInfo )
{
  return ( nl->IsEqual( type, "chessgame" ) );
}

bool
OpenChessgame(
  SmiRecord & valueRecord, size_t & offset,
  const ListExpr typeInfo, Word & value )
{
  Chessgame * game;
  game = ( Chessgame* ) Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( game );
  return true;
}

bool
SaveChessgame(
  SmiRecord & valueRecord, size_t & offset,
  const ListExpr typeInfo, Word & value )
{
  Chessgame * p = ( Chessgame * ) value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, p );
  return true;
}


/*
3 Operators

*/

/*
3.1 Operator readpgn

reads a pgn file and returns a realtion of tuples, each containing only one
attribute of type games

*/
const string readpgnSpec =
  "( " "( " "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>text -> stream (game)</text--->"
  "<text>readpgn( _ )</text--->"
  "<text>Reads a file and returns a stream of games</text--->"
  "<text>query readpgn(\"ICCF2006.pgn\")</text--->"
  ")" ")";

int readpgnValueMap( Word * args, Word & result,
                     int message, Word & local, Supplier s )
{

  ifstream * file;
  const char* filename;
  FText* fn;
  Chessgame* game;
  switch ( message )
  {
  case OPEN:
    fn = ( ( FText * ) args[ 0 ].addr );
    filename = fn->Get();
    file = new ifstream( filename );
    local.addr = file;
    cout << "Parsing file '" << filename << "' - please wait..." << endl;
    return 0;
  case REQUEST:
    file = ( ifstream* ) local.addr;

    if ( file->is_open() )
    {
      if ( !file->eof() )
        game = ParseFile( file );
      else
        return CANCEL;
    }
    else
      ErrorReporter::ReportError( "File not open." );

    if ( game != NULL )
    {
      cout << ".";
      result.addr = game;
      return YIELD;
    }
    else
    {
      cout << endl << "Finished parsing file.";
      return CANCEL;
    }
  case CLOSE:
    file = ( ifstream* ) local.addr;
    file->close();
    cout << endl;
    return 0;
  }
  return -1;
}

ListExpr readpgnTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( nl->IsAtom( arg ) && nl->IsEqual( arg, "text" ) )
    {
      return nl->TwoElemList(
               nl->SymbolAtom( "stream" ),
               nl->SymbolAtom( "chessgame" ) );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.2 Operators on move type constructor

*/
const string agentSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> string</text--->"
  "<text>_ move</text--->"
  "<text>Returns agent figure of a move.</text--->"
  "<text>query move1 agent</text--->"
  ")" ")";

const string capturedSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> string</text--->"
  "<text>_ move</text--->"
  "<text>Returns captured figure of a move.</text--->"
  "<text>query move1 captured</text--->"
  ")" ")";

const string startrowSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> int</text--->"
  "<text>_ move</text--->"
  "<text>Returns start row of a move.</text--->"
  "<text>query move1 startrow</text--->"
  ")" ")";

const string endrowSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> int</text--->"
  "<text>_ move</text--->"
  "<text>Returns end row of a move.</text--->"
  "<text>query move1 endrow</text--->"
  ")" ")";

const string startfileSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> string</text--->"
  "<text>_ move</text--->"
  "<text>Returns start file of a move.</text--->"
  "<text>query move1 startfile</text--->"
  ")" ")";

const string endfileSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> string</text--->"
  "<text>_ move</text--->"
  "<text>Returns end file of a move.</text--->"
  "<text>query move1 endfile</text--->"
  ")" ")";

const string checkSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> bool</text--->"
  "<text>_ move</text--->"
  "<text>Returns true, if check was offered.</text--->"
  "<text>query move1 check</text--->"
  ")" ")";

const string capturesSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> bool</text--->"
  "<text>_ move</text--->"
  "<text>Returns true, if some figure was captured.</text--->"
  "<text>query move1 captured</text--->"
  ")" ")";

int agentValueMap( Word * args, Word & result,
                   int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  STRING resultString;
  int len = ( move->GetAgent() ).string::copy( resultString, 48, 0 );
  resultString[ len ] = '\0';
  ( ( CcString* ) result.addr ) ->Set( true, &resultString );
  return 0;
}

int capturedValueMap( Word * args, Word & result,
                      int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  STRING resultString;
  int len = ( move->GetCaptured() ).string::copy( resultString, 48, 0 );
  resultString[ len ] = '\0';
  ( ( CcString* ) result.addr ) ->Set( true, &resultString );
  return 0;
}

int startrowValueMap( Word * args, Word & result,
                      int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  ( ( CcInt* ) result.addr ) ->Set( true, move->GetStartRow() );
  return 0;
}

int endrowValueMap( Word * args, Word & result,
                    int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  ( ( CcInt* ) result.addr ) ->Set( true, move->GetEndRow() );
  return 0;
}

int startfileValueMap( Word * args, Word & result,
                       int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  STRING resultString;
  int len = ( move->GetStartFile() ).string::copy( resultString, 48, 0 );
  resultString[ len ] = '\0';
  ( ( CcString* ) result.addr ) ->Set( true, &resultString );
  return 0;
}

int endfileValueMap( Word * args, Word & result,
                     int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  STRING resultString;
  int len = ( move->GetEndFile() ).string::copy( resultString, 48, 0 );
  resultString[ len ] = '\0';
  ( ( CcString* ) result.addr ) ->Set( true, &resultString );
  return 0;
}

int checkValueMap( Word * args, Word & result,
                   int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  ( ( CcBool* ) result.addr ) ->Set( true, move->IsCheck() );
  return 0;
}

int capturesValueMap( Word * args, Word & result,
                      int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  ( ( CcBool* ) result.addr ) ->Set( true, move->GetCaptures() );
  return 0;
}

ListExpr moveStrTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( nl->IsEqual( arg, "move" ) )
    {
      return nl->SymbolAtom( "string" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

ListExpr moveIntTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( nl->IsEqual( arg, "move" ) )
    {
      return nl->SymbolAtom( "int" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

ListExpr moveBoolTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( nl->IsEqual( arg, "move" ) )
    {
      return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.3 Operator count

*/
const string countSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>material string -> bool</text--->"
  "<text>_ count( _ )</text--->"
  "<text>Returns number of respective figures in game.</text--->"
  "<text>query mat1 count [\"Pawn\"],\n"
  "query mat1 count [\"all\"],\n"
  "query mat1 count [\"white\"]</text--->"
  ")" ")";

int countValueMap( Word * args, Word & result, 
                   int message, Word & local, Supplier s )
{
  Material * material = ( ( Material* ) args[ 0 ].addr );
  CcString * value = ( ( CcString* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );

  ( ( CcInt* ) result.addr ) ->Set( true, 
                            material->Count( *value->GetStringval() ) );

  return 0;
}

ListExpr countTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( nl->IsEqual( arg1, "material" )
         && nl->IsAtom( arg2 ) && nl->IsEqual( arg2, "string" ) )
    {
      return nl->SymbolAtom( "int" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.3 Operators equal and less than

*/
const string equalSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>material material -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Returns true, if both materials are equal.</text--->"
  "<text>query mat1 = mat2 [\"Pawn\"]</text--->"
  ")" ")";

const string lessSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>material material -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Returns true, if mat2 have more figures than mat1.</text--->"
  "<text>query mat1 < mat2 [\"Pawn\"]</text--->"
  ")" ")";

int equalValueMap( Word * args, Word & result,
                   int message, Word & local, Supplier s )
{
  Material * mat1 = ( ( Material* ) args[ 0 ].addr );
  Material * mat2 = ( ( Material* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );

  ( ( CcBool* ) result.addr ) ->Set( true, mat1->IsEqual( mat2 ) );
  return 0;
}

int lessValueMap( Word * args, Word & result,
                  int message, Word & local, Supplier s )
{
  Material * mat1 = ( ( Material* ) args[ 0 ].addr );
  Material * mat2 = ( ( Material* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );

  ( ( CcBool* ) result.addr ) ->Set( true, ( mat1->Compare( mat2 ) < 0 ) );

  return 0;
}

ListExpr MatMatBoolTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( nl->IsEqual( arg1, "material" ) && nl->IsEqual( arg2, "material" ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.4 Operator pieces

*/
const string piecesSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>material string -> bool</text--->"
  "<text>_ pieces</text--->"
  "<text>Returns respective material object.</text--->"
  "<text>query pos1 pieces</text--->"
  ")" ")";

int piecesValueMap( Word * args, Word & result,
                    int message, Word & local, Supplier s )
{
  Position * position = ( ( Position* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  if ( position->IsDefined() )
    position->GetMaterial( ( ( Material* ) result.addr ) );
  else
    ( ( Material* ) result.addr ) ->SetDefined( false );

  return 0;
}

ListExpr piecesTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( nl->IsEqual( arg, "position" ) )
      return nl->SymbolAtom( "material" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.5 Operator moveNo

*/
const string moveNoSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>move -> int, position -> int</text--->"
  "<text>_ moveNo</text--->"
  "<text>Returns move number of move or position object.</text--->"
  "<text>query move1 moveNo</text--->"
  ")" ")";

int moveNoValueMapM( Word * args, Word & result,
                     int message, Word & local, Supplier s )
{
  Move * move = ( ( Move* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  if ( move->IsDefined() )
    ( ( CcInt* ) result.addr ) ->Set( true, move->GetMoveNumber() );
  else
    ( ( CcInt* ) result.addr ) ->SetDefined( false );


  return 0;
}

int moveNoValueMapP( Word * args, Word & result,
                     int message, Word & local, Supplier s )
{
  Position * position = ( ( Position* ) args[ 0 ].addr );
  result = qp->ResultStorage( s );
  if ( position->IsDefined() )
    ( ( CcInt* ) result.addr ) ->Set( true, position->GetMoveNumber() );
  else
    ( ( CcInt* ) result.addr ) ->SetDefined( false );

  return 0;
}

ListExpr moveNoTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg = nl->First( args );
    if ( ( nl->IsEqual( arg, "move" ) ) || ( nl->IsEqual( arg, "position" ) ) )
      return nl->SymbolAtom( "int" );
  }
  return nl->SymbolAtom( "typeerror" );
}

ValueMapping moveNoMap[] = { moveNoValueMapM, moveNoValueMapP };
int moveNoSelect( ListExpr args )
{
  ListExpr arg = nl->First( args );
  if ( nl->IsEqual( arg, "move" ) )
    return ( 0 );
  if ( nl->IsEqual( arg, "position" ) )
    return ( 1 );
  return ( -1 ); // This point should never be reached
}

/*
3.6 Operator range

*/
const string rangeSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>string int string int-> position</text--->"
  "<text>_ range[_,_,_,_]</text--->"
  "<text>Returns a partial position object.</text--->"
  "<text>query pos1 range[\"b\",2,\"g\",7]</text--->"
  ")" ")";

int rangeValueMap( Word * args, Word & result, 
                   int message, Word & local, Supplier s )
{
  Position * position = ( ( Position* ) args[ 0 ].addr );
  CcString * startFile = ( ( CcString* ) args[ 1 ].addr );
  CcInt * startRow = ( ( CcInt* ) args[ 2 ].addr );
  CcString * endFile = ( ( CcString* ) args[ 3 ].addr );
  CcInt * endRow = ( ( CcInt* ) args[ 4 ].addr );
  result = qp->ResultStorage( s );
  position->Range( ( ( Position* ) result.addr ),
                   startFile->GetValue() [ 0 ], startRow->GetValue(),
                   endFile->GetValue() [ 0 ], endRow->GetValue() );
  return 0;
}

ListExpr rangeTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 5 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    ListExpr arg3 = nl->Third( args );
    ListExpr arg4 = nl->Fourth( args );
    ListExpr arg5 = nl->Fifth( args );
    if ( ( nl->IsEqual( arg1, "position" ) )
         && nl->IsAtom( arg2 ) && nl->IsEqual( arg2, "string" )
         && nl->IsAtom( arg3 ) && nl->IsEqual( arg3, "int" )
         && nl->IsAtom( arg4 ) && nl->IsEqual( arg4, "string" )
         && nl->IsAtom( arg5 ) && nl->IsEqual( arg5, "int" ) )
      return nl->SymbolAtom( "position" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.7 Operator includes

*/
const string includesSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>position position -> bool</text--->"
  "<text>_ includes [ _ ]</text--->"
  "<text>Returns true, if pos2 is included in pos2</text--->"
  "<text>query pos1 includes [pos2]</text--->"
  ")" ")";

int includesValueMap( Word * args, Word & result, 
                      int message, Word & local, Supplier s )
{
  Position * pos1 = ( ( Position* ) args[ 0 ].addr );
  Position * pos2 = ( ( Position* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );
  ( ( CcBool* ) result.addr ) ->Set( true, pos2->IsIncluded( pos1 ) );
  return 0;
}

ListExpr includesTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( ( nl->IsEqual( arg1, "position" ) )
         && ( nl->IsEqual( arg2, "position" ) ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.8 Operator getposition

*/
const string getpositionSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>game int -> position</text--->"
  "<text>_ getposition [_]</text--->"
  "<text>Returns position of a game after the specificated move</text--->"
  "<text>query game getposition [4]</text--->"
  ")" ")";

int getpositionValueMap( Word * args, Word & result, 
                         int message, Word & local, Supplier s )
{
  Chessgame * game = ( ( Chessgame* ) args[ 0 ].addr );
  CcInt * moveNumber = ( ( CcInt* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );
  if ( ( moveNumber->GetValue() < 0 )
       || ( moveNumber->GetValue() > game->GetMovesCount() ) )
    ( ( Position* ) result.addr ) ->SetDefined( false );
  else
    game->GetPosition( ( Position* ) result.addr, moveNumber->GetValue() );
  return 0;
}

ListExpr getpositionTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( ( nl->IsEqual( arg1, "chessgame" ) )
         && ( nl->IsAtom( arg2 ) && nl->IsEqual( arg2, "int" ) ) )
      return nl->SymbolAtom( "position" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.8 Operator getmove

*/
const string getmoveSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>game int -> move</text--->"
  "<text>_ getmove [_]</text--->"
  "<text>Returns specified move of a game</text--->"
  "<text>query game getmove [4]</text--->"
  ")" ")";

int getmoveValueMap( Word * args, Word & result, 
                     int message, Word & local, Supplier s )
{
  Chessgame * game = ( ( Chessgame* ) args[ 0 ].addr );
  CcInt * moveNumber = ( ( CcInt* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );
  if ( ( moveNumber->GetValue() < 1 )
       || ( moveNumber->GetValue() > game->GetMovesCount() ) )
    ( ( Move* ) result.addr ) ->SetDefined( false );
  else
    game->GetMove( ( Move* ) result.addr, moveNumber->GetValue() );
  return 0;
}

ListExpr getmoveTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( ( nl->IsEqual( arg1, "chessgame" ) )
         && ( nl->IsAtom( arg2 ) && nl->IsEqual( arg2, "int" ) ) )
      return nl->SymbolAtom( "move" );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.9 Operator getkey

*/
const string getkeySpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>game string -> string</text--->"
  "<text>_ getkey [_]</text--->"
  "<text>Returns specified data of a game</text--->"
  "<text>query game getkey [\"name_w\"]</text--->"
  ")" ")";

int getkeyValueMap( Word * args, Word & result, 
                    int message, Word & local, Supplier s )
{
  Chessgame * game = ( ( Chessgame* ) args[ 0 ].addr );
  CcString * key = ( ( CcString* ) args[ 1 ].addr );
  result = qp->ResultStorage( s );
  STRING resultStr;
  game->GetMetainfoValue( key->GetValue(), &resultStr );
  ( ( CcString* ) result.addr ) ->Set( true, &resultStr ) ;

  return 0;
}

ListExpr getkeyTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    if ( ( nl->IsEqual( arg1, "chessgame" ) )
         && ( nl->IsAtom( arg2 ) && nl->IsEqual( arg2, "string" ) ) )
      return nl->SymbolAtom( "string" );
  }
  return nl->SymbolAtom( "typeerror" );
}

struct Localinfo
{
  Chessgame * game;
  int cntr;
};

/*
3.10 Operator moves

*/
const string movesSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>game -> moves</text--->"
  "<text>_ moves</text--->"
  "<text>Returns a stream of all moves of a game</text--->"
  "<text>query game moves</text--->"
  ")" ")";

int movesValueMap( Word * args, Word & result, 
                   int message, Word & local, Supplier s )
{
  Localinfo * localinfo = new Localinfo;
  switch ( message )
  {
  case OPEN:
    localinfo->game = ( Chessgame* ) args[ 0 ].addr;
    localinfo->cntr = 1;
    local.addr = localinfo;
    return 0;
  case REQUEST:
    localinfo = ( Localinfo* ) local.addr;
    if ( localinfo->cntr <= ( localinfo->game->GetMovesCount() ) )
    {
      result.addr = ( localinfo->game->GetMove( localinfo->cntr++ ) );
      return YIELD;
    }
    else
      return CANCEL;
  case CLOSE:
    localinfo = ( Localinfo* ) local.addr;
    delete localinfo;
    return 0;
  }
  return -1;
}

ListExpr movesTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    if ( nl->IsEqual( nl->First( args ), "chessgame" ) )
      return nl->TwoElemList( nl->SymbolAtom( "stream" ),
                              nl->SymbolAtom( "move" ) );
  }
  return nl->SymbolAtom( "typeerror" );
}



/*
3.10 Operator positions

*/
const string positionsSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>game -> positions</text--->"
  "<text>_ moves</text--->"
  "<text>Returns a stream of all moves of a game</text--->"
  "<text>query game positions</text--->"
  ")" ")";

int positionsValueMap( Word * args, Word & result, 
                       int message, Word & local, Supplier s )
{
  Localinfo * localinfo = new Localinfo();
  switch ( message )
  {
  case OPEN:
    localinfo->game = ( Chessgame* ) args[ 0 ].addr;
    localinfo->cntr = 0;
    local.addr = localinfo;
    return 0;
  case REQUEST:
    localinfo = ( Localinfo* ) local.addr;
    if ( localinfo->cntr <= ( localinfo->game->GetMovesCount() ) )
    {
      result.addr = ( localinfo->game->GetPosition( localinfo->cntr++ ) );
      return YIELD;
    }
    else
      return CANCEL;
  case CLOSE:
    return 0;
  }
  return -1;
}


ListExpr positionsTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    if ( nl->IsEqual( nl->First( args ), "chessgame" ) )
      return nl->TwoElemList( nl->SymbolAtom( "stream" ),
                              nl->SymbolAtom( "position" ) );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
3.11 Operator movingpoints

Dieser Fehler ist in der movingpoints-Implementierung enthalten:

Wenn eine Figur geschlagen wird, wandert sie auf die Koordinaten (x,
y+-2), wenn seine Anfangskoordinaten (x,y) waren; dabei gilt "+" für
schwarze und "-" für weiße Figuren. Z.B. wandert die weiße Dame auf die
Koordinaten (4.0, -1.0).
Handelt es ich bei der geschlagenen Figur um einen Offizier, der aus
einer Bauernumwandlung hervorging, so zeigen seine Anfangskoordinaten
auf die gegnerische Grundlinie. Er zieht also auf die Linie 6, wenn er
weiß ist, auf die Linie 3, wenn er schwarz ist - und da gehört er nun
wirklich nicht hin.

Mögliche Lösungsvarianten wären:

- Änderung des Figurtyps beim umgewandelten Bauern statt Erzeugung einer
neuen Figur (das hat Vor- und Nachteile für manche analytischen Abfragen)

- Setzen eines Flags "isPromotedPawn" oder so, das bei der Berechnung
des Abstellplatzes ausgewertet wird

- Festlegen des Abstellplatzes bei der Initialisierung, statt sie erst
im Moment des Schlagens zu berechnen

Bei der 2. und 3. Lösung müßte man noch überlegen, was ein geeigneter
Abstellplatz ist und wie man zuverlässig vermeidet, daß dort mehrere
Figuren abgestellt werden.

*/

const string movingpointsSpec =
  "(" "(" "\"Signature\"" "\"Syntax\"" "\"Meaning\"" "\"Example\"" ")" "("
  "<text>chessgame -> stream(tup(Piece White Route)</text--->"
  "<text>with Piece: string, White: bool, Route: mpoint</text--->"
  "<text>Returns a stream with all pieces' movements</text--->"
  "<text>query game movingpoints consume</text--->"
  ")" ")";

int movingpointsValueMap( Word * args, Word & result, 
                          int message, Word & local, Supplier s )
{
  MovingChessPieces * mcps;
  MovingChessPiece* mcp;
  Chessgame* game;
  Move mv;
  string startfile, endfile, argstr;
  CcString* ki;
  STRING hilfstr;
  CcBool* iw;
  MPoint* mp;
  int i;
  ListExpr resultType;
  TupleType* resultTupleType;
  Tuple* tup;
  switch ( message )
  {
  case OPEN:
    game = ( Chessgame* ) args[ 0 ].addr;
    resultType = GetTupleResultType( s );
    resultTupleType = new TupleType( nl->Second( resultType ) );
    /*
     As the result will be a stream of tuples with an attribute of type mpoint,
     these mpoints must be completely created from the chessgame before
     the first tuple can be written. Therefor this work is done when the
     stream is opened.
     1st step: create the movingpoints for the start position of the game
    */
    mcps = new MovingChessPieces();
    local.addr = mcps;
    mcps->setTupleTypeRemember( resultTupleType );
    /*
     2nd step: read all moves of the game and realize it in the mcps structure
    */
    for ( i = 1;i <= game->GetMovesCount();i++ )
    {
      mcps->realizeMove( game->GetMove( i ), game->GetMoveData( i-1 ) );
    }
    mcps->closeMPoints();

    return 0;
  case REQUEST:
    mcps = ( MovingChessPieces* ) local.addr; 
          // reestablish access to moving points array
    i = mcps->getMPointWriteNextCount();  
          // look how many tuples have been written
    if ( i < 48 )
    {
      while ( i < 48 )
      {
        if ( mcps->isValid( i ) ) break; 
              // only write mpoints representing chesspieces used
        i++;
        mcps->incMPointWriteNextCount();
      }

      if ( i >= 48 ) return CANCEL; 
          // if i !< 48: no valid mpoints any more to write

      mcps->incMPointWriteNextCount();
      mcp = mcps->getMovingChessPiece( i );
      resultTupleType = mcps->getTupleTypeRemember();
      tup = new Tuple( resultTupleType );
      strcpy( hilfstr, ( char* ) ( mcp->getKind() ->c_str() ) );
      ki = new CcString( true, &hilfstr );
      tup->PutAttribute( 0, ( StandardAttribute* ) ki );
      iw = new CcBool( true, *( mcp->trueIsWhite() ) );
      tup->PutAttribute( 1, ( StandardAttribute* ) iw );
      mp = mcp->getMPoint();
      tup->PutAttribute( 2, ( ( StandardAttribute* ) mp ) ->Clone() );
      result = SetWord( tup );
      return YIELD;

    }
    else
      return CANCEL;
  case CLOSE:
    //cout << "movingpointsValueMap entered for CLOSE" << endl;
    mcps = ( MovingChessPieces* ) local.addr; 
        // reestablish access to moving points array
    mcps->getTupleTypeRemember() ->DeleteIfAllowed();
    delete mcps;
    return 0;
  }
  return -1;
}


ListExpr movingpointsTypeMap( ListExpr args )
{
  ListExpr streamList, tupleList;
  string argstr;

  nl->WriteToString( argstr, args ); 
  if ( nl->ListLength( args ) == 1 )
  {
    tupleList = nl->ThreeElemList(
                  nl->TwoElemList(
                    nl->SymbolAtom( "Piece" ), nl->SymbolAtom( "string" ) ),
                  nl->TwoElemList(
                    nl->SymbolAtom( "White" ), nl->SymbolAtom( "bool" ) ) ,
                  nl->TwoElemList(
                    nl->SymbolAtom( "Route" ), nl->SymbolAtom( "mpoint" ) )
                );
    streamList = nl->TwoElemList( nl->SymbolAtom( "stream" ),
                                  nl->TwoElemList( nl->SymbolAtom( "tuple" ),
                                                   tupleList ) );
    if ( nl->IsEqual( nl->First( args ), "chessgame" ) )
    {
      nl->WriteToString( argstr, streamList ); 
      return streamList;
    }
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
4 Build Type Constructors

*/
TypeConstructor materialTC(
  "material",
  MaterialProp,
  OutMaterial, InMaterial,
  0, 0,
  CreateMaterial, DeleteMaterial,
  0, 0,
  CloseMaterial, CloneMaterial,
  CastMaterial,
  SizeOfMaterial,
  MaterialType
);

TypeConstructor moveTC(
  "move",
  MoveProp,
  OutMove, InMove,
  0, 0,
  CreateMove, DeleteMove,
  0, 0,
  CloseMove, CloneMove,
  CastMove,
  SizeOfMove,
  MoveType
);

TypeConstructor positionTC(
  "position",
  PositionProp,
  OutPosition, InPosition,
  0, 0,
  CreatePosition, DeletePosition,
  0, 0,
  ClosePosition, ClonePosition,
  CastPosition,
  SizeOfPosition,
  PositionType
);

#ifdef RELALG_PERSISTENT
TypeConstructor gameTC(
  "chessgame",
  ChessgameProp,
  OutChessgame, InChessgame,
  0, 0,
  CreateChessgame, DeleteChessgame,
  OpenChessgame, SaveChessgame,
  CloseChessgame, CloneChessgame,
  CastChessgame,
  SizeOfChessgame,
  ChessgameType
);
#else
TypeConstructor gameTC(
  "chessgame",
  ChessgameProp,
  OutChessgame, InChessgame,
  0, 0,
  CreateChessgame, DeleteChessgame,
  0, 0,
  CloseChessgame, CloneChessgame,
  CastChessgame,
  SizeOfChessgame,
  ChessgameType
);
#endif // RELALG_PERSISTENT




/*
5 Build Operators

*/
Operator readpgnOp (
  "readpgn",
  readpgnSpec,
  readpgnValueMap,
  Operator::SimpleSelect,
  readpgnTypeMap
);

Operator agentOp (
  "agent",
  agentSpec,
  agentValueMap,
  Operator::SimpleSelect,
  moveStrTypeMap
);

Operator capturedOp (
  "captured",
  capturedSpec,
  capturedValueMap,
  Operator::SimpleSelect,
  moveStrTypeMap
);

Operator startrowOp (
  "startrow",
  startrowSpec,
  startrowValueMap,
  Operator::SimpleSelect,
  moveIntTypeMap
);

Operator endrowOp (
  "endrow",
  endrowSpec,
  endrowValueMap,
  Operator::SimpleSelect,
  moveIntTypeMap
);

Operator startfileOp (
  "startfile",
  startfileSpec,
  startfileValueMap,
  Operator::SimpleSelect,
  moveStrTypeMap
);

Operator endfileOp (
  "endfile",
  endfileSpec,
  endfileValueMap,
  Operator::SimpleSelect,
  moveStrTypeMap
);

Operator checkOp (
  "check",
  checkSpec,
  checkValueMap,
  Operator::SimpleSelect,
  moveBoolTypeMap
);

Operator capturesOp (
  "captures",
  capturesSpec,
  capturesValueMap,
  Operator::SimpleSelect,
  moveBoolTypeMap
);

Operator countOp (
  "cnt",
  countSpec,
  countValueMap,
  Operator::SimpleSelect,
  countTypeMap
);

Operator equalOp (
  "=",
  equalSpec,
  equalValueMap,
  Operator::SimpleSelect,
  MatMatBoolTypeMap
);

Operator lessOp (
  "<",
  lessSpec,
  lessValueMap,
  Operator::SimpleSelect,
  MatMatBoolTypeMap
);

Operator piecesOp (
  "pieces",
  piecesSpec,
  piecesValueMap,
  Operator::SimpleSelect,
  piecesTypeMap
);

Operator moveNoOp (
  "moveNo",
  lessSpec,
  2, moveNoMap,
  moveNoSelect,
  moveNoTypeMap
);

Operator rangeOp (
  "posrange",
  rangeSpec,
  rangeValueMap,
  Operator::SimpleSelect,
  rangeTypeMap
);

Operator includesOp (
  "includes",
  includesSpec,
  includesValueMap,
  Operator::SimpleSelect,
  includesTypeMap
);

Operator getpositionOp (
  "getposition",
  getpositionSpec,
  getpositionValueMap,
  Operator::SimpleSelect,
  getpositionTypeMap
);

Operator getmoveOp (
  "getmove",
  getmoveSpec,
  getmoveValueMap,
  Operator::SimpleSelect,
  getmoveTypeMap
);

Operator getkeyOp (
  "getkey",
  getkeySpec,
  getkeyValueMap,
  Operator::SimpleSelect,
  getkeyTypeMap
);

Operator movesOp (
  "moves",
  movesSpec,
  movesValueMap,
  Operator::SimpleSelect,
  movesTypeMap
);

Operator positionsOp (
  "positions",
  positionsSpec,
  positionsValueMap,
  Operator::SimpleSelect,
  positionsTypeMap
);

Operator movingpointsOp (
  "movingpoints",
  movingpointsSpec,
  movingpointsValueMap,
  Operator::SimpleSelect,
  movingpointsTypeMap
);

/*
6 Build Algebra

*/
class ChessAlgebra : public Algebra
{
public:
  ChessAlgebra() : Algebra()
  {
    AddTypeConstructor( &materialTC );
    AddTypeConstructor( &moveTC );
    AddTypeConstructor( &positionTC );
    AddTypeConstructor( &gameTC );

    materialTC.AssociateKind( "DATA" );
    moveTC.AssociateKind( "DATA" );
    positionTC.AssociateKind( "DATA" );
    gameTC.AssociateKind( "DATA" );

    materialTC.AssociateKind( "INDEXABLE" );
    moveTC.AssociateKind( "INDEXABLE" );
    positionTC.AssociateKind( "INDEXABLE" );
    gameTC.AssociateKind( "INDEXABLE" );

    AddOperator( &readpgnOp );
    AddOperator( &agentOp );
    AddOperator( &capturedOp );
    AddOperator( &startrowOp );
    AddOperator( &endrowOp );
    AddOperator( &startfileOp );
    AddOperator( &endfileOp );
    AddOperator( &checkOp );
    AddOperator( &capturesOp );
    AddOperator( &countOp );
    AddOperator( &equalOp );
    AddOperator( &lessOp );
    AddOperator( &piecesOp );
    AddOperator( &moveNoOp );
    AddOperator( &rangeOp );
    AddOperator( &includesOp );
    AddOperator( &getpositionOp );
    AddOperator( &getmoveOp );
    AddOperator( &getkeyOp );
    AddOperator( &movesOp );
    AddOperator( &positionsOp );
    AddOperator( &movingpointsOp );
  }
  ~ChessAlgebra() {}
  ;
};

ChessAlgebra chessAlgebra;
extern "C"
  Algebra*
  InitializeChessAlgebra( NestedList * nlRef, QueryProcessor * qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return ( &chessAlgebra );
}

} // Namespace chessAlgebra
