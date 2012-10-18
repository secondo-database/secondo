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
#include "ListUtils.h"
#include "StandardTypes.h"
#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"
#include "Base64.h"
#include "BinaryFileAlgebra.h"
#include "FTextAlgebra.h"
#include <cstdlib>

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace ftext;

/*
2 Constructor ~binfile~

*/

BinaryFile::BinaryFile( const int size, const bool defined /* =0 */ ) :
Attribute(defined),
binData( size ),
canDelete( false )
{ }

inline BinaryFile::~BinaryFile(){
  if( canDelete ){
    binData.destroy();
  }
}

inline void BinaryFile::Destroy(){
  canDelete = true;
}

inline size_t BinaryFile::Sizeof() const{
  return sizeof( *this );
}

inline size_t BinaryFile::HashValue() const{
  // somewhat simplistic, but better than always returning '0':
  if( !IsDefined() ){
    return 0;
  }
  return static_cast<size_t>(binData.getSize()) ;
}

void BinaryFile::CopyFrom(const Attribute* right){
  const BinaryFile *r = static_cast<const BinaryFile*>(right);
  binData.copyFrom( r->binData );
  canDelete = r->canDelete;
  SetDefined( r->IsDefined() );
}

inline int BinaryFile::Compare(const Attribute* arg) const{
  const BinaryFile* r = static_cast<const BinaryFile*>(arg);
  if( !IsDefined() ){
    if(r->IsDefined()){
      return -1;
    } else {
      return 0;
    }
  } else if(!r->IsDefined()){
    return 1;
  }
  // both are defined...
  size_t cmpsize = min(binData.getSize(),r->binData.getSize());
  if(cmpsize == 0) {
    return 0;
  }
  char me[cmpsize];
  char other[cmpsize];
  binData.read(me, cmpsize);
  r->binData.read(other, cmpsize);
  int cmpresult = memcmp(me, other, cmpsize);
  if(cmpresult < 0){
    return -1;
  } else if(cmpresult > 0){
    return 1;
  }
  return 0;
}

inline bool BinaryFile::Adjacent(const Attribute * arg) const{
  return false;
}

BinaryFile* BinaryFile::Clone() const{
  BinaryFile *newBinaryFile = new BinaryFile( 0 );
  newBinaryFile->CopyFrom( this );
  return newBinaryFile;
}

ostream& BinaryFile::Print( ostream &os ) const{
  os << "BinaryFile: ";
  if( IsDefined() ){
    os << "DEFINED, size = " << binData.getSize() << endl;
  } else {
    os << "UNDEFINED." << endl;
  }
  return os;
}

inline int BinaryFile::NumOfFLOBs() const{
  return 1;
}

inline Flob *BinaryFile::GetFLOB(const int i){
  assert( i == 0 );
  return &binData;
}

void BinaryFile::Encode( string& textBytes ) const{
  Base64 b;
  if( !IsDefined() ){
    textBytes = "";
    return;
  }
  size_t mysize = binData.getSize();
  char bytes[mysize];
  binData.read( bytes, mysize, 0 );
  b.encode( bytes, mysize , textBytes );
}

void BinaryFile::Decode( const string& textBytes ){
  Base64 b;
  int sizeDecoded = b.sizeDecoded( textBytes.size() );
  char *bytes = (char *)malloc( sizeDecoded );

  int result = b.decode( textBytes, bytes );

  assert( result <= sizeDecoded );

  if( result <= sizeDecoded ){
    binData.resize( result );
    binData.write( bytes, result, 0 );
    SetDefined( true );
  } else {
    binData.clean();
    SetDefined( true );
  }
  free( bytes );
}


int BinaryFile::GetSize() const{
  return binData.getSize();
}

void BinaryFile::Get(const size_t offset, const size_t size,
                     char* bytes) const{
  assert( (offset + size) >= binData.getSize() );
  binData.read(bytes, size, offset);
}

void BinaryFile::Resize(const int newSize){
    if(newSize<=0){
        binData.clean();
    } else {
        binData.resize(newSize);
    }
}

void BinaryFile::Put(const size_t offset, const size_t size, const char* bytes){
  if( offset+size > binData.getSize() ){
    binData.resize( offset + size );
  }
  binData.write( bytes, offset, size );
}

bool BinaryFile::SaveToFile( const char *fileName ) const{
  if( !IsDefined() ){ return false; }

  FILE *f = fopen( fileName, "wb" );
  if( f == NULL ) { return false; }

  cout << "write " << binData.getSize() << " bytes " << endl;

  char* bytes  = new char[binData.getSize()];
  binData.read( bytes, 0, binData.getSize() );

  if( fwrite( bytes, 1, binData.getSize(), f ) != binData.getSize() ){
    delete[] bytes;
    return false;
  }
  delete[] bytes;
  fclose( f );
  return true;
}

void* BinaryFile::Cast(void* addr){
  return new ( addr ) BinaryFile();
}


/*
2.2 List Representation

The list representation of a ~binfile~ are

----    ( <file>filename</file---> )
----

and

----    ( <text>filename</text---> )
----

and

----    ( "undef" )
----

If first representation is used, then the contents of a file is read
into the second representation. This is done automatically by the
Secondo parser.

2.3 ~Out~-Function

*/
ListExpr
OutBinaryFile( ListExpr typeInfo, Word value ){
  ListExpr result;
  BinaryFile* binFile = static_cast<BinaryFile*>(value.addr);

  if( binFile->IsDefined() ){
    result = nl->TextAtom();
    string encoded;
    binFile->Encode( encoded );
    nl->AppendText( result, encoded );
  } else {
    result = nl->SymbolAtom(Symbol::UNDEFINED());
  }
  return result;
}

