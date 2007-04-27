/*
---- 
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science, 
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

1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"  /*needed because we 
                             return a CcBool in an op*/
#include <string>
#include <math.h>
#include "DBArray.h"
#include "StringTokenizer.h"
#include "FTextAlgebra.h"
#include "RelationAlgebra.h"
#include <stdlib.h>
#include "TemporalAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;




/*
2 Typconstructors

2.1 ChessMove

Class ChessMove

*/
class ChessMove : public IndexableStandardAttribute
{
 public:
  //Constructor
  ChessMove();
  ChessMove(bool def, char FigureName, char StartPosition, 
    char EndPosition, char Action, char Captured, 
    int MoveNr);
    
  //Destructor
  ~ChessMove() {;}
  
  ChessMove*   Clone() const;  
  
  //Methods
   const char* GetFigureName()const;
   const char* GetStartPosition()const;
   const char* GetEndPosition()const;
   const char* GetAction()const;
   const char* GetCapturedFigure()const;
   const int GetMoveNr() const;

  void SetFigureName(const char* name);
  void SetStartPosition(const char* position);
  void SetEndPosition(const char* position);
  void SetAction(const char* Action);
  void SetCapturedFigure(const char* captured);
  void SetMoveNr(const int movenr);

  bool IsDefined()const;
  void SetDefined(bool Defined); 
   
  int CompareTo(const ChessMove* P2) const;
  int Compare(const Attribute* arg) const;
  size_t HashValue() const; 
  size_t Sizeof() const;
  void Equalize(const ChessMove* move);
  void CopyFrom(const StandardAttribute* arg);
  bool Adjacent(const Attribute*) const;
  SmiSize SizeOfChars() const;
  bool ReadFrom(const string ChessMove);
  void ReadFrom( const char *src );
  string ToString() const;
  void WriteTo( char *dest )const;
  
  int startrow();
  int endrow();
  bool check();
  bool captures();
  
 private:
   /*Figure which is moved is in figureName[0], 
     figureName[1] contains the endsymbol*/
   char figureName [2];
   /*Start position of figure
     startPosition[0] contains the row, 
     startPosition[1] contains the file, 
     startPosition[2] contains the endsymbol 
   */
   char startPosition [3];
   /*End position of figure
     endPosition[0] contains the row, 
     endPosition[1] contains the file, 
     endPosition[2] contains the endsymbol 
   */
   char endPosition [3];
   /* action 
      action [0]: c = check; m = checkmate; -  = nothing
      action [1]: P,B,N,R,Q,K,p,b,n,r,q,k = promotion figure;
                  - = nothing 
      action [2]: s = short castling; l = long castling;
                  - = nothing
      action [3]: endsymbol
   */
   char action [4];
   /*name of figure what was captured is in capturedFigure[0], 
     capturedFigure[1] contains the endsymbol*/
   char capturedFigure [2];
   int moveNr;
   bool defined;     
};

ChessMove::ChessMove() {
    defined = true;
}

ChessMove::ChessMove(bool def, char FigureName, char StartPosition, 
           char EndPosition, char Action, char Captured, int MoveNr)
{
    defined = def;
    void SetFigureName (char* FigureName);
    void SetStartPosition (char* StartPosition);
    void SetEndPosition (char* EndPosition);
    void SetAction (char* Action);
    void SetCapturedFigure (char* Captured);
    void SetMoveNr (int MoveNr);
//    cout << "in chessmove with moveNr" << endl;
}

//Methoddefinitions
const char* ChessMove::GetFigureName() const 
{     
    return figureName;
}

const char* ChessMove::GetStartPosition() const {
    return startPosition;
}

const char* ChessMove::GetEndPosition() const {
    return endPosition;
}

const char* ChessMove::GetAction() const {
    return action;
}

const int ChessMove::GetMoveNr() const {
    return moveNr;
}

const char* ChessMove::GetCapturedFigure() const 
{
    if ( capturedFigure[0] == 'P' || capturedFigure[0] == 'B' || 
         capturedFigure[0] == 'N' || capturedFigure[0] == 'R' || 
         capturedFigure[0] == 'Q' || capturedFigure[0] == 'K' ||
         capturedFigure[0] == 'p' || capturedFigure[0] == 'b' || 
         capturedFigure[0] == 'n' || capturedFigure[0] == 'r' || 
         capturedFigure[0] == 'q' || capturedFigure[0] == 'k' )
    {
         return capturedFigure;
    }
    else
         return "-";
}

bool ChessMove::IsDefined() const {
    return defined;
}

void ChessMove::SetDefined(bool Defined) {
    defined = Defined;
}

void ChessMove::SetFigureName(const char* name)
{     
    if ( name[0] == 'P' || name[0] == 'B' || name[0] == 'N' ||
             name[0] == 'R' || name[0] == 'Q' || name[0] == 'K' ||
         name[0] == 'p' || name[0] == 'b' || name[0] == 'n' ||
         name[0] == 'r' || name[0] == 'q' || name[0] == 'k' )
    {
        figureName[0] = name[0];
        figureName[1] = '\0';
    }
    else
    {
        //defined = false;
        figureName[0] = '-';
        figureName[1] = '\0';
    }
}

void ChessMove::SetStartPosition(const char* position)
{    
    if ( ( position[0] == 'a' || position[0] == 'b' || 
           position[0] == 'c' || position[0] == 'd' ||
           position[0] == 'e' || position[0] == 'f' || 
           position[0] == 'g' || position[0] == 'h' ) &&
         ( position[1] == '1' || position[1] == '2' || 
           position[1] == '3' || position[1] == '4' ||
           position[1] == '5' || position[1] == '6' || 
           position[1] == '7' || position[1] == '8' ) )
    {
        startPosition[0] = position[0];
        startPosition[1] = position[1];
        startPosition[2] = '\0';
    }
    else 
    {
        startPosition[0] = '-';
        startPosition[1] = '-';
        startPosition[2] = '\0';
    }
}

void ChessMove::SetEndPosition(const char* position)
{
    if ( ( position[0] == 'a' || position[0] == 'b' || 
           position[0] == 'c' || position[0] == 'd' ||
           position[0] == 'e' || position[0] == 'f' || 
           position[0] == 'g' || position[0] == 'h' ) &&
         ( position[1] == '1' || position[1] == '2' || 
           position[1] == '3' || position[1] == '4' ||
           position[1] == '5' || position[1] == '6' || 
           position[1] == '7' || position[1] == '8' ) )
    {
        endPosition[0] = position[0];
        endPosition[1] = position[1];
        endPosition[2] = '\0';
    }
    else
    {
        endPosition[0] = '-';
        endPosition[1] = '-';
        endPosition[2] = '\0';
    }
}
  
void ChessMove::SetAction(const char* Action){
    action[0] = Action[0];
    action[1] = Action[1];
    action[2] = Action[2];
    action[3] = '\0';

}
  
void ChessMove::SetMoveNr(const int MoveNr){
    moveNr = MoveNr;
}

void ChessMove::SetCapturedFigure(const char* captured)
{
    if ( captured[0] == 'P' || captured[0] == 'B' || 
         captured[0] == 'N' || captured[0] == 'R' || 
         captured[0] == 'Q' || captured[0] == 'K' ||
         captured[0] == 'p' || captured[0] == 'b' || 
         captured[0] == 'n' || captured[0] == 'r' || 
         captured[0] == 'q' || captured[0] == 'k' /*||
         captured[0] == '-' || captured[0] == ' '*/ )
    {
        capturedFigure[0] = captured[0];
        capturedFigure[1] = '\0';
    }
    else
    {
        //defined = false;
        capturedFigure[0] = '-';
        capturedFigure[1] = '\0';
    }
}

/*
~HashValue~

This function returns the HashValue for this ChessMove instance.

*/
size_t ChessMove::HashValue() const
{
    int hashvalue;
    string hashstring;
    hashstring = figureName[0] + startPosition[0] + startPosition[1] 
        + endPosition[0]  + endPosition[1] + action[0] + action[1] 
    + action[2] + capturedFigure[0] + moveNr;
    for (int i = 0; i < 10; i++)
    {
       hashvalue = hashvalue + (int) hashstring[i];
    }
//    hashvalue = hashvalue % 10;
    return (size_t) hashvalue;
}
 
/*
~Clone~

This funtion returns a copy of this instance.

*/
ChessMove* ChessMove::Clone() const {
    return new ChessMove( *this );
}

/*
~Sizeof~

The function ~Sizeof~ returns the ~sizeof~ of the instance 
of the ~ChessMove~ class.

*/
size_t ChessMove::Sizeof() const{
    return sizeof(ChessMove);
}

/*
~Equalize~

This function changes the value of this ChessMove instance to be equal to
move.

*/
void ChessMove::Equalize(const ChessMove* move)
{
   defined = move->defined;
   figureName[0] = move->figureName[0];
   figureName[1] = move->figureName[1];
   startPosition[0] = move->startPosition[0];
   startPosition[1] = move->startPosition[1];
   startPosition[2] = move->startPosition[2];
   endPosition[0] = move->endPosition[0];
   endPosition[1] = move->endPosition[1];
   endPosition[2] = move->endPosition[2];
   for ( int i = 0; i < 4; i++ )
   {
      action[i] = move->action[i];
   }
   capturedFigure[0] = move->capturedFigure[0];
   capturedFigure[1] = move->capturedFigure[1];
   moveNr = move->moveNr;
}

/*
~CopyFrom~

This ChessMove instance takes its value from arg if this function is
called.

*/
void ChessMove::CopyFrom(const StandardAttribute* arg){
   Equalize((const ChessMove*) arg);
}

/*
~CompareTo~

This function compares this ChessMove instance with another one.
The result will be:

  * -1 if this instance is before P2

  * 0 if this instance is equals to P2

  * 1 if this instance is after P2

The types of the arguments has to be equals.

*/
int ChessMove::CompareTo(const ChessMove* P2) const
{
  const ChessMove* P1 = (const ChessMove* )(P2);
  if(!IsDefined() && !P1->IsDefined())
      return 0;
   if(!IsDefined() && P1->IsDefined())
      return -1;
   if(IsDefined() && !P1->IsDefined())
      return 1;
   if((figureName[0] == ((P1->GetFigureName())[0]))     &&
      (figureName[1] == ((P1->GetFigureName())[1]))      &&
      (startPosition[0] == ((P1->GetStartPosition())[0])) &&
      (startPosition[1] == ((P1->GetStartPosition())[1])) &&
      (startPosition[2] == ((P1->GetStartPosition())[2])) &&
      (endPosition[0] == ((P1->GetEndPosition())[0])) &&
      (endPosition[1] == ((P1->GetEndPosition())[1])) &&
      (endPosition[2] == ((P1->GetEndPosition())[2])) &&
      (action[0] == ((P1->GetAction())[0])) &&          
      (action[1] == ((P1->GetAction())[1])) &&          
      (action[2] == ((P1->GetAction())[2])) &&          
      (action[3] == ((P1->GetAction())[3])) &&
      (capturedFigure[0] == ((P1->GetCapturedFigure())[0])) &&
      (capturedFigure[1] == ((P1->GetCapturedFigure())[1])) &&
      (moveNr == (P1->GetMoveNr()))       
     ) return 0;
     
   return -1;
}

/*
~Compare~

This function compares this ChessMove with the given Attribute.

*/
int ChessMove::Compare(const Attribute* arg) const{
  return CompareTo( (const ChessMove*) arg);
}

/*
~ToString~

This function returns the string representation of this ChessMove instance.

*/
string ChessMove::ToString() const
{
   ostringstream tmp;
  if(!defined)
  {
    tmp << "undefined";
  }
  tmp << (GetFigureName())[0] << "," << (GetStartPosition())[0] << 
     (GetStartPosition())[1] << "," <<  (GetEndPosition())[0] << 
     (GetEndPosition())[1] << "," << (GetAction())[0] << 
     (GetAction())[1]  << (GetAction())[2] << "," << 
     (GetCapturedFigure())[0] << "," << (GetMoveNr());

  return tmp.str(); 
}


void ChessMove::WriteTo( char *dest ) const
{
  strcpy( dest, ToString().c_str() );
}

/*
~Adjacent~

This function returns true if the moveNr from this is 
one more or one less than the moveNr from arg.

*/
bool ChessMove::Adjacent(const Attribute* arg) const{
    const ChessMove* move2 = (const ChessMove*) arg;
      if( defined && move2->defined &&
         ( moveNr == move2->moveNr + 1 ||
           moveNr == move2->moveNr - 1 ) )
            return true;
      return false;
}

SmiSize ChessMove::SizeOfChars() const
{                                               
    return ToString().length();  
}

/*
~ReadFrom~

This function reads the ChessMove from a given string. If the string
don't represent a valid ChessMove value, this instance remains unchanged
and false is returned. In the other case, this instance will hold the
ChessMove represented by the string and the result is true.
The format of the string must be:
    figureName,startPosition,endPosition,action,capturedFigure
Spaces are not allowed in this string.

*/
bool ChessMove::ReadFrom(const string move)
{
   cout << "in chessmove::readfrom: string move: " 
        << move << endl;
   if( move == "undefined" )
   {
      defined=false;
      return false;
   }
   int len = move.length();
   if( len < 15 ) return false;
   if ( move[1] == ',' && move[4] == ',' && move[7] == ',' && 
        move[11] == ',' && move[13] == ',' )
   { 
      char*figure = new char[2];
      figure[0] = move[0];
      figure[1] = '\0';
      char* start = new char[3];
      start[0] = move[2];
      start[1] = move[3];
      start[2] = '\0';
      char* end = new char[3];
      end[0] = move[5];
      end[1] = move[6];
      end[2] = '\0';
      char* act = new char[5];
      act[0] = move[8];
      act[1] = move[9];
      act[2] = move[10];
      act[3] = '\0';      
      char* captured = new char[2];
      captured[0] = move[12];
      captured[1] = '\0';
      int nr = 0;
      int pos = 14;
      
//      int* testint;
//      testint = (int*)move[14];
//      cout << "in chessmove::readfrom before while: testint " << 
//             testint << endl;
     while ( move[pos] != '\0' )
      {
         int digit = int(move[pos]) - 48;
//     cout << "in chessmove::readfrom in while: 
//              digit: " << digit << endl;
     nr = (10 * nr) + digit;
         pos++;
      }
     cout << "in chessmove::readfrom after while: nr: " 
          << nr << endl;
     
       // At this place we have all needed information 
       // to create a ChessMove
       this->figureName[0] = figure[0];
       this->figureName[1] = figure[1];
       this->startPosition[0] = start[0];
       this->startPosition[1] = start[1];
       this->startPosition[2] = start[2];
       this->endPosition[0] = end[0];
       this->endPosition[1] = end[1];
       this->endPosition[2] = end[2];
     cout << "in chessmove::readfrom before for " << endl;
       for ( int i = 0; i < 4; i++ )
       {
          this->action[i] = act[i];
       }
       this->capturedFigure[0] = captured[0];
       this->capturedFigure[1] = captured[1];
       this->moveNr = nr;
       this->defined = true;
    cout << "come out chessmove::readfrom then" << endl;
    
       delete[] figure;
       delete[] start;
       delete[] end;
       delete[] act;
       delete[] captured;
    
       return true;
    }
    else
    {
     cout << "come out chessmove::readfrom else" << endl;
      defined = false;
       return false;
    }
}

void ChessMove::ReadFrom( const char *src )
{
  ReadFrom( string(src) );
}

/*
Property Function of ChessMove

*/
ListExpr
ChessMoveProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList
           (nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
        nl->StringAtom("Example List"),
        nl->StringAtom("Remarks")),
            nl->FiveElemList
           (nl->StringAtom("-> DATA"),
            nl->StringAtom("chessmove"),
        nl->StringAtom("(<FName><SPos><EPos>"
                       "<Action><cap><MNr>)"),
        nl->StringAtom("(\"N\" \"g1\" \"f3\""
                       " \"c--\" \"k\" 3)"),
        nl->StringAtom("represents a move with "
                       "five char[]+int"))));
}

/*
~In~ and ~Out~ functions

*/
ListExpr
OutChessMove( ListExpr typeInfo, Word value )
{ 
  if( ((ChessMove*)value.addr)->IsDefined() )
  { 
    ChessMove* move = (ChessMove*)(value.addr);
    return nl->SixElemList
       (nl->StringAtom(move->GetFigureName()),
        nl->StringAtom(move->GetStartPosition()),
    nl->StringAtom(move->GetEndPosition()),
    nl->StringAtom(move->GetAction()),
    nl->StringAtom(move->GetCapturedFigure()),
    nl->IntAtom(move->GetMoveNr()) );
  }
  else
    return (nl->SymbolAtom("undef"));
}

Word
InChessMove( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{ 
  if ( (nl->ListLength( instance ) == 1) && (nl->IsAtom(instance)) && 
       (nl->AtomType(instance) == SymbolType) && 
       (nl->SymbolValue(instance) == "undef") )
  {
       correct = true;
       ChessMove* newmove = new ChessMove
                  (false, '-', '-', '-', '-', '-', 0);
       return SetWord(newmove);
  }
  else if ( nl->ListLength( instance ) == 6 )
  { 
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);
    ListExpr Fourth = nl->Fourth(instance);
    ListExpr Fifth  = nl->Fifth(instance);
    ListExpr Sixth  = nl->Sixth(instance);
    if ( nl->IsAtom(First) && 
         (nl->AtomType(First) == StringType) &&
         ( nl->StringValue(First) == "P" || 
       nl->StringValue(First) == "B" || 
       nl->StringValue(First) == "N" || 
       nl->StringValue(First) == "R" || 
       nl->StringValue(First) == "Q" || 
       nl->StringValue(First) == "K" ||
       nl->StringValue(First) == "p" || 
       nl->StringValue(First) == "b" || 
       nl->StringValue(First) == "n" || 
       nl->StringValue(First) == "r" || 
       nl->StringValue(First) == "q" || 
       nl->StringValue(First) == "k" ) &&
         nl->IsAtom(Second) && 
     (nl->AtomType(Second) == StringType) &&
     ( (nl->StringValue(Second))[0] == 'a' || 
       (nl->StringValue(Second))[0] == 'b' || 
       (nl->StringValue(Second))[0] == 'c' || 
       (nl->StringValue(Second))[0] == 'd' ||
       (nl->StringValue(Second))[0] == 'e' || 
       (nl->StringValue(Second))[0] == 'f' ||
       (nl->StringValue(Second))[0] == 'g' || 
       (nl->StringValue(Second))[0] == 'h' ) &&
     ( (nl->StringValue(Second))[1] == '1' || 
       (nl->StringValue(Second))[1] == '2' || 
       (nl->StringValue(Second))[1] == '3' || 
       (nl->StringValue(Second))[1] == '4' ||
       (nl->StringValue(Second))[1] == '5' || 
       (nl->StringValue(Second))[1] == '6' ||
       (nl->StringValue(Second))[1] == '7' || 
       (nl->StringValue(Second))[1] == '8' ) &&
     nl->IsAtom(Third) && (nl->AtomType(Third) == StringType) &&
     ( (nl->StringValue(Third))[0] == 'a' || 
       (nl->StringValue(Third))[0] == 'b' || 
       (nl->StringValue(Third))[0] == 'c' || 
       (nl->StringValue(Third))[0] == 'd' ||
       (nl->StringValue(Third))[0] == 'e' || 
       (nl->StringValue(Third))[0] == 'f' ||
       (nl->StringValue(Third))[0] == 'g' || 
       (nl->StringValue(Third))[0] == 'h' ) &&
     ( (nl->StringValue(Third))[1] == '1' || 
       (nl->StringValue(Third))[1] == '2' || 
       (nl->StringValue(Third))[1] == '3' || 
       (nl->StringValue(Third))[1] == '4' ||
       (nl->StringValue(Third))[1] == '5' || 
       (nl->StringValue(Third))[1] == '6' ||
       (nl->StringValue(Third))[1] == '7' || 
       (nl->StringValue(Third))[1] == '8' ) &&
         nl->IsAtom(Fourth) && 
     (nl->AtomType(Fourth) == StringType) &&
     ( (nl->StringValue(Fourth))[0] == 'c' || 
       (nl->StringValue(Fourth))[0] == 'm' || 
       (nl->StringValue(Fourth))[0] == '-' || 
       (nl->StringValue(Fourth))[0] == ' ' ) &&
     ( (nl->StringValue(Fourth))[1] == 'P' || 
       (nl->StringValue(Fourth))[1] == 'B' || 
       (nl->StringValue(Fourth))[1] == 'N' || 
       (nl->StringValue(Fourth))[1] == 'R' ||
       (nl->StringValue(Fourth))[1] == 'Q' || 
       (nl->StringValue(Fourth))[1] == 'K' || 
       (nl->StringValue(Fourth))[1] == 'p' || 
       (nl->StringValue(Fourth))[1] == 'b' ||
       (nl->StringValue(Fourth))[1] == 'n' || 
       (nl->StringValue(Fourth))[1] == 'r' || 
       (nl->StringValue(Fourth))[1] == 'q' || 
       (nl->StringValue(Fourth))[1] == 'k' ||
       (nl->StringValue(Fourth))[1] == '-' || 
       (nl->StringValue(Fourth))[1] == ' ' ) &&
     ( (nl->StringValue(Fourth))[2] == 's' || 
       (nl->StringValue(Fourth))[2] == 'l' || 
       (nl->StringValue(Fourth))[2] == '-' || 
       (nl->StringValue(Fourth))[2] == ' ' ) &&
     nl->IsAtom(Fifth) && (nl->AtomType(Fifth) == StringType) && 
     ( nl->StringValue(Fifth) == "P" || 
       nl->StringValue(Fifth) == "B" || 
       nl->StringValue(Fifth) == "N" || 
       nl->StringValue(Fifth) == "R" || 
       nl->StringValue(Fifth) == "Q" || 
       nl->StringValue(Fifth) == "K" ||
       nl->StringValue(Fifth) == "p" || 
       nl->StringValue(Fifth) == "b" || 
       nl->StringValue(Fifth) == "n" || 
       nl->StringValue(Fifth) == "r" || 
       nl->StringValue(Fifth) == "q" || 
       nl->StringValue(Fifth) == "k" ||
       nl->StringValue(Fifth) == "-" || 
       nl->StringValue(Fifth) == " " || 
       nl->StringValue(Fifth) == "none" ) &&
     nl->IsAtom(Sixth) && (nl->AtomType(Sixth) == IntType))
    {  
      correct = true;
      string figurename = nl->StringValue(First);
      string startposition = nl->StringValue(Second);
      string endposition = nl->StringValue(Third);
      string action = nl->StringValue(Fourth);
      string capturedfigure;
      if ( nl->StringValue(Fifth) == "none" )
          capturedfigure = "-";
      else
          capturedfigure = nl->StringValue(Fifth);
      int movenr = nl->IntValue(Sixth);
      ChessMove* newmove = new ChessMove ();
      newmove->SetFigureName(figurename.c_str());
      newmove->SetStartPosition(startposition.c_str());
      newmove->SetEndPosition(endposition.c_str());
      newmove->SetAction(action.c_str());
      newmove->SetCapturedFigure(capturedfigure.c_str());
      newmove->SetMoveNr(movenr);
      return SetWord(newmove);
    }
   }
   correct = false;
   cout << " Wrong range of values! Move ";
   return SetWord(Address(0));
}

/*
~Cast~ function

*/
void* CastChessMove( void* addr )
{
   return (new (addr) ChessMove);
}

/*
~Create~ function

*/
Word CreateChessMove( const ListExpr typeInfo )
{
    return (SetWord( new ChessMove( )));
}

/*
~Delete~ function

*/
void DeleteChessMove( const ListExpr typeInfo, Word& w )
{
    delete (ChessMove *)w.addr;
    w.addr = 0;
}

/*
~Open~ function
 
*/
bool OpenChessMove( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
 ChessMove *m =
      (ChessMove*)Attribute::Open( valueRecord, offset, typeInfo );
 value = SetWord( m );
 return true;
}

/*
~Save~ function

*/
bool SaveChessMove( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
 ChessMove *m = (ChessMove *)value.addr;
 Attribute::Save( valueRecord, offset, typeInfo, m );
 return true;
}

/*
~Close~ function

*/
void CloseChessMove( const ListExpr typeInfo, Word& w )
{
    delete (ChessMove *)w.addr;
    w.addr = 0;
}

/*
~Clone~ function

*/
Word CloneChessMove( const ListExpr typeInfo, const Word& w )
{
    return SetWord( ((ChessMove *)w.addr)->Clone() );
}

/*
~SizeOf~ function

*/
int SizeOfChessMove()
{
    return sizeof(ChessMove);
}

/*
Kind Checking Function

*/
bool CheckChessMove( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "chessmove" ));
}

/*
Type Constructor

*/
TypeConstructor chessmove( "chessmove",
               ChessMoveProperty,
                           OutChessMove, InChessMove,
                           0, 0, // SaveToList and RestoreFromList
               CreateChessMove, DeleteChessMove,
                   /*OpenChessMove, SaveChessMove, */ 0,0,
                           CloseChessMove, CloneChessMove,
               CastChessMove,        
                           SizeOfChessMove,   
                           CheckChessMove);


//---------------------------------------------------------------

/*
2.2 ChessMaterial

Class ChessMaterial

*/
class ChessMaterial : public IndexableStandardAttribute 
{
  public:
    //Constructor
    ChessMaterial();
    ChessMaterial(bool def, int WhitePlayer[6], 
                  int BlackPlayer[6]);
    
    //Destructor
    ~ChessMaterial(){;}
    
    ChessMaterial* Clone() const;
    
