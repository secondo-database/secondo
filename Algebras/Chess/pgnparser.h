/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science,
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
#ifndef PGNPARSER_H
#define PGNPARSER_H

#include <iostream>
#include <string>
#include <fstream>
#include "chessTypes.h"

namespace ChessAlgebra {

/*
The flag below may help to find bugs in the parser ot the game
representation.

*/
bool trace = false;

/*
player constants - used in pgn-parser

*/
enum player
{
  WHITE, BLACK
};


void traceMove( const string& pgn, char agent, char startfile, 
		 int startrow, char endfile, int endrow, bool captures )
{

  if (trace) 
  {
  cout << "pgn: " << pgn 
       << "  recognized: " << agent 
       << startfile << startrow << endfile << endrow
       << "  captures: " << captures << endl; 
  }	  
}

bool 
parseMove( char &startFile, int &startRow,
           char &endFile, int &endRow,
           int &playerNo, string &pgn,
           Position* position )
{
  char agent = ' ';
  int distance = 0;
  
  bool captures = false;
  bool agentSet = false;
  bool doall = false;

  startFile = ' ';
  endFile = ' ';

  startRow = 0;
  endRow = 0;


  for ( int i = 0; i < min( 5, ( int ) pgn.length() ); i++ )
  {
    switch ( pgn[ i ] )
    {
    case 'K':
    case 'Q':
    case 'R':
    case 'B':
    case 'N':
      if ( agentSet )
      {
        cerr << "unexpected Capital Letter in '" + 
                                    pgn + "'";
	
        return false;
      }
      agent = ( playerNo == WHITE ) ? toupper( pgn[ i ] ) : tolower( pgn[ i ] );
      agentSet = true;
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
      if ( not agentSet )
      {
        agent = ( playerNo == WHITE ) ? 'P' : 'p';
        agentSet = true;
      }
      if ( endFile != ' ' ) { 
         startFile = endFile;
      }	      
      endFile = pgn[ i ];
      break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      if ( endRow != 0 ) { 
        startRow = endRow;
      }	
      endRow = atoi( pgn.substr( i, 1 ).c_str() );
      break;
    case 'x':
      captures = true;
      break;
    case 'O':
      if ( pgn.substr( 0, 5 ) == "O-O-O" )
      {
        endFile = 'c';
      }
      else if ( pgn.substr( 0, 3 ) == "O-O" )
      {
        endFile = 'g';
      }
      else
      {
        cerr << "unexpected continuation of 'O'\n" << endl;
        return false;
      }
      agent = ( playerNo == WHITE ) ? 'K' : 'k';
      startFile = 'e';
      startRow = endRow = ( playerNo == WHITE ) ? 1 : 8;
      traceMove(pgn, agent, startFile, startRow, endFile, endRow, captures);
      return true;
    default:
      i = 6; //?
    }
  }

  if ( endRow == 0 || endFile == ' ' ) { // what is this good for?	  
      traceMove(pgn, agent, startFile, startRow, endFile, endRow, captures);
      return true;
  }

  if ( startRow == 0 || startFile == ' ' )
  { 
    // Calculate startRow and startFile
    doall = ( startRow == 0 && startFile == ' ' );
    char origStartFile = ' ';
    int origStartRow = 0;
    bool agentFound = false;
    string tmpAgent = "";
    bool filepos = true, fileneg = true;
    bool rowpos = true, rowneg = true;
    bool diag1 = true, diag2 = true, diag3 = true, diag4 = true;
    switch ( agent )
    { // Pawns may move only forward so black and white
    case 'P':
      // have to be handled separately
      if ( captures )
      {
        startRow = endRow - 1;
        if ( startFile != ' ' ) break;
        startFile = endFile - 1;
        if ( startFile >= 'a' &&
             position->TestField( "P", startFile, startRow ) ) break;
        startFile = endFile + 1;
        if ( startFile <= 'h' &&
             position->TestField( "P", startFile, startRow ) ) break;
      }
      else
      { // doesn't capture
        startFile = endFile;
        startRow = endRow - 1;
        if ( position->TestField( "P", startFile, startRow ) ) break;
        startRow = endRow - 2;
        if ( startRow == 2 &&
             position->TestField( "P", startFile, startRow ) ) break;
      }
      cerr << "Didn't find white pawn to make move\n" << endl;
      
      return false;
      break;
    case 'p':
      if ( captures )
      {
        startRow = endRow + 1;
        if ( startFile != ' ' ) break;
        startFile = endFile - 1;
        if ( startFile >= 'a' &&
             position->TestField( "p", startFile, startRow ) ) break;
        startFile = endFile + 1;
        if ( startFile <= 'h' &&
             position->TestField( "p", startFile, startRow ) ) break;
      }
      else
      { // doesn't capture
        startFile = endFile;
        startRow = endRow + 1;
        if ( position->TestField( "p", startFile, startRow ) ) break;
        startRow = endRow + 2;
        if ( startRow == 7 &&
             position->TestField( "p", startFile, startRow ) ) break;
      }
      cerr << "Didn't find black pawn to make move\n" << endl;
      
      return false;
      break;
    case 'N':
    case 'n':
      if ( startFile != ' ' )
      {
        distance = 3 - abs( startFile - endFile );
        startRow = endRow - distance;
        if ( startRow >= 1 && 
           ( agentFound = position->TestField( string( 1, agent ),
             startFile, startRow ) ) ) break;
        startRow = endRow + distance;
        if ( startRow <= 8 && 
            ( agentFound = position->TestField( string( 1, agent ),
                                  startFile, startRow ) ) ) break;
        cerr << "Didn't find Knight to make move\n" << endl;
        
	return false;
        break;
      }
      else if ( startRow != 0 )
      {
        distance = 3 - abs( startRow - endRow );
        startFile = endFile - distance;
        if ( startFile >= 'a' && 
            ( agentFound = position->TestField( string( 1, agent ),
                                  startFile, startRow ) ) ) break;
        startFile = endFile + distance;
        if ( startFile <= 'h' && 
            ( agentFound = position->TestField( string( 1, agent ),
                           startFile, startRow ) ) ) break;
        cerr << "Didn't find Knight to make move\n" << endl;
	
	return false;
        break;
      }
      else
      {
        startFile = endFile - 2;
        startRow = endRow - 1;
        if ( startRow >= 1 && startFile >= 'a' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile - 2;
        startRow = endRow + 1;
        if ( startRow <= 8 && startFile >= 'a' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile - 1;
        startRow = endRow - 2;
        if ( startRow >= 1 && startFile >= 'a' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile - 1;
        startRow = endRow + 2;
        if ( startRow <= 8 && startFile >= 'a' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile + 1;
        startRow = endRow - 2;
        if ( startRow >= 1 && startFile <= 'h' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile + 1;
        startRow = endRow + 2;
        if ( startRow <= 8 && startFile <= 'h' &&
             ( agentFound = position->TestField( string( 1, agent ),
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile + 2;
        startRow = endRow - 1;
        if ( startRow >= 1 && startFile <= 'h' &&
             ( agentFound = position->TestField( string( 1, agent ),
                                                 startFile, startRow ) ) )
          break;
        startFile = endFile + 2;
        startRow = endRow + 1;
        if ( startRow <= 8 && startFile <= 'h' &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
      }
      cerr << "Didn't find Knight to make move\n" << endl;
      
      return false;
      break;
    case 'B':
    case 'b':
      origStartRow = startRow;
      origStartFile = startFile;
      for ( int i = -7; i <= 7; i++ )
      {
        startFile = endFile + i;
        startRow = endRow + i;
        if ( startRow >= 1 && startFile >= 'a' && 
             startRow <= 8 && startFile <= 'h' &&
             ( doall || origStartRow == startRow ||
               origStartFile == startFile ) &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
      }
      if ( agentFound ) break;
      for ( int i = -7; i <= 7; i++ )
      {
        startFile = endFile + i;
        startRow = endRow - i;
        if ( startRow >= 1 && startFile >= 'a' && 
             startRow <= 8 && startFile <= 'h' &&
             ( doall || origStartRow == startRow || 
               origStartFile == startFile ) &&
             ( agentFound = position->TestField( string( 1, agent ), 
                                                 startFile, startRow ) ) )
          break;
      }
      if ( agentFound ) break;
      cerr << "Didn't find Bishop to make move\n" << endl;
      
      return false;
      break;
    case 'R':
    case 'r':
      if ( startFile == endFile )
      { // need to search only this file
        filepos = fileneg = false;
      }
      else if ( startFile != ' ' )
      { // startFile != endFile => startRow == endRow
        startRow = endRow;
        break;
      }
      else if ( startRow == endRow )
      { // need to search only this row
        rowpos = rowneg = false;
      }
      else if ( startRow != 0 )
      { // startRow != endRow => startFile == endFile
        startFile = endFile;
        break;
      }
      for ( int i = 1; i <= 7; i++ )
      {
        if ( filepos )
        {
          if ( (endFile + i) > 'h' )
          {
            filepos = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile + i , endRow );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow;
              startFile = endFile + i;
              break;
            }
            else if ( tmpAgent != "-" && tmpAgent != "x" )
            {
              filepos = false;
            }
          }
        }
        if ( fileneg )
        {
          if ( (endFile - i) < 'a' )
          {
            fileneg = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile - i, endRow );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow;
              startFile = endFile - i;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              fileneg = false;
            }
          }
        }
        if ( rowpos )
        {
          if ( (endRow + i) > 8 )
          {
            rowpos = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile, endRow + i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow + i;
              startFile = endFile;
              break;
            }
            else if ( tmpAgent != "-" && tmpAgent != "x" )
            {
              rowpos = false;
            }
          }
        }
        if ( rowneg )
        {
          if ( (endRow - i) < 1 )
          {
            rowneg = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile, endRow - i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow - i;
              startFile = endFile;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              rowneg = false;
            }
          }
        }
      }
      if ( startFile == ' ' || startRow == 0 )
      {
        cerr << "Didn't find Rook to make move\n";
	
      }
      break;
    case 'Q':
    case 'q':
      if ( startFile == endFile )
      { // need to search only this file
        filepos = fileneg = diag1 = diag2 = diag3 = diag4 = false;

      }
      else if ( startFile != ' ' )
      {
        if ( position->TestField( string( 1, agent ), startFile, endRow ) )
        {
          startRow = endRow;
          break;
        }
        else if ( ( startRow = endRow + abs( endFile - startFile ) ) <= 8 &&
                  position->TestField( string( 1, agent ), 
                                       startFile, startRow ) )
        {
          break;
        }
        else if ( ( startRow = endRow - abs( endFile - startFile ) ) >= 1 &&
                  position->TestField( string( 1, agent ),
                                        startFile, startRow ) )
        {
          break;
        }
      }
      else if ( startRow == endRow )
      { // need to search only this row
        rowpos = rowneg = diag1 = diag2 = diag3 = diag4 = false;
      }
      else if ( startRow != 0 )
      {
        if ( position->TestField( string( 1, agent ), endFile, startRow ) )
        {
          startFile = endFile;
          break;
        }
        else if ( ( startFile = endFile + abs( endRow - startRow ) ) <= 'h' &&
                  position->TestField( string( 1, agent ), 
                                       startFile, startRow ) )
        {
          break;
        }
        else if ( ( startFile = endFile - abs( endRow - startRow ) ) >= 'a' &&
                  position->TestField( string( 1, agent ), 
                                       startFile, startRow ) )
        {
          break;
        }
      }
      for ( int i = 1; i <= 7; i++ )
      {
        if ( filepos )
        {
          if ( (endFile + i) > 'h' )
          {
            filepos = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile + i, endRow );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow;
              startFile = endFile + i;
              break;
            }
            else if ( tmpAgent != "-" && tmpAgent != "x" )
            {
              filepos = false;
            }
          }
        }
        if ( fileneg )
        {
          if ( (endFile - i) < 'a' )
          {
            fileneg = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile - i, endRow );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow;
              startFile = endFile - i;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              fileneg = false;
            }
          }
        }
        if ( rowpos )
        {
          if ( (endRow + i) > 8 )
          {
            rowpos = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile, endRow + i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow + i;
              startFile = endFile;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              rowpos = false;
            }
          }
        }
        if ( rowneg )
        {
          if ( endRow - i < 1 )
          {
            rowneg = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile, endRow - i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow - i;
              startFile = endFile;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              rowneg = false;
            }
          }
        }
        if ( diag1 )
        {
          if ( (endFile + i) > 'h' || (endRow + i) > 8 )
          {
            diag1 = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile + i, endRow + i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow + i;
              startFile = endFile + i;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              diag1 = false;
            }
          }
        }
        if ( diag2 )
        {
          if ( (endFile + i) > 'h' || (endRow - i) < 1 )
          {
            diag2 = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile + i, endRow - i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow - i;
              startFile = endFile + i;
              break;
            }
            else if ( (tmpAgent != "-") && (tmpAgent != "x") )
            {
              diag2 = false;
            }
          }
        }
        if ( diag3 )
        {
          if ( (endFile - i) < 'a' || (endRow + i) > 8 )
          {
            diag3 = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile - i, endRow + i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow + i;
              startFile = endFile - i;
              break;
            }
            else if ( tmpAgent != "-" && tmpAgent != "x" )
            {
              diag3 = false;
            }
          }
        }
        if ( diag4 )
        {
          if ( (endFile - i) < 'a' || (endRow - i) < 1 )
          {
            diag4 = false;
          }
          else
          {
            tmpAgent = position->GetAgentShort( endFile - i, endRow - i );
            if ( tmpAgent[ 0 ] == agent )
            {
              startRow = endRow - i;
              startFile = endFile - i;
              break;
            }
            else if ( tmpAgent != "-" && tmpAgent != "x" )
            {
              diag4 = false;
            }
          }
        }
      }
      if ( startFile == ' ' || startRow == 0 )
      {
        cerr << "Didn't find Rook to make move\n" << endl;
	
	return false;
      }
      break;
    case 'K':
    case 'k':
      for ( int i = -1; i <= 1; i++ ) {
        
	if (agentFound) // short exit
          break;

        for ( int j = -1; j <= 1; j++ ) {

          if (agentFound) // short exit
            break;

          startFile = endFile + i;
          startRow = endRow + j;
          if ( startRow >= 1 && startFile >= 'a' && 
               startRow <= 8 && startFile <= 'h' &&
	       !((i == 0) && (j == 0)) )
	  {
            agentFound = position->TestField( string( 1, agent ), 
                                              startFile, startRow );
	    //cerr << "Test for " << agent << ": " 
	    //	 << startFile << startRow << " " << agentFound << endl;
          }
        }
      }
      if (!agentFound) { 
        cmsg.error() << "Didn't find King to make move\n" << endl;
        cmsg.send();
        traceMove(pgn, agent, startFile, startRow, endFile, endRow, captures);
        return false;
      }	
      break;
    default: {
      cerr << "Internal Error: wrong agent"
                                  " in move calculation\n" << endl;
      
      return false; }  
    } // switch
  } // calculate startRow and startFile

  traceMove(pgn, agent, startFile, startRow, endFile, endRow, captures);
  return true;
}

