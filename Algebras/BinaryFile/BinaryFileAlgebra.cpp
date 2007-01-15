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

//paragraph [1] title: [{\Large \bf ] [}]


[1] Binary File Algebra

December 2003 VTA

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

This is a little example of the usage of the Base 64 converting 
tool and also FLOBs. The algebra intends to store the contents of 
a binary file.

An operator to save the contents into a file is provided.

1 Defines and includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "Base64.h"
#include "BinaryFileAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

/*
2 Constructor ~binfile~

*/

inline BinaryFile::BinaryFile( const int size ) :
binData( size ),
canDelete( false )
{
}

inline BinaryFile::~BinaryFile()
{
  if( canDelete )
    binData.Destroy();
}

inline void BinaryFile::Destroy()
{
  canDelete = true;
}

inline bool BinaryFile::IsDefined() const
{
  return true;
}

inline void BinaryFile::SetDefined( bool Defined)
{
}

inline size_t BinaryFile::Sizeof() const
{
  return sizeof( *this );
}

inline size_t BinaryFile::HashValue() const
{
  return 0;
}

void BinaryFile::CopyFrom(const StandardAttribute* right) 
{
  const BinaryFile *r = (const BinaryFile *)right;
  binData.Resize( r->binData.Size() );
  const char *bin;
  r->binData.Get( 0, &bin );
  binData.Put( 0, r->binData.Size(), bin );
}

inline int BinaryFile::Compare(const Attribute * arg) const
{
  return 0;
}

inline bool BinaryFile::Adjacent(const Attribute * arg) const
{
  return false;
}

BinaryFile* BinaryFile::Clone() const
{
  BinaryFile *newBinaryFile = new BinaryFile( 0 );
  newBinaryFile->CopyFrom( this );
  return newBinaryFile;
}

ostream& BinaryFile::Print( ostream &os ) const
{
  return os << "BinaryFile Algebra" << endl;
}

inline int BinaryFile::NumOfFLOBs() const
{
  return 1;
}

inline FLOB *BinaryFile::GetFLOB(const int i)
{
  return &binData;
}

void BinaryFile::Encode( string& textBytes ) const
{
  Base64 b;
  const char *bytes;
  binData.Get( 0, &bytes );
  b.encode( bytes, binData.Size(), textBytes );
}

void BinaryFile::Decode( const string& textBytes )
{
  Base64 b;
  int sizeDecoded = b.sizeDecoded( textBytes.size() );
  char *bytes = (char *)malloc( sizeDecoded );

  int result = b.decode( textBytes, bytes );

  assert( result <= sizeDecoded );

  binData.Resize( result );
  binData.Put( 0, result, bytes );
  free( bytes );
}


int BinaryFile::GetSize() const{
   return binData.Size();
}

void BinaryFile::Get(size_t offset,const char** bytes) const{
   binData.Get(offset,bytes); 
}

void BinaryFile::Resize(const int newSize){
    if(newSize<=0){
        binData.Clean();
    } else {
        binData.Resize(newSize);
    }
}

void BinaryFile::Put(const int offset, int size, char* bytes){
   Put(offset,size,bytes);
}

bool BinaryFile::SaveToFile( const char *fileName ) const
{
  FILE *f = fopen( fileName, "wb" );

  if( f == NULL )
    return false;

  const char *bytes;
  binData.Get( 0, &bytes );

  if( fwrite( bytes, 1, binData.Size(), f ) != binData.Size() )
    return false;

  fclose( f );
  return true;
}

/*
2.2 List Representation

The list representation of a ~binfile~ are

----    ( <file>filename</file---> )
----

and

----    ( <text>filename</text---> )
----

If first representation is used, then the contents of a file is read
into the second representation. This is done automatically by the
Secondo parser.

2.3 ~Out~-Function

*/
ListExpr
OutBinaryFile( ListExpr typeInfo, Word value )
{
  ListExpr result = nl->TextAtom();

  BinaryFile *binFile = (BinaryFile *)value.addr;
  string encoded;
  binFile->Encode( encoded );

  nl->AppendText( result, encoded );

  return result;
}

/*
2.4 ~In~-Function

*/
Word
InBinaryFile( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct )
{
  BinaryFile *binFile = new BinaryFile( 0 );

  if( nl->IsAtom( instance ) &&
      nl->AtomType( instance ) == TextType )
  {
    string encoded;
    nl->Text2String( instance, encoded );
    binFile->Decode( encoded );

    correct = true;
    return SetWord( binFile );
  }
  correct = false;
  return SetWord( Address(0) );
}

/*
2.5 The ~Property~-function

*/
ListExpr
BinaryFileProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("binfile"),
                             nl->StringAtom("<file>filename</file--->"),
                             nl->StringAtom("<file>Document.pdf</file--->"),
                             nl->StringAtom(""))));
}

/*
2.6 ~Create~-function

*/
Word
CreateBinaryFile( const ListExpr typeInfo )
{
  return SetWord( new BinaryFile( 0 ) );
}

