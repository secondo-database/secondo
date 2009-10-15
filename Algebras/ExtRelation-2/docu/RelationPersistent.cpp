/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~
instead of ~DefaultPersistValue~ which keeps relations that have
been built in memory in a small cache, so that they need not be
rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra
organization

November 2004 M. Spiekermann. The declarations of the
PrivateRelation have been moved to the files RelationPersistent.h
and RelationMainMemory.h. This was necessary to implement some
little functions as inline functions.

June 2005 M. Spiekermann. The tuple's size information will now be i
stored in member variables and only recomputed after attributes
were changed. Changes in class ~TupleBuffer~ which allow to store
tuples as "free" or "non-free" tuples in it.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

April 2006, M. Spiekermann. Introduction of a new function ~clearAll~ in class
~PrivateTupleBuffer~. This function calls the tuples' member functions
~DecReference~ and ~DeleteIfAllowed~ instead of deleting the tuple pointer
directly and is called by the destructor.

January 2007, M. Spiekermann. A memory leak in ~PrivateTupleBuffer~ has been
fixed.

April 2007, T Behr. The concept of solid tuples has been removed.

May 2007, M. Spiekermann. From now on the function TupleBuffer::Clear()
deletes a the pointer to a relation object and marks the buffer's state
as memory only.

June 2009, S.Jungnickel. Added implementation for classes ~TupleFile~ and
~TupleFileIterator~. Added implementation for new methods ~Save~ and ~Open~ 
of class ~Tuple~.

October 2009, S.Jungnickel. ~TupleFile::Append()~ now destroys a tuple when
writing it to disk. Constructor of TupleFileIterator now rewinds read/write
position of a TupleFile. Solved some problems when temporary closing and
reopening a ~TupleFile~.

...

1 Extension of class ~tuple~

*/

void Tuple::Save(TupleFile& tuplefile)
{
  double extSize = 0;
  double size = 0;
  vector<double> attrExtSize(tupleType->GetNoAttributes());
  vector<double> attrSize(tupleType->GetNoAttributes());

  // Calculate the size of the small FLOB data which will be
  // saved together with the tuple attributes and save the LOBs
  // in the lobFile.
  extSize += tupleType->GetCoreSize();
  size += tupleType->GetCoreSize();
  size_t coreSize = 0;
  size_t extensionSize = CalculateBlockSize( coreSize, extSize,
                                             size, attrExtSize,
                                             attrSize, true );

  // Put core and extension part into a single memory block
  // Note: this memory block contains already the block size
  // as uint16_t value
  char* data = WriteToBlock(coreSize, extensionSize);

  // Append data to temporary tuple file
  tuplefile.Append(data, coreSize, extensionSize);

  // Free the allocated memory block
  free(data);
}


bool Tuple::Open(TupleFileIterator *iter)
{
  if ( iter->MoreTuples() )
  {
    size_t rootSize;

    char* data = iter->ReadNextTuple(rootSize);

    if (data)
    {
      assert(rootSize < MAX_TUPLESIZE);
      InitializeAttributes(data, rootSize);
      free(data);
      return true;
    }
  }

  return false;
}



/*
2 Implementation of the class ~TupleFileIterator~

*/

TupleFileIterator::TupleFileIterator(TupleFile& f)
: tupleFile(f)
, data(0)
, size(0)
{
  // First close stream if it still open
  tupleFile.Close();

  // Open stream for reading
  tupleFile.stream = fopen((char*)tupleFile.pathName.c_str(), "rb");

  if( !tupleFile.stream )
  {
    cerr << "TupleFileIterator: Cannot open file '" 
         << tupleFile.pathName << "' for binary reading!\n" << endl;
    return;
  }

  // reposition stream to beginning of file
  rewind(tupleFile.stream);

  if ( tupleFile.traceMode )
  {
    cmsg.info() << "TupleFile " << tupleFile.pathName 
                << " opened for reading." << endl;
    cmsg.send();
  }

  // Read data of first tuple
  data = readData(size);
}

TupleFileIterator::~TupleFileIterator()
{
  tupleFile.Close();
}

Tuple* TupleFileIterator::GetNextTuple()
{
  if ( data )
  {
    Tuple* t = new Tuple((TupleType*)tupleFile.tupleType);

    // Read tuple data from disk buffer
    if ( t->Open(this) )
    {
      return t;
    }
    else
    {
      delete t;
      return 0;
    }
  }

  return 0;
}

char* TupleFileIterator::ReadNextTuple(size_t& size)
{
  if ( data )
  {
    char* tmp = data;

    size = this->size;

    // Read data of next tuple
    data = readData(this->size);

    return tmp;
  }
  else
  {
    size = 0;
    return 0;
  }
}