Chessgame* ParseFile( ifstream* file )
{
  // this version only supports pgn format
  Chessgame * result = new Chessgame( 0, 0 );
  string line, key, value, pgn, pgntmp;
  char inChar=' ', startFile=' ', endFile=' ';
  int startRow=0, endRow=0, parenthesis=0;
  string::size_type pos=0, pos2=0;
  bool comment = false, nextcomment = false;
  bool skipToNextMatch = false;

  // read Tag pair section
  bool foundTagPair = false;
  while ( getline( *file, line ) && (line == "" || line[0] == 13 ));
  while ( line.find( "]", 0 ) != string::npos )
  {
    if ( line[ 0 ] == '[' )
    {
      pos = line.find( " ", 1 );
      key = line.substr( 1, pos - 1 );
      int valuelen = line.find( "\"", pos + 2 ) - pos - 2;
      value = line.substr( pos + 2, valuelen );
      result->AddMetainfoEntry( key, value );
      foundTagPair = true;
    }
    getline( *file, line );
  }
  result->SortMetainfos();
  // should only happen, if empty line after last game was read
  if ( !foundTagPair ) {
    return 0;
  }


  STRING_T white;
  STRING_T black; 
  result->GetMetainfoValue("White", &white);
  result->GetMetainfoValue("Black", &black);

  cerr << "Parsing game notation for " 
       << white << " vs. " << black << endl << endl;
  
  // read Movetext section
  int playerNo=0;
  *file >> pgn;
  while ( comment || parenthesis != 0 ||
          ( pgn != "1-0" ) && ( pgn != "0-1" ) &&
          ( pgn != "1/2-1/2" ) && ( pgn != "*" )
          && !file->eof() )
  {
    if ( comment )
    {
      if ( ( pos = pgn.find( "}", 0 ) ) != string::npos )
      {
        pgn = pgn.substr( pos + 1, pgn.length() - pos - 1 );
        comment = false;
      }
      else
      {
        *file >> pgn;
        if ( pgn == "1/2-1/2" ) break;
      }
    }
    else
    {
      if ( nextcomment )
      {
        comment = true;
        nextcomment = false;
      }
      if ( ( pos = pgn.find( "{", 0 ) ) != string::npos )
      {
        if ( ( pos2 = pgn.find( "}", pos ) ) == string::npos )
        {
          nextcomment = true;
          pgn = pgn.substr( 0, pos );
        }
        else
        {
          pgn = pgn.substr( 0, pos ) + pgn.substr( pos2 + 1,
                                                   pgn.length() - pos2 - 1 );
        }
      }
      else if ( ( pos = pgn.find( "(", 0 ) ) != string::npos )
      {
        // alternatives
        parenthesis ++;
        if ( parenthesis == 1 )
        {
          pgn = pgn.substr( 0, pos );
        }
        else
        {
          *file >> pgn;
        }
      }
      else if ( parenthesis > 0 )
      {
        if ( ( pos = pgn.find( ")", 0 ) ) != string::npos )
        {
          parenthesis--;
          if ( parenthesis <= 0 )
          {
            pgn = pgn.substr( pos + 1, pgn.length() - pos - 1 );
          }
          else
          {
            while ( ( pos = pgn.find( ")", pos + 1 ) ) != string::npos )
            {
              parenthesis--;
            }
            if ( parenthesis <= 0 && pos < pgn.length() - 1 )
            {
              pgn = pgn.substr( pos + 1, pgn.length() - pos - 1 );
            }
            else
            {
              *file >> pgn;
            }
          }
        }
        else
        {
          *file >> pgn;
          if ( pgn == "1/2-1/2" ) break;
        }
      }
      else if ( pgn.find( "...", 0 ) != string::npos )
      {
        playerNo = BLACK;
        *file >> pgn;
      }
      else if ( pgn.find( ".", 0 ) != string::npos )
      {
        playerNo = WHITE;
        pos = pgn.find( ".", 0 );
        pgn = pgn.substr( pos + 1, pgn.size() );
      }
      else if ( pgn == "" )
      {
        *file >> pgn;
      }
      else
      {
	if (!skipToNextMatch) {

	if (trace)	
	  result->GetLastPosition()->ShowBoard(cerr);

        bool ok = parseMove( startFile, startRow, endFile, 
			     endRow, playerNo, pgn,
                             result->GetLastPosition() );
       
        if (ok)
        {
          result->AddMove( startFile, startRow, endFile, endRow, pgn );
          if ( playerNo == WHITE ) playerNo = BLACK;
        } else {
	  result->AddMetainfoEntry( "Result", "Parse Error for " + pgn );
          skipToNextMatch = true;
        }
        }	
        *file >> pgn;
      }
    }
  }
  // ignore rest of current line (usually only the '\n' char)
  while ( file->get( inChar ) && inChar != '\n' ) cout << inChar;
  return result;
}

} // namespace ChessAlgebra

#endif // PST_TYPES_H