    //Methods
     int GetWhitePawns ()const;
     int GetBlackPawns ()const;
     int GetWhiteKnights ()const;
     int GetBlackKnights ()const;
     int GetWhiteBishops ()const;
     int GetBlackBishops ()const;
     int GetWhiteRooks ()const;
     int GetBlackRooks ()const;
     int GetWhiteQueens ()const;
     int GetBlackQueens ()const;
     int GetWhiteKings ()const;
     int GetBlackKings ()const;
     void SetWhitePawns (int anzahl){whitePlayer[0] = anzahl;}
     void SetBlackPawns (int anzahl){blackPlayer[0] = anzahl;}
     void SetWhiteKnights (int anzahl){whitePlayer[1] = anzahl;}
     void SetBlackKnights (int anzahl){blackPlayer[1] = anzahl;}
     void SetWhiteBishops (int anzahl){whitePlayer[2] = anzahl;}
     void SetBlackBishops (int anzahl){blackPlayer[2] = anzahl;}
     void SetWhiteRooks (int anzahl){whitePlayer[3] = anzahl;}
     void SetBlackRooks (int anzahl){blackPlayer[3] = anzahl;}
     void SetWhiteQueens (int anzahl){whitePlayer[4] = anzahl;}
     void SetBlackQueens (int anzahl){blackPlayer[4] = anzahl;}
     void SetWhiteKings (int anzahl){whitePlayer[5] = anzahl;}
     void SetBlackKings (int anzahl){blackPlayer[5] = anzahl;}
     void SetDefined(bool Defined);
     bool IsDefined()const;
     int CompareTo(const ChessMaterial* P2) const;
     int Compare(const Attribute* arg) const;
    
     size_t Sizeof() const;
     void Equalize(const ChessMaterial* material);
     void CopyFrom(const StandardAttribute* arg);
     bool Adjacent(const Attribute*) const;
     SmiSize SizeOfChars() const;
     bool ReadFrom(const string ChessMaterial);
     void ReadFrom( const char *src );
     size_t HashValue() const; 
     string ToString() const;
     void WriteTo( char *dest )const;
     
     bool chessequal(const ChessMaterial material);
     bool chesslower(const ChessMaterial material);
     
  private:
    // 0 = pawn
    // 1 = knight
    // 2 = bishop
    // 3 = rook
    // 4 = queen
    // 5 = king
    int whitePlayer[6];
    int blackPlayer[6];
    bool defined;
};

//Method definitions
ChessMaterial::ChessMaterial() {
    defined = true;
}

ChessMaterial::ChessMaterial(bool def, int WhitePlayer[6], 
               int BlackPlayer[6]){
//    cout << " in constructor chessmaterial " << endl;
    defined = def;
    for (int i = 0; i < 6; i++)
    {
       whitePlayer[i] = WhitePlayer[i];
       blackPlayer[i] = BlackPlayer[i];
    }
}

int ChessMaterial::GetWhitePawns() const{
    return whitePlayer[0];
}

int ChessMaterial::GetBlackPawns() const{
    return blackPlayer[0];
}

int ChessMaterial::GetWhiteKnights() const{
    return whitePlayer[1];
}

int ChessMaterial::GetBlackKnights() const{
    return blackPlayer[1];
}

int ChessMaterial::GetWhiteBishops() const{
    return whitePlayer[2];
}

int ChessMaterial::GetBlackBishops() const{
    return blackPlayer[2];
}

int ChessMaterial::GetWhiteRooks() const{
    return whitePlayer[3];
}

int ChessMaterial::GetBlackRooks() const{
    return blackPlayer[3];
}

int ChessMaterial::GetWhiteQueens() const{
    return whitePlayer[4];
}

int ChessMaterial::GetBlackQueens() const{
    return blackPlayer[4];
}

int ChessMaterial::GetWhiteKings() const{
    return whitePlayer[5];
}

int ChessMaterial::GetBlackKings() const{
    return blackPlayer[5];
}

bool ChessMaterial::IsDefined() const{
    return defined;
}

void ChessMaterial::SetDefined(bool Defined) {
    defined = Defined;
}

/*
~Sizeof~

The function ~Sizeof~ returns the ~sizeof~ of the instance 
of the ~ChessMaterial~ class.

*/
size_t ChessMaterial::Sizeof() const{
    return sizeof(ChessMaterial);
}

/*
~Equalize~

This function changes the value of this ChessMaterial instance to 
be equal to material.

*/
void ChessMaterial::Equalize(const ChessMaterial* material)
{
   defined = material->defined;
   for ( int i = 0; i < 6; i++ )
   {
      whitePlayer[i] = material->whitePlayer[i];
      blackPlayer[i] = material->blackPlayer[i];
   }
}

/*
~CopyFrom~

This ChessMaterial instance takes its value from arg if this function is
called.

*/
void ChessMaterial::CopyFrom(const StandardAttribute* arg){
   Equalize((const ChessMaterial*) arg);
}

/*
~Adjacent~

This function returns true if the count from this is 
one more or one less than count from arg.

*/
bool ChessMaterial::Adjacent(const Attribute* arg) const{
    const ChessMaterial* material2 = (const ChessMaterial*) arg;
    int diff = 0;
    for ( int i = 0; i < 6; i++ )
    {
        if ( whitePlayer[i] < material2->whitePlayer[i] )
             diff = diff + material2->whitePlayer[i] 
                - whitePlayer[i];
        else
             diff = diff + whitePlayer[i] 
                - material2->whitePlayer[i];
        if ( blackPlayer[i] < material2->blackPlayer[i] )
             diff = diff + material2->blackPlayer[i] 
                - blackPlayer[i];
        else
             diff = diff + blackPlayer[i] 
                - material2->blackPlayer[i];
    }
      if( defined && material2->defined && diff == 1 )
        return true;
      return false;
}

SmiSize ChessMaterial::SizeOfChars() const
{                                               
    return ToString().length();  
}

/*
~ReadFrom~

This function reads the ChessMaterial from a given string. If the string
don't represent a valid ChessMaterial value, this instance remains unchanged
and false is returned. In the other case, this instance will hold the
ChessMaterial represented by the string and the result is true.
The format of the string must be:
    WhitePawns,WhiteKnights,WhiteBishops,WhiteRooks,WhiteQueens,WhiteKings,
    BlackPawns,BlackKnights,BlackBishops,BlackRooks,BlackQueens,BlackKings    
Spaces are not allowed in this string.

*/
bool ChessMaterial::ReadFrom(const string material)
{
   if( material == "undefined" )
   {
      defined=false;
      return false;
   }
   
   int len = material.length();
   if( len != 23 ) return false;
   if ( material[1] == ',' && material[3] == ',' && 
        material[5] == ',' && material[7] == ',' &&
        material[9] == ',' && material[11] == ',' && 
    material[13] == ',' && material[15] == ',' &&
    material[17] == ',' && material[19] == ',' && 
    material[21] == ',' )
   { 
      int*whitePlayer = new int[6];
      int*blackPlayer = new int[6];
      whitePlayer[0] = material[0];
      whitePlayer[1] = material[2];
      whitePlayer[2] = material[4];
      whitePlayer[3] = material[6];
      whitePlayer[4] = material[8];
      whitePlayer[5] = material[10];
      
      blackPlayer[0] = material[12];
      blackPlayer[1] = material[14];
      blackPlayer[2] = material[16];
      blackPlayer[3] = material[18];
      blackPlayer[4] = material[20];
      blackPlayer[5] = material[22];
      
      // At this place we have all needed information 
      // to create a ChessMaterial
      for ( int i = 0; i < 6; i++ )
      {
          this->whitePlayer[i] = whitePlayer[i];
          this->blackPlayer[i] = blackPlayer[i];
      }
      this->defined = true;
      
      delete[] whitePlayer;
      delete[] blackPlayer;
      
      return true;
    }
    else
    {
       defined=false;
       return false;
    }
}

void ChessMaterial::ReadFrom( const char *src )
{
  ReadFrom( string(src) );
}

/*
~HashValue~

This function returns the HashValue for this ChessMaterial instance.

*/
size_t ChessMaterial::HashValue() const
{
    int hashvalue = 0;
    for (int i = 0; i < 6; i++)
    {
       hashvalue = hashvalue + whitePlayer[i] + blackPlayer[i];
    }
//    hashvalue = hashvalue % 6;
    return (size_t) hashvalue;
}

/*
~Clone~

This funtion returns a copy of this instance.

*/
ChessMaterial* ChessMaterial::Clone() const {
    return new ChessMaterial( *this );
}

/*
~CompareTo~

This function compares this ChessMaterial instance with another one.
The result will be:

  * 0 if this instance is equals to P2
  
  * -1 if this instance is not equals P2
  
  * 1 if this instance is defined and P2 is not defined

The types of the arguments has to be equals.

*/
int ChessMaterial::CompareTo(const ChessMaterial* P2) const
{
  const ChessMaterial* P1 = (const ChessMaterial* )(P2);
  if(!IsDefined() && !P1->IsDefined())
      return 0;
   if(!IsDefined() && P1->IsDefined())
      return -1;
   if(IsDefined() && !P1->IsDefined())
      return 1;

   if((whitePlayer[0] == (P1->GetWhitePawns())) &&
      (whitePlayer[1] == (P1->GetWhiteKnights())) &&
      (whitePlayer[2] == (P1->GetWhiteBishops())) &&
      (whitePlayer[3] == (P1->GetWhiteRooks())) &&           
      (whitePlayer[4] == (P1->GetWhiteQueens())) &&           
      (whitePlayer[5] == (P1->GetWhiteKings())) &&   
      (blackPlayer[0] == (P1->GetBlackPawns())) &&
      (blackPlayer[1] == (P1->GetBlackKnights())) &&
      (blackPlayer[2] == (P1->GetBlackBishops())) &&
      (blackPlayer[3] == (P1->GetBlackRooks())) &&           
      (blackPlayer[4] == (P1->GetBlackQueens())) &&           
      (blackPlayer[5] == (P1->GetBlackKings()))        
     ) return 0;
     
   return -1;
}

/*
~Compare~

This function compares this ChessMaterial with the given Attribute.

*/
int ChessMaterial::Compare(const Attribute* arg) const{
  return CompareTo( (const ChessMaterial*) arg);
}

/*
~ToString~

This function returns the string representation of this ChessMaterial instance.

*/
string ChessMaterial::ToString() const
{
  ostringstream tmp;
  if(!defined)
  {
    tmp << "undefined";
  }
  tmp << GetWhitePawns() << "," << GetWhiteKnights() << "," << 
         GetWhiteBishops() << "," << GetWhiteRooks() << "," << 
     GetWhiteQueens() << "," << GetWhiteKings() << "," << 
     GetBlackPawns() << "," << GetBlackKnights() << "," << 
     GetBlackBishops() << "," << GetBlackRooks()<< "," << 
     GetBlackQueens() << "," << GetBlackKings() ;
  return tmp.str(); 
}

void ChessMaterial::WriteTo( char *dest ) const
{
  strcpy( dest, ToString().c_str() );
}

/*
Property Function of ChessMaterial

*/
ListExpr
ChessMaterialProperty()
{
  return 
   (nl->TwoElemList(
       nl->FiveElemList
           (nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List"),
        nl->StringAtom("Remarks")),
       nl->FiveElemList
           (nl->StringAtom("-> DATA"),
        nl->StringAtom("chessmaterial"),
        nl->StringAtom("((P(w)(b))(N(w)(b))...(K(w)(b)))"),
        nl->StringAtom("((3 4)(1 0)...(1 0)(1 1))"),
        nl->StringAtom("represents the numbers of all pieces "
               "in the game"))));
}

/*
~In~ and ~Out~ functions

*/
ListExpr
OutChessMaterial( ListExpr typeInfo, Word value )
{ 
  //cout << "in OutChessMaterial " << endl;
  if( ((ChessMaterial*)value.addr)->IsDefined() )
  {
    //cout << "in if " << endl;
    ChessMaterial* material = (ChessMaterial*)(value.addr);
    ListExpr result = nl->OneElemList
                    (nl->TwoElemList
             ( nl->IntAtom(material->GetWhitePawns()),
                           nl->IntAtom(material->GetBlackPawns()) ));
    ListExpr last = result;
    last = nl->Append( last, nl->TwoElemList
                   (nl->IntAtom(material->GetWhiteKnights()),
                        nl->IntAtom(material->GetBlackKnights()) ) );
    last = nl->Append( last, nl->TwoElemList
                   (nl->IntAtom(material->GetWhiteBishops()),
                        nl->IntAtom(material->GetBlackBishops()) ) );
    last = nl->Append( last, nl->TwoElemList
                   ( nl->IntAtom(material->GetWhiteRooks()),
                         nl->IntAtom(material->GetBlackRooks())  ) );
    last = nl->Append( last, nl->TwoElemList
                   ( nl->IntAtom(material->GetWhiteQueens()),
                         nl->IntAtom(material->GetBlackQueens()) ) );
    last = nl->Append( last, nl->TwoElemList
                   ( nl->IntAtom(material->GetWhiteKings()),
                         nl->IntAtom(material->GetBlackKings()) ) );
    //cout << "come out OutChessMaterial 1" << endl;
    return result;
  }
  else
  {
    //cout << "come out OutChessMaterial 2" << endl;
    return (nl->SymbolAtom("undef"));
  }
}

Word
InChessMaterial( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{ 
  ListExpr first;
  ListExpr rest = instance;
  first = nl->First(rest);
  rest = nl->Rest(rest);
  
  int whitePlayer[6], blackPlayer[6];
  //cout << "in InChessMaterial-listlength:" << 
  //        nl->ListLength( first ) << endl;
  if ( (nl->ListLength( instance ) == 1) && (nl->IsAtom(instance)) && 
             (nl->AtomType(instance) == SymbolType) && 
         (nl->SymbolValue(instance) == "undef") )
  {
       correct = true;
       ChessMaterial* newmaterial = new ChessMaterial(false, 
                                      whitePlayer, blackPlayer);
       return SetWord(newmaterial);
  }
  else if ( nl->ListLength( first ) == 2 && 
       nl->IsAtom(nl->First(first)) && 
       nl->AtomType(nl->First(first)) == IntType &&
       nl->IsAtom(nl->Second(first)) && 
       nl->AtomType(nl->Second(first)) == IntType &&
       nl->IntValue(nl->First(first)) > -1 && 
       nl->IntValue(nl->First(first)) < 9 &&
       nl->IntValue(nl->Second(first)) > -1 && 
       nl->IntValue(nl->Second(first)) < 9 )
  {  
    // count pawns between 0 and 8
    whitePlayer[0] = nl->IntValue(nl->First(first));
    blackPlayer[0] = nl->IntValue(nl->Second(first)); 
    //cout << "round 0" << " content: " <<  
    //        nl->IntValue(nl->First(first)) << endl;
    //cout << "round 0" << " content: " <<  
    //        nl->IntValue(nl->Second(first)) << endl;
    for (int i = 1; i < 4; i++)
    {
        first = nl->First(rest);
        rest = nl->Rest(rest);
    if ( nl->ListLength( first ) == 2 && 
             nl->IsAtom(nl->First(first)) && 
         nl->AtomType(nl->First(first)) == IntType &&
             nl->IsAtom(nl->Second(first)) && 
         nl->AtomType(nl->Second(first)) == IntType &&
             nl->IntValue(nl->First(first)) > -1 && 
         nl->IntValue(nl->First(first)) < 3 &&
             nl->IntValue(nl->Second(first)) > -1 && 
         nl->IntValue(nl->Second(first)) < 3 )
    {
       // count knights, bishops, rooks between 0 and 2
           whitePlayer[i] = nl->IntValue(nl->First(first));
           blackPlayer[i] = nl->IntValue(nl->Second(first)); 
           //cout << "round " << i << " content: " <<  
       //        nl->IntValue(nl->First(first)) << endl;
           //cout << "round " << i << " content: " <<  
       //        nl->IntValue(nl->Second(first)) << endl;
    }
    else
    {
       cout << " Wrong range of values ! Material ";
       correct = false;
       return SetWord( Address(0));
    }
     }
     int i = 4;
     first = nl->First(rest);
     rest = nl->Rest(rest);
     if ( nl->ListLength( first ) == 2 && 
          nl->IsAtom(nl->First(first)) && 
      nl->AtomType(nl->First(first)) == IntType &&
          nl->IsAtom(nl->Second(first)) && 
      nl->AtomType(nl->Second(first)) == IntType &&
          nl->IntValue(nl->First(first)) > -1 && 
      nl->IntValue(nl->First(first)) < 2 &&
          nl->IntValue(nl->Second(first)) > -1 && 
      nl->IntValue(nl->Second(first)) < 2 )
     {
        // count queens between 0 and 1
        whitePlayer[i] = nl->IntValue(nl->First(first));
        blackPlayer[i] = nl->IntValue(nl->Second(first)); 
        //cout << "round " << i << " content: " <<  
    //        nl->IntValue(nl->First(first)) << endl;
        //cout << "round " << i << " content: " <<  
    //        nl->IntValue(nl->Second(first)) << endl;
     }
     else
     {
    cout << " Wrong range of values ! ";
    correct = false;
    return SetWord( Address(0));
     }
     i = 5;
     first = nl->First(rest);
     rest = nl->Rest(rest);
     if ( nl->ListLength( first ) == 2 && 
          nl->IsAtom(nl->First(first)) && 
      nl->AtomType(nl->First(first)) == IntType &&
          nl->IsAtom(nl->Second(first)) && 
      nl->AtomType(nl->Second(first)) == IntType &&
          nl->IntValue(nl->First(first)) > -1 && 
      nl->IntValue(nl->First(first)) < 2 &&
          nl->IntValue(nl->Second(first)) > -1 && 
      nl->IntValue(nl->Second(first)) < 2 )
     {
        // count king must be between 0 and 1 
    // (0 is possible because operator chessrange)
        whitePlayer[i] = nl->IntValue(nl->First(first));
        blackPlayer[i] = nl->IntValue(nl->Second(first)); 
        //cout << "round " << i << " content: " <<  
    //        nl->IntValue(nl->First(first)) << endl;
        //cout << "round " << i << " content: " <<  
    //        nl->IntValue(nl->Second(first)) << endl;
     }
     else
     {
    cout << " Wrong range of values ! ";
    correct = false;
    return SetWord( Address(0));
     }
     ChessMaterial* material = new ChessMaterial ( true, 
                            whitePlayer, blackPlayer );
     correct = true;
     //cout << "end of InChessMaterial" << endl;
     return SetWord(material);
  }
  else
  {
     cout << "Wrong values! ";
     correct = false;
     return SetWord( Address(0));
  }
}

/*
~Cast~ function

*/
void* CastChessMaterial( void* addr )
{
       return (new (addr) ChessMaterial);
}

/*
~Create~ function

*/
Word CreateChessMaterial( const ListExpr typeInfo )
{
    return (SetWord( new ChessMaterial() ));
}

/*
~Delete~ function

*/
void DeleteChessMaterial( const ListExpr typeInfo, Word& w )
{
    delete (ChessMaterial *)w.addr;
    w.addr = 0;
}

/*
~Open~ function

*/
bool OpenChessMaterial( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
 ChessMaterial *m =
      (ChessMaterial*)Attribute::Open( 
                       valueRecord, offset, typeInfo );
 value = SetWord( m );
 return true;
}

/*
~Save~ function

*/
bool SaveChessMaterial( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
 ChessMaterial *m = (ChessMaterial *)value.addr;
 Attribute::Save( valueRecord, offset, typeInfo, m );
 return true;
}

/*
~Close~ function

*/
void CloseChessMaterial( const ListExpr typeInfo, Word& w )
{
    delete (ChessMaterial *)w.addr;
    w.addr = 0;
}

/*
~Clone~ function

*/
Word CloneChessMaterial( const ListExpr typeInfo, const Word& w )
{
    return SetWord( ((ChessMaterial *)w.addr)->Clone() );
}

/*
~SizeOf~ function

*/
int SizeOfChessMaterial()
{
    return sizeof(ChessMaterial);
}

/*
Kind Checking Function

*/
bool CheckChessMaterial( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "chessmaterial" ));
}

/*
Type Constructor

*/
TypeConstructor 
   chessmaterial(     "chessmaterial",
            ChessMaterialProperty,
                        OutChessMaterial, InChessMaterial,
                        0, 0, // SaveToList and RestoreFromList
            CreateChessMaterial, DeleteChessMove,
            /*OpenChessMaterial, SaveChessMaterial,*/ 0,0,
                        CloseChessMaterial, CloneChessMaterial,
            CastChessMaterial,        
                        SizeOfChessMaterial,   
                        CheckChessMaterial ); 


//--------------------------------------------------------------


/*
2.3 ChessPosition

Class ChessPosition

*/
class ChessPosition : public IndexableStandardAttribute 
{
  public:
    //Constructor
    ChessPosition();
    ChessPosition(bool def, char ChessField[8][8]);
    ChessPosition(bool def, char ChessField[8][8], int PosNr);
       
    //Destructor
    ~ChessPosition(){;}
    
    ChessPosition* Clone() const;
    
    //Methods
     char GetPosition (int row, int file) const;
     void SetPosition (int row, int file, char figure)
             {chessField[row][file] = figure;}
     void SetChessPosition (char ChessField[8][8]);
     void GetChessPosition(char ChessField[8][8]);
     void SetPosNr (int MoveNr);
     int GetPosNr() const;
     void SetDefined(bool Defined); 
     bool IsDefined()const;
     size_t Sizeof() const;
     void Equalize(const ChessPosition* position);
     void CopyFrom(const StandardAttribute* arg);
     bool Adjacent(const Attribute*) const;
     SmiSize SizeOfChars() const;
     bool ReadFrom(const string ChessPosition);
     void ReadFrom( const char *src );
     int CompareTo(const ChessPosition* P2) const;
     int Compare(const Attribute* arg) const;
     size_t HashValue() const; 
     string ToString() const;
     void WriteTo( char *dest )const;
     
     ChessMaterial* pieces();
     ChessPosition* chessrange(string filefrom, 
                 int rowfrom, string fileto, int rowto);
     bool chessincludes(const ChessPosition position);
     
  private:
    //intern representation of the chess field
    char chessField[8][8];
    int posNr;   
    bool defined;
};

//Method definitions
ChessPosition::ChessPosition() {
    defined = true;
}

ChessPosition::ChessPosition(bool def, char ChessField[8][8]){
        defined = def;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            chessField[i][j] = ChessField[i][j];
            }
}

ChessPosition::ChessPosition(bool def, char ChessField[8][8], 
                 int PosNr)
{
        defined = def;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            chessField[i][j] = ChessField[i][j];
            }
    posNr = PosNr;
}

void ChessPosition::SetChessPosition(char ChessField[8][8]){
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            chessField[i][j] = ChessField[i][j];
            }
}

void ChessPosition::GetChessPosition(char ChessField[8][8]) {
    for (int i = 0; i< 8 ; i++) {
        for(int j = 0; j < 8; j++) {
            ChessField[i][j] = chessField[i][j];
        }
    }    
}

void ChessPosition::SetPosNr(int PosNr){
    posNr = PosNr;
}

int ChessPosition::GetPosNr()const{
    return posNr;
}

char ChessPosition::GetPosition (int row, int file) const{
      return chessField[row][file];
}

bool ChessPosition::IsDefined() const{
    return defined;
}

void ChessPosition::SetDefined(bool Defined) {
    defined = Defined;
}

/*
~Sizeof~

The function ~Sizeof~ returns the ~sizeof~ of the instance 
of the ~ChessPosition~ class.

*/
size_t ChessPosition::Sizeof() const{
    return sizeof(ChessPosition);
}

/*
~Equalize~

This function changes the value of this ChessPosition instance
to be equal to position.

*/
void ChessPosition::Equalize(const ChessPosition* position)
{
   defined = position->defined;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
      {
         chessField[i][j] = position->chessField[i][j];
      }
   posNr = position->posNr;
}

/*
~CopyFrom~

This ChessPosition instance takes its value from arg if this function is
called.

*/
void ChessPosition::CopyFrom(const StandardAttribute* arg){
   Equalize((const ChessPosition*) arg);
}

/*
~Adjacent~

This function returns true if the posNr from this is 
one more or one less than the posNr from arg.

*/
bool ChessPosition::Adjacent(const Attribute* arg) const{
    const ChessPosition* position2 = (const ChessPosition*) arg;
    
      if( defined && position2->defined && 
        ( posNr == position2->posNr + 1 ||
          posNr == position2->posNr - 1 ) )
        return true;
      return false;
}

SmiSize ChessPosition::SizeOfChars() const
{                                               
    return ToString().length();  
}

/*
~ReadFrom~

This function reads the ChessPosition from a given string. If the string
don't represent a valid ChessPosition value, this instance remains unchanged
and false is returned. In the other case, this instance will hold the
ChessPosition represented by the string and the result is true.
The format of the string must be for example:
    rnbqkbnrpppppppp--------    ...   --PPPPPPPPRNBQKBNR,5    
Spaces are not allowed in this string.

*/
bool ChessPosition::ReadFrom(const string position)
{
   if( position == "undefined" )
   {
      defined=false;
      return false;
   }
   int pos = 0;
   int len = position.length();
   if( len < 66 || position[64] != ',' ) return false;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
      {
         this->chessField[i][j] = position[pos];
     pos++;
      }
   pos++;
   int nr = 0;
   while ( position[pos] != '\0' )
   {
      int digit = int(position[pos]) - 48;
      nr = (10 * nr) + digit;
      pos++;
   }
   this->posNr = nr;
   this->defined = true;
   return true;
}

void ChessPosition::ReadFrom( const char *src )
{
  ReadFrom( string(src) );
}