char* TupleFileIterator::readData(size_t& size)
{
  if ( tupleFile.stream )
  {
    uint16_t blockSize;

    // Read the size of the next data block
    size_t rc = fread(&blockSize, 1, sizeof(blockSize), tupleFile.stream);

    if ( feof(tupleFile.stream) )
    {
      size = 0;
      return 0;
    }
    else if ( rc < sizeof(blockSize) )
    {
      cerr << "TupleFileIterator::ReadNextTuple: error "
           << "reading data block size in file '"
           << tupleFile.pathName << "'!\n" << endl;
      size = 0;
      return 0;
    }

    size = sizeof(blockSize) + blockSize;

    // Allocate a single memory block for data block size and tuple data
    char* data = (char*)malloc(size);

    // Store block size in memory block
    *((uint16_t*)data) = blockSize;

    // Read tuple data into memory block
    rc = fread(data + sizeof(blockSize), 1, blockSize, tupleFile.stream);

    if ( rc < blockSize )
    {
      cerr << "TupleFileIterator::ReadNextTuple: error "
           << "reading tuple data block in file '"
           << tupleFile.pathName << "'!\n" << endl;
      size = 0;
      return 0;
    }

    return data;
  }
  else
  {
    size = 0;
    return 0;
  }
}

/*
3 Implementation of class ~TupleFile~

*/

bool TupleFile::traceMode = false;

TupleFile::TupleFile( TupleType* tupleType,
                        const size_t bufferSize )
: tupleType(tupleType)
, bufferSize(bufferSize)
, stream(0)
, tupleCount(0)
, totalSize(0)
, totalExtSize(0)
{
  this->tupleType->IncReference();

  // create a unique temporary filename
  string str = FileSystem::GetCurrentFolder();
  FileSystem::AppendItem(str, "tmp");
  FileSystem::AppendItem(str, "TF");
  pathName = FileSystem::MakeTemp(str);

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " created." << endl;
    cmsg.send();
  }
}

TupleFile::TupleFile( TupleType* tupleType,
                        string pathName,
                        const size_t bufferSize )
: tupleType(tupleType)
, pathName(pathName)
, bufferSize(bufferSize)
, stream(0)
, tupleCount(0)
, totalSize(0)
, totalExtSize(0)
{
  this->tupleType->IncReference();

  if ( pathName == "" )
  {
    // create a unique temporary filename
    string str = FileSystem::GetCurrentFolder();
    FileSystem::AppendItem(str, "tmp");
    FileSystem::AppendItem(str, "TF");
    pathName = FileSystem::MakeTemp(str);
  }

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " created." << endl;
    cmsg.send();
  }
}

TupleFile::~TupleFile()
{
  // close stream if still open
  Close();

  // delete reference to tuple type
  tupleType->DeleteIfAllowed();

  // delete temporary disk file
  FileSystem::DeleteFileOrFolder(pathName);

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " deleted." << endl;
    cmsg.send();
  }
}

bool TupleFile::Open()
{
  if ( tupleCount == 0 )
  {
    // Open fresh binary stream
    stream = fopen(pathName.c_str(), "w+b" );

    if ( traceMode )
    {
      cmsg.info() << "TupleFile opened (w+b)." << endl;
      cmsg.send();
    }
  }
  else
  {
    // Append to existing file if tupleCount > 0
    stream = fopen(pathName.c_str(), "a+b" );

    if ( traceMode )
    {
      cmsg.info() << "TupleFile opened (a+b)" << endl;
      cmsg.send();
    }
  }

  if( !stream )
  {
    cerr << "TupleFile::Open(): Cannot open file '" 
         << pathName << "'!\n" << endl;
    return false;
  }

  // Set stream buffer size
  if ( setvbuf(stream, 0, bufferSize >= 2 ? _IOFBF : _IONBF, bufferSize) != 0 )
  {
    cerr << "TupleFile::Open(): illegal buffer type or size specified '" 
         << bufferSize << "'!\n" << endl;
    return false;
  }

  if ( traceMode )
  {
    cmsg.info() << "TupleFile " << pathName << " opened for writing." << endl;
    cmsg.send();
  }

  return true;
}

void TupleFile::Close()
{
  if ( stream != 0 )
  {
    if ( fclose(stream) == EOF )
    {
      cerr << "TupleFile::Close(): error while closing file '" 
           << pathName << "'!\n" << endl;
    }
    stream = 0;

    if ( traceMode )
    {
      cmsg.info() << "TupleFile " << pathName << " closed." << endl;
      cmsg.send();
    }
  }
}

void TupleFile::Append(Tuple* t)
{
  t->Save(*this);
  t->DeleteIfAllowed();
}

void TupleFile::Append(char *data, size_t core, size_t ext)
{
  if ( stream == 0 )
  {
    this->Open();
  }

  uint16_t size = core + ext;

  // append data block to file
  size_t rc = fwrite(data, 1, sizeof(size) + size, stream);

  // check the number of written
  if ( rc < sizeof(size) + size )
  {
    cerr << "TupleFile::Append(): error writing to file '"
         << pathName << "'!\n" << endl;
    cerr << "(" << sizeof(size) + size
         << ") bytes should be written, but only ("
         << rc << ") bytes were written" "!\n" << endl;
    return;
  }

  tupleCount++;
  totalSize += size;
  totalExtSize += ext;

  return;
}

TupleFileIterator* TupleFile::MakeScan()
{
  return new TupleFileIterator(*this);
}