/*
2.7 ~Delete~-function

*/
void
DeleteBinaryFile( const ListExpr typeInfo, Word& w )
{
  BinaryFile *binFile = (BinaryFile *)w.addr;
  binFile->Destroy();
  delete binFile;
  w.addr = 0;
}

/*
2.8 ~Open~-function

*/
bool
OpenBinaryFile( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  // This Open function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to open objects
  BinaryFile *bf = 
    (BinaryFile*)Attribute::Open( valueRecord, offset, typeInfo );
  value = SetWord( bf );
  return true;
}

/*
2.9 ~Save~-function

*/
bool
SaveBinaryFile( SmiRecord& valueRecord,
                size_t& offset,
                const ListExpr typeInfo,
                Word& value )
{
  BinaryFile *bf = (BinaryFile *)value.addr;

  // This Save function is implemented in the Attribute class
  // and uses the same method of the Tuple manager to save objects
  Attribute::Save( valueRecord, offset, typeInfo, bf );
  return true;
}

/*
2.10 ~Close~-function

*/
void
CloseBinaryFile( const ListExpr typeInfo, Word& w )
{
  delete (BinaryFile *)w.addr;
  w.addr = 0;
}

/*
2.11 ~Clone~-function

*/
Word
CloneBinaryFile( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((BinaryFile *)w.addr)->Clone() );
}

/*
2.12 ~SizeOf~-function

*/
int
SizeOfBinaryFile()
{
  return sizeof(BinaryFile);
}

/*
2.13 ~Cast~-function

*/
void* CastBinaryFile( void* addr )
{
  return new (addr) BinaryFile;
}

/*
2.14 Kind Checking Function

This function checks whether the type constructor is applied 
correctly. Since type constructor ~binfile~ does not have arguments, this is trivial.

*/
bool
CheckBinaryFile( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "binfile" ));
}

/*
2.15 Creation of the Type Constructor Instance

*/
TypeConstructor binfile(
  "binfile",                           //name
  BinaryFileProperty,                  //property function describing signature
  OutBinaryFile,     InBinaryFile,     //Out and In functions
  0,                 0,                //SaveTo and RestoreFrom List functions
  CreateBinaryFile,  DeleteBinaryFile, //object creation and deletion
  OpenBinaryFile,    SaveBinaryFile,   //object open and save
  CloseBinaryFile,   CloneBinaryFile,  //object close and clone
  CastBinaryFile,                      //cast function
  SizeOfBinaryFile,                    //sizeof function
CheckBinaryFile );                     //kind checking function

/*
4 Operators

4.1 Operator ~saveto~

Saves the bynary contents of into a file.

5.6.1 Type mapping function of operator ~saveto~

Operator ~saveto~ accepts a binary file object and a string 
representing the name of the file, and returns a boolean meaning 
success or not.

----    (binfile string)               -> bool
----

*/
ListExpr
SaveToTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "binfile") && 
         nl->IsEqual(arg2, "string") )
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping functions of operator ~saveto~

*/
int
SaveToFun(Word* args, Word& result, int message, 
          Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  BinaryFile *binFile = (BinaryFile*)args[0].addr;
  CcString *fileName = (CcString*)args[1].addr;

  if( binFile->SaveToFile( *(fileName->GetStringval()) ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*

4.1.3 Specification of operator ~saveto~

*/
const string SaveToSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(binfile string) -> bool"
  "</text--->"
  "<text>_ saveto _</text--->"
  "<text>Saves the contents of the object into a "
  "file.</text--->"
  "<text>query bin saveto \"filename.dat\"</text--->"
  ") )";

/*

4.1.4 Definition of operator ~saveto~

*/
Operator saveto (
        "saveto",               //name
        SaveToSpec,             //specification
        SaveToFun,              //value mapping
        Operator::SimpleSelect, //trivial selection function
        SaveToTypeMap           //type mapping
);


/*
5 Creating the Algebra

*/
class BinaryFileAlgebra : public Algebra
{
 public:
  BinaryFileAlgebra() : Algebra()
  {
    AddTypeConstructor( &binfile );

    binfile.AssociateKind("DATA");
    binfile.AssociateKind("FILE");

    AddOperator( &saveto );

  }
  ~BinaryFileAlgebra() {};
};

BinaryFileAlgebra binFileAlgebra;

/*
6 Initialization

Each algebra module needs an initialization function. The algebra 
manager has a reference to this function if this algebra is 
included in the list of required algebras, thus forcing the linker 
to include this module.

The algebra manager invokes this function to get a reference to the 
instance of the algebra class and to provide references to the 
global nested list container (used to store constructor, type, 
operator and object information) and to the query processor.

The function has a C interface to make it possible to load the 
algebra dynamically at runtime.

*/
extern "C"
Algebra*
InitializeBinaryFileAlgebra( NestedList* nlRef, 
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (&binFileAlgebra);
}