/*
~CompareTo~

This function compares this ChessPosition instance with another one.
The result will be:

  * -1 if this instance is not equals to P2

  * 0 if this instance is equals to P2

  * 1 if this instance is defined and P2 is not defined

The types of the arguments has to be equals.

*/
int ChessPosition::CompareTo(const ChessPosition* P2) const
{
  const ChessPosition* P1 = (const ChessPosition* )(P2);
   if(!IsDefined() && !P1->IsDefined())
      return 0;
   if(!IsDefined() && P1->IsDefined())
      return -1;
   if(IsDefined() && !P1->IsDefined())
      return 1;

   for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
       {
           if((chessField[i][j]) != (P1->GetPosition(i, j)))
       {
         return -1;
       }
       };
   if ((posNr) != (P1->GetPosNr()))
      {
        return -1;
      }
   return 0;
 }

/*
~Compare~

This function compares this ChessPosition with the given Attribute.

*/
int ChessPosition::Compare(const Attribute* arg) const
{
  return CompareTo( (const ChessPosition*) arg);
}

/*
~HashValue~

This function returns the HashValue for this ChessPosition instance.

*/
size_t ChessPosition::HashValue() const
{
    int hashvalue = 0;
    for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
       {
              hashvalue = hashvalue + (int) chessField[i][j];
      }
//    hashvalue = hashvalue % 64;
    return (size_t) hashvalue;
}

/*
~Clone~

This funtion returns a copy of this instance.

*/
ChessPosition* ChessPosition::Clone() const {
    return new ChessPosition( *this );
}

/*
~ToString~

This function returns the string representation of this ChessPosition instance.

*/
string ChessPosition::ToString() const
{
  ostringstream tmp;
  if(!defined)
  {
    tmp << "undefined";
  }
  else
  {
  for (int i = 0; i < 8; i++)
   for (int j = 0; j < 8; j++)
    {
           tmp << GetPosition(i, j);
    }
  tmp << ',' << posNr;
  }
  return tmp.str(); 
}

void ChessPosition::WriteTo( char *dest ) const
{
  strcpy( dest, ToString().c_str() );
}

/*
Property Function of ChessPosition

*/
ListExpr
ChessPositionProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList
           (nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List"),
        nl->StringAtom("Remarks")),
            nl->FiveElemList
           (nl->StringAtom("-> DATA"),
            nl->StringAtom("chessposition"),
        nl->StringAtom("char ChessField[8][8], int PosNr"),
        nl->StringAtom("(rnbqk.......KBNR, 3)"),
        nl->StringAtom("represents the whole actual "
                       "chessboard + PosNr"))));
}

/*
~In~ and ~Out~ functions

*/
ListExpr
OutChessPosition( ListExpr typeInfo, Word value )
{ cout << " in outchessposition. ";
  if( ((ChessPosition*)value.addr)->IsDefined() )
  {
    ChessPosition* position = (ChessPosition*)(value.addr);
    string buffer = "";
    for (int i = 0; i < 8; i++)
    {
       for ( int j = 0; j < 8; j++ )
       {  
      buffer = buffer + position->GetPosition(i, j);
       }
    }
    //cout << buffer << " ";
    ListExpr result = nl->TwoElemList
                    (nl->TextAtom(buffer),
                                 nl->IntAtom(position->GetPosNr()));
    cout << nl->Text2String(nl->First(result)) << endl;
    return result;
  }
  else
    return (nl->SymbolAtom("undef"));
}

Word
InChessPosition( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{ 
  char chessField[8][8];
  if ( (nl->ListLength( instance ) == 1) && (nl->IsAtom(instance)) && 
             (nl->AtomType(instance) == SymbolType) && 
         (nl->SymbolValue(instance) == "undef") )
  {
       for (int i = 0; i < 8; i++)
           for (int j = 0; j < 8; j++)
           chessField[i][j] = '-';
       correct = true;
       ChessPosition* newposition = new ChessPosition(
                                        false, chessField);
       return SetWord(newposition);
  }
  else if ( nl->ListLength(instance) == 2 && 
            nl->IsAtom(nl->First(instance)) && 
        nl->AtomType(nl->First(instance)) == TextType &&
        nl->IsAtom(nl->Second(instance)) && 
        nl->AtomType(nl->Second(instance)) == IntType )
  {
     cout << "in inchessposition. ";
     string buffer = nl->Text2String(nl->First(instance));
     int k = 0;
     for (int i = 0; i < 8; i++)
     {
        for (int j = 0; j < 8; j++)
        {  cout << buffer[k];
       if (buffer[k] == 'P' || buffer[k] == 'B' || 
           buffer[k] == 'N' || buffer[k] == 'R' || 
           buffer[k] == 'Q' || buffer[k] == 'K' || 
           buffer[k] == 'p' || buffer[k] == 'b' || 
           buffer[k] == 'n' || buffer[k] == 'r' || 
           buffer[k] == 'q' || buffer[k] == 'k' ||
           buffer[k] == '-' || buffer[k] == ' ' )
       {
           chessField[i][j] = buffer[k];
           k++;
       }
       else
       {
           cout << " Wrong values! ";
           return SetWord( Address(0));
       }
    }
     }
     cout << endl;
     ChessPosition* position = new ChessPosition();
     position->SetChessPosition(chessField);
     position->SetPosNr(nl->IntValue(nl->Second(instance)));
     correct = true;
     return SetWord(position);
  }
  else
  {
     correct = false;
     return SetWord( Address(0));
  }  
}

/*
~Cast~ function

*/
void* CastChessPosition( void* addr )
{
   return (new (addr) ChessPosition);
}

/*
~Create~ function

*/
Word CreateChessPosition( const ListExpr typeInfo )
{
    return (SetWord( new ChessPosition() ));
}

/*
~Delete~ function

*/
void DeleteChessPosition( const ListExpr typeInfo, Word& w )
{
    delete (ChessPosition *)w.addr;
    w.addr = 0;
}

/*
~Open~ function
 
*/
bool OpenChessPosition( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
 ChessPosition *p =
      (ChessPosition*)Attribute::Open( 
                       valueRecord, offset, typeInfo );
 value = SetWord( p );
 return true;
}

/*
~Save~ function

*/
bool SaveChessPosition( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
 ChessPosition *p = (ChessPosition *)value.addr;
 Attribute::Save( valueRecord, offset, typeInfo, p );
 return true;
}

/*
~Close~ function

*/
void CloseChessPosition( const ListExpr typeInfo, Word& w )
{
    delete (ChessPosition *)w.addr;
    w.addr = 0;
}

/*
~Clone~ function

*/
Word CloneChessPosition( const ListExpr typeInfo, const Word& w )
{
    return SetWord( ((ChessPosition *)w.addr)->Clone() );
}

/*
~SizeOf~ function

*/
int SizeOfChessPosition()
{
    return sizeof(ChessPosition);
}

/*
Kind Checking Function

*/
bool CheckChessPosition( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "chessposition" ));
}

/*
Type Constructor

*/
TypeConstructor 
    chessposition(     "chessposition",
            ChessPositionProperty,
                        OutChessPosition, InChessPosition,
                        0, 0, // SaveToList and RestoreFromList
            CreateChessPosition, DeleteChessPosition,
            /*OpenChessPosition, SaveChessPosition,*/ 0,0,
                        CloseChessPosition, CloneChessPosition,
            CastChessPosition,        
                        SizeOfChessPosition,   
                        CheckChessPosition ); 

//--------------------------------------------------------------


/*
2.4 ChessGame

Class ChessGame

*/

class ChessGame : public IndexableStandardAttribute
{
  public:
    //Constructor
    ChessGame();

    ChessGame( string Event, string Site, string Date, 
         string Round, string White, string Black, string Result, 
     string WhiteElo, string BlackElo, string EventDate, 
     string Eco, const int n, const char *FigureName, 
     const char *StartPosition, const char *EndPosition, 
     const char *Action, const char *CapturedFigure, int MoveNr);
    
    ChessGame(const int n);

    //Destructor
    ~ChessGame();

    ChessGame* Clone() const;
  
    //Methods
    string GetEvent()const;
    string GetSite()const;
    string GetDate()const;
    string GetRound()const;
    string GetWhite()const;
    string GetBlack()const;
    string GetResult()const;
    string GetWhiteElo()const;
    string GetBlackElo()const;
    string GetEventDate()const;
    string GetECO()const;
    ChessPosition* GetPosition(int nr);
    int GetColumnDigit(char column);
    void GetCastlingPosition(char pos[8][8], int player, char type);
    
    void SetEvent(string Event);
    void SetSite(string Site);
    void SetDate(string Date);
    void SetRound(string Round);
    void SetWhite(string White);
    void SetBlack(string Black);
    void SetResult(string Result);
    void SetWhiteElo(string WhiteElo);
    void SetBlackElo(string BlackElo);
    void SetEventDate(string EventDate);
    void SetECO(string ECO);
    void SetDefined(bool Defined);
    bool IsDefined()const; 
    int NumOfFLOBs() const;
    FLOB *GetFLOB(const int i); 
    void Append(const ChessMove& m);
    void Destroy();
    int GetNoChessMoves() const; 
    const ChessMove* GetChessMove(int i) const;
    const bool IsEmpty() const;
    size_t Sizeof() const;
    void Equalize(const ChessGame* game);
    void CopyFrom(const StandardAttribute* arg);
    bool Adjacent(const Attribute*) const;
    SmiSize SizeOfChars() const;
    bool ReadFrom(const string ChessGame);
    void ReadFrom( const char *src );
    int CompareTo(const ChessGame* P2) const;
    int Compare(const Attribute* arg) const;
    size_t HashValue() const; 
    string ToString() const;
    void WriteTo( char *dest )const;
    
  
  private:

     //All moves of the game
     DBArray<ChessMove> moves;
     
     //Metadata
     char event[41];
     char site[21];
     char round[11];
     char date[21];
     char white[21];
     char black[21];
     char result[11];
     char whiteElo[11];
     char blackElo[11];
     char eventDate[21];
     char eco[11]; 
     
     bool defined;
};

//Method definitions

ChessGame::ChessGame(){}

ChessGame::ChessGame( string Event, string Site, string Date, 
     string Round, string White, string Black, string Result, 
     string WhiteElo, string BlackElo, string EventDate, 
     string Eco, const int n, const char *FigureName = 0, 
     const char *StartPosition = 0, const char *EndPosition = 0, 
     const char *Action = 0, const char *CapturedFigure = 0, 
     int MoveNr = 0 ) :
     moves ( n )
{
//    cout << "in constructor of chessgame" << endl;    
    
    event[0] = '\0';
    site[0] = '\0';
    date[0] = '\0';
    round[0] = '\0';
    white[0] = '\0';
    black[0] = '\0';
    result[0] = '\0';
    whiteElo[0] = '\0';
    blackElo[0] = '\0';
    eventDate[0] = '\0';
    eco[0] = '\0';
    
    strncpy(event, Event.c_str(), 40);
    strncpy(site, Site.c_str(), 20);
    strncpy(date, Date.c_str(), 20);
    strncpy(round, Round.c_str(), 10);
    strncpy(white, White.c_str(), 20);
    strncpy(black, Black.c_str(), 20);
    strncpy(result, Result.c_str(), 10);
    strncpy(whiteElo, WhiteElo.c_str(), 10);
    strncpy(blackElo, BlackElo.c_str(), 10);
    strncpy(eventDate, EventDate.c_str(), 20);
    strncpy(eco, Eco.c_str(), 10);
    
//    cout << "eventDate Size ====== " << EventDate.size() << endl;
    
    //When a metadate is longer than 10, 20 or 40, it will cut
    //off and whe have to set '\0' manually'
    if(Event.size() >= 40)
        event[40] = '\0';
    if(Site.size() >= 20)
        site[20] = '\0';
    if(Date.size() >= 20)
        date[20] = '\0';
    if(Round.size() >= 10)
        round[10] = '\0';
    if(White.size() >= 20)
        white[20] = '\0';
    if(Black.size() >= 20)
        black[20] = '\0';
    if(Result.size() >= 10)
        result[10] = '\0';
    if(WhiteElo.size() >= 10)
        whiteElo[10] = '\0';
    if(BlackElo.size() >= 10)
        blackElo[10] = '\0';
    if(EventDate.size() >= 20)
        eventDate[20] = '\0';
    if(Eco.size() >= 10)
        eco[10] = '\0';
    
    
    defined = true;
    if ( n > 0 )
    {
       for ( int i = 0; i < n; i++)
       {
         int movenr = i + 1;
         cout << "chessgame constructor: in for "
                 "(construction of moves )" << i << endl;
         ChessMove move( true, FigureName[i], StartPosition[i], 
                   EndPosition[i], Action[i], CapturedFigure[i], 
               movenr );
         Append( move);
        }
    }
}


ChessGame::ChessGame(const int n) :
moves( n ) 
{
    event[0] = '\0';
    site[0] = '\0';
    date[0] = '\0';
    round[0] = '\0';
    white[0] = '\0';
    black[0] = '\0';
    result[0] = '\0';
    whiteElo[0] = '\0';
    blackElo[0] = '\0';
    eventDate[0] = '\0';
    eco[0] = '\0';
    defined =  true;
}

ChessGame::~ChessGame()
{
    //moves.Destroy();    
}

bool ChessGame::IsDefined() const{
    return defined;
}

void ChessGame::SetDefined(bool Defined) {
    defined = Defined;
}

string ChessGame::GetEvent()const {
    string ret(event);
    return ret;
}

string ChessGame::GetSite()const {
    string ret(site);
    return ret;
}

string ChessGame::GetDate()const {
    string ret(date);
    return ret;
}

string ChessGame::GetRound()const {
    string ret (round);
    return ret;
}

string ChessGame::GetWhite()const {
    string ret (white);
    return ret;
}

string ChessGame::GetBlack() const{
    string ret (black);
    return ret;
}

string ChessGame::GetResult() const{
    string ret (result);
    return ret;
}

string ChessGame::GetWhiteElo() const{
    string ret (whiteElo);
    return ret;
}
    
string ChessGame::GetBlackElo() const{
    string ret (blackElo);
    return ret;
}

string ChessGame::GetEventDate() const{
    string ret (eventDate);
    return ret;
}

string ChessGame::GetECO() const{
    string ret(eco);
    return ret;
}

void ChessGame::SetEvent(string Event){
    strncpy(event, Event.c_str(), 40);
    if(Event.size() >= 40)
    event[40] = '\0';    
}

void ChessGame::SetSite(string Site){
    strncpy(site, Site.c_str(), 20);
    if(Site.size() >= 20)
    site[20] = '\0';    
}

void ChessGame::SetDate(string Date){
    strncpy(date, Date.c_str(), 20);    
    if(Date.size() >= 20)
    date[20] = '\0';
}

void ChessGame::SetRound(string Round){
    strncpy(round, Round.c_str(), 10);
    if(Round.size() >= 10)
    round[10] = '\0';    
}
    
void ChessGame::SetWhite(string White) {
    strncpy(white, White.c_str(), 20);    
    if(White.size() >= 20)
    white[20] = '\0';
}
    
void ChessGame::SetBlack(string Black) {
    strncpy(black, Black.c_str(), 20);    
    if(Black.size() >= 20)
    black[20] = '\0';
}

void ChessGame::SetResult(string Result) {
    strncpy(result, Result.c_str(), 10);
    if(Result.size() >= 10)
    result[10] = '\0';    
}

void ChessGame::SetWhiteElo(string WhiteElo){
     strncpy(whiteElo, WhiteElo.c_str(), 10);
    if(WhiteElo.size() >= 10)
    whiteElo[10] = '\0';    
}
    
void ChessGame::SetBlackElo(string BlackElo){
    strncpy(blackElo, BlackElo.c_str(), 10);    
    if(BlackElo.size() >= 10)
    blackElo[10] = '\0';
}

void ChessGame::SetEventDate(string EventDate) {
    strncpy(eventDate, EventDate.c_str(), 20);
    if(EventDate.size() >= 20)
    eventDate[20] = '\0';    
}

void ChessGame::SetECO(string ECO) {
    strncpy(eco, ECO.c_str(), 10);    
    if(ECO.size() >= 10)
    eco[10] = '\0';
}

/*
~Clone~

This funtion returns a copy of this instance.

*/
ChessGame* ChessGame::Clone() const {
    
    ChessGame *game = new ChessGame( 0 );
    strcpy(game->event,this->event);
    strcpy(game->site,this->site);
    strcpy(game->date,this->date);
    strcpy(game->round,this->round);
    strcpy(game->white,this->white);
    strcpy(game->black,this->black);
    strcpy(game->result,this->result);
    strcpy(game->whiteElo,this->whiteElo);
    strcpy(game->blackElo,this->blackElo);
    strcpy(game->eventDate,this->eventDate);
    strcpy(game->eco,this->eco);
    for ( int i = 0; i < GetNoChessMoves(); i++ )
    {
       game->Append( *this->GetChessMove(i) );
    }    
    return game;
}

//old wrong clone
/*ChessGame* ChessGame::Clone() const {
    return new ChessGame( *this );

}*/

/*
~NumOfFLOBs~

Needed to be a tuple attribute.

*/
int ChessGame::NumOfFLOBs() const{
    return 1;
}  

/*
~GetFLOB~

Needed to be a tuple attribute.

*/
FLOB* ChessGame::GetFLOB(const int i){
    assert( i >= 0 && i < NumOfFLOBs() );
    return &moves;
}

/*
~Append~

Appends a ChessMove ~m~ at the end of the ChessGame.

*/
void ChessGame::Append(const ChessMove& m){
    moves.Append(m);

}   

/*
~Destroy~

not yet implemented

*/
void ChessGame::Destroy()
{
    
}

/*
~GetNoChessMoves~

Returns the number of ChessMoves of the ChessGame.

*/
int ChessGame::GetNoChessMoves() const {
    return moves.Size();
}

/*
~GetChessMove~

Returns a ChessMove indexed by ~i~.

*/
const ChessMove* ChessGame::GetChessMove(int i) const{
    assert( 0 <= i && i < GetNoChessMoves() );

     const ChessMove *m;
     moves.Get( i, m );
     return m;
}

/*
~IsEmpty~

Returns if the ChessGame is empty or not.

*/
const bool ChessGame::IsEmpty() const
{
    return GetNoChessMoves() == 0;
}

/*
~Sizeof~

The function ~Sizeof~ returns the ~sizeof~ of the instance 
of the ~ChessGame~ class.

*/
size_t ChessGame::Sizeof() const{
    return sizeof( *this );//(ChessGame);
}

/*
~Equalize~

This function changes the value of this ChessGame instance 
to be equal to game.

*/
void ChessGame::Equalize(const ChessGame* game)
{
   defined = game->defined;
   moves = game->moves;
   
   strcpy(event,game->event);
   strcpy(site,game->site);
   strcpy(date,game->date);
   strcpy(round,game->round);
   strcpy(white,game->white);
   strcpy(black,game->black);
   strcpy(result,game->result);
   strcpy(whiteElo,game->whiteElo);
   strcpy(blackElo,game->blackElo);
   strcpy(eventDate,game->eventDate);
   strcpy(eco,game->eco);
}

/*
~CopyFrom~

This ChessGame instance takes its value from arg if this function is
called.

*/
void ChessGame::CopyFrom(const StandardAttribute* arg){
   Equalize((const ChessGame*) arg);
}

/*
~Adjacent~

This function returns true if the event and the date from this are 
equals to event and date from arg.

*/
bool ChessGame::Adjacent(const Attribute* arg) const{
    const ChessGame* game2 = (const ChessGame*) arg;
    if( defined && game2->defined && 
        event == game2->event &&
        date == game2->date )
        return true;
      return false;
}

SmiSize ChessGame::SizeOfChars() const
{                                               
    return ToString().length();  
}

/*
~ReadFrom~

This function reads the ChessGame from a given string. If the string
don't represent a valid ChessGame value, this instance remains unchanged
and false is returned. In the other case, this instance will hold the
ChessGame represented by the string and the result is true.
The format of the string must be:
    event,site,date,round,white,black,result,whiteElo,blackElo,eventDate    
Spaces are not allowed in this string.

*/
bool ChessGame::ReadFrom(const string game)
{
   if( game == "undefined" )
   {
      defined=false;
      return false;
   }
   cout << " ReadFrom: " << game;
   int pos = 0;
   int len = game.length();
   if( len == 0 ) return false;
   while ( game[pos] != ',' )
      pos++;
   char*event = new char[pos + 1];
   for ( int i = 0; i < pos; i++ )
   {
      event[i] = game[i];
   }
   event[pos] = '\0';
   pos++;
   int pos2 = pos;
   while ( game[pos2] != ',' )
      pos2++;
   char*site = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      site[i - pos] = game[i];
   }
   site[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*date = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      date[i - pos] = game[i];
   }
   date[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*round = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      round[i - pos] = game[i];
   }
   round[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*white = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      white[i - pos] = game[i];
   }
   white[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*black = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      black[i - pos] = game[i];
   }
   black[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*result = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      result[i - pos] = game[i];
   }
   result[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*whiteElo = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      whiteElo[i - pos] = game[i];
   }
   whiteElo[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*blackElo = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      blackElo[i - pos] = game[i];
   }
   blackElo[pos2] = '\0';
   pos2++;
   pos = pos2;
   while ( game[pos2] != ',' )
      pos2++;
   char*eventDate = new char[pos2 - pos + 1];
   for ( int i = pos; i < pos2; i++ )
   {
      eventDate[i - pos] = game[i];
   }
   eventDate[pos2] = '\0';
   pos2++;
   pos = pos2;
      
   strcpy(this->event,event);
   strcpy(this->site, site);
   strcpy(this->date,date);
   strcpy(this->round,round);
   strcpy(this->white,white);
   strcpy(this->black,black);
   strcpy(this->result,result);
   strcpy(this->whiteElo,whiteElo);
   strcpy(this->blackElo,blackElo);
   strcpy(this->eventDate,eventDate);
   this->defined = true;
   
   delete[] event;
   delete[] site;
   delete[] round;
   delete[] white;
   delete[] black;
   delete[] result;
   delete[] whiteElo;
   delete[] blackElo;
   delete[] eventDate;
   
   return true;
}

void ChessGame::ReadFrom( const char *src )
{
  ReadFrom( string(src) );
}

/*
~CompareTo~

This function compares this ChessGame instance with another one.
The result will be:

  * -1 if metadates or numbers of ChessMoves of this instance 
     and of P2 are not equals

  * 0 if this instance is equals to P2

  * 1 if metadates and numbers of ChessMoves of this instance
    and of P2 are equals and ChessMoves are not equals

The types of the arguments has to be equals.

*/
int ChessGame::CompareTo(const ChessGame* P2) const
{
  const ChessGame* P1 = (const ChessGame* )(P2);
  if(!IsDefined() && !P1->IsDefined())
      return 0;
   if(!IsDefined() && P1->IsDefined())
      return -1;
   if(IsDefined() && !P1->IsDefined())
      return 1;
   if((event == (P1->GetEvent())) &&
      (site == (P1->GetSite())) &&
      (date == (P1->GetDate())) &&
      (round == (P1->GetRound())) &&
      (white == (P1->GetWhite())) &&
      (black == (P1->GetBlack())) &&
      (result == (P1->GetResult())) &&
      (whiteElo == (P1->GetWhiteElo())) &&
      (blackElo == (P1->GetBlackElo())) &&
      (eventDate == (P1->GetEventDate())) &&
      (eco == (P1->GetECO())) &&
      (GetNoChessMoves() == (P1->GetNoChessMoves()))
     )
     {
         for (int i = 0; i < GetNoChessMoves(); i++)
    {
       if (((GetChessMove(i))->Compare((Attribute*) 
                               P1->GetChessMove(i))) != 0)
      { 
        return 1;
      }
    }
    return 0;
     };
   return -1;

}

/*
~Compare~

This function compares this ChessGame with the given Attribute.

*/
int ChessGame::Compare(const Attribute* arg) const
{
  return CompareTo( (const ChessGame*) arg);
}

/*
~HashValue~

This function returns the HashValue for this ChessGame instance.

*/
size_t ChessGame::HashValue() const
{
    int hashvalue;
    string hashstring;
    string Event (event), Site (site), Date (date), 
           Round (round), White (white), Black (black);
    
    hashstring = Event + Site + Date + Round + White + Black;
    for (unsigned i = 0; i < hashstring.length(); i++)
    {
       hashvalue = hashvalue + (int) hashstring[i];
    }
//    hashvalue = hashvalue % hashstring.length();
    return (size_t) hashvalue;
}

/*
~ToString~

This function returns the string representation of this ChessGame instance.

*/
string ChessGame::ToString() const
{
  ostringstream tmp;
  if(!defined)
  {
    tmp << "undefined";
  }
  tmp << GetEvent() << "," << GetSite() << "," << GetDate() << "," << 
         GetRound() << "," << GetWhite() << "," << GetBlack() << 
     "," << GetResult() << "," << GetWhiteElo() << "," << 
     GetBlackElo() << "," << GetEventDate();
  return tmp.str(); 
  cout << " ToString: " << tmp;
}

void ChessGame::WriteTo( char *dest ) const
{
  strcpy( dest, ToString().c_str() );
}

/*
~GetColumnDigit~

Returns the digit of a column. e.g. a = 1, b = 2 etc.

*/
int ChessGame::GetColumnDigit(char column) {
    if(column == 'a') return 1;
    if(column == 'b') return 2;
    if(column == 'c') return 3;
    if(column == 'd') return 4;
    if(column == 'e') return 5;
    if(column == 'f') return 6;
    if(column == 'g') return 7;
    if(column == 'h') return 8;
    return 0;
}

