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

[1] Header file  of the Relational Algebra

March 2003 Victor Almeida created the new Relational Algebra
organization

Oct 2004 M. Spiekermann changed some frequently called ~small~
functions into inline functions implemented in the header file. This
reduced code redundance since the same code written in
RelationMainMemory and RelationPersistent can be kept together here
and may improve performance when the code is compiled with
optimization flags.

June-July 2004 M. Spiekermann. Changes in class ~Tuple~ and
~TupleBuffer~. Storing the attribute array as member variable in
class Tuple reduces processing overhead. Moreover the array is
needed for both implementations "persistent" and "memory" hence it
should not be maintained in class ~privateTuple~. The TupleBuffer
was extended. The TupleBuffer constructor was extended by a new
boolean parameter which indicates whether the tuples in it are
stored as "free" or "non-free" tuples.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

May 2006 M. Spiekermann. Changes in ~TupleCompareBy~. The number of compare
operations was antisymmetric, e.g. recognizing that two attributes fulfill
$A > B$ needed one integer comparison more than $A < B$. Now first $A = B$ is
tested and determining one of the remaining cases needs only one extra
comparison. Hence in the average we will use less ${=,<}$ operations than
before.

April 2007, T. Behr, M. Spiekermann. Removal of the main memory implementation
of this algebmain memory implementation
of this algebra module.

September 2007, M. Spiekermann. Dependencies to algebra OldRelationAlgebra removed.

June 2009, S. Jungnickel. Added classes ~TupleFile~ and ~TupleFileIterator~.
New methods ~Save~ and ~Open~ in class ~Tuple~ to save and restore tuples to/from
a ~TupleFile~.

Sept 2009. M. Spiekermann. Due to problems with too many open files communicated 
by S. Jungnickel several codelines of the classes FLOBCache, Relation and in the 
StorageManagement need to be changed. Moreover, the class ~PrivateRelation~ has been
merged into class Relation. The relation itself will now request the FLOBCache to create
LOB-files if necessary. Before, files were created on the fly indicated by a zero 
lobFile-id which was not a satisfying solution for all situations.

*/

class TupleFile;
class TupleFileIterator;
/*
Necessary forward declarations for class ~Tuple~.

*/

/*
1 Extension of class ~Tuple~

Methods ~Open()~ and ~Save()~ for storing and restoring a tuple
to resp. from a ~TupleFile~ were added to this class.

*/

class Tuple
{
  public:

/*
Saves a tuple into a temporary ~tuplefile~.

*/

  void Save( TupleFile& tuplefile );

/*
Opens a tuple from a temporary ~TupleFile~ reading the
current record of ~iter~.

*/
  bool Open( TupleFileIterator *iter );

};


/*
2 Class ~TupleFileIterator~

This class implements a sequential iterator for a ~TupleFile~.

*/

class TupleFileIterator
{
public:

  TupleFileIterator(TupleFile& f);
/*
The constructor. Opens the tuple file ~f~ for reading and
initializes a sequential scan of ~f~.

*/

  ~TupleFileIterator();
/*
The destructor. Closes the tuple file.

*/

  inline bool MoreTuples() { return ( data != 0 ); }
/*
Test if the are more tuples to read in the corresponding ~TupleFile~.

*/

  Tuple* GetNextTuple();
/*
Returns the next tuple from the corresponding ~TupleFile~. If all
tuples have been read the method returns 0;

*/

  friend class Tuple;
/*
Class ~Tuple~ already implements an internal method for
unpacking a tuple and its attributes from a memory block.
In order to reuse this code class ~Tuple~ is declared as a friend
class of ~TupleFileIterator~. ~TupleFileIterator~ simply reads
the next memory block from the ~TupleFile~ and delegates the
unpacking to class ~Tuple~.

*/

protected:

  char* ReadNextTuple(size_t& size);
/*
Returns a pointer to the current data block and its ~size~ and
prefetches the next tuple data block.

*/

  char* readData(size_t& size);
/*
Reads the next tuple data block into memory. Returns a pointer
and the ~size~ of the data block. Used to prefetch the next
data block.

*/

private:

  TupleFile& tupleFile;
/*
Reference to a tuple file.

*/
  char* data;
/*
Pointer to the current data block.

*/

  size_t size;
/*
Size of the current data block.

*/

};

/*
3 Class ~TupleFile~

This class implements a temporary operating system file used
to collect tuples from temporary relations.

*/

class TupleFile
{
public:

  TupleFile(TupleType* tupleType, const size_t bufferSize);
/*
First constructor. Creates a ~TupleFile~ for tuple type ~tupleType~
and sets the I/O Buffer to ~bufferSize~. The filename is created
automatically from the current working directory. The file is stored
in subfolder /tmp and starts with prefix ~TF~.

*/

  TupleFile(TupleType* tupleType, string pathName, const size_t bufferSize);
/*
Second constructor. Creates a ~TupleFile~ for tuple type ~tupleType~
with the specified filename ~pathName~ and sets the I/O Buffer
to ~bufferSize~.

*/

  ~TupleFile();
/*
The destructor. Closes and deletes the tuple file if necessary.

*/

  bool Open();
/*
Opens the tuple file for writing.

*/

  void Close();
/*
Closes the tuple file.

*/

  void Append(Tuple* t);
/*
Appends a tuple to the tuple file.

*/

  inline int GetNoTuples() const { return tupleCount; }
/*
Returns the number of tuples stored in the file.

*/

  inline size_t GetTotalExtSize() const { return totalExtSize; }
/*
Returns the total size of the extension part (including
small FLOBs) of all tuples stored in the file.

*/

  inline size_t GetTotalSize() const { return totalSize; }
/*
Returns the total size of all tuples stored in the file.

*/

  inline const string GetPathName() const { return pathName; }
/*
Returns the file path of the tuple file.

*/

  TupleFileIterator* MakeScan();
/*
Starts a sequential scan of the tuples stored in the tuple file.
Returns a pointer to a ~TupleFileIterator~.

*/

  friend class Tuple;
  friend class TupleFileIterator;
/*
Class ~Tuple~ already implements an internal method for
packing a tuple and its attributes into a memory block.
In order to reuse this code class ~Tuple~ is declared as a friend
class of ~TupleFile~. ~TupleFile~ delegates calls to Append() to
~Tuple~. ~TupleFileIterator~ gains in this way access to
the internal stream object ~stream~.

*/

protected:

  void Append(char *data, size_t core, size_t ext);
/*
Appends the tuple data block ~data~ to the end of the tuple file.
The sizes ~core~ and ~ext~ are used to calculate the total
memory block size. The total size is prepended to the written
data block. This method is called from class ~Tuple~ after
the tuple and its attributes have been packed into a
memory block.

*/

private:

  TupleType* tupleType;
/*
Tuple type of this temporary file.

*/

  string pathName;
/*
File path.

*/

  const size_t bufferSize;
/*
I/O Buffer Size for read and write operations.

*/

  FILE* stream;
/*
Stream object.

*/

  size_t tupleCount;
  size_t totalSize;
  size_t totalExtSize;
/*
Some statistics about number of tuples and tuple sizes.

*/

  static bool traceMode;
/*
Flag to control the trace mode. If set to true, some log
messages on the standard output will be produced.

*/
};

#endif // _RELATION_ALGEBRA_H_