/*
2.4 ~In~-Function

*/
Word
InBinaryFile( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct ){
  BinaryFile *binFile = 0;
  ListExpr First;
  if (nl->ListLength( instance ) == 1)
    First = nl->First(instance);
  else
    First = instance;

  if ( listutils::isSymbolUndefined(First) )
  {
    binFile = new BinaryFile( 0, false );
    correct = true;
    return SetWord(binFile);
  }

  if( nl->IsAtom( First ) && nl->AtomType( First ) == TextType ){
    binFile = new BinaryFile( 0, true );
    string encoded;
    nl->Text2String( instance, encoded );
    //ofstream f;
    //f.open("binfile.base64");
    //f << encoded << endl;
    //f.close();
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
BinaryFileProperty(){
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(BinaryFile::BasicType()),
                             nl->StringAtom("<file>filename</file--->"),
                             nl->StringAtom("<file>Document.pdf</file--->"),
                             nl->StringAtom(""))));
}

ListExpr
FilePathProperty(){
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(FilePath::BasicType()),
                             nl->StringAtom("<text>filename</text--->"),
                             nl->StringAtom("<text>../image.jpg</text--->"),
                             nl->StringAtom(""))));
}
/*
2.6 ~Create~-function

*/
Word
CreateBinaryFile( const ListExpr typeInfo ){
  return SetWord( new BinaryFile( 0, true ) );
}

/*
2.7 ~Delete~-function

*/
void
DeleteBinaryFile( const ListExpr typeInfo, Word& w )
{
  BinaryFile *binFile = static_cast<BinaryFile*>(w.addr);
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
    static_cast<BinaryFile*>(Attribute::Open( valueRecord, offset, typeInfo ));
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
  BinaryFile *bf = static_cast<BinaryFile*>(value.addr);

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
  delete static_cast<BinaryFile*>(w.addr);
  w.addr = 0;
}

/*
2.11 ~Clone~-function

*/
Word
CloneBinaryFile( const ListExpr typeInfo, const Word& w )
{
  return SetWord( (static_cast<BinaryFile*>(w.addr))->Clone() );
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

We use a static cast function here...

*/

/*
2.14 Kind Checking Function

This function checks whether the type constructor is applied
correctly. Since type constructor ~binfile~ does not have arguments, this is trivial.

*/
bool
CheckBinaryFile( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, BinaryFile::BasicType() ));
}

bool
CheckFilePath( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, FilePath::BasicType() ));
}
/*
2.15 Creation of the Type Constructor Instances

*/
TypeConstructor binfile(
  BinaryFile::BasicType(),                           //name
  BinaryFileProperty,                  //property function describing signature
  OutBinaryFile,     InBinaryFile,     //Out and In functions
  0,                 0,                //SaveTo and RestoreFrom List functions
  CreateBinaryFile,  DeleteBinaryFile, //object creation and deletion
  OpenBinaryFile,    SaveBinaryFile,   //object open and save
  CloseBinaryFile,   CloneBinaryFile,  //object close and clone
  BinaryFile::Cast,                    //cast function
  SizeOfBinaryFile,                    //sizeof function
CheckBinaryFile );                     //kind checking function


TypeConstructor filepath(
  FilePath::BasicType(),                           //name
  FilePathProperty,                  //property function describing signature
  OutFText,  InFText,     //Out and In functions
  0,                 0,                //SaveTo and RestoreFrom List functions
  CreateFText,  DeleteFText, //object creation and deletion
  OpenFText,    SaveFText,   //object open and save
  CloseFText,   CloneFText,  //object close and clone
  CastFText,                      //cast function
  SizeOfFText,                    //sizeof function
  CheckFilePath );                     //kind checking function
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
    if ( nl->IsEqual(arg1, BinaryFile::BasicType()) &&
         nl->IsEqual(arg2, CcString::BasicType()) )
    return nl->SymbolAtom(CcBool::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

4.1.2 Value mapping function of operator ~saveto~

*/
int
SaveToFun(Word* args, Word& result, int message,
          Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  BinaryFile *binFile = static_cast<BinaryFile*>(args[0].addr);
  CcString *fileName = static_cast<CcString*>(args[1].addr);

  if( !binFile->IsDefined() || !fileName->IsDefined() ){
    (static_cast<CcBool*>(result.addr))->Set( false, false );
  } else if( binFile->SaveToFile( *(fileName->GetStringval()) ) )
    (static_cast<CcBool*>(result.addr))->Set( true, true );
  else
    (static_cast<CcBool*>(result.addr))->Set( true, false );

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
  "file. if either of the objects is undefined, so is the result. Otherwise, "
  "the result of the save operation is returne (success:TRUE, failure:FALSE)"
  "</text--->"
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

    binfile.AssociateKind(Kind::DATA());
    binfile.AssociateKind(Kind::FILE());

    AddTypeConstructor( &filepath );
    filepath.AssociateKind(Kind::DATA());

    AddOperator( &saveto );

  }
  ~BinaryFileAlgebra() {};
};


const string FilePath::BasicType() { return "filepath"; }


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
  return (new BinaryFileAlgebra());
}