/*
~GetCastlingPosition~

Calculates the kingside and queenside castling.

*/
void ChessGame::GetCastlingPosition(char pos[8][8], 
                              int player, char type)
{
    int line = -1;
    int column = -1;
    
    //get line of king position
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if((pos[i][j] == 'K' && player == 0) || 
               (pos[i][j] == 'k' && player == 1)) {
                line = i;
                column = j;
                break;
            }        
        }
        if(line != -1)
            break;
    }
            
    //king side castling
    if(type == 's') {
        if(column + 3 < 8 && 
          ((pos[line][column + 3] == 'R' && player == 0) || 
           (pos[line][column + 3] == 'r' && player == 1))) {
              //move king two columns
              pos[line][column + 2] = pos[line][column];
              pos[line][column] = '-';
            
              //move rook
              pos[line][column + 1] = pos[line][column + 3];
              pos[line][column + 3] = '-';
        }
        else if(column - 3 >= 0 && 
            ((pos[line][column - 3] == 'R' && player == 0) || 
             (pos[line][column - 3] == 'r' && player == 1)))
             {
             //move king two columns
             pos[line][column - 2] = pos[line][column];
             pos[line][column] = '-';
                                
             //move rook
             pos[line][column - 1] = pos[line][column - 3];
             pos[line][column - 3] = '-';        
        }
                
    }
    //queenside castling
    else {
        if(column + 4 < 8 && 
          ((pos[line][column + 4] == 'R' && player == 0) || 
           (pos[line][column + 4] == 'r' && player == 1))) {
              //move king two columns
              pos[line][column + 2] = pos[line][column];
              pos[line][column] = '-';
                        
              //move rook
              pos[line][column + 1] = pos[line][column + 4];
              pos[line][column + 4] = '-';
        }
        else if(column - 4 >= 0 && 
            ((pos[line][column - 4] == 'R' && player == 0) || 
              (pos[line][column - 4] == 'r' && player == 1))) 
                  {            
              //move king two columns
              pos[line][column - 2] = pos[line][column];
              pos[line][column] = '-';
                        
              //move rook
              pos[line][column - 1] = pos[line][column - 4];
              pos[line][column - 4] = '-';        
        }
    }
    
}

/*
~GetPosition~

Calculates the position at the given half move number.

nr = 0 --> Startposition

*/
ChessPosition* ChessGame::GetPosition(int nr) {
    string action;
    char columns, lines, columne, linee;
    string figure;
    int posNr = 0;
        
    //startposition
    char pos[8][8] = {{'r','n','b','q','k','b','n','r'},
              {'p','p','p','p','p','p','p','p'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'P','P','P','P','P','P','P','P'},
              {'R','N','B','Q','K','B','N','R'}};
              
    ChessPosition* chessPosition = new ChessPosition();
    
    if(nr > GetNoChessMoves()) {
        cout << "Invalid position nr. Nr greater "
                "than number of moves" << endl;
        chessPosition->SetDefined(false);
    }
    
    
    //calculate the turns
    for(int i = 0; i < nr; i++) {
       posNr++;
       //Get move
       const ChessMove* move = GetChessMove(i);
       //get action
       action = move->GetAction();
        
       if(action[2] == '-') {
         //startposition
         columns = GetColumnDigit(
                          move->GetStartPosition()[0]) - 1;
         lines = 8 - ((int) move->GetStartPosition()[1] - '0') ;
            
         //endposition
         columne = GetColumnDigit(move->GetEndPosition()[0]) - 1;
         linee = 8 - ((int) move->GetEndPosition()[1] - '0');
            
         //figure
         figure = move->GetFigureName();
            
         //Pawn promotion
         if(action[1] != '-') {
        //promotioned figure
        figure[0] = action[1];
         }
            
         //player black. 
         if((i % 2) != 0) {
        switch(figure[0]) {
            case 'P':   figure[0] = 'p';
                    break;
            case 'N':   figure[0] = 'n';
                    break;
            case 'B':   figure[0] = 'b';
                    break;
            case 'R':   figure[0] = 'r';
                    break;
            case 'Q':   figure[0] = 'q';
                    break;
            case 'K':   figure[0] = 'k';
                    break;                
        }            
         }
         
         if((figure[0] == 'P' || figure[0] == 'p') && 
            (move->GetCapturedFigure()[0] == 'P' || 
             move->GetCapturedFigure()[0] == 'p')  && 
        pos[linee][columne] == '-') {
        //catch en passant
        
        //black
        if((i % 2) != 0) {
            if(linee - 1 >= 0 && 
               pos[linee - 1][columne] == 'P')
                pos[linee - 1][columne] = '-';
        }
        else{
            if(linee + 1 <= 7 && 
               pos[linee + 1][columne] == 'p')
                pos[linee + 1][columne] = '-';
        }
         }
             
         //set position
         pos[linee][columne] = figure[0];
         pos[lines][columns] = '-';
       }
    else {
         //short castling
         if(action[2] == 's') {
        GetCastlingPosition(pos, (i % 2), 's');
         }
         //long castling
         else if(action[2] == 'l') {
        GetCastlingPosition(pos, (i % 2), 's');
         }
         else
        chessPosition->SetDefined(false);        
       }    
        }
    //set position
    chessPosition->SetPosNr(posNr);
    chessPosition->SetChessPosition(pos);
    return chessPosition;
}

/*
Property Function of ChessGame

*/
ListExpr
ChessGameProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList
          (nl->StringAtom("Signature"),
           nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Example List"),
           nl->StringAtom("Remarks")),
            nl->FiveElemList
          (nl->StringAtom("-> DATA"),
           nl->StringAtom("chessgame"),
           nl->StringAtom("(event, site, ... , "
                          "*action, *capturedfigure...)"),            
           nl->StringAtom("(\"Tal Memorial\" \"Moscow RUS\""
                          " ... \"+\" \"+\")"),
           nl->StringAtom("represents a chessgame with "
                          "all data"))));
}

/*
~In~ and ~Out~ functions

*/
ListExpr
OutChessGame( ListExpr typeInfo, Word value )
{ 
  if( ((ChessGame*)value.addr)->IsDefined() )
  { 
    ListExpr moveslist, last;
    ChessGame* game = (ChessGame*)(value.addr);
    if ( game->GetNoChessMoves() > 0 )
    {
      moveslist = nl->OneElemList(nl->SixElemList
       (nl->StringAtom((game->GetChessMove(0))->GetFigureName()),
        nl->StringAtom((game->GetChessMove(0))->GetStartPosition()),
    nl->StringAtom((game->GetChessMove(0))->GetEndPosition()),
    nl->StringAtom((game->GetChessMove(0))->GetAction()),
    nl->StringAtom((game->GetChessMove(0))->GetCapturedFigure()),
    nl->IntAtom((game->GetChessMove(0))->GetMoveNr())) );
      last = moveslist;
    }
    else
       moveslist = nl->OneElemList(nl->TheEmptyList());
    for (int i = 1; i < game->GetNoChessMoves(); i++)
    { 
     last = nl->Append( last, nl->SixElemList
       (nl->StringAtom((game->GetChessMove(i))->GetFigureName()),
          nl->StringAtom((game->GetChessMove(i))->GetStartPosition()),
    nl->StringAtom((game->GetChessMove(i))->GetEndPosition()),
    nl->StringAtom((game->GetChessMove(i))->GetAction()),
    nl->StringAtom((game->GetChessMove(i))->GetCapturedFigure()),
    nl->IntAtom((game->GetChessMove(i))->GetMoveNr())) );
    }       
    return nl->ThreeElemList
     (nl->SixElemList
        ( nl->StringAtom(game->GetEvent()),
          nl->StringAtom(game->GetSite()),
      nl->StringAtom(game->GetDate()),
      nl->StringAtom(game->GetRound()),
      nl->StringAtom(game->GetWhite()),
      nl->StringAtom(game->GetBlack()) ),
      nl->FiveElemList
    ( nl->StringAtom(game->GetResult()),
      nl->StringAtom(game->GetWhiteElo()),
      nl->StringAtom(game->GetBlackElo()),
      nl->StringAtom(game->GetEventDate()),
      nl->StringAtom(game->GetECO()) ),
      nl->OneElemList( moveslist ));
  }
  else {
    return (nl->SymbolAtom("undef"));
  }
}

Word
InChessGame( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{ 
  if ( nl->ListLength( instance ) == 3 )
  {
    ListExpr first = nl->First(instance);
    ListExpr second = nl->Second(instance);
    ListExpr third = nl->Third(instance);
    
    if ( nl->ListLength(first) == 6 && 
         nl->ListLength(second) == 5 && 
     nl->ListLength(third) == 1 &&
         nl->IsAtom(nl->First(first)) && 
     (nl->AtomType(nl->First(first)) == StringType) &&
         nl->IsAtom(nl->Second(first)) && 
     (nl->AtomType(nl->Second(first)) == StringType) &&
     nl->IsAtom(nl->Third(first)) && 
     (nl->AtomType(nl->Third(first)) == StringType) &&
         nl->IsAtom(nl->Fourth(first)) && 
     (nl->AtomType(nl->Fourth(first)) == StringType) &&
     nl->IsAtom(nl->Fifth(first)) && 
     (nl->AtomType(nl->Fifth(first)) == StringType) &&
         nl->IsAtom(nl->Sixth(first)) && 
     (nl->AtomType(nl->Sixth(first)) == StringType) &&
     nl->IsAtom(nl->First(second)) && 
     (nl->AtomType(nl->First(second)) == StringType) &&
         nl->IsAtom(nl->Second(second)) && 
     (nl->AtomType(nl->Second(second)) == StringType) &&
     nl->IsAtom(nl->Third(second)) && 
     (nl->AtomType(nl->Third(second)) == StringType) &&
         nl->IsAtom(nl->Fourth(second)) && 
     (nl->AtomType(nl->Fourth(second)) == StringType) &&
     nl->IsAtom(nl->Fifth(second)) && 
     (nl->AtomType(nl->Fifth(second)) == StringType) )
    {  
       ListExpr rest = third;
       ListExpr firstmove;
       ChessGame* game;
       game = new ChessGame( 
          nl->StringValue(nl->First(first)), 
      nl->StringValue(nl->Second(first)),
          nl->StringValue(nl->Third(first)), 
      nl->StringValue(nl->Fourth(first)),
      nl->StringValue(nl->Fifth(first)), 
      nl->StringValue(nl->Sixth(first)),
          nl->StringValue(nl->First(second)), 
      nl->StringValue(nl->Second(second)),
          nl->StringValue(nl->Third(second)), 
      nl->StringValue(nl->Fourth(second)),
          nl->StringValue(nl->Fifth(second)),
      0 );
    rest = nl->First(rest);
        while ( !nl->IsEmpty(rest) )
        {  
          firstmove = nl->First(rest);
      rest = nl->Rest(rest);
          if ( nl->ListLength(firstmove) == 6 &&
           nl->IsAtom( nl->First( firstmove ) ) && 
       nl->AtomType( nl->First( firstmove ) ) == StringType &&
           nl->IsAtom( nl->Second( firstmove ) ) && 
       nl->AtomType( nl->Second( firstmove ) ) == StringType &&
           nl->IsAtom( nl->Third( firstmove ) ) && 
       nl->AtomType( nl->Third( firstmove ) ) == StringType &&
       nl->IsAtom( nl->Fourth( firstmove ) ) && 
       nl->AtomType( nl->Fourth( firstmove ) ) == StringType &&
       nl->IsAtom( nl->Fifth( firstmove ) ) && 
       nl->AtomType( nl->Fifth( firstmove ) ) == StringType &&
       nl->IsAtom( nl->Sixth( firstmove ) ) && 
       nl->AtomType( nl->Sixth( firstmove ) ) == IntType )
          {  
        string figurename, capturedfigure;
        if ( nl->StringValue(nl->First(firstmove)) == "none" )
                 figurename = "-";
            else
             figurename = nl->StringValue(nl->First(firstmove));
              string startposition = nl->StringValue
                        (nl->Second(firstmove));
              string endposition = nl->StringValue
                        (nl->Third(firstmove));
              string action = nl->StringValue(nl->Fourth(firstmove));
        if ( nl->StringValue(nl->Fifth(firstmove)) == "none" )
              capturedfigure = "-";
            else
                capturedfigure = nl->StringValue(nl->Fifth(firstmove));
              int MoveNr = nl->IntValue(nl->Sixth(firstmove));
              ChessMove* newmove = new ChessMove ();
              newmove->SetFigureName(figurename.c_str());
              newmove->SetStartPosition(startposition.c_str());
              newmove->SetEndPosition(endposition.c_str());
              newmove->SetAction(action.c_str());
              newmove->SetCapturedFigure(capturedfigure.c_str());
              newmove->SetMoveNr(MoveNr);
        game->Append(*newmove);
          }
          else if ( nl->ListLength(firstmove) != 0 )
      {
             correct = false;
             return SetWord(Address(0));
          }
       }  
       correct = true;
       return SetWord( game );     
    }
    else
    {
       correct = false;
       return SetWord(Address(0));
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
~Cast~ function

*/
void* CastChessGame( void* addr )
{  
   return (new (addr) ChessGame);
}

/*
~Create~ function

*/
Word CreateChessGame( const ListExpr typeInfo )
{    
    ChessGame* game = new ChessGame( 0 );
    return (SetWord( game ));
}

/*
~Delete~ function

*/
void DeleteChessGame( const ListExpr typeInfo, Word& w )
{    
    ChessGame* game = (ChessGame *)w.addr;
    game->Destroy();
    delete game;
}

/*
~Open~ function
 
*/
bool OpenChessGame( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
 ChessGame *g =
      (ChessGame*)Attribute::Open( valueRecord, offset, typeInfo );
 value = SetWord( g );
 return true;
}

/*
~Save~ function

*/
bool SaveChessGame( SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value )
{
 ChessGame *g = (ChessGame *)value.addr;
 Attribute::Save( valueRecord, offset, typeInfo, g );
 return true;
}

/*
~Close~ function

*/
void CloseChessGame( const ListExpr typeInfo, Word& w )
{    
    ChessGame* game = (ChessGame *)w.addr;
    delete game;
}

/*
~Clone~ function

*/
Word CloneChessGame( const ListExpr typeInfo, const Word& w )
{    
    return SetWord( ((ChessGame *)w.addr)->Clone() );
}

/*
~SizeOf~ function

*/
int SizeOfChessGame()
{    
    return sizeof(ChessGame);
}

/*
Kind Checking Function

*/
bool CheckChessGame( ListExpr type, ListExpr& errorInfo )
{    
    return (nl->IsEqual( type, "chessgame" ));
}

/*
Type Constructor

*/
TypeConstructor 
    chessgame(     "chessgame",
        ChessGameProperty,
                   OutChessGame, InChessGame,
                   0, 0, // SaveToList and RestoreFromList
        CreateChessGame, DeleteChessGame,
            /*OpenChessGame, SaveChessGame,*/ 0,0,
                CloseChessGame, CloneChessGame,
        CastChessGame,        
                SizeOfChessGame,   
                CheckChessGame ); 

                

/*
3 Operators

3.1 Operator ~getkey~

Type mapping function of operator ~getkey~

*/
ListExpr
getkeyTypeMap( ListExpr args )
{
 CHECK_COND(nl->ListLength(args) == 2,
  "Operator getkey expects a list of length two.");
  
 if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (( nl->IsEqual(arg1, "chessgame")) && 
        (nl->IsEqual(arg2, "string")))
    {
      return nl->SymbolAtom("string");
    }
  }
  return nl->SymbolAtom("typeerror");
}    

/*
Value mapping function of operator ~getkey~

*/
int
getkeyValueMap (Word* args, Word& result, int message, 
                              Word& local, Supplier s)
{
  ChessGame* s1 = ((ChessGame*) args[0].addr);
  CcString* s2  = ((CcString*) args[1].addr);
  result = qp->ResultStorage(s);    
  string t1 = *(s2->GetStringval());
  char moveno[5] ;
  sprintf(moveno, "%5d", s1->GetNoChessMoves());

  ((CcString*) result.addr)->Set(true, 
            ((STRING*)"wrong input key"));
  if (t1 == "name_w")
  ((CcString*) result.addr)->Set(true, 
            ((STRING*)s1->GetWhite().c_str()));
   if (t1 == "name_b")
   ((CcString*) result.addr)->Set(true, 
             ((STRING*)s1->GetBlack().c_str()));;
    if (t1 == "rating_w")
    ((CcString*) result.addr)->Set(true, 
              ((STRING*)s1->GetWhiteElo().c_str()));
     if (t1 == "rating_b")
     ((CcString*) result.addr)->Set(true, 
               ((STRING*)s1->GetBlackElo().c_str()));
      if (t1 == "event")
      ((CcString*) result.addr)->Set(true, 
                ((STRING*)s1->GetEvent().c_str()));
       if (t1 == "eventdate")
       ((CcString*) result.addr)->Set(true, 
                 ((STRING*)s1->GetEventDate().c_str()));
        if (t1 == "site")
        ((CcString*) result.addr)->Set(true, 
              ((STRING*)s1->GetSite().c_str()));
         if (t1 == "date")
         ((CcString*) result.addr)->Set(true, 
               ((STRING*)s1->GetDate().c_str()));
          if (t1 == "result")
          ((CcString*) result.addr)->Set(true, 
                ((STRING*)s1->GetResult().c_str()));
           if (t1 == "eco_code")
           ((CcString*) result.addr)->Set(true, 
                 ((STRING*)s1->GetECO().c_str()));
            if (t1 == "moves")
            ((CcString*) result.addr)->Set(true, 
                  ((STRING*)moveno));
         
              
         if (t1 == "round")
             ((CcString*) result.addr)->Set(true, 
                   ((STRING*)s1->GetRound().c_str()));

  return 0;
}    

/*
Specificaton of operator ~getkey~

*/
const string getkeySpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessgame x string) -> (string) </text--->"
  "<text>_ getkey _ </text--->"
  "<text>Get a key of a pgn file and returns the key-data"
  "</text--->"
  "<text>query chessgame getkey \"date\" </text--->"
  ") )";

/*
Definition of operator ~getkey~

*/
Operator getkey (
   "getkey",                 // name
   getkeySpec,               // specification
   getkeyValueMap,              // value mapping
   Operator::SimpleSelect,     // trivial selection function
   getkeyTypeMap             // type mapping
);


/*
3.2 Operator ~getmove~

Type mapping function of operator ~getmove~

*/            
ListExpr
getmoveTypeMap( ListExpr args )
{
// cout << "in getmoveTypeMap" << endl;
 CHECK_COND(nl->ListLength(args) == 2,
  "Operator getkey expects a list of length two.");
  
 if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (( nl->IsEqual(arg1, "chessgame")) && 
        (nl->IsEqual(arg2, "int")))
    {
//     cout << "come out getmoveTypeMap -ok" << endl;
       return nl->SymbolAtom("chessmove");
    }
  }
//     cout << "come out getmoveTypeMap -nok" << endl;
    return nl->SymbolAtom("typeerror");
}    

/*
Value mapping function of operator ~getmove~

*/
int
getmoveValueMap (Word* args, Word& result, int message, 
                               Word& local, Supplier s)
{
//  cout << "in getmoveValueMap" << endl;
  ChessGame* s1 = ((ChessGame*) args[0].addr);
  int* s2  = ((int*) args[1].addr);
  result = qp->ResultStorage(s);
  int m1 = ((CcInt*) s2)->GetIntval();
  char no [1];
  
  if (m1 > 0)   
  {
    m1 = m1 - 1;
  }
  else
  {
     *((ChessMove*)result.addr) = ChessMove(false, 
                              *no, *no, *no, *no, *no, 0);
     return 0;
  }

//  cout << "m1 = " << m1 << endl;
//  cout << "s1->GetNoChessMoves()" << s1->GetNoChessMoves() << endl;
  
  if ( m1 < s1->GetNoChessMoves() ) 
     {
//       cout << "come out getmoveValueMap -ok" << endl;
       ((ChessMove*)result.addr) = s1->GetChessMove(m1);

// uhuh eingefuegt fuer writeto-test  ANFANG
  /*
    const ChessMove* move = s1->GetChessMove(m1);
    char dest [20];
    move->WriteTo( dest );
    bool test;
    ChessMove* dest2 = new ChessMove;
    test = dest2->ReadFrom( (const string) dest);
    
  */
// uhuh eingefuegt fuer writeto-test  ENDE

    

     }
  else
  {
//       cout << "come out getmoveValueMap -nok" << endl;
     *((ChessMove*)result.addr) = ChessMove(false, 
                               *no, *no, *no, *no, *no, 0);
  }
  return 0;
}    

/*
Specificaton of operator ~getmove~

*/
const string getmoveSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessgame x int) -> (move) </text--->"
  "<text>_ getmove _ </text--->"
  "<text>Get a number of a half-move returns the move"
  "</text--->"
  "<text>query chessgame getmove 3 </text--->"
  ") )";

/*
Definition of operator ~getmove~

*/
Operator getmove (
   "getmove",               // name
   getmoveSpec,             // specification
   getmoveValueMap,         // value mapping
   Operator::SimpleSelect,  // trivial selection function
   getmoveTypeMap           // type mapping
);


/*
3.3 Operator ~chesscount~

Type mapping function of operator ~chesscount~

*/
ListExpr
countTypeMap( ListExpr args )
{
// cout << "in countTypeMap" << endl;
  if ( nl->ListLength(args) == 2 )
    {
      ListExpr arg1 = nl->First(args);
      ListExpr arg2 = nl->Second(args);
//      if ( nl->IsEqual(arg1, "chessmaterial") && 
//           nl->IsEqual(arg2, "string") )
      if ( nl->IsEqual(arg1, "chessmaterial") && 
           ( nl->IsEqual(arg2, "string") || 
       ( nl->IsEqual(arg2, "int") )))
      {
//        cout << "in countTypeMap. return 1" << endl;
        return nl->SymbolAtom("int");
      }
    }
    else 
    {
     if ( nl->ListLength(args) == 1 )
     {
      ListExpr arg1 = nl->First(args);
      if ( nl->IsEqual(arg1, "chessmaterial")) 
      {
//        cout << "in countTypeMap. return 2" << endl;
        return nl->SymbolAtom("int");
      }
     }
//     cout << "in countTypeMap. return error" << endl;
     return nl->SymbolAtom("typeerror");
    } 
  return nl->SymbolAtom("typeerror");
}    

/*
Value mapping functions of operator ~chesscount~

*/
int
count_ms_ValueMap (Word* args, Word& result, int message, 
                                   Word& local, Supplier s)
{
//  cout << "in count_ms_ValueMap" << endl;
  ChessMaterial* m1 = ((ChessMaterial*) args[0].addr);
  CcString* s2  = ((CcString*) args[1].addr);
  result = qp->ResultStorage(s);    
  string f1 = *(s2->GetStringval());

  ((CcInt*)result.addr)->Set(false, 0);
  if (f1 == "Pawn")
       ((CcInt*)result.addr)->Set(true, m1->GetWhitePawns());
  if (f1 == "pawn")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackPawns());
  if (f1 == "Knight")
       ((CcInt*)result.addr)->Set(true, m1->GetWhiteKnights());
  if (f1 == "knight")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackKnights());
  if (f1 == "Bishop")
       ((CcInt*)result.addr)->Set(true, m1->GetWhiteBishops());
  if (f1 == "bishop")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackBishops());
  if (f1 == "Rook")
       ((CcInt*)result.addr)->Set(true, m1->GetWhiteRooks());
  if (f1 == "rook")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackRooks());
  if (f1 == "Queen")
       ((CcInt*)result.addr)->Set(true, m1->GetWhiteQueens());
  if (f1 == "queen")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackQueens());
  if (f1 == "King")
       ((CcInt*)result.addr)->Set(true, m1->GetWhiteKings());
  if (f1 == "king")
       ((CcInt*)result.addr)->Set(true, m1->GetBlackKings());
//  cout << "come out count_ms_ValueMap" << endl;
  return 0;
  return 0;
}    

int
count_m_ValueMap (Word* args, Word& result, int message, 
                                    Word& local, Supplier s)
{
//  cout << "in count_m_ValueMap" << endl;
  ChessMaterial* m1 = ((ChessMaterial*) args[0].addr);
  int anzahl = 0;
  result = qp->ResultStorage(s);    
  
  anzahl = anzahl + m1->GetWhitePawns() + m1->GetBlackPawns() 
                  + m1->GetWhiteKnights() + m1->GetBlackKnights();
  anzahl = anzahl + m1->GetWhiteBishops() + m1->GetBlackBishops() 
                  + m1->GetWhiteRooks() + m1->GetBlackRooks();
  anzahl = anzahl + m1->GetWhiteQueens() + m1->GetBlackQueens() 
                  + m1->GetWhiteKings() + m1->GetBlackKings();
  ((CcInt*)result.addr)->Set(true, anzahl);
  return 0;
}

/*
Specificaton of operator ~chesscount~

*/
const string countSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessmaterial x string) -> (int),"
        " (chessmaterial) -> (int) </text--->"
  "<text>_ count _ </text--->"
  "<text>Get the number of figures of a material"
  "</text--->"
  "<text>query chessmaterial chesscount \"Pawn\" </text--->"
  ") )";

/*
Selection function of operator ~chesscount~

*/
ValueMapping countMap[] = {count_ms_ValueMap, count_m_ValueMap};

int countSelect(ListExpr args)
{
 ListExpr arg1 = nl->First ( args );
 ListExpr arg2 = nl->Second ( args );
 if (nl->IsEqual( arg1, "chessmaterial" ) && 
    (nl->IsEqual( arg2, "string" )))
  return (0);
 if (nl->IsEqual( arg1, "chessmaterial" ) && 
    (nl->IsEqual( arg2, "int" )))
  return (1);

// if ( nl->ListLength(args) == 2 )
//  return (0);
// if ( nl->ListLength(args) == 1 )
//  return (1);
 return (-1);
}  

