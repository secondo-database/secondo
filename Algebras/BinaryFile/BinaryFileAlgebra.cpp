/*
//paragraph [1] title: [{\Large \bf ]	[}]


[1] Binary File Algebra

December 2003 VTA

This is a little example of the usage of the Base 64 converting tool and
also FLOBs. The algebra intends to store the contents of a binary file.
An operator to save the contents into a file is provided.

1 Defines and includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "Base64.h"

extern NestedList* nl;
extern QueryProcessor *qp;

/*
2 Type Constructor ~binfile~

2.1 Class ~BinaryFile~

*/
class BinaryFile : public StandardAttribute
{
  public:

    BinaryFile() {};
/*
This constructor should not be used.

*/
    BinaryFile( const int size );
    ~BinaryFile();
    void Destroy();

    bool IsDefined() const;
    void SetDefined( bool Defined);
    size_t HashValue();
    void CopyFrom(StandardAttribute* right);
    int Compare(Attribute * arg);
    bool Adjacent(Attribute * arg);
    BinaryFile* Clone();
    ostream& Print( ostream &os );
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    void Encode( string& textBytes );
    void Decode( string& textBytes );
    bool SaveToFile( char *fileName );

  private:

    FLOB binData;
    bool canDelete;
};

BinaryFile::BinaryFile( const int size ) :
binData( size ),
canDelete( false )
{
}

BinaryFile::~BinaryFile()
{
  if( canDelete )
    binData.Destroy();
}

void BinaryFile::Destroy()
{
  canDelete = true;
}

bool BinaryFile::IsDefined() const
{
  return true;
}

void BinaryFile::SetDefined( bool Defined)
{
}

size_t BinaryFile::HashValue()
{
  return 0;
}

void BinaryFile::CopyFrom(StandardAttribute* right)
{
  BinaryFile *r = (BinaryFile *)right;
  binData.Resize( r->binData.Size() );
  char *bin = (char *)malloc( r->binData.Size() );
  r->binData.Get( 0, r->binData.Size(), bin );
  binData.Put( 0, r->binData.Size(), bin );
  free( bin );
}

int BinaryFile::Compare(Attribute * arg)
{
  return 0;
}

bool BinaryFile::Adjacent(Attribute * arg)
{
  return false;
}

BinaryFile* BinaryFile::Clone()
{
  BinaryFile *newBinaryFile = new BinaryFile( 0 );
  newBinaryFile->CopyFrom( this );
  return newBinaryFile;
}

ostream& BinaryFile::Print( ostream &os )
{
  return os << "BinaryFile Algebra" << endl;
}

int BinaryFile::NumOfFLOBs()
{
  return 1;
}

FLOB *BinaryFile::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &binData;
}

void BinaryFile::Encode( string& textBytes )
{
  Base64 b;
  char *bytes = (char *)malloc( binData.Size() );
  binData.Get( 0, binData.Size(), bytes );
  b.encode( bytes, binData.Size(), textBytes );
  free( bytes );
}

void BinaryFile::Decode( string& textBytes )
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

bool BinaryFile::SaveToFile( char *fileName )
{
  FILE *f = fopen( fileName, "wb" );

  if( f == NULL )
    return false;

  char *bytes = (char *)malloc( binData.Size() );
  binData.Get( 0, binData.Size(), bytes );

  if( (int)fwrite( bytes, 1, binData.Size(), f ) != binData.Size() )
    return false;

  fclose( f );
  return true;
}

/*
2.2 List Representation

The list representation of a ~binfile~ are

----	( <file>filename</file---> )
----

and

----	( <text>filename</text---> )
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
DeleteBinaryFile( Word& w )
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
  // This Open function is implemented in the TupleElement class
  // and uses the same method of the Tuple manager to open objects
  BinaryFile *bf = (BinaryFile*)TupleElement::Open( valueRecord, offset, typeInfo );
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

  // This Save function is implemented in the TupleElement class
  // and uses the same method of the Tuple manager to save objects
  TupleElement::Save( valueRecord, offset, typeInfo, bf );
  return true;
}

/*
2.10 ~Close~-function

*/
void
CloseBinaryFile( Word& w )
{
  delete (BinaryFile *)w.addr;
  w.addr = 0;
}

/*
2.11 ~Clone~-function

*/
Word
CloneBinaryFile( const Word& w )
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

This function checks whether the type constructor is applied correctly. Since
type constructor ~binfile~ does not have arguments, this is trivial.

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
	"binfile",				//name
	BinaryFileProperty, 	                //property function describing signature
        OutBinaryFile,     InBinaryFile,	//Out and In functions
        0,                 0,	        	//SaveToList and RestoreFromList functions
	CreateBinaryFile,  DeleteBinaryFile,	//object creation and deletion
        OpenBinaryFile,    SaveBinaryFile, 	//object open and save
        CloseBinaryFile,   CloneBinaryFile,    	//object close and clone
	CastBinaryFile,				//cast function
        SizeOfBinaryFile, 			//sizeof function
	CheckBinaryFile,	                //kind checking function
	0, 					//predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
4 Operators

4.1 Operator ~saveto~

Saves the bynary contents of into a file.

5.6.1 Type mapping function of operator ~saveto~

Operator ~saveto~ accepts a binary file object and a string representing
the name of the file, and returns a boolean meaning success or not.

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
    if ( nl->IsEqual(arg1, "binfile") && nl->IsEqual(arg2, "string") )
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*

4.1.2 Value mapping functions of operator ~saveto~

*/
int
SaveToFun(Word* args, Word& result, int message, Word& local, Supplier s)
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
const string SaveToSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
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
        "saveto",           //name
        SaveToSpec,         //specification
        SaveToFun,          //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect,         //trivial selection function
        SaveToTypeMap            //type mapping
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

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeBinaryFileAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&binFileAlgebra);
}