/*
Definition of operator ~chesscount~

*/
Operator chesscount (
   "chesscount",             // name
   countSpec,                // specification
   2,                 // number of functions
   countMap,                 // value mapping
   countSelect,             // selection function
   countTypeMap              // type mapping
);


/*
3.4 Operator ~chesscountall~

Type mapping function of operator ~chesscountall~

*/    
ListExpr
countallTypeMap( ListExpr args )
{
// cout << "in countallTypeMap" << endl;
  if ( nl->ListLength(args) == 1 )
    {
      ListExpr arg1 = nl->First(args);
//      if ( nl->IsEqual(arg1, "chessmaterial") && 
//           nl->IsEqual(arg2, "string") )
      if ( nl->IsEqual(arg1, "chessmaterial") )
      {
//        cout << "in countallTypeMap. return 1" << endl;
        return nl->SymbolAtom("int");
      }
    }
    else 
    {
     return nl->SymbolAtom("typeerror");
    } 
 return nl->SymbolAtom("typeerror");
}    

/*
Value mapping function of operator ~chesscountall~

*/
int
countallValueMap (Word* args, Word& result, int message, 
                                    Word& local, Supplier s)
{
//  cout << "in countallValueMap" << endl;
  ChessMaterial* m1 = ((ChessMaterial*) args[0].addr);
  int anzahl = 0;
  result = qp->ResultStorage(s);    
  
  anzahl = anzahl + m1->GetWhitePawns() + m1->GetBlackPawns() 
                  + m1->GetWhiteKnights() + m1->GetBlackKnights();
  anzahl = anzahl + m1->GetWhiteBishops() + m1->GetBlackBishops() 
                  + m1->GetWhiteRooks() + m1->GetBlackRooks();
  anzahl = anzahl + m1->GetWhiteQueens() + m1->GetBlackQueens() 
                  + m1->GetWhiteKings() + m1->GetBlackKings();
  ((CcInt*)result.addr)->Set(true, anzahl);
  return 0;
}

/*
Specificaton of operator ~chesscountall~

*/
const string countallSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessmaterial) -> (int) </text--->"
  "<text>_ countall </text--->"
  "<text>Get the number of figures of a material"
  "</text--->"
  "<text>query chessmaterial countall </text--->"
  ") )";

/*
Selection function of operator ~chesscountall~

*/

ValueMapping countallMap[] = {countallValueMap};

int countallSelect(ListExpr args)
{
// ListExpr arg1 = nl->First ( args );
 return (0);
}  

/*
Definition of operator ~chesscountall~

*/
Operator chesscountall (
   "chesscountall",             // name
   countallSpec,                // specification
   1,                    // number of functions
   countallMap,                 // value mapping
   countallSelect,        // trivial selection function
   countallTypeMap              // type mapping
);

/*
3.5 Operator ~getpos~

Type mapping function of operator ~getpos~

*/
ListExpr
getposTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 2,
  "Operator getpos expects a list of length two.");
  
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (( nl->IsEqual(arg1, "chessgame")) && 
        (nl->IsEqual(arg2, "int")))
       return nl->SymbolAtom("chessposition");
    else
       return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}    

/*
Value mapping function of operator ~getpos~

*/
int
getposValueMap (Word* args, Word& result, int message, 
                                 Word& local, Supplier s)
{
  ChessGame* s1 = ((ChessGame*) args[0].addr);
  int* nr  = ((int*) args[1].addr);
  int no = ((CcInt*) nr)->GetIntval();
  result = qp->ResultStorage(s);
  if ( ( no < s1->GetNoChessMoves() || 
         no == s1->GetNoChessMoves() ) && 
       s1->IsDefined() )
  { 
     ((ChessPosition*)result.addr) = s1->GetPosition(no);
  }
  else
  {  
     char ChessField[8][8];
     for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
        ChessField[i][j] = '-';
     *((ChessPosition*)result.addr) = ChessPosition(
                                     false, ChessField, 0);
  }
  return 0;
}    

/*
Specification of operator ~getpos~

*/
const string getposSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessgame x int) -> (chessposition) </text--->"
  "<text>getpos( _, _ )</text--->"
  "<text>Get a number of a chessposition, returns the chessposition."
  "</text--->"
  "<text>query getpos(game, 3)</text--->"
  ") )";

/*
Definition of operator ~getpos~

*/
Operator getpos (
   "getpos",                 // name
   getposSpec,               // specification
   getposValueMap,           // value mapping
   Operator::SimpleSelect,   // trivial selection function
   getposTypeMap             // type mapping
);

/*
3.6 Operator ~positions~

Type mapping function of operator ~positions~

*/
ListExpr
positionsTypeMap( ListExpr args )
{ 
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessgame") )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
                             nl->SymbolAtom("chessposition"));
    else
      return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~positions~

*/
struct LocalInfo
{
  ChessGame* gameinfo;
  int number;
//  Interval<Instant> iv(const DateTime& sta, const DateTime& 
//                         end, const bool lc, const bool rc);
  MPoint* mpointaddr;
  string movpoints;
  char figure[32];
  char figureno[32];
  bool white[32];
  int posix [32];
// allocate NON dynamic memory - start
  char pos [32][200][2];
// allocate NON dynamic memory - end
  const DateTime start, ende;
  TupleType *resultTupleType;

//  char* pos;
//  allocate dynamic memory - start1
//  typedef char (*wert_a)[32][2];
//  wert_a pos;     
//  allocate dynamic memory - end1
    
  LocalInfo( ChessGame* game, int no)
  {
    gameinfo = game;
    number = no;
  }
  
  LocalInfo( ChessGame* game, int no, char* moveix)
  {

    gameinfo = game;
    number = no;

// allocate dynamic memory - start2
//    pos = (wert_a)moveix;
// allocate dynamic memory - end2

   
  }
  
  
};

struct LocalInfo1
{
  ChessGame* gameinfo;
  int number;
    
  LocalInfo1( ChessGame* game, int no)
  {
    gameinfo = game;
    number = no;
  }  
};

int
positionsFun (Word* args, Word& result, int message, 
                                Word& local, Supplier s)
{ 
  LocalInfo *info;
  ChessGame* game;
  int n;
  switch( message )
  { 
    case OPEN:
      game = ((ChessGame*)args[0].addr);      
      local.addr = new LocalInfo(game, 0);
      return 0;

    case REQUEST:
      info = (LocalInfo*)local.addr;
      n = info->number;
      game = info->gameinfo;
      if ( ( n < game->GetNoChessMoves() || 
             n == game->GetNoChessMoves() ) )
      { 
        const ChessPosition* pos;
        pos = game->GetPosition(n);
    info->number++;
    ChessPosition* pos2 = pos->Clone();
        result.addr = pos2;
        return YIELD;
      }
      else return CANCEL;

    case CLOSE:
      info = ((LocalInfo*) local.addr);
      delete info;
      return 0;
  }
  /* should not happen */
  return -1;
 
}

/*
Specification of operator ~positions~

*/
const string positionsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessgame -> stream(chessposition)</text--->"
  "<text>positions (_) </text--->"
  "<text>Creates a stream of chesspositions from the chessgame."
  "</text--->"
  "<text>query positions(game)</text--->"
  ") )";

/*
Definition of operator ~positions~

*/
Operator positions (
   "positions",                      // name
   positionsSpec,                    // specification
   positionsFun,                     // value mapping
   Operator::SimpleSelect,           // trivial selection function
   positionsTypeMap                  // type mapping
);
  
/*
3.7 Operator ~moves~

Type mapping function of operator ~moves~

*/
ListExpr
movesTypeMap( ListExpr args )
{ 
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessgame") )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
                             nl->SymbolAtom("chessmove"));
    else
      return nl->SymbolAtom("typeerror");
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~moves~

*/
int
movesFun (Word* args, Word& result, int message, 
                                Word& local, Supplier s)
{ 
  LocalInfo1 *info;
  ChessGame* game;
  int n;
  switch( message )
  { 
    case OPEN:
      game = ((ChessGame*)args[0].addr);      
      local.addr = new LocalInfo1(game, 0);
      return 0;

    case REQUEST:
      info = (LocalInfo1*)local.addr;
      n = info->number;
      game = info->gameinfo;
      if ( n < game->GetNoChessMoves() )
      { 
        const ChessMove* elem;
        elem = game->GetChessMove(n);
    info->number++;
    ChessMove* elem2 = elem->Clone();
        result.addr = elem2;
        return YIELD;
      }
      else {
        return CANCEL;
      }
      
    case CLOSE:
      info = ((LocalInfo1*) local.addr);
      delete info;
      return 0;
  }
  /* should not happen */
  return -1;
 
}

/*
Specification of operator ~moves~

*/
const string movesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessgame -> stream(chessmove)</text--->"
  "<text>moves (_)</text--->"
  "<text>Creates a stream of chessmoves from the chessgame."
  "</text--->"
  "<text>query ten feed head[1] extendstream[Moves: "
             "moves(game)] project[Moves] consume</text--->"
  ") )";

/*
Definition of operator ~moves~

*/
Operator moves (
   "moves",                      // name
   movesSpec,                    // specification
   movesFun,                     // value mapping
   Operator::SimpleSelect,       // trivial selection function
   movesTypeMap                  // type mapping
);
  
/*
3.8 Operator ~moveno~

Type mapping function of operator ~moveno~

*/        
ListExpr
moveNoTypeMap( ListExpr args )
{
// cout << "in moveNoTypeMap" << endl;
  if ( nl->ListLength(args) == 1 )
    {
      ListExpr arg1 = nl->First(args);
      if (nl->IsEqual(arg1, "chessposition") || 
         (nl->IsEqual(arg1, "chessmove") ))
      {
        return nl->SymbolAtom("int");
      }
    }
    else 
    {
     return nl->SymbolAtom("typeerror");
    } 
  return nl->SymbolAtom("typeerror");
}    

/*
Value mapping function of operator ~moveno~

*/
int
moveNo_pos_ValueMap (Word* args, Word& result, int message, 
                                       Word& local, Supplier s)
{
//  cout << "in count_ms_ValueMap" << endl;
  ChessPosition* p1 = (( ChessPosition*) args[0].addr);
  result = qp->ResultStorage(s);    

  ((CcInt*)result.addr)->Set(true, p1->GetPosNr());
  return 0;
}    

int
moveNo_mov_ValueMap (Word* args, Word& result, int message, 
                                       Word& local, Supplier s)
{
//  cout << "in moveno_mov_ValueMap" << endl;
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  ((CcInt*)result.addr)->Set(true, m1->GetMoveNr());
  return 0;
}

/*
Specification of operator ~moveno~

*/
const string moveNoSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(chessposition) -> (int), (chessmove) -> (int) </text--->"
  "<text>moveno (_)</text--->"
  "<text>Get the number of a position or a half-move"
  "</text--->"
  "<text>query moveno (chessmove) </text--->"
  ") )";

/*
Selection function of operator ~moveno~

*/
ValueMapping moveNoMap[] = {moveNo_pos_ValueMap, 
                            moveNo_mov_ValueMap};

int moveNoSelect(ListExpr args)
{
 ListExpr arg1 = nl->First ( args );
 if (nl->IsEqual( arg1, "chessposition" ))
  return (0);
 if (nl->IsEqual( arg1, "chessmove" ))
  return (1);

 return (-1);
}  
 
/*
Definition of operator ~moveno~

*/
Operator moveno (
   "moveno",             // name
   moveNoSpec,           // specification
   2,             // number of functions
   moveNoMap,            // value mapping
   moveNoSelect,     // selection function
   moveNoTypeMap         // type mapping
);

/*
3.9 Operator ~pieces~

Type mapping function of operator ~pieces~

*/
ListExpr
piecesTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 1, 
        "Operator pieces expects a list of length one."); 
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessposition") )
       return nl->SymbolAtom("chessmaterial"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~pieces~

*/
ChessMaterial*
ChessPosition::pieces()
{
   int white[6];
   int black[6];
   white[0] = 0;
   white[1] = 0;
   white[2] = 0;
   white[3] = 0;
   white[4] = 0;
   white[5] = 0;
   black[0] = 0;
   black[1] = 0;
   black[2] = 0;
   black[3] = 0;
   black[4] = 0;
   black[5] = 0;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
      {
          if ( chessField[i][j] == 'P' )
         white[0] = white[0] + 1;
      else if ( chessField[i][j] == 'N' )
         white[1] = white[1] + 1;
      else if ( chessField[i][j] == 'B' )
         white[2] = white[2] + 1;
      else if ( chessField[i][j] == 'R' )
         white[3] = white[3] + 1;
      else if ( chessField[i][j] == 'Q' )
         white[4] = white[4] + 1;
      else if ( chessField[i][j] == 'K' )
         white[5] = white[5] + 1;
      else if ( chessField[i][j] == 'p' )
         black[0] = black[0] + 1;
      else if ( chessField[i][j] == 'n' )
         black[1] = black[1] + 1;
      else if ( chessField[i][j] == 'b' )
         black[2] = black[2] + 1;
      else if ( chessField[i][j] == 'r' )
         black[3] = black[3] + 1;
      else if ( chessField[i][j] == 'q' )
         black[4] = black[4] + 1;
      else if ( chessField[i][j] == 'k' )
         black[5] = black[5] + 1;
       }
    ChessMaterial* mat = new ChessMaterial(true, white, black);
    return mat;
}

int
piecesFun (Word* args, Word& result, int message, 
                             Word& local, Supplier s)
{
  ChessPosition* p = ((ChessPosition*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( p->IsDefined() && (p->pieces())->IsDefined() )
     ((ChessMaterial*)result.addr) = p->pieces();
  else
  {
     int nullPlayer[6]; 
     *((ChessMaterial*)result.addr) = ChessMaterial(
                        false, nullPlayer, nullPlayer);
  }
  return 0;
}

/*
Specification of operator ~pieces~

*/
const string piecesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessposition -> chessmaterial</text--->"
  "<text>pieces (_)</text--->"
  "<text>Gives the material of a position."
  "</text--->"
  "<text>query pieces (position)</text--->"
  ") )";

/*
Definition of operator ~pieces~

*/
Operator pieces (
   "pieces",                      // name
   piecesSpec,                    // specification
   piecesFun,                     // value mapping
   Operator::SimpleSelect,        // trivial selection function
   piecesTypeMap                  // type mapping
);
  
/*
3.10 Operator ~agent~ and ~captured~

Type mapping function of operator ~agent~ and ~captured~

*/

ListExpr
agentcapturedTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 1, 
         "Operator agent/captured expects a list of length one."); 
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessmove"))
    { 
       return nl->SymbolAtom("string"); }
  }
  return nl->SymbolAtom("typeerror");
}

char* GetName(const char figure[2])
{
  if ( figure[0] == 'P' )
     return "Pawn";
  else if ( figure[0] == 'p' )
     return "pawn";
  else if ( figure[0] == 'N' )
     return "Knight";
  else if ( figure[0] == 'n' )
     return "knight";
  else if ( figure[0] == 'B' )
     return "Bishop";
  else if ( figure[0] == 'b' )
     return "bishop"; 
  else if ( figure[0] == 'R' )
     return "Rook";  
  else if ( figure[0] == 'r' )
     return "rook";
  else if ( figure[0] == 'Q' )
     return "Queen"; 
  else if ( figure[0] == 'q' )
     return "queen";
  else if ( figure[0] == 'K' )
     return "King";
  else if ( figure[0] == 'k' )
     return "king";
  else
     return "none";
}
 
/*
Value mapping function of operator ~agent~

*/    
int
agentFun (Word* args, Word& result, int message, 
                            Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
  {  
     ((CcString*)result.addr)->Set(true, 
              ((STRING*)GetName(m1->GetFigureName())) );
  }
  else
     ((CcString*)result.addr)->Set(false, ((STRING*)"none"));
  return 0;
}

/*
Specification of operator ~agent~

*/
const string agentSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> string</text--->"
  "<text>agent (_) </text--->"
  "<text>Get the name of the figure in the chessmove."
  "</text--->"
  "<text>query agent (move1)</text--->"
  ") )";

/*
Definition of operator ~agent~

*/
Operator agent (
   "agent",                  // name
   agentSpec,                // specification
   agentFun,                 // value mapping
   Operator::SimpleSelect,   // trivial selection function
   agentcapturedTypeMap      // type mapping
);

/*
Value mapping function of operator ~captured~

*/
int
capturedFun (Word* args, Word& result, int message, 
                               Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
  {  
     ((CcString*)result.addr)->Set(true, 
               ((STRING*)GetName(m1->GetCapturedFigure())) );
  }
  else
     ((CcString*)result.addr)->Set(false, ((STRING*)"none"));
  return 0;
}

/*
Specification of operator ~captured~

*/
const string capturedSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> string</text--->"
  "<text>captured (_)</text--->"
  "<text>Get the name of the captured figure in the chessmove."
  "</text--->"
  "<text>query captured (move1)</text--->"
  ") )";

/*
Definition of operator ~captured~

*/
Operator captured (
   "captured",                  // name
   capturedSpec,                // specification
   capturedFun,                 // value mapping
   Operator::SimpleSelect,      // trivial selection function
   agentcapturedTypeMap         // type mapping
);
 
/*
3.11 Operator ~startrow~ and ~endrow~

Type mapping function of operator ~startrow~ and ~endrow~

*/
ListExpr
startendrowTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 1, 
         "Operator startrow/endrow expects a list of length one."); 
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessmove"))
       return nl->SymbolAtom("int"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~startrow~

*/
int
ChessMove::startrow()
{ 
   int start = int(startPosition[1]) - 48;
   return start;
}

int
startrowFun (Word* args, Word& result, int message, 
                               Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
     ((CcInt*)result.addr)->Set(true, m1->startrow());
  else
     ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
Specification of operator ~startrow~

*/
const string startrowSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> int</text--->"
  "<text>startrow (_) </text--->"
  "<text>Gives the startrow of the move."
  "</text--->"
  "<text>query startrow (move1)</text--->"
  ") )";

/*
Definition of operator ~startrow~

*/
Operator startrow (
   "startrow",                  // name
   startrowSpec,                // specification
   startrowFun,                 // value mapping
   Operator::SimpleSelect,      // trivial selection function
   startendrowTypeMap           // type mapping
);

/*
Value mapping function of operator ~endrow~

*/
int
ChessMove::endrow()
{ 
   int end = int(endPosition[1]) - 48;
   return end;
}

int
endrowFun (Word* args, Word& result, int message, 
                             Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
     ((CcInt*)result.addr)->Set(true, m1->endrow());
  else
     ((CcInt*)result.addr)->Set(false, 0);
  return 0;
}

/*
Specification of operator ~endrow~

*/
const string endrowSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> int</text--->"
  "<text>endrow (_) </text--->"
  "<text>Gives the endrow of the move."
  "</text--->"
  "<text>query endrow (move1)</text--->"
  ") )";

/*
Definition of operator ~endrow~

*/
Operator endrow (
   "endrow",                  // name
   endrowSpec,                // specification
   endrowFun,                 // value mapping
   Operator::SimpleSelect,    // trivial selection function
   startendrowTypeMap         // type mapping
);

/*
3.12 Operator ~startfile~ and ~endfile~

Type mapping function of operator ~startfile~ and ~endfile~

*/
ListExpr
startendfileTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 1, 
         "Operator startfile/endfile expects a list of length one."); 
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessmove"))
       return nl->SymbolAtom("string"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~startfile~

*/
int
startfileFun (Word* args, Word& result, int message, 
                                Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);
  if ( m1->IsDefined() )
  {
     char* start;
     start[0] = (m1->GetStartPosition()[0]);
     start[1] = '\0';
     ((CcString*)result.addr)->Set(true, ((STRING*)start)); 
  }
  else
     ((CcString*)result.addr)->Set(false, ((STRING*)"none"));
  return 0;
}

/*
Specification of operator ~startfile~

*/
const string startfileSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> string</text--->"
  "<text>startfile (_) </text--->"
  "<text>Gives the startfile of the move."
  "</text--->"
  "<text>query startfile (move1)</text--->"
  ") )";

/*
Definition of operator ~startfile~

*/
Operator startfile (
   "startfile",                  // name
   startfileSpec,                // specification
   startfileFun,                 // value mapping
   Operator::SimpleSelect,       // trivial selection function
   startendfileTypeMap           // type mapping
);

/*
Value mapping function of operator ~endfile~

*/
int
endfileFun (Word* args, Word& result, int message, 
                              Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
  {
     char* end;
     end[0] = (m1->GetEndPosition()[0]);
     end[1] = '\0';
     ((CcString*)result.addr)->Set(true, ((STRING*)end));
  }
  else
     ((CcString*)result.addr)->Set(false, ((STRING*)"none"));
  return 0;
}

/*
Specification of operator ~endfile~

*/
const string endfileSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> string</text--->"
  "<text>endfile (_) </text--->"
  "<text>Gives the endfile of the move."
  "</text--->"
  "<text>query endfile (move1)</text--->"
  ") )";

/*
Definition of operator ~endfile~

*/
Operator endfile (
   "endfile",                  // name
   endfileSpec,                // specification
   endfileFun,                 // value mapping
   Operator::SimpleSelect,     // trivial selection function
   startendfileTypeMap         // type mapping
);

/*
Operator ~check~ and ~captures~

Type mapping function of operator ~check~ and ~captures~

*/
ListExpr
checkcapturesTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 1, 
         "Operator check/captures expects a list of length one."); 
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessmove"))
       return nl->SymbolAtom("bool"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~check~

*/
bool
ChessMove::check()
{
   if ( action[0] == 'c' )
      return true;
   else
      return false;
}

int
checkFun (Word* args, Word& result, int message, 
                            Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
     ((CcBool*)result.addr)->Set(true, m1->check());
  else
     ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
Specification of operator ~check~

*/
const string checkSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> bool</text--->"
  "<text>check (_) </text--->"
  "<text>Checks whether check."
  "</text--->"
  "<text>query check (move1)</text--->"
  ") )";

/*
Definition of operator ~check~

*/
Operator check (
   "check",                  // name
   checkSpec,                // specification
   checkFun,                 // value mapping
   Operator::SimpleSelect,   // trivial selection function
   checkcapturesTypeMap      // type mapping
);

/*
Value mapping function of operator ~captures~

*/
bool
ChessMove::captures()
{
   if ( capturedFigure[0] == 'P' || capturedFigure[0] == 'B' || 
        capturedFigure[0] == 'N' || capturedFigure[0] == 'R' || 
    capturedFigure[0] == 'Q' || capturedFigure[0] == 'K' ||
    capturedFigure[0] == 'p' || capturedFigure[0] == 'b' || 
    capturedFigure[0] == 'n' || capturedFigure[0] == 'r' || 
    capturedFigure[0] == 'q' || capturedFigure[0] == 'k' )
      return true;
   else
      return false;
}

int
capturesFun (Word* args, Word& result, int message, 
                               Word& local, Supplier s)
{
  ChessMove* m1 = ((ChessMove*) args[0].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() )
     ((CcBool*)result.addr)->Set(true, m1->captures());
  else
     ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
Specification of operator ~captures~

*/
const string capturesSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmove -> bool</text--->"
  "<text>captures (_) </text--->"
  "<text>Checks whether figure is captures."
  "</text--->"
  "<text>query captures (move1)</text--->"
  ") )";

/*
Definition of operator ~captures~

*/
Operator captures (
   "captures",                  // name
   capturesSpec,                // specification
   capturesFun,                 // value mapping
   Operator::SimpleSelect,      // trivial selection function
   checkcapturesTypeMap         // type mapping
);

/*
3.13 Operator ~chessrange~

Type mapping function of operator ~chessrange~

*/
ListExpr
chessrangeTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 5, 
         "Operator chessrange expects a list of length five."); 
  if ( nl->ListLength(args) == 5 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    ListExpr arg3 = nl->Third(args);
    ListExpr arg4 = nl->Fourth(args);
    ListExpr arg5 = nl->Fifth(args);
    if ( nl->IsEqual(arg1, "chessposition") &&
         nl->IsEqual(arg2, "string") && nl->IsEqual(arg3, "int") &&
     nl->IsEqual(arg4, "string") && nl->IsEqual(arg5, "int") )
       return nl->SymbolAtom("chessposition"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~chessrange~

*/
ChessPosition*
ChessPosition::chessrange(string f1, int r1, string f2, int r2)
{
   char ChessField[8][8];
   int PosNr = 0;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
      {
         ChessField[i][j] = '-';
      }
   if ( f1 <= f2 && r1 <= r2 )
   {
      const char f1char = f1.c_str()[0];
      const char f2char = f2.c_str()[0];
      int file1 = int(f1char) - 97;
      int file2 = int(f2char) - 97;
//    cout << " file1: " << file1 << " file2: " << file2;
//    for ( int i = r1 -1; i < r2; i++ )
//    for ( int i = 8 - r1 - 1; i > 8 - r2 - 2; i-- )
      for ( int i = 8 - r2; i < 8 - r1 + 1; i++ )
         for ( int j = file1; j < file2 + 1; j++ )
     {
        ChessField[i][j] = this->chessField[i][j];
     }
      PosNr = this->posNr;
   }
   ChessPosition* pos = new ChessPosition(true, ChessField, PosNr);
   return pos;
}

int
chessrangeFun (Word* args, Word& result, int message, 
                                    Word& local, Supplier s)
{
  ChessPosition* p = ((ChessPosition*) args[0].addr);
  CcString* file1  = ((CcString*) args[1].addr);    
  string f1 = *(file1->GetStringval());
  CcInt* row1 = ((CcInt*) args[2].addr);
  int r1 = row1->GetIntval();
  CcString* file2  = ((CcString*) args[3].addr);    
  string f2 = *(file2->GetStringval());
  CcInt* row2 = ((CcInt*) args[4].addr);
  int r2 = row2->GetIntval();
  result = qp->ResultStorage(s);    
  if ( p->IsDefined() )
     ((ChessPosition*)result.addr) = p->chessrange(f1, r1, f2, r2);
  else
  {
     char ChessField[8][8]; 
     *((ChessPosition*)result.addr) = ChessPosition(
                                             false, ChessField, 0);
  }
  return 0;
}

/*
Specification of operator ~chessrange~

*/
const string chessrangeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessposition x string x int x string x int "
              "-> chessposition</text--->"
  "<text>chessrange (_, _, _, _, _)</text--->"
  "<text>Gives an extract of the figures from the position, "
                          "they are between the four values."
  "</text--->"
  "<text>query chessrange (position, \"a\", 1, \"a\", 8)</text--->"
  ") )";

/*
Definition of operator ~chessrange~

*/
Operator chessrange (
   "chessrange",                      // name
   chessrangeSpec,                    // specification
   chessrangeFun,                     // value mapping
   Operator::SimpleSelect,            // trivial selection function
   chessrangeTypeMap                  // type mapping
);
  
/*
3.14 Operator ~chessincludes~

Type mapping function of operator ~chessincludes~

*/
ListExpr
chessincludesTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 2, 
                "Operator includes expects a list of length two."); 
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "chessposition") && 
         nl->IsEqual(arg2, "chessposition") )
       return nl->SymbolAtom("bool"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~chessincludes~

*/
bool
ChessPosition::chessincludes(const ChessPosition p)
{
   bool chessinclude = true;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
         if ( chessField[i][j] != '-' && 
          chessField[i][j] != p.chessField[i][j] )
         chessinclude = false;
   return chessinclude;
}

int
chessincludesFun (Word* args, Word& result, int message, 
                                     Word& local, Supplier s)
{
  ChessPosition* p1 = ((ChessPosition*) args[0].addr);
  ChessPosition* p2 = ((ChessPosition*) args[1].addr);
  result = qp->ResultStorage(s);    
  if ( p1->IsDefined() && p2->IsDefined() )
     ((CcBool*)result.addr)->Set(true, p1->chessincludes(*p2));
  else
     ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
Specification of operator ~chessincludes~

*/
const string chessincludesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessposition x chessposition -> bool</text--->"
  "<text>_ includes _</text--->"
  "<text>Checks whether first chessposition includes "
                        "in the second chessposition."
  "</text--->"
  "<text>query position1 includes position2</text--->"
  ") )";

/*
Definition of operator ~chessincludes~

*/
Operator chessincludes (
   "includes",                      // name
   chessincludesSpec,               // specification
   chessincludesFun,                // value mapping
   Operator::SimpleSelect,          // trivial selection function
   chessincludesTypeMap             // type mapping
);

/*
3.15 Operator ~chessequal~ and ~chesslower~

Type mapping function of operator ~chessequal~ and ~chesslower~

*/

ListExpr
chessequallowerTypeMap( ListExpr args )
{
  CHECK_COND(nl->ListLength(args) == 2, 
             "Operator =/ < expects a list of length two."); 
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "chessmaterial") && 
         nl->IsEqual(arg2, "chessmaterial") )
       return nl->SymbolAtom("bool"); 
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~chessequal~

*/
bool
ChessMaterial::chessequal(const ChessMaterial m)
{
   int value = 0;
   for ( int i = 0; i < 6; i++ )
   {
       if ( whitePlayer[i] == m.whitePlayer[i] )
          value = value + 1;
       if ( blackPlayer[i] == m.blackPlayer[i] )
          value = value + 1;
   }
   if ( value == 12 )
       return true;
   else
      return false;
}

int
chessequalFun (Word* args, Word& result, int message, 
                                      Word& local, Supplier s)
{
  ChessMaterial* m1 = ((ChessMaterial*) args[0].addr);
  ChessMaterial* m2 = ((ChessMaterial*) args[1].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() && m2->IsDefined() )
     ((CcBool*)result.addr)->Set(true, m1->chessequal(*m2));
  else
     ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
Specification of operator ~chessequal~

*/
const string chessequalSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmaterial x chessmaterial -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Checks whether two chessmaterials are equal."
  "</text--->"
  "<text>query material1 = material2</text--->"
  ") )";

/*
Definition of operator ~chessequal~

*/
Operator chessequal (
   "=",                               // name
   chessequalSpec,                    // specification
   chessequalFun,                     // value mapping
   Operator::SimpleSelect,            // trivial selection function
   chessequallowerTypeMap             // type mapping
);

/*
Value mapping function of operator ~chesslower~

*/
bool
ChessMaterial::chesslower(const ChessMaterial m)
{
   int sum = 0;
   int summ = 0;
   for ( int i = 0; i < 6; i++ )
   {
       sum = sum + whitePlayer[i] + blackPlayer[i];
       summ = summ + m.whitePlayer[i] + m.blackPlayer[i];
   }
   if ( sum < summ )
       return true;
   else
       return false;
}

int
chesslowerFun (Word* args, Word& result, int message, 
                                         Word& local, Supplier s)
{
  ChessMaterial* m1 = ((ChessMaterial*) args[0].addr);
  ChessMaterial* m2 = ((ChessMaterial*) args[1].addr);
  result = qp->ResultStorage(s);    
  if ( m1->IsDefined() && m2->IsDefined() )
     ((CcBool*)result.addr)->Set(true, m1->chesslower(*m2));
  else
     ((CcBool*)result.addr)->Set(false, false);
  return 0;
}

/*
Specification of operator ~chesslower~

*/
const string chesslowerSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessmaterial x chessmaterial -> bool</text--->"
  "<text>_ < _</text--->"
  "<text>Checks whether one chessmaterial is lower than the other."
  "</text--->"
  "<text>query material1 < material2</text--->"
  ") )";

/*
Definition of operator ~chesslower~

*/
Operator chesslower (
   "<",                               // name
   chesslowerSpec,                    // specification
   chesslowerFun,                     // value mapping
   Operator::SimpleSelect,            // trivial selection function
   chessequallowerTypeMap             // type mapping
);

/*
3.16 Operator ~movingpoints~

Type mapping function of operator ~movingpoints~

*/
ListExpr
movingpointsTypeMap( ListExpr args )
{
//  cout << "in mpTypeMap" << endl;
  ListExpr arg1, outlist;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "chessgame") )
    {
    outlist = nl->TwoElemList
            ( nl->SymbolAtom("stream"), nl->TwoElemList
           ( nl->SymbolAtom("tuple"), nl->ThreeElemList
              (nl->TwoElemList( nl->SymbolAtom("Pieces"),
                                nl->SymbolAtom("string")),
               nl->TwoElemList( nl->SymbolAtom("White"), 
                            nl->SymbolAtom("bool")),
               nl->TwoElemList( nl->SymbolAtom("Route"), 
                            nl->SymbolAtom("mpoint")))));
//    cout << "come out mpTypeMap: if ";
    return outlist;
           
//      nl->SymbolAtom("upoint"));
     
    }
    else
       return nl->SymbolAtom("typeerror");
  }
//  cout << "come out mpTypeMap: error" << endl;
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~movingpoints~

*/
void
movingpointsinittab (LocalInfo *info)
{
//    INIT - figurename - pawns 
      for (int i = 0; i < 8; i ++)
      {
      info->figure[i+8] = 'P';
//      cout << "figure[i+8]: " << info->figure[i+8] << endl;
      info->figure[i+16] = 'p';
//      info->figureno[i+8] = c;
//      cout << "figureno[i+8]: " << info->figureno[i+8] << endl;
 //     info->figureno[i+16] = (char) j;
      }
//      cout << "inittab 1" << endl;
      info->figureno[8] = '1';
      info->figureno[9] = '2';
      info->figureno[10] = '3';
      info->figureno[11] = '4';
      info->figureno[12] = '5';
      info->figureno[13] = '6';
      info->figureno[14] = '7';
      info->figureno[15] = '8';
       
      info->figureno[16] = '1';
      info->figureno[17] = '2';
      info->figureno[18] = '3';
      info->figureno[19] = '4';
      info->figureno[20] = '5';
      info->figureno[21] = '6';
      info->figureno[22] = '7';
      info->figureno[23] = '8';
     
//      cout << "inittab 2" << endl;
//    INIT - figurename - white figures
      info->figure[0] = 'R';
      info->figureno[0] = '1';
      info->figure[7] = 'R';
      info->figureno[7] = '2';
      info->figure[1] = 'N';
      info->figureno[1] = '1';
      info->figure[6] = 'N';
      info->figureno[6] = '2';
      info->figure[2] = 'B';
      info->figureno[2] = '1';
      info->figure[5] = 'B';
      info->figureno[5] = '2';
      info->figure[3] = 'Q';
      info->figureno[3] = '1';
      info->figure[4] = 'K';
      info->figureno[4] = '1';
//    INIT - figurename - black figures
      info->figure[24] = 'r';
      info->figureno[24] = '1';
      info->figure[31] = 'r';
      info->figureno[31] = '2';
      info->figure[25] = 'n';
      info->figureno[25] = '1';
      info->figure[30] = 'n';
      info->figureno[30] = '2';
      info->figure[26] = 'b';
      info->figureno[26] = '1';
      info->figure[29] = 'b';
      info->figureno[29] = '2';
      info->figure[27] = 'q';
      info->figureno[27] = '1';
      info->figure[28] = 'k';
      info->figureno[28] = '1';


//      cout << "inittab 3" << endl;
//    INIT - color - white figures
      for (int i = 0; i < 16; i ++)
      {
      info->white[i] = true;
      }
//    INIT - color black figures
      for (int i = 16; i < 32; i ++)
      {
      info->white[i] = false;
      }
//    INIT - actual position-index
      for (int i = 0; i < 32; i ++)
      {
        info->posix[i] = 0;
 //     cout << "posix(" << i << "): " << info->posix[i] << endl;
      }
//      cout << "inittab 4" << endl;
//    INIT - figures in row 1
      for (int i = 0; i < 8; i ++)
      {
      info->pos [i][0][1] = '1';
      }
//      cout << "inittab 5" << endl;

//    INIT - figures in row 2
      for (int i = 8; i < 16; i ++)
      {
      info->pos [i][0][1] = '2';
      }
//    INIT - figures in row 7
      for (int i = 16; i < 24; i ++)
      {
      info->pos [i][0][1] = '7';
      }
//      cout << "inittab 7" << endl;
//    INIT - figures in row 8
      for (int i = 24; i < 32; i ++)
      {
      info->pos [i][0][1] = '8';
      }
//      cout << "inittab 8" << endl;
//    INIT - figures in file a
      for (int i = 0; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'a';
      }
//      cout << "inittab 9" << endl;
//    INIT - figures in file b
      for (int i = 1; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'b';
      }
//    INIT - figures in file c
      for (int i = 2; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'c';
      }
//      cout << "inittab 10" << endl;
//    INIT - figures in file d
      for (int i = 3; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'd';
      }
//      cout << "inittab 11" << endl;
//    INIT - figures in file e
      for (int i = 4; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'e';
      }
//      cout << "inittab 12" << endl;
//    INIT - figures in file f
      for (int i = 5; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'f';
      }
 
//      cout << "inittab 13" << endl;
//    INIT - figures in file g
      for (int i = 6; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'g';
      }
//      cout << "inittab 14" << endl;
 //    INIT - figures in file h
      for (int i = 7; i < 32; i = i+8)
      {
      info->pos [i][0][0] = 'h';
      }
   
// cout << "gehe aus init" << endl;
}

void
movingpointsmovintab (LocalInfo *info, ChessGame* game)
{
string action;
//    
//     Filling moves
//      
      
      for (int i = 0; i < game->GetNoChessMoves(); i++)
      {
//
//     Filling moves -  captured figure 
// 
       
    if ((( game->GetChessMove(i))->GetCapturedFigure()) != "-" )
    {
          for (int j = 0; j < 32; j ++)
          {
        if ((( game->GetChessMove(i))->GetEndPosition())[0] == 
                       info->pos [j][info->posix[j]][0] &&
            (( game->GetChessMove(i))->GetEndPosition())[1] == 
                   info->pos [j][info->posix[j]][1]  )
        {
//          figure was captured
          info->posix[j] = i + 1;
              info->pos [j][i+1][0] = '0';
              info->pos [j][i+1][1] = '0';
          j = 32;
        }
            else
        {
//          not the correct figure - try next
        }     
          }
        }
//        
//    Filling moves -  write the moved position (except castling)
//        
        for (int j = 0; j < 32 ; j ++)
        {
         if ((( game->GetChessMove(i))->GetStartPosition())[0] == 
                          info->pos [j][info->posix[j]][0] &&
        (( game->GetChessMove(i))->GetStartPosition())[1] == 
                          info->pos [j][info->posix[j]][1]  )
      {
//          moved figure was found
        info->posix[j] = i + 1;
            info->pos [j][i+1][0] = (( game->GetChessMove(i))
                        ->GetEndPosition())[0];
            info->pos [j][i+1][1] = (( game->GetChessMove(i))
                        ->GetEndPosition())[1];
          }
          else
      {
//          not the correct figure - no move and not captured
        if (info->posix[j] != (i + 1))
        {
          info->pos [j][i+1][0] = '-';
              info->pos [j][i+1][1] = '-';
        }
      } 
        }
//        
//Filling moves - rochade - write the moved position (only castling!)
//        
        action = (game->GetChessMove(i))->GetAction();
        if (action[2] != '-' )
    {
          if (!(i % 2)) 
          {
//         figurecolor white
            if (action[2] == 's') 
            {
//           figurecolor white and short castling
//           king move
          info->posix[4] = i + 1;
              info->pos [4][i+1][0] = 'g';
              info->pos [4][i+1][1] = '1';
//           rook move
          info->posix[7] = i + 1;
              info->pos [7][i+1][0] = 'f';
              info->pos [7][i+1][1] = '1';
        }
            else
        {
//           figurecolor white and long castling
//           king move
          info->posix[4] = i + 1;
              info->pos [4][i+1][0] = 'c';
              info->pos [4][i+1][1] = '1';
//           rook move
          info->posix[0] = i + 1;
              info->pos [0][i+1][0] = 'd';
              info->pos [0][i+1][1] = '1';
        }     
          }
      else
      {
//         figurecolor black
            if (action[2] == 's') 
            {
//           figurecolor white and short castling
//           king move
          info->posix[28] = i + 1;
              info->pos [28][i+1][0] = 'g';
              info->pos [28][i+1][1] = '8';
//           rook move
          info->posix[31] = i + 1;
              info->pos [31][i+1][0] = 'f';
              info->pos [31][i+1][1] = '8';
        }
            else
        {
//           figurecolor white and long castling
//           king move
          info->posix[28] = i + 1;
              info->pos [28][i+1][0] = 'c';
              info->pos [28][i+1][1] = '8';
//           rook move
          info->posix[24] = i + 1;
              info->pos [24][i+1][0] = 'd';
              info->pos [24][i+1][1] = '8';
        }     
      }
        }

     }
}

double
cindouble (char pos)
{
  double result;
  result = 0;
  if (pos == 'a')
    result = 1;
  if (pos == 'b')
    result = 2;
  if (pos == 'c')
    result = 3;
  if (pos == 'd')
    result = 4;
  if (pos == 'e')
    result = 5;
  if (pos == 'f')
    result = 6;
  if (pos == 'g')
    result = 7;
  if (pos == 'h')
    result = 8;
   if (pos == '1')
    result = 1;
  if (pos == '2')
    result = 2;
  if (pos == '3')
    result = 3;
  if (pos == '4')
    result = 4;
  if (pos == '5')
    result = 5;
  if (pos == '6')
    result = 6;
  if (pos == '7')
    result = 7;
  if (pos == '8')
    result = 8;
  result = result * 1000;
  return (result);
}

int
movingpointsFun (Word* args, Word& result, int message, 
                                      Word& local, Supplier s)
{
  ChessGame* game;
  LocalInfo *info;
  MPoint* mpResult;
  int n = 0;
  int movix;
  double rows, files, rowe, filee;
  ListExpr resultType;
  TupleType *newType;
  switch( message )
  {
    case OPEN:
      game = ((ChessGame*)args[0].addr);
      
// allocate NON dynamic memory - start
  //    char* buff;
// allocate NON dynamic memory - end

// allocate dynamic memory - start 
//typedef char* wert_all;
//    typedef char (*wert_a)[32][2];
//    char*** buffer; //wertprt_all
//    wert_all buff;
 //   wert_a buffer;
//    buff = (wert_all) malloc (32 * (game->GetNoChessMoves()) * 2);
//    buffer = (wert_a) buff;     
//    cout << "in mpFun: after malloc" << endl;
// allocate dynamic memory - end 

      local.addr = new LocalInfo(game, 0); //, buff);
      info = (LocalInfo*)local.addr;
      
//    
//     init tab
//      
     resultType = GetTupleResultType( s );
      info->resultTupleType = new TupleType( 
                                  nl->Second( resultType ) );
      
      movingpointsinittab (info);
//    
//     fill tab with moves
//      
      movingpointsmovintab (info, game);    

/*//  testoutput - start
  for (int i = 0; i < 32 ; i ++)
  {
   cout << "figure: " << info->figure[i] << info->figureno[i]
        << " " << info->white[i] << " " ;
   cout << " route: " << info->pos[i][0][0] << info->pos[i][0][1];
   for (int j = 1; j <= game->GetNoChessMoves() ; j ++)
   {
     cout << " " << info->pos[i][j][0] << info->pos[i][j][1] ;
   }     
   cout << endl;
  }     
//  testoutput - end
//*/
    local.addr = info;
    
      return 0;

    case REQUEST:
     { 
       info = (LocalInfo*)local.addr;
       n = info->number;
       game = info->gameinfo;
       movix = 0;
       
       info->mpointaddr = new MPoint(game->GetNoChessMoves());
       
       if (n < 32)
       {
         Instant inst1, inst2,
//              threesecond( days, milliseconds, durationtype );
              threesecond( 0, 3000, durationtype );
     
      
         game = info->gameinfo;
         mpResult = info->mpointaddr;
     
     
         inst1.SetType( instanttype );
//         inst1.Set( intyear, intmonth, intday, inthour, 
//                               intminute, intsecond, 0 );
         inst1.Set( 2007, 2, 10, 10, 0, 0, 0 );
         inst2.SetType( instanttype );
         inst2 = inst1;
         inst2 = inst2 + threesecond;
         Interval<Instant> timeInterval( inst1, inst2, true, false );
         
     const UPoint *uPoint;
         uPoint = new UPoint( timeInterval, 0.0, 0.0, 0.0, 0.0 );
         UPoint aux( *uPoint );
 
// translate chesspos in double
         files = cindouble (info->pos [n][0][0]);
         rows = cindouble (info->pos [n][0][1]);
         mpResult->Clear();
         for( int i = 1; i < game->GetNoChessMoves(); i++ )
         {
           if (info->pos [n][i][0] == '0')
       {
//           figure was captured
             inst2 = inst2 + threesecond;
         i = game->GetNoChessMoves();
       }
       else
       {
             if (info->pos [n][i][0] == '-')
         {
//             no move
               inst2 = inst2 + threesecond;
         }
         else
         {
//             figure was moved  

//             the no-moving-time of the figure  
           filee = files;
               rowe = rows;
               aux.p0.Set(files, rows);
               aux.p1.Set(filee, rowe);
               Interval<Instant> timeInterval1( inst1, inst2, 
                                          true, false );
               aux.timeInterval = timeInterval1;
               mpResult->Add(aux);


               filee = cindouble (info->pos [n][i][0]);
               rowe = cindouble (info->pos [n][i][1]);
               aux.p0.Set(files, rows);
               aux.p1.Set(filee, rowe);
               inst1 = inst2;
           inst2 = inst2 + threesecond;
               Interval<Instant> timeInterval2( inst1, inst2, 
                                            true, false );
               aux.timeInterval = timeInterval2;
               mpResult->Add(aux);

//             the startposition for the figures next move 
               files = cindouble (info->pos [n][i][0]);
               rows = cindouble (info->pos [n][i][1]);
               inst1 = inst2;
           movix = i;
             }
       }
         }
         
//       the endposition of the figure  
     filee = files;
         rowe = rows;
         aux.p0.Set(files, rows);
         aux.p1.Set(filee, rowe);
         Interval<Instant> timeInterval3( inst1, inst2, true, true );
         aux.timeInterval = timeInterval3;
         mpResult->Add(aux);
     

// ***************************  testoutput - start
//        cout << "Anzahl: GetNoComponents: " <<  
//                mpResult->GetNoComponents() << endl;
//        for (int i = 0; i < mpResult->GetNoComponents() ; i ++)
//        {
//      const UPoint *ausgabeup;
//      mpResult->Get(i, uPoint);
//          UPoint ausgabeup( *uPoint );
//      cout << "************* i = " << i << " ********" << endl;
//      cout << "upoint geholt p0 - getx * 1000: " << 
//           ausgabeup.p0.GetX() << endl;
//      cout << "upoint geholt p0 - gety * 1000: " << 
//           ausgabeup.p0.GetY() << endl;
// for testoutput the interval (threesecond) has to be set 
// greater one day      
//      cout << "upoint start intervall (Tage): " << 
//           ausgabeup.timeInterval.start.ToDouble() << endl;
//      cout << "upoint ende intervall (Tage): " << 
//           ausgabeup.timeInterval.end.ToDouble() << endl;
//      cout << "upoint geholt p1 - getx * 1000: " << 
//           ausgabeup.p1.GetX() << endl;
//      cout << "upoint geholt p1 - gety * 1000: " << 
//           ausgabeup.p1.GetY() << endl;
 //       }     
// **************************  testoutput - ende
//     
         info->number++;
         STRING name;
     name[0] = info->figure[n];
     name[1] = info->figureno[n];
     name[2] = '\0';
     if (info->figure[n] == 'R' or info->figure[n] == 'r')
     {
       name[0] = 'R';
       name[1] = 'o';
       name[2] = 'o';
       name[3] = 'k';
       name[4] = '(';
       name[5] = info->figureno[n];
       name[6] = ')';
       name[7] = '\0';
     }
     if (info->figure[n] == 'N' or info->figure[n] == 'n')
     {
       name[0] = 'K';
       name[1] = 'n';
       name[2] = 'i';
       name[3] = 'g';
       name[4] = 'h';
       name[5] = 't';
       name[6] = '(';
       name[7] = info->figureno[n];
       name[8] = ')';
       name[9] = '\0';
     }
     if (info->figure[n] == 'B' or info->figure[n] == 'b')
     {
       name[0] = 'B';
       name[1] = 'i';
       name[2] = 's';
       name[3] = 'h';
       name[4] = 'o';
       name[5] = 'p';
       name[6] = '(';
       name[7] = info->figureno[n];
       name[8] = ')';
       name[9] = '\0';
     }
     if (info->figure[n] == 'Q' or info->figure[n] == 'q')
     {
       name[0] = 'Q';
       name[1] = 'u';
       name[2] = 'e';
       name[3] = 'e';
       name[4] = 'n';
       name[5] = '\0';
     }
     if (info->figure[n] == 'K' or info->figure[n] == 'k')
     {
       name[0] = 'K';
       name[1] = 'i';
       name[2] = 'n';
       name[3] = 'g';
       name[4] = '\0';
     }
     if (info->figure[n] == 'P' or info->figure[n] == 'p')
     {
       name[0] = 'P';
       name[1] = 'a';
       name[2] = 'w';
       name[3] = 'n';
       name[4] = '(';
       name[5] = info->figureno[n];
       name[6] = ')';
       name[7] = '\0';
     }
     
     CcString* attr1 = new CcString(true, &name);
     bool white = info->white[n];
     CcBool* attr2 = new CcBool(true, white);
     
     newType = info->resultTupleType;
     Tuple *newTuple = new Tuple(newType);
     newTuple->PutAttribute(0, dynamic_cast<Attribute*>(attr1));
     newTuple->PutAttribute(1, dynamic_cast<Attribute*>(attr2));
     newTuple->PutAttribute(2, dynamic_cast<Attribute*>
                             (mpResult));
     result.addr = newTuple;
        return YIELD;
      }   
      else
      {
        return CANCEL;
      }     
     }
    case CLOSE:
      info = ((LocalInfo*) local.addr);
      delete info->mpointaddr;
      delete info;
      return 0;
  }
  /* should not happen */
  return -1;
 
}

/*
Specification of operator ~movingpoints~

*/
const string movingpointsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>chessgame -> stream(tuple(string, bool,</text--->"
  "<text> mpoint)) </text--->"
  "<text>Creates a stream of the moves from the pieces."
  "</text--->"
  "<text>query movingpoints (game)</text--->"
  ") )";

/*
Definition of operator ~movingpoints~

*/
Operator movingpoints (
   "movingpoints",                      // name
   movingpointsSpec,                    // specification
   movingpointsFun,                     // value mapping
   Operator::SimpleSelect,               // trivial selection function
   movingpointsTypeMap                  // type mapping
);

/*
3.17 Operator ~chessreadgames~

Type mapping function of operator ~chessreadgames~

Expects a text element returns a stream of chessgames

*/
ListExpr readgamesTypeMap(ListExpr args)
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    
    if ( nl->IsEqual(arg1, "text") ) {
      return nl->TwoElemList(nl->SymbolAtom("stream"), 
             nl->SymbolAtom("chessgame"));
    }
    else
        ErrorReporter::ReportError("Type mapping function "
                "got no parameter of type text");
  }
  else 
       ErrorReporter::ReportError("Operator readgames "
             "expects a list of length one");
  return nl->SymbolAtom("typeerror");   
}


/* 
3.18 Operator readonegame

Type mapping function for operator ~readonegame~

Expects a text string with a path to a pgn file

*/
ListExpr readonegameTypeMap(ListExpr args) {
  ListExpr first, outlist;
  string argstr, out;
 
  CHECK_COND(nl->ListLength(args) == 1,
    "Operator readonegame expects a list of length one.");
     cout << "type map 1 " << endl;
  first = nl->First(args);

  if ( nl->IsEqual(first, "text"))
        return nl->SymbolAtom("chessgame");
  else {
        ErrorReporter::ReportError("Type mapping function got "
            "no parameter of type text");
        return nl->SymbolAtom("typeerror");
  }
     
  return outlist;           
}




/* 
4. Parser
Parser
Class ~Parser~

Reads and saves PGN Files in the database

*/
class Parser{

   public:
    Parser(ifstream* Infile);
    Parser(ifstream* Infile, ChessGame* game);
    ~Parser();
    ifstream* ReadPNGFile(ifstream* infile);
    ChessGame* GetChessGame();
    
   private:
       ifstream* infile;
    string gameLine, helpLine;
    //help variables
    int moveNo;
    bool moves;
    bool endgame;
    bool chessgamecreated;

    
    char line[1024];        
    ChessGame *chessGame;
    
    void ParseLine();
    void CreateGame();
    void ParseTurn(string turn, char pos[8][8], int player);
    int GetColumnDigit(char column);
    bool IsFigure(char figure);
    string GetPawnPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetKnightPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetRookPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetBishopPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetQueenPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetKingPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline);
    string GetColumnLetter(int column);
    void GetMovePosition(ChessMove &move, char pos[8][8], 
        int player, char figure, string endpos, 
        char startcolumn, char startline);
    void GetCastlingPosition(ChessMove &move, 
        char pos[8][8], int player, char type);
    char GetFigureName(char figure, int player);
};

/*
Constructor of class ~parser~

*/
Parser::Parser(ifstream* Infile) {
    infile = Infile;
    chessgamecreated = false;
    moveNo = 0;
}

Parser::Parser(ifstream* Infile, ChessGame* game) {
    infile = Infile;
    chessGame = game;
    chessgamecreated = true;
    moveNo = 1;
}

/* 
Destructor

*/
Parser::~Parser() {

}


/*
This method reads one chessGame from a PGN File

*/
ifstream* Parser::ReadPNGFile(ifstream* infile) {
    helpLine = trim(helpLine);
    endgame = false;
    moves = false;
    
    //read one line of the PGN File
    while(infile->good() && !endgame ) {
        infile->getline(line, 1024);        
        //parse the readed line
        ParseLine();
        
        
    }    
    
    if(!infile->good())
        infile->close();
    
    //Not end of file
    if(endgame && infile->good()) {
        int pos;
        pos = infile->tellg();
        
        while(!infile->eof()) {
            infile->getline(line, 1024);
            helpLine = line;
            helpLine = trim(helpLine);
            
            //new game
            if(helpLine.length() > 0) {
                infile->seekg(pos);
                break;
            }
        }    
        
    }
        
    return infile;

}

/* 
Parses one line 

*/
void Parser::ParseLine() {
    helpLine = line;
    helpLine = trim(helpLine);
    
    
    //Metadata
    if(helpLine.length() > 0 && helpLine[0] == '[') {
        
        //create new chessGame
        if(!chessgamecreated) {
            chessGame = new ChessGame( 0 );
            chessgamecreated = true;
            moveNo = 1;
        }
        
        //split metadata
        StringTokenizer stringTokenizer (helpLine);
        if(stringTokenizer.countElements() >= 2) {
           try{
            string meta = 
               stringTokenizer.getNextElement();
            //cut off '[' Symbol
            meta = meta.substr(1, meta.length()-1);
            
            string detail = helpLine.substr(
               meta.length()+3, helpLine.length() 
               - meta.length() - 5 );                
            //metadata event
            if(meta.compare("Event") == 0) {    
                chessGame->SetEvent(detail);
            }
            
            //metadata site
            if(meta.compare("Site") == 0) {
                chessGame->SetSite(detail);
            }
                
            //metadata date
            if(meta.compare("Date") == 0) {
                chessGame->SetDate(detail);
            }
                
            //metadata round
            if(meta.compare("Round") == 0) {
                chessGame->SetRound(detail);
            }
                
            //metadate white
            if(meta.compare("White") == 0) {
                chessGame->SetWhite(detail);
            }
            
            //metadata blackGetNoChessMoves
            if(meta.compare("Black") == 0) {
                chessGame->SetBlack(detail);
            }
            
            //metadata result
            if(meta.compare("Result") == 0) {
                chessGame->SetResult(detail);
            }
            
            //metadata whiteElo
            if(meta.compare("WhiteElo") == 0) {
                chessGame->SetWhiteElo(detail);
            }
            
            //metadata blackelo
            if(meta.compare("BlackElo") == 0) {
                chessGame->SetBlackElo(detail);
            }
            
            //metadata eventdate
            if(meta.compare("EventDate") == 0) {
                chessGame->SetEventDate(detail);
            }
            
            //metadata eco
            if(meta.compare("ECO") == 0) {
                chessGame->SetECO(detail);
            }
            
           }
           catch(string s) {
            cout << "Error:" << s << endl;
            chessGame->SetDefined(false);
           }
        }
        
            
    } //the moves in the file. put them together in one file
    else if (helpLine.length() > 1) {
        moves = true;            
        gameLine = gameLine +  helpLine + " ";
    }
    else {
        //End of game
        if(moves && !endgame) {
            //moves are off. calculate the game;
            CreateGame();                    
            endgame = true;            
        }
    }
}

/*
Help function which prints the position in chessfield format

*/
void printPos(char pos[8][8]) {
    cout << endl;
    for(int i = 0; i< 8 ; i++) {
        for(int j = 0; j< 8; j++) {
            cout << pos[i][j] << " , ";
        
        }
        cout << endl;
    }
}


/*
Creates the game with moves

*/
void Parser::CreateGame() {
    string turn;
    int count = 0;
    
    //start position    
    char pos[8][8] = {{'r','n','b','q','k','b','n','r'},
              {'p','p','p','p','p','p','p','p'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'-','-','-','-','-','-','-','-'},
              {'P','P','P','P','P','P','P','P'},
              {'R','N','B','Q','K','B','N','R'}};
    
              
    StringTokenizer stringTokenizer (gameLine);
    if(stringTokenizer.countElements() >= 1) {
        
       try{
        while(count < stringTokenizer.countElements() - 1)
        {
            turn = stringTokenizer.getNextElement();
        
            //It's the move number
            if(count % 3 == 0) {
                if(turn.length() == 0 || 
                  (turn.length() > 0 && 
                   isdigit(turn[0]) == 0)) {
                    ErrorReporter::ReportError(
                    "Error in parsing PGN File. "
                    "Move number expectet!");
                }  
            }
            //white move
            else if (count % 3 == 1) {
                ParseTurn(turn, pos, 0);                    
                
            }
            //black move
            else {
                ParseTurn(turn, pos, 1);
                                
            }
            count++;                
    
        }
    
        }catch(string s) {
            cout << "Error: " << s << endl;
            chessGame->SetDefined(false);
        }        
    
    }
    else {
        //Wrong game
        cout << "Error: Wrong game" << endl;
        chessGame->SetDefined(false);
    }    
}



/*
Parses one half move. player: 0 = white, 1 = black

*/
void Parser::ParseTurn(string turn, char pos[8][8], int player) {
  
  if(turn.length() > 1) {
      //create new move
    ChessMove move;    
    
    string endpos;
    
    char startcolumn = '-';
    char startline = '-';
    int help;
    char helpFigure, capturedFigure;
    bool error = false;
    bool finish = false;
    string action ="---";
    char column, line;
    
    //set move number
    move.SetMoveNr(moveNo);
    
    
                  
      //turn lenght = 2, first character is a letter and the 
    //second a digit --> pawn turn
    if((turn.length() == 2 && isdigit(turn[0]) == 0 && 
        isdigit(turn[1]) != 0 && 
        (help=turn.find("x")) == (int) string::npos) ||
       (turn.length() >= 2 && isdigit(turn[0]) == 0 && 
        isdigit(turn[1]) != 0 && 
        (help=turn.find("x")) == (int) string::npos &&
        GetColumnDigit(turn[2]) == 0 && isdigit(turn[2]) == 0 
        && turn[2] != '=')) {
        
        GetMovePosition(move, pos, player, 'P', turn, 
                    startcolumn, startline);        
    }
    
    //Turn length >= 4, pawn move with given startposition
    //c4c5
    else if(turn.length() >= 4 && GetColumnDigit(turn[0]) != 0 &&
            isdigit(turn[1]) != 0 && GetColumnDigit(turn[2]) != 0 
        && isdigit(turn[3]) != 0){
        startcolumn = turn[0];
        startline = turn[1];
        endpos = turn.substr(2,4);
        GetMovePosition(move, pos, player, 'P', endpos, 
                         startcolumn, startline);
    }
    
    //First character is Figure, second a column and third a  
    // digit --> figure turn (not pawn)
    else if((help=turn.find("x")) == (int) string::npos && 
        ((turn.length() > 2 && IsFigure(turn[0]) && 
          isdigit(turn[1]) == 0 && isdigit(turn[2]) != 0) || 
         (turn.length() > 3 && IsFigure(turn[0]) && 
          GetColumnDigit(turn[1]) != 0 && isdigit(turn[2]) == 0
          && isdigit(turn[3]) != 0) ||
         (turn.length() > 4 && IsFigure(turn[0]) && 
          GetColumnDigit(turn[1]) != 0 && isdigit(turn[2]) != 0 
          && isdigit(turn[3]) == 0 && isdigit(turn[4]) != 0) || 
         (turn.length() > 3 && IsFigure(turn[0]) && 
          isdigit(turn[1]) != 0 && GetColumnDigit(turn[2]) != 0 
          && isdigit(turn[3]) != 0)
      )) {
        
        //e.g  Nc6  only figure is specified
          if((turn.length() == 3 && IsFigure(turn[0]) && 
            GetColumnDigit(turn[1]) != 0 && 
            isdigit(turn[2]) != 0) || 
           (turn.length() > 3 && IsFigure(turn[0]) && 
            GetColumnDigit(turn[1]) != 0 && 
            isdigit(turn[2]) != 0 && 
            GetColumnDigit(turn[3]) == 0)) {
            endpos = turn.substr(1,3);    

        } 
        //e.g Nhc6  figure an column is specified
        else if(turn.length() >= 4 && IsFigure(turn[0]) && 
                GetColumnDigit(turn[1]) != 0 && 
            GetColumnDigit(turn[2]) != 0 && 
            isdigit(turn[3]) != 0) {
              startcolumn = turn[1];
              endpos = turn.substr(2,4);

        }
        
        //e.g Nh3c6 figure column and line is specified
        else if(turn.length() > 4 && IsFigure(turn[0]) && 
                GetColumnDigit(turn[1]) != 0 && 
            isdigit(turn[2]) != 0 && 
            GetColumnDigit(turn[3]) != 0 && 
            isdigit(turn[4]) != 0){            
              startcolumn = turn[1];
              startline = turn[2];
              endpos = turn.substr(3,5);    

        }
        //e.g.  N8c6 figure and line is specified
        else if(turn.length() >= 4 && IsFigure(turn[0]) && 
                isdigit(turn[1]) != 0 && 
            GetColumnDigit(turn[2]) != 0 && 
            isdigit(turn[3]) != 0) {
              startline = turn[1];
              endpos = turn.substr(2,4);

        }
        
        else {
          cout << "Error: unallowed move turn: " << 
                  turn << endl;
          chessGame->SetDefined(false);
        }
        
        GetMovePosition(move, pos, player, turn[0], 
                endpos, startcolumn, startline);        
    }
    
    //captured figure
    else if((help=turn.find("x")) != (int) string::npos) {
        
       switch(help) {
        case 1:    if(turn.length() >= 4) {
            //first character is a column. 
            //a pawn captures
            if(turn[0] != 'N' && turn[0] != 'B' && 
               turn[0] != 'R' && turn[0] != 'Q' &&
               turn[0] != 'K') {                           
                helpFigure = 'P';
                startcolumn = turn[0];
                startline = 'x';
                
            }
            //First character is the figure that captures
            else {
                helpFigure = turn[0];
            }
                
            endpos = turn.substr(2,4);
           }
           else {
            error = true;
           }                    
            
            break;
            
        case 2:    //first character is the figure that 
            //captures, second the column from where
            //Nhxc4
            if(turn.length() >= 5 && IsFigure(turn[0]) 
               && GetColumnDigit(turn[1]) != 0) {
                helpFigure = turn[0];
                startcolumn = turn[1];
                endpos = turn.substr(3,5);
                
            }
            //first character is the figure that 
            //captures, second the line from where
            else if(turn.length() >= 5 && 
                    IsFigure(turn[0]) && 
                isdigit(turn[1]) != 0) {
                   helpFigure = turn[0];
                   startline = turn[1];
                   endpos = turn.substr(3,5);
            }
            //a pawn capture with given startposition 
            //d5xc6
            else if (turn.length() >= 5 && 
                     GetColumnDigit(turn[0]) != 0 && 
                 isdigit(turn[1]) != 0) {
                   helpFigure = 'P';
                   startcolumn = turn[0];
                   startline = turn[1];
                   endpos = turn.substr(3,5);
            }
                
            else
                error = true;
            break;
            
        case 3:    //first character is the figure that 
                //captures, second the column and 
            //third the line from where its captures
            if(turn.length() >= 6) {
                helpFigure = turn[0];
                startcolumn = turn[1];
                startline = turn[2];
                endpos = turn.substr(4,6);
                
            }
            else    
                error = true;
            break;        
       }
       column = GetColumnDigit(endpos[0]) - 1;
       line = 8 - ((int) endpos[1] - '0') ;
        
       //set captured figure
        
       //pawn captures and catch en passant
       if(startline == 'x' && pos[line][column] == '-'){
        if(player == 0) {
           if(pos[line + 1][column] == 'p') {
             capturedFigure = 'p';
            pos[line+1][column] = '-';
           }
           else
            cout << "catch en passant error" << endl;
        }
        else {
           if(pos[line - 1][column] == 'P') {
            capturedFigure = 'P';
            pos[line-1][column] = '-';
           }
           else
             cout << "catch en passant error" << endl;
        }
             
       }
       else
          capturedFigure = pos[line][column];    
           
       move.SetCapturedFigure((const char*) &capturedFigure);
            
       if(!error) {
        GetMovePosition(move, pos, player, helpFigure, 
                endpos, startcolumn, startline);
                    
        //it's possible that there is a pawn promotion too
        //fxg1=Q+
        if((help=turn.find("=")) != (int) string::npos) {
           if(player == 0) {
            pos[8 - ((int) endpos[1] - '0')]
              [GetColumnDigit(endpos[0])-1] = 
              turn[help + 1];
            action[1] = turn[help + 1];
           }
           else {
            
            switch(turn[help + 1]) {
               case 'Q':    
                   pos[8 - ((int) endpos[1] - '0')]
                 [GetColumnDigit(endpos[0])-1] = 'q';
                action[1] = 'q';
                break;
                    
               case 'R':
                pos[8 - ((int) endpos[1] - '0')]
                 [GetColumnDigit(endpos[0])-1] = 'r';
                action[1] = 'r';
                break;
                    
               case 'B':
                pos[8 - ((int) endpos[1] - '0')]
                 [GetColumnDigit(endpos[0])-1] = 'b';
                action[1] = 'b';
                break;
                    
               case 'N':
                pos[8 - ((int) endpos[1] - '0')]
                 [GetColumnDigit(endpos[0])-1] = 'n';
                action[1] = 'n';
                break;                
            }
            
           }
        }
       }
    }
    
    //kingside castling
    else if(turn.length() == 3 && turn[0] == 'O' && 
           turn[1] == '-' && turn[2] == 'O' ){
        char figure = '-';
        move.SetFigureName((const char*) &figure);
        string nopos = "--";
        move.SetStartPosition(nopos.c_str());
        move.SetEndPosition(nopos.c_str());
        GetCastlingPosition(move, pos, player, 's');        
        action[2] = 's';        
    }
    
    //queenside castling
    else if(turn.length() >= 5 && turn[0] == 'O' && 
           turn[1] == '-' && turn[2] == 'O' && turn[3] == '-' && 
           turn[4] == 'O') {
        char figure = '-';
        move.SetFigureName((const char*) &figure);
        string nopos = "--";
        move.SetStartPosition(nopos.c_str());
        move.SetEndPosition(nopos.c_str());
        GetCastlingPosition(move, pos, player, 'l');
        action[2] = 'l';
    }
    
    //end of game white player wins 1-0
    else if(turn.length() >= 3 && turn[0] == '1' && 
           turn[1] == '-' && turn[2] == '0') {
        finish = true;        
        
    } 
    //end of game black player wins 0- 1
    else if(turn.length() >= 3 && turn[0] == '0' && 
           turn[1] == '-' && turn[2] == '1') {
        finish = true;
    }
    //end of game no winner 1/2-1/2
    else if(turn.length() >= 7 && turn[0] == '1' && 
           turn[1] == '/' && turn[2] == '2' && turn[3] == '-' && 
           turn[4] == '1' && turn[5] == '/' && turn[6] == '2') {
        finish = true;
    }
    //end of game. no result
    else if(turn.length() >= 1 && turn[0] == '*') {
        finish = true;
    }
    
    
    //pawn promotion
    else if((help=turn.find("=")) != (int) string::npos) {
                
       if(turn[2] == '=') {
        
        //Calculate Move
        GetMovePosition(move, pos, player, 'P', 
             turn.substr(0,2), startcolumn, startline);
        
            
        if(player == 0) {
           pos[8 - ((int) turn[1] - '0')]
                 [GetColumnDigit(turn[0])-1] = turn[3];
           action[1] = turn[3];
            
        }
        else {
        
           switch(turn[3]) {
            case 'Q':pos[8 - ((int) turn[1] - '0')]
                   [GetColumnDigit(turn[0])-1] = 'q';
                 action[1] = 'q';
                 break;
            
            case 'R':pos[8 - ((int) turn[1] - '0')]
                   [GetColumnDigit(turn[0])-1] = 'r';
                 action[1] = 'r';
                 break;
            
            case 'B':pos[8 - ((int) turn[1] - '0')]
                   [GetColumnDigit(turn[0])-1] = 'b';
                 action[1] = 'b';
                 break;
            
            case 'N':pos[8 - ((int) turn[1] - '0')]
                   [GetColumnDigit(turn[0])-1] = 'n';
                 action[1] = 'n';
                 break;                
           }
        
        }
       }
       else {
        cout << "Error: Wrong pawn promotion" << endl;
        chessGame->SetDefined(false);
       }
        
    }
    
    
    //check
    if((help=turn.find("+")) != (int) string::npos) {
        action[0] = 'c';
    }
    
    //checkmate
    if((help=turn.find("#")) != (int) string::npos) {
        action[0] = 'm';
    }
    
        
    
    //An error occured
    if(error) {
        cout << "Error occured" << endl;
        chessGame->SetDefined(false);
    }
    
    move.SetAction(action.c_str());

    //Add move to chessgame
    if(!finish) {
        chessGame->Append(move);
        moveNo++;
    }
  }  
}

/*
Calculates the startmove

*/
void Parser::GetMovePosition(ChessMove &move, char pos[8][8], 
             int player, char figure, string endpos, 
         char startcolumn, char startline) {
    
    string startpos;
    int column, line;
    char figurehelp;
    column = GetColumnDigit(endpos[0]) - 1;
    line = 8 - ((int) endpos[1] - '0') ;
    
    //set Endposition
    move.SetEndPosition(endpos.c_str());
    figurehelp = GetFigureName(figure,player);
    //set FigureName
    move.SetFigureName((const char*) &figurehelp);
    
    switch(figure) {
       //Pawn turn
       case 'P' :   //get startposition
            startpos = GetPawnPosition(endpos, pos, 
                 player, startcolumn, startline);
            
            //set position
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'P';
            }
            else {
                pos[line][column] = 'p';
            }
        
            //move.SetEndPosition(turn);
            break;
    
       //Knight turn
       case 'N' :    //get startposition                
            startpos = GetKnightPosition(endpos, pos, 
                   player, startcolumn, startline);
            
            //set positions
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'N';
            }
            else {
                pos[line][column] = 'n';
            }
            break;
        
       //Rook turn        
       case 'R' :    //get startposition                
            startpos = GetRookPosition(endpos, pos, 
                  player, startcolumn, startline);
            
            //set positions
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'R';
            }
            else {
                pos[line][column] = 'r';
            }
            break;
        
       //Bishop turn        
       case 'B' :    //get startposition                
            startpos = GetBishopPosition(endpos, pos, 
                   player, startcolumn, startline);
            
            //set positions
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'B';
            }
            else {
                pos[line][column] = 'b';
            }
            
            break;
            
       //Queen turn        
       case 'Q' :    //get startposition                
            startpos = GetQueenPosition(endpos, pos, 
                  player, startcolumn, startline);
            
            //set positions
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'Q';
            }
            else {
                pos[line][column] = 'q';
            }
            
            break;
        
       //King turn        
       case 'K' :    //get startposition                
            startpos = GetKingPosition(endpos, pos, 
                 player, startcolumn, startline);
            
            //set positions
            pos[8 - (int)(startpos[1] - '0')]
                [GetColumnDigit(startpos[0])-1]  = '-';
            
            //white player
            if(player == 0) {
                pos[line][column] = 'K';
            }
            else {
                pos[line][column] = 'k';
            }
            
            break;
        
    }
    if(startpos.length() == 0) {
        //Should not happen. 
        cout << endl << endl << "!!!!!!!!! Error  startpos "
                     "!!!!!!!!!!!" << endl << endl << endl;
        chessGame->SetDefined(false);
    
    }
    //set startposition
    move.SetStartPosition (startpos.c_str());    
}


/*
Returns the startposition of a pawn when its endposition is known. 
It is also possible that its startcolumn is known or its 
startcolumn and startline

*/
string Parser::GetPawnPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline) {
    int line, column;
    string ret;
    char posm[1];
    
    
    //startposition already known
    if(startcolumn != '-' && 
      (startline != '-' && startline != 'x')) {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;        
    }
    
    //No startcolumn known
    if(startcolumn == '-') {
        column = GetColumnDigit(endpos[0]) - 1;
    }
    else
        column = GetColumnDigit(startcolumn);
    
    //No startline is known
    if(startline == '-' || startline == 'x') {    
        line = 8 - ((int) endpos[1] - '0') ;
    }
    else {
        line = 8 - ((int) startline - '0');
    } 
    
    //startposition already known
    if(startcolumn != '-' && (startline !='-' && 
       startline != 'x')) {
    
        ret = GetColumnLetter(column + 1);
        sprintf( posm, "%i", 8 - (line + 2));
        ret += posm;        
        return ret;
    }
    
    //white player
    if(player == 0) {
    
        //pawn capture move
        if(startline == 'x' && 
           pos[line + 1][column - 1] == 'P' && 
           startcolumn == GetColumnLetter(column - 1 + 1)[0])
        {
            sprintf(posm, "%i", 8 - (line + 1));
            ret = startcolumn;
            ret += posm;
            return ret;                        
        }
        else if(startline == 'x' && 
           pos[line + 1][column + 1] == 'P' && 
           startcolumn == GetColumnLetter(column + 1 + 1)[0])
        {
            sprintf(posm, "%i", 8 - (line + 1));
            ret = startcolumn;
            ret += posm;
            return ret;
        }
        
        //pawn move one line
        if(pos[line + 1][column] == 'P' ) {
        
            //set positions            
            ret = GetColumnLetter(column + 1);
            sprintf( posm, "%i",  8 - (line + 1));
            ret += posm;            
            return ret;
            
            
        
        }            
        //pawn move two lines
        else if(pos[line + 2][column] == 'P' ){
              //set move
            ret = GetColumnLetter(column + 1);
            sprintf( posm, "%i", 8 - (line + 2));
            ret += posm;            
            return ret;            
        }
        else {
            cout << "Wrong pawn turn."  << endl;
            chessGame->SetDefined(false);
        }        
    }
    else {
        //pawn capture move
        if(startline == 'x' && 
           pos[line - 1][column - 1] == 'p' && 
           startcolumn == GetColumnLetter(column - 1 + 1)[0])
        {
            sprintf(posm, "%i", 8 - (line - 1));
            ret = startcolumn;
            ret += posm;
            return ret;                        
        }
        else if(startline == 'x' && 
           pos[line - 1][column + 1] == 'p' && 
           startcolumn == GetColumnLetter(column + 1 + 1)[0])
        {
            sprintf(posm, "%i", 8 - (line - 1));
            ret = startcolumn;
            ret += posm;
            return ret;
        }
        
        
        //pawn move one line
        if(pos[line - 1][column] == 'p' ) {
            ret = GetColumnLetter(column + 1);
            sprintf( posm, "%i", 8 - (line - 1) );
            ret += posm;            
            return ret;        
        }            
        //pawn move two lines
        else if(pos[line - 2][column] == 'p' ){
              ret = GetColumnLetter(column + 1);            
            sprintf( posm, "%i", 8 - (line - 2) );
            ret += posm;            
            return ret;    
            
        }
        else {
            cout << "Wrong pawn turn. " << endl;
            chessGame->SetDefined(false);
        }            
    }    
    chessGame->SetDefined(false);
    ret = "";
    return ret;
    
}


/*
Returns the startposition of a knight when its endposition is known.
It is also possible that its startcolumn is known or its 
startcolumn and startline

*/
string Parser::GetKnightPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline) {
    int column = GetColumnDigit(endpos[0]) - 1;
    int line = 8 - ((int) endpos[1] - '0') ;
    string ret;
    char posm[1];
    
    
    //startposition already known
    if(startcolumn != '-' && startline != '-') {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;
        
    }
    
    
    //column - 1 
    if(endpos[0] != 'a') {    
       //line + 2    
       if(line  < 6) {            
        if((player == 0 && 
            pos[line + 2][column - 1] == 'N') ||
           (player == 1 && 
            pos[line + 2][column - 1] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column - 1 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 2)))
            {
               ret = GetColumnLetter(column - 1 + 1);
               sprintf( posm, "%i", 8 - (line + 2) );
               ret += posm;
               return  ret;
            }
           }
        }    
       }
       //line - 2
       if(line  >= 2) {
        if((player == 0 && 
            pos[line - 2][column - 1] == 'N') ||
           (player == 1 && 
            pos[line - 2][column - 1] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column - 1 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 2)))
            {
               ret = GetColumnLetter(column - 1 + 1);
               sprintf( posm, "%i", 8 - (line - 2));
               ret += posm;
               return  ret;
            }
            }
        }    
       }
    }
    
    //column + 1
    if(endpos[0] != 'h') {
       //line + 2    
       if(line  < 6) {            
        if((player == 0 && 
            pos[line + 2][column + 1] == 'N') ||
           (player == 1 && 
            pos[line + 2][column + 1] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column + 1 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 2)))
            {
               ret = GetColumnLetter(column + 1 + 1);
               sprintf( posm, "%i", 8 - (line + 2));
               ret += posm;
               return  ret;
            }
            }
        }
       }
       //line - 2
       if(line >= 2) {
        if((player == 0 && 
            pos[line - 2][column + 1] == 'N') ||
           (player == 1 && 
            pos[line - 2][column + 1] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column + 1 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 2)))
            {
               ret = GetColumnLetter(column + 1 + 1);
               sprintf( posm, "%i", 8 - (line - 2));
               ret += posm;
               return  ret;
            }
            }
        }    
       }
    }
    
    //column - 2
    if(endpos[0] != 'a' && endpos[0] != 'b') {
       //line + 1    
       if(line  < 7) {            
        if((player == 0 && 
            pos[line + 1][column - 2] == 'N') ||
           (player == 1 && 
            pos[line + 1][column - 2] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column- 2 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 1)))
            {
               ret = GetColumnLetter(column - 2 + 1);
               sprintf( posm, "%i", 8 - (line + 1));
               ret += posm;
               return  ret;
            }
            }
        }    
       }
       //line - 1
       if(line  >= 1) {
        if((player == 0 && 
            pos[line - 1][column - 2] == 'N') ||
           (player == 1 && 
            pos[line - 1][column - 2] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column- 2 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 1)))
            {
               ret = GetColumnLetter(column - 2 + 1);
               sprintf( posm, "%i", 8 - (line - 1));
               ret += posm;
               return  ret;
            }
            }
        }
       }
    
    }
    
    //column + 2
    if(endpos[0] != 'h' && endpos[0] != 'g') {
       //line + 1    
       if(line  < 7) {            
        if((player == 0 && 
            pos[line + 1][column + 2] == 'N') ||
           (player == 1 && 
            pos[line + 1][column + 2] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column + 2 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 1)))
            {
               ret = GetColumnLetter(column + 2 + 1);
               sprintf( posm, "%i", 8 - (line + 1));
               ret += posm;
               return  ret;
            }
            }
        }    
       }
       //line - 1
       if(line >= 1) {
        if((player == 0 && 
            pos[line - 1][column + 2] == 'N') ||
           (player == 1 && 
            pos[line - 1][column + 2] == 'n')) {
            //if startcolumn is known compare them
            if(startcolumn == '-' || 
              (startcolumn != '-' && startcolumn == 
               GetColumnLetter(column + 2 + 1)[0])) {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 1)))
            {
               ret = GetColumnLetter(column + 2 + 1);
               sprintf( posm, "%i", 8 - (line - 1));
               ret += posm;
               return  ret;
            }
            }
        }
       }
    }    
    cout << "Wrong Knight turn " << endl;
    chessGame->SetDefined(false);
    ret = "";
    return ret;
}

/*
Returns the startposition of a rook when its endposition is known.
It is also possible that its startcolumn is known or its 
startcolumn and startline

*/
string Parser::GetRookPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline){
    int column = GetColumnDigit(endpos[0]) - 1;
    int line = 8 - ((int) endpos[1] - '0') ;
    string ret;
    char posm[1];

    
    //startposition already known
    if(startcolumn != '-' && startline != '-') {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;        
    }
    
    // line --
    for(int i = line - 1; i>= 0; i--) {
       if((player == 0 && pos[i][column] == 'R') ||
          (player == 1 && pos[i][column] == 'r')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[i][column] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // line ++
    for(int i = line + 1; i< 8; i++) {
       if((player == 0 && pos[i][column] == 'R') ||
          (player == 1 && pos[i][column] == 'r')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[i][column] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // column --
    for(int i = column - 1; i>= 0; i--) {
       if((player == 0 && pos[line][i] == 'R') ||
          (player == 1 && pos[line][i] == 'r')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(i + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                ret = GetColumnLetter(i + 1);
                sprintf(posm, "%i", 8 - line);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[line][i] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // column ++
    for(int i = column + 1; i< 8; i++) {
       if((player == 0 && pos[line][i] == 'R') ||
          (player == 1 && pos[line][i] == 'r')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(i + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                ret = GetColumnLetter(i + 1);
                sprintf(posm, "%i", 8 -  line );
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[line][i] != '-') {
            //There is already an other figure
        break;
       }
    }    
    cout << "Wrong rook turn " << endl;
    chessGame->SetDefined(false);
    ret = "";
    return ret;
}


/*
Returns the startposition of a bishop when its endposition is known.
It is also possible that its startcolumn is known or its 
startcolumn and startline

*/
string Parser::GetBishopPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline) {
    int column = GetColumnDigit(endpos[0]) - 1;
    int line = 8 - ((int) endpos[1] - '0') ;
    string ret;
    char posm[1];
    
    
    //startposition already known
    if(startcolumn != '-' && startline != '-') {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;        
    }
    
    // line -- column --
    for(int i = line - 1,  j = column - 1; i>= 0 && j >= 0; 
        i--, j--) {
        if((player == 0 && pos[i][j] == 'B') ||
           (player == 1 && pos[i][j] == 'b')) {
        
            if(startcolumn == '-' || 
              (startcolumn != '-' && 
               startcolumn == GetColumnLetter(j + 1)[0]))
            {
                //startline is know compare them
                if(startline == '-' || 
                   startline == 'x' || 
                   (startline != '-' && 
                   ((int) startline - '0') == 8 - i))
                {
                    ret = GetColumnLetter(j + 1);
                    sprintf(posm, "%i",8 -  i);
                    ret += posm;
                    return ret;
                }
            }     
        }
        else if(pos[i][j] != '-') {
            //There is already an other figure
            break;
        }
    }
    
    // line ++ column --
    for(int i = line + 1, j = column - 1; i< 8 && j>= 0; 
        i++, j--) {
        if((player == 0 && pos[i][j] == 'B') ||
           (player == 1 && pos[i][j] == 'b')) {
        
            if(startcolumn == '-' || 
              (startcolumn != '-' && 
               startcolumn == GetColumnLetter(j + 1)[0]))
            {
                //startline is know compare them
                if(startline == '-' || 
                   startline == 'x' || 
                   (startline != '-' && 
                   ((int) startline - '0') == 8 - i))   
                {
                    ret = GetColumnLetter(j + 1);
                    sprintf(posm, "%i", 8 - i );
                    ret += posm;
                    return ret;
                }
            }     
        }
        else if(pos[i][j] != '-') {
            //There is already an other figure
            break;
        }
    }
    
    // column ++ line--
    for(int j = column + 1, i = line - 1; j< 8 && i >= 0 ; 
        j++, i--) {        
        if((player == 0 && pos[i][j] == 'B') ||
           (player == 1 && pos[i][j] == 'b')) {
        
            if(startcolumn == '-' || 
              (startcolumn != '-' && 
               startcolumn == GetColumnLetter(j + 1)[0]))
            {
                //startline is know compare them
                if(startline == '-' || 
                   startline == 'x' || 
                   (startline != '-' && 
                   ((int) startline - '0') == 8 - i))
                {
                    ret = GetColumnLetter(j + 1);
                    sprintf(posm, "%i", 8 - i);
                    ret += posm;
                    return ret;
                }
            }     
        }
        else if(pos[i][j] != '-') {
            //There is already an other figure
            break;
        }
    }
    
    // column ++ line ++
    for(int j = column + 1, i = line + 1; j< 8 && i < 8; 
        j++, i++) {
        if((player == 0 && pos[i][j] == 'B') ||
           (player == 1 && pos[i][j] == 'b')) {
        
            if(startcolumn == '-' || 
              (startcolumn != '-' && 
               startcolumn == GetColumnLetter(j + 1)[0]))
            {
                //startline is know compare them
                if(startline == '-' || 
                   startline == 'x' || 
                   (startline != '-' && 
                   ((int) startline - '0') == 8 - i))
                {
                    ret = GetColumnLetter(j + 1);
                    sprintf(posm, "%i", 8 - i);
                    ret += posm;
                    return ret;
                }
            }     
        }    
        else if(pos[i][j] != '-') {
            //There is already an other figure
            break;
        }
    }
    cout << "Wrong bishop turn" << endl;    
    chessGame->SetDefined(false);
    ret = "";
    return ret;

}


/*
Returns the startposition of a queen when its endposition is known.
It is also possible that its startcolumn is known or its startcolumn and startline

*/
string Parser::GetQueenPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline) {
    int column = GetColumnDigit(endpos[0]) - 1;
    int line = 8 - ((int) endpos[1] - '0') ;
    string ret;
    char posm[1];
    
    
    //startposition already known
    if(startcolumn != '-' && startline != '-') {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;        
    }
    
    // line --
    for(int i = line - 1; i>= 0; i--) {
       if((player == 0 && pos[i][column] == 'Q') ||
          (player == 1 && pos[i][column] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[i][column] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // line ++
    for(int i = line + 1; i< 8; i++) {
       if((player == 0 && pos[i][column] == 'Q') ||
          (player == 1 && pos[i][column] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[i][column] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // column --
    for(int i = column - 1; i>= 0; i--) {        
       if((player == 0 && pos[line][i] == 'Q') ||
          (player == 1 && pos[line][i] == 'q')) {
          
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(i + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                ret = GetColumnLetter(i + 1);
                sprintf(posm, "%i", 8 - line);
                ret += posm;
                return ret;
            }
        }
       }
       else if(pos[line][i] != '-') {
            //There is already an other figure        
        break;
       }
    }
    
    // column ++
    for(int i = column + 1; i< 8; i++) {
       if((player == 0 && pos[line][i] == 'Q') ||
          (player == 1 && pos[line][i] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(i + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                ret = GetColumnLetter(i + 1);
                sprintf(posm, "%i", 8 - line );
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[line][i] != '-') {
            //There is already an other figure
        break;
       }
    }

    // line -- column --
    for(int i = line - 1,  j = column - 1; i>= 0 && j >= 0; 
        i--, j--) {
       if((player == 0 && pos[i][j] == 'Q') ||
          (player == 1 && pos[i][j] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(j + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(j + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }
       }
       else if(pos[i][j] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // line ++ column --
    for(int i = line + 1, j = column - 1; i< 8 && j>= 0; 
        i++, j--) {
        if((player == 0 && pos[i][j] == 'Q') ||
           (player == 1 && pos[i][j] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(j + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(j + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }
       }
       else if(pos[i][j] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // column ++ line--
    for(int j = column + 1, i = line - 1; j< 8 && i >= 0 ; 
        j++, i--) {        
        if((player == 0 && pos[i][j] == 'Q') ||
           (player == 1 && pos[i][j] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(j + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(j + 1);
                sprintf(posm, "%i", 8 - i);
                ret += posm;
                return ret;
            }
        }
       }
       else if(pos[i][j] != '-') {
            //There is already an other figure
        break;
       }
    }
    
    // column ++ line ++
    for(int j = column + 1, i = line + 1; j< 8 && i < 8; 
        j++, i++) {
        if((player == 0 && pos[i][j] == 'Q') ||
           (player == 1 && pos[i][j] == 'q')) {
        
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(j + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - i)) {
                ret = GetColumnLetter(j + 1);
                sprintf(posm, "%i", 8 -  i);
                ret += posm;
                return ret;
            }
        }     
       }
       else if(pos[i][j] != '-') {
            //There is already an other figure
        break;
       }
    }
    cout << "Wrong queen turn" << endl;
    chessGame->SetDefined(false);
    ret = "";
    return ret;
    
}

/*
Returns the startposition of a king when its endposition is known.
It is also possible that its startcolumn is known or its 
startcolumn and startline

*/
string Parser::GetKingPosition(string endpos, char pos[8][8], 
        int player, char startcolumn, char startline) {
    int column = GetColumnDigit(endpos[0]) - 1;
    int line = 8 - ((int) endpos[1] - '0') ;
    string ret;
    char posm[1];
    

    //startposition already known
    if(startcolumn != '-' && startline != '-') {
        char s[2];
        s[0] = startcolumn;
        s[1] = startline;
        ret = s;
        return ret;        
    }
    
    //line - 1
    if(line > 0 && ((player == 0 && pos[line - 1][column] == 'K')
            || (player == 1 && pos[line - 1][column] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 1)))
            {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i",8 -  (line - 1));
                ret += posm;
                return ret;
            }
        }    
    }
    
    //line + 1
    if(line < 7 && ((player == 0 && pos[line + 1][column] == 'K')
         || (player == 1 && pos[line + 1][column] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
           startcolumn == GetColumnLetter(column + 1)[0])){
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line +1)))
            {
                ret = GetColumnLetter(column + 1);
                sprintf(posm, "%i", 8 - (line + 1));
                ret += posm;
                return ret;
            }
        }    
    }
    
    //column - 1    
    if(column > 0 && 
      ((player == 0 && pos[line][column - 1] == 'K') || 
       (player == 1 && pos[line][column - 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column - 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                   ret = GetColumnLetter(column - 1 + 1);
                   sprintf(posm, "%i", 8 - line);
                   ret += posm;
                   return ret;
            }
        }    
    }
    
    //column + 1    
    if(column < 7 && 
      ((player == 0 && pos[line][column + 1] == 'K') || 
       (player == 1 && pos[line][column + 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column + 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - line)) {
                   ret = GetColumnLetter(column + 1 + 1);
                   sprintf(posm, "%i", 8 - line);
                   ret += posm;
                   return ret;
            }
        }    
    }
    
    //line - 1 column - 1
    if(line > 0 && column > 0  && 
      ((player == 0 && pos[line - 1][column-1] == 'K') || 
       (player == 1 && pos[line - 1][column - 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column - 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
             (startline != '-' && 
             ((int) startline - '0') == 8 - (line  - 1)))
            {
                  ret = GetColumnLetter(column - 1  + 1);
                  sprintf(posm, "%i", 8 - (line - 1) );
                  ret += posm;
                  return ret;
            }
        }    
    }
    
    //line + 1 column - 1
    if(line < 7 && column > 0 && 
      ((player == 0 && pos[line + 1][column - 1] == 'K') || 
       (player == 1 && pos[line + 1][column- 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column - 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 1)))
            {
                   ret = GetColumnLetter(column - 1 + 1);
                   sprintf(posm, "%i", 8 - (line + 1) );
                   ret += posm;
                   return ret;
            }
        }
    }
    
    //column + 1 line - 1    
    if(column < 7 && line > 0 && 
      ((player == 0 && pos[line - 1][column + 1] == 'K') || 
       (player == 1 && pos[line - 1][column + 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column + 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line - 1)))
            {
                   ret = GetColumnLetter(column + 1 + 1);
                   sprintf(posm, "%i", 8 - (line - 1));
                   ret += posm;
                   return ret;
            }
        }
    }
    
    //column + 1     line + 1
    if(column < 7 && line < 7 && 
      ((player == 0 && pos[line + 1][column + 1] == 'K') || 
       (player == 1 && pos[line + 1][column + 1] == 'k'))) {
        if(startcolumn == '-' || 
          (startcolumn != '-' && 
          startcolumn == GetColumnLetter(column + 1 + 1)[0]))
        {
            //startline is know compare them
            if(startline == '-' || startline == 'x' || 
              (startline != '-' && 
              ((int) startline - '0') == 8 - (line + 1)))
            {
                   ret = GetColumnLetter(column + 1 + 1);
                   sprintf(posm, "%i", 8 - (line + 1));
                   ret += posm;
                   return ret;
            }
        }    
    }
    
    cout << "Wrong king turn" << endl;
    chessGame->SetDefined(false);
    ret = "";
    return ret;
}

/*
Calculates the kingside and quenside castling

*/
void Parser::GetCastlingPosition(ChessMove &move, char pos[8][8], 
            int player, char type) {
    int line = -1;
    int column = -1;
    
    //get line of king position
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if((pos[i][j] == 'K' && player == 0) || 
               (pos[i][j] == 'k' && player == 1)) {
                line = i;
                column = j;
                break;
            }        
        }
        if(line != -1)
            break;
    }
            
    //king side castling
    if(type == 's') {
        if(column + 3 < 8 && ((pos[line][column + 3] == 'R'
           && player == 0) || (pos[line][column + 3] == 'r' 
           && player == 1))) {
               //move king two columns
               pos[line][column + 2] = pos[line][column];
               pos[line][column] = '-';
            
               //move rook
               pos[line][column + 1] = pos[line][column + 3];
               pos[line][column + 3] = '-';
        }
        else if(column - 3 >= 0 && 
               ((pos[line][column - 3] == 'R' && player == 0)
           || (pos[line][column - 3] == 'r' && player == 1)))
        {
               //move king two columns
               pos[line][column - 2] = pos[line][column];
               pos[line][column] = '-';
        
               //move rook
               pos[line][column - 1] = pos[line][column - 3];
               pos[line][column - 3] = '-';
        }
        
    }
    //queenside castling
    else {
        if(column + 4 < 8 && ((pos[line][column + 4] == 'R' 
           && player == 0) || (pos[line][column + 4] == 'r' 
           && player == 1))) {
               //move king two columns
               pos[line][column + 2] = pos[line][column];
               pos[line][column] = '-';
        
               //move rook
               pos[line][column + 1] = pos[line][column + 4];
               pos[line][column + 4] = '-';
        }
        else if(column - 4 >= 0 && 
               ((pos[line][column - 4] == 'R' && player == 0)
           || (pos[line][column - 4] == 'r' && player == 1)))
        {
               //move king two columns
               pos[line][column - 2] = pos[line][column];
               pos[line][column] = '-';
        
               //move rook
               pos[line][column - 1] = pos[line][column - 4];
               pos[line][column - 4] = '-';        
        }
    }
    
}


/*
Returns the digit of a column. e.g. a = 1, b = 2 etc.

*/
int Parser::GetColumnDigit(char column) {
    if(column == 'a') return 1;
    if(column == 'b') return 2;
    if(column == 'c') return 3;
    if(column == 'd') return 4;
    if(column == 'e') return 5;
    if(column == 'f') return 6;
    if(column == 'g') return 7;
    if(column == 'h') return 8;
    return 0;
}

/* 
Returns the letter of a column. e.g. 1 = a, 2 = b etc.

*/
string Parser::GetColumnLetter(int column) {
    if(column == 1) return "a";
    if(column == 2) return "b";
    if(column == 3) return "c";
    if(column == 4) return "d";
    if(column == 5) return "e";
    if(column == 6) return "f";
    if(column == 7) return "g";
    if(column == 8) return "h";
    return "z";
}


/*
Checks if its a valid figure

*/
bool Parser::IsFigure(char figure) {
    if(figure == 'P')
        return true;
    if(figure == 'N')
        return true;
    if(figure == 'B')
        return true;
    if(figure == 'R')
        return true;
    if(figure == 'Q')
        return true;
    if(figure == 'K')
        return true;    
    return false;
}

/*
Returns the figure name. 

*/
char Parser::GetFigureName(char figure, int player){
    if(player == 0)
        return figure;
    else {
        switch(figure) {
            case 'P':    return 'p';
                    break;
                    
            case 'N':    return 'n';
                    break;
                            
            case 'B':    return 'b';
                    break;
                    
            case 'R':    return 'r';
                    break;
                    
            case 'Q':    return 'q';
                    break;    
            
            case 'K':    return 'k';
                    break;    
        }    
    }
    return ' ';
}




/*
Returns the generated ChessGame

*/
ChessGame* Parser::GetChessGame() {
    return chessGame;
}


/* 
Value Mapping opeator ~chessreadgames~ 

*/
int readgamesFun (Word* args, Word& result, int message, 
            Word& local, Supplier s)
{
 struct Range {bool read; ifstream* infile;}* range;
 
  FText *myFile;
  ifstream* infile;
  Parser* parser;
  
  switch(message)
  {
    case OPEN:
      range = new Range();
      
      infile = new ifstream();
      range->infile = infile;
      range->read = false;
      
      local.addr = range;
      return 0;
      
    case REQUEST:
      range = ((Range*) local.addr); 
      infile = range->infile;
      
      //There are still chessgames in the file
      if(infile->good() && infile->is_open()) {
     ChessGame* game = new ChessGame( 0 );            
         parser = new Parser(infile, game);
     infile = parser->ReadPNGFile(infile);
     result.addr = game;    
     delete parser;     
     return YIELD;
    
      }
      else if (!range->read) {
      //New file
      ChessGame* game = new ChessGame( 0 );            
          parser = new Parser(infile, game);
      myFile = ((FText*)args[0].addr);
      
      if(qp->Received(args[0].addr)) {
            //Get Filename
            string filename = myFile->Get();
        //Open file
        delete infile;
        infile = new ifstream(filename.c_str());
        
        infile  = parser->ReadPNGFile(infile);
        range->infile = infile;
        local.addr = range;
        result.addr = game;
        delete parser;
        range->read = true;
        return YIELD;
         }
         else {
            return CANCEL;
     }
      }
      else {
        return CANCEL;
      }      
    
    case CLOSE:
      range = ((Range*) local.addr);
      infile = range->infile;
      
      if(infile->is_open()) {
          infile->close();
      }
      
      delete infile;
      delete range;
           
      return 0;
  }
  return -0;

}


/* 
Value Mapping opeator ~chessreadonegame~ 

*/
int
readonegameFun (Word* args, Word& result, int message, 
            Word& local, Supplier s)
{
  ifstream* infile;
  Parser* parser;
  
  //get result chessGame
  result = qp -> ResultStorage(s);
  ChessGame* game = (ChessGame*)result.addr;
  
   //get filename
  FText *myFile = (FText*)args[0].addr;    
  string pgnfile = myFile->Get();
  
  //open file
  infile = new ifstream(pgnfile.c_str());
  //create Parser
  parser = new Parser(infile, game);        
 
  //read chessgame        
  infile  = parser->ReadPNGFile(infile);

  if(infile->is_open())
      infile->close();
  
  
  delete parser; 
  delete infile;
  return 0;

}


/* 
Specificaton of operator ~chessreadgames~ 

*/
const string MreadgamesSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(text) -> stream (chessgame) </text--->"
  "<text>chessreadgames ( _ ) </text--->"
  "<text>Get a list of png files and returns a stream of chessgames"
  "</text--->"
  "<text>query relOfFiles feed extendstream... </text--->"
  ") )";
  
/* 
Specificaton of operator ~chessreadonegame~ 

*/
const string MreadonegameSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> (text) -> (chessgame) </text--->"
  "<text>chessreadonegame </text--->"
  "<text>Get a path to a PGN file and returns one chessgame"
  "</text--->"
  "<text>query chessreadonegame (mytext) </text--->"
  ") )";


/* 
Definition of operator ~chessreadgames~ 

*/

Operator chessreadgames (
   "chessreadgames",             // name
   MreadgamesSpec,               // specification
   readgamesFun,                 // value mapping
   Operator::SimpleSelect,       // trivial selection function
   readgamesTypeMap              // type mapping
);

/* 
Definition of operator ~chessreadonegame~ 

*/

Operator chessreadonegame (
   "chessreadonegame",           // name
   MreadonegameSpec,             // specification
   readonegameFun,               // value mapping
   Operator::SimpleSelect,       // trivial selection function
   readonegameTypeMap            // type mapping
);

/*
5. Creating Algebra 

*/

class ChessAlgebra : public Algebra
{
 public:
  ChessAlgebra() : Algebra()
  {
    AddTypeConstructor( &chessmove );
    AddTypeConstructor( &chessmaterial );
    AddTypeConstructor( &chessposition );
    AddTypeConstructor( &chessgame );
    
    chessmove.AssociateKind("DATA");         
    chessmaterial.AssociateKind("DATA");
    chessposition.AssociateKind("DATA");
    chessgame.AssociateKind("DATA");
        
    AddOperator( &getkey );
    AddOperator( &getmove );
    AddOperator( &getpos );
    AddOperator( &moves );
    AddOperator( &positions );
    AddOperator( &moveno );
    AddOperator( &pieces );
    AddOperator( &agent );
    AddOperator( &captured );
    AddOperator( &startrow );
    AddOperator( &endrow );
    AddOperator( &startfile );
    AddOperator( &endfile );
    AddOperator( &check );
    AddOperator( &captures );
    AddOperator( &chessrange );
    AddOperator( &chessincludes );
    AddOperator( &chesscount );
    AddOperator( &chesscountall );
    AddOperator( &chessequal );
    AddOperator( &chesslower );
    AddOperator( &movingpoints );
    AddOperator( &chessreadgames );
    AddOperator( &chessreadonegame );
    
  }
  ~ChessAlgebra() {};
};

ChessAlgebra chessAlgebra;


extern "C"
Algebra*
InitializeChessAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&chessAlgebra);
}


