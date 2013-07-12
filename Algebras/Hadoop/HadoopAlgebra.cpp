/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]
//[<] [$<$]
//[>] [$>$]
//[INSET] [$\in$]

[1] Implementation of HadoopParallelAlgebra

April 2010 Jiamin Lu

[TOC]

[newpage]

1 Abstract

HadoopParallelAlgebra implements all operators containing precast Hadoop jobs.
This algebra cannot be completely compiled, if the \$HADOOP\_HOME variable is
not correctly set.

This algebra includes follow operators:

  * ~hadoopMap~.

  * ~hadoopReduce~.

  * ~hadoopReduce2~.


1 Includes,  Globals

*/

#include "../HadoopParallel/HadoopParallelAlgebra.h"
#include "HadoopAlgebra.h"


/*
9 Data Type fList

During the parallel processing, we need to indicate Secondo objects
distributed in slave nodes of the cluster. There are two situations
need to be considered:

  * Object's pieces are kept in Secondo databases, with a same name.

  * Object's pieces are kept in a series of binary files, start with a 
same name.

The fList is used to process both situations.

Assume a Secondo relation is divided to ~r~ * ~c~ a matrix relation,
with ~r~ rows and ~c~ columns. All part(cell) inside this metrix
are all either stored in slaves' Secondo databases,
or exported as data and type files, kept in slaves' file systems.
If they are kept in database system, it must be a  ~n~ * 1 matrix,
~n~ is the number of the slaves,
as each slave database only has one part of the object with a same name.
If the data are kept in partition files, then each slave may contain
several rows of partition files.

For each fList, during the procedures with Hadoop jobs,
each map task process one row data in the matrix relation,
and divide it into columns of partition files after the internal processing.
Each reduce task process one column partition files,
and transpose it into one row of the output fList.

A fList contains following variables:

  * objectName. The name of the distributed Secondo object. If data is
distributed as Secondo objects in slaves, then all objects
are named as this value. If data is distributed as partition files,
then all these partition file names start with this value.

  * objectType. The schema of this distributed Secondo Object.
At present, it must be a relation.

  * nodesList. Indicate the locations of slaves where the data are 
    distributed on.
It's used to be set by user manually, but now can be read from
the PARALLEL\_SECONDO\_SLAVES list when the fList is built up at the first time,
and then is kept independently in the fList.

While reading an exist fList, and its kept nodesList is different from the
current slave list, then it will be marked as unavailable.
The master node doesn't take part in the distribution of data.

  * fileLocList. Indicate the location of the objects.

    * [<] READ DB / [>] : Here indicate the object is distributed as Secondo 
      Objects in every slave Secondo databases.

    * [<] file path [>] : Here indicate the file location of a partition file, 
      and it must be a absolute file path.

    * null value : Here indicate the partition file is kept in the default file
       path, which is defined in master or slave list.

  * duplicateTimes. Indicate the duplication times for each partition file.
If data are distributed as Secondo objects, then this value must be 1.

  * Available. Denotes whether the last operator which created this 
    list is successfully performed.

  * Distributed. A boolean used to indicate whether data are distributed.

The *Distributed* variable only can be set by operators like ~spread~
which create a flist data, together with data files.
When a flist is undistributed, then it cannot be read data from it,
like what the operator ~collect~ does.


Update at 6th Feb. 2012
For building up the generic parallel Secondo,
a fList object should be used to describe three different kinds of objects:

  * DGO : Distributed global object. Its value is kept on the master node only.

  * DLO : Distributed local object. Its value is distributed on slaves, 
    as Secondo objects.

  * DLF : Distributed local file list. Its value is distributed on slaves, 
    as disk files, only for relation.

Update at 19th Mar. 2012

Remove the DGO kind, since I decided to not create special flist object,
only for pointing out some objects that have already been created is a flist.
Besides, I also need to remove two useless attributes from the value list:

  * available:  when the objectKind field is set as UNDEF kind
or the distributed field is set as false, then available is false, or else 
it is true.

  * inDB: it can be replaced by the objectKind too.

*/
fList::fList(string _on, NList _tl, clusterInfo* _ci, NList _fll,
      size_t _dp, bool _idd, fListKind _fkd, size_t _mrn, size_t _mcn,
      NList _uemq):
    subName(_on), objectType(_tl),
    interCluster(new clusterInfo(*_ci)),
    fileLocList(_fll),
    dupTimes(_dp),
    mrNum(_mrn), mcNum(_mcn),
    isDistributed(_idd),
    objKind(_fkd),
    UEMapQuery(_uemq)   //By default, it is an empty list
{
  if (mrNum == 0 || mcNum == 0){
    if (!verifyLocList()){
      isDistributed = false;
    }
  }
}

fList::fList(fList& rhg):
    subName(rhg.getSubName()),
    objectType(rhg.objectType),
    interCluster(new clusterInfo(*rhg.interCluster)),
    fileLocList(rhg.getLocList()),
    dupTimes(rhg.getDupTimes()),
    mrNum(rhg.getMtxRowNum()),
    mcNum(rhg.getMtxColNum()),
    isDistributed(rhg.isDistributed),
    objKind(rhg.objKind)
{}


/*
9.1 fList::In Function

The ~In~ function accepts following parameters

  * A Secondo object name.
This is a string value express the name of an exist Secondo object.
We use it to get the ~objectName~ and its type expression.
In some cases, the object may not exist in the current database,
then it must has been exported into the file system,
and its type file must be kept in the local *parallel* directory.
If neither the object or the type file exists,
then set the ~correct~ as FALSE.

  * A nodesList.
This is a list of string values, each specifies a IP address of
a node in the cluster. And the first one is viewed as the master
node by default.

  * A fileLocMatrix
This is a nested list that composed by integer numbers,
which denotes the matrix of cell files
E.g., it may looks like this:

---- (  (1 ( 1 2 3 4 5))
        (2 ( 1 2 3 4 5))
        (3 ( 1 2 3 4 5))
        (4 ( 1 2 3 4 5)) )
----

The above example shows that a Secondo objects is divided into
a 4x5 matrix file, and is distributed to a cluster with 4 nodes.
Each node, including the master node, contains five cell files.

  * A duplicate times
This is a integer number used to tell how many duplications of a
cell file are kept inside the cluster.
At present, we adopt a simple chained declustering mechanism to
backup the duplications of the cell files.
Besides the primary node that is denoted in the fileLoc matrix,
it will be copied to (~dupTimes~ - 1) nodes that are listed after
the primary node within the nodesList.

Update at 26/12/2011
The ~nodesList~  is set up at the first time when the fList is built,
by creating a clusterInfo object, and doesn't need to be manually indicated.
After reading, the node list is kept inside the fList,
in case it need to be reloaded into another database.
Therefore it also can be manually indicated,
but the given nodelist must be a subset of the current node list,
so as to keep a fList object while the cluster scale increases.

In clusters like ours, that each node contains two hard disks,
and contains two independent miniSecondo databases,
these databases will be viewed as different slaves inside the cluster.

For the fileLocMatrix, it's possible that one slave may contains
several rows of files, some partition files may don't exist.
All partition files belong to one row must be stored at one slave,
and also in one file path, which is indicated as the last text of each row.
The row number of these partition files have nothing to do with the
slave nodes that store these files.
Hence it may looks like:

----(  (2 (1 2 3 4 5) '')
       (2 (1 2 3 4 5) '')
       (1 (1 2 5)     '\/mnt\/diskb')
       (4 (1 2 3 4 5) '') )
----

The above example also shows a 4x5 matrix relation,
distributed on a cluster at least has 4 nodes.
Each row data has been partitioned into at most 5 pieces.
The 2th node has two rows partition files, while the 3th node doesn't have
 any one of it.
And in the 1th node, the third and fourth column partition files don't exist.
The first row partition files are kept in the 2th node,
while the third row partition files are kept in the 1th node.
All files are kept in slaves' default parallel location,
except the third row, which are kept in a specific path of 1th node.

Update at 01/09/12

As some relation may cannot produce partitions for all rows,
we left an empty row inside the fileLocMatrix,
to denote a row without any partition files.
Hence now the fileLocMatrix should looks like:

----(  (1 (1 2 3 4 5) '')
       (2 (1 2 3 4 5) '')
       (1 () '')
       (2 (1 2 5)     '\/mnt\/diskb')
       (1 (1 2 3 4 5) '') )
----

Here the example indicates a 5x5 matrix relation,
distributed on a cluster with 2 slaves.
The third row is an empty row, it's column list is empty,
since there is no partition files produced for this row.

Update at 01/13/12

Since ~flist~ describes the distribution of tuples,
the type of a flist should be changed from simply flist
to flist(tuple(....)).
At the same time, there is no necessary to indicate an exist
object or type file anymore, since the type is given by users.
The anonymous of flist objects are checked by comparing the
type files if they exist.
The type file keeps the schema as tuple relation,
since it may be required by file-relevant operators.

Updated when ~spread2~ operator is created.
The type for DLF kind flist is set as

----
flist(stream(tuple(....)))
----

The type for DLO kind flist with relation type object is set as

----
flist(rel(tuple(....)))
----

So we can distinguish these two different types.

Update 23th Mar. 2012

I disable the ~In~ function of the fList,
since all the fileLocation is set by operators.

But it may still be needed, in case we want to reload
a fList to another cluster.

Update 18th Apr. 2012

An empty row in the fileLocList is expressed as an empty list directly,
as there is no slave keep the data of this row. Afterwards,
a fileLocMatrix containing empty rows looks like:

----(  (1 (1 2 3 4 5) '')
       (2 (1 2 3 4 5) '')
       ()
       (2 (1 2 5)     '\/mnt\/diskb')
       (1 (1 2 3 4 5) '') )
----

Here this relation is divided into 5 rows, one row is empty, and the left 4 rows
are kept in two slaves equally.


*/

Word fList::In(const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  return SetWord(Address(0));
}

ListExpr fList::Out(ListExpr typeInfo, Word value)
{
  if (value.addr)
  {
    fList* fl = static_cast<fList*>(value.addr);
    NList outList;

    outList.append(NList(fl->getSubName(), true, false));
    outList.append(fl->getNodeList());
    outList.append(fl->getLocList());
    outList.append(NList(fl->getMtxRowNum()));
    outList.append(NList(fl->getMtxColNum()));
    outList.append(NList(fl->getDupTimes()));
    outList.append(NList(fl->isDistributed, false));
    outList.append(NList(fl->objKind));
    outList.append(fl->getUEMapQuery());
    return outList.listExpr();
  }
  else
    return nl->SymbolAtom("undefined");
}

Word fList::Create(const ListExpr typeInfo)
{
  return SetWord(
    new fList("", NList(),new clusterInfo(), NList(), 1));
}

void fList::Delete(const ListExpr typeInfo, Word& w)
{
  fList* data = (fList*)w.addr;

  if (data)
  {
    string objName = data->getSubName();
    string typeFilePath = getLocalFilePath("", objName, "_type");
    if (FileSystem::FileOrFolderExists(typeFilePath)){
      FileSystem::DeleteFileOrFolder(typeFilePath);
    }
    delete data;
  }
  w.addr = 0;
}

void fList::Close(const ListExpr typeInfo, Word& w)
{
  delete (fList*)w.addr;
  w.addr = 0;
}


Word fList::Clone(const ListExpr typeInfo, const Word& w)
{
  return SetWord(new fList(*(fList*)w.addr));
}

bool fList::Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& w)
{
  bool ok = true;

  ListExpr valueList = Out(typeInfo, w);
  valueList = nl->OneElemList(valueList);
  string valueStr;
  nl->WriteToString(valueStr, valueList);
  int valueLength = valueStr.length();
  ok = ok && valueRecord.Write(&valueLength, sizeof(int), offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Write(valueStr.data(), valueLength, offset);
  offset += valueLength;

  return ok;
}

bool fList::Open(SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value)
{
  int valueLength;
  string valueStr = "";
  ListExpr valueList = 0;
  char *buf = 0;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERRORS"));
  bool correct;

  bool ok = true;
  ok = ok && valueRecord.Read(&valueLength, sizeof(int), offset);
  offset += sizeof(int);
  buf = new char[valueLength];
  ok = ok && valueRecord.Read(buf, valueLength, offset);
  offset += valueLength;
  valueStr.assign(buf, valueLength);
  delete []buf;
  nl->ReadFromString(valueStr, valueList);

  value = RestoreFromList(typeInfo, nl->First(valueList),
      1, errorInfo, correct);

  if (errorInfo != 0)
    nl->Destroy(errorInfo);
  nl->Destroy(valueList);
  return ok;
}

Word fList::RestoreFromList(
    const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct )
{
  NList il = NList(instance);
  string objName = il.first().str();
  NList typeList = NList(AntiNumericType(typeInfo));
  NList nodeList = il.second();
  NList locList = il.third();
  size_t maxRNum = il.fourth().intval();
  size_t maxCNum = il.fifth().intval();
  size_t dupTimes = il.sixth().intval();
  bool distributed = il.seventh().boolval();
  fListKind kind = (fListKind)il.eigth().intval();
  NList ueMapQuery;
  if (il.length() == 9)
    ueMapQuery = il.nineth();

  clusterInfo *ci = new clusterInfo();
  fList* fl = 0;
  if (ci->covers(nodeList))
  {
    fl = new fList(objName, typeList, ci, locList, dupTimes,
        distributed, kind, maxRNum, maxCNum, ueMapQuery);
    return SetWord(fl);
  }
  else{
    correct = false;
    return SetWord(Address(0));
  }
}

bool CheckFList(ListExpr type, ListExpr& errorInfo)
{
  if (  (nl->ListLength(type) == 2)
      &&(nl->IsEqual(nl->First(type), fList::BasicType()))){
    return true;
  }
  else{
    cmsg.otherError("FList Data Type Check Fails!");
    return false;
  }
}

/*
The ~In~ function only checks the type of the fileLocList,
but doesn't check the availability of the matrix relation.
This function checks whether the value of the flist is available,
by checking following conditions:

  * each slave index must be less than nodesNum

  * each column number must be a positive number

  * duplicate number is less than nodesNum

  * data path is set as [<]READ DB\/[>] while the data are not in database,
or the other way round.

Check the available of a file location list, also get the maximum row 
and column number.

*/
bool fList::verifyLocList()
{

  if (isAvailable() && (mrNum > 0 || mcNum > 0)){
    return true;
  }

  if (!fileLocList.isEmpty())
  {
    mrNum = fileLocList.length();
    mcNum = 0;

    NList fll = fileLocList;
    while (!fll.isEmpty())
    {
      NList aRow = fll.first();

      if (!aRow.isEmpty())
      {
        int nodeNum = aRow.first().intval();
        if (nodeNum >= (int)interCluster->getClusterSize())
        {
          cerr << "Improper data server number: " << nodeNum << endl;
          return false;
        }
        NList cfList = aRow.second();
        while (!cfList.isEmpty())
        {
          NList aPF = cfList.first();  //A partition file suffix
          int partNum = aPF.intval();
          if (partNum < 1)
          {
            cerr << "Improper column number: " << partNum << endl;
            return false;
          }
          mcNum = (partNum > (int)mcNum) ? partNum : mcNum;
          cfList.rest();
        }

        string dataLoc = aRow.third().str();
  /*
  If the fileLocList is not empty, then it must belong to DLF kind
  If the path value is ~dbLoc~, or is not an absolute path
  then it returns false
  It is impossible to check the available of these paths,
  since they exist on remote machines

  */
        if (dataLoc.length() > 0)
        {
          if (  dataLoc.find('/') != 0
             && dataLoc.compare(dbLoc) != 0){
            cerr << "Improper file path" << dataLoc << endl;
            return false;
          }
        }

      }
      fll.rest();
    }

    if ((isInDB() && dupTimes > 1)
        || (dupTimes >= interCluster->getClusterSize()) )
    {
      cerr << "Improper duplication times: " << dupTimes
          << (isInDB() ? " , when data are kept in databases " : "")
          << endl;
      return false;
    }
    return true;
  }
  return true;
}

size_t fList::getPartitionFileLoc(
    size_t row, vector<string>& locations)
{
  if (!isAvailable()){
    cerr << "not available"
        << " with " << objKind << "," << (isDistributed?"T":"F")
        << endl;
    return 0;
  }

  if ( row > mrNum ){
    cerr << "The row (" << row << ":" << mrNum << ") "
        << " is illegal" << endl;
    return 0;
  }

  ListExpr rowLoc = nl->Nth(row, fileLocList.listExpr());

  if (nl->IsEmpty(rowLoc)){
    return 0; //The current row is empty.
  }

  locations.resize(0);
  stringstream ss;
  int ssIndex = nl->IntValue(nl->First(rowLoc));
  for (size_t i = 0; i < dupTimes; i++){
    string sIPAddr = interCluster->getIP(ssIndex + i, true);
    string dataLoc = nl->Text2String(nl->Third(rowLoc));
    if (objKind == DLO){
      dataLoc = dbLoc;
    }
    else if ((dataLoc.length() == 0) || (i > 0)){
      // duplicated files are kept at remote node's default path
      // Only output the remote folder, not complete file path.
      dataLoc = interCluster->getRemotePath(ssIndex + i,
          false, true, false);
    }
    string remotePath = sIPAddr + ":" + dataLoc;
    locations.push_back(remotePath);
  }
  return dupTimes;
}

ListExpr fList::getColumnList(size_t row)
{
  if ( isAvailable() && row <= mrNum){
    if (!fileLocList.elem(row).isEmpty()){
      return nl->Second(nl->Nth(row, fileLocList.listExpr()));
    }
  }
  return nl->TheEmptyList();
}


void fList::appendFileLocList(NList elem)
{
  if (isDistributed){
    //cannot append new loc information to distributed flist
    cerr << "Error! Cannot append new locations to "
        "a distributed flist."
        << endl;
    return;
  }
  fileLocList.append(elem);
}

struct fListInfo: ConstructorInfo
{
  fListInfo()
  {
    name = "flist";
    signature = " ANY -> FLIST ";
    typeExample = "(flist(rel(tuple((PLZ int)(Ort string)))))";
    listRep = "( objName fileLocList dupTimes inDB )";
    valueExample =
      "( \"plz\" ( (1 (1 2) '') (2 (1 2) '') (1 () '') ) 2 FALSE) ";
    remarks = "Describe distributed data over computer clusters.";
  }
};
struct fListFunctions: ConstructorFunctions<fList>
{
  fListFunctions()
  {
    in = fList::In;
    out = fList::Out;

    create = fList::Create;
    deletion = fList::Delete;
    close = fList::Close;
    clone = fList::Clone;
    kindCheck = CheckFList;

    save = fList::Save;
    open = fList::Open;
  }
};

fListInfo fli;
fListFunctions flf;
TypeConstructor flTC(fli, flf);

/*
5 Operator ~spread~

This operator accepts a tuple stream,
distributes tuples into a matrix relation based on their values of
a given attribute and returns a flist.
The map of the ~spread~ operator is:

----
stream(tuple(a1 ... ai ... aj ... an))
x fileName x filePath x [dupTime]
x ai x [scale] x [keepAI]
x [aj] x [scale2] x [keepAJ]
  -> flist
----

All paratition files are kept as disk files in slave nodes,
and their file names are built up as: fileName\_row\_column,
row in [1,Scale],column in[1,Scale2].
Both fileName and filePath parameters are indispensable.
But the filePath can be set as an empty string,
and its default value listed in the SecondoConfig.ini is then used.

For the purpose of fault-tolerance,  each partition file is
duplicated on ~dupTime~ continuous slave nodes,
the default value of ~dupTime~ is 1.
All duplicated files are kepts in nodes' default paths.

By default, a tuple stream is divided into ~sn~ * 1 partition files,
based on the value of the indispensable parameter ~ai~.
The ~sn~ is the number of slave nodes indicated in the
 PARALLEL\_SECONDO\_SLAVES.
The partition attribute ~ai~ will be removed after the operation, except 
the ~keepAI~ is set as true.


The ~sn~ can also be replaced by the optional parameter ~scale~ parameter,
and files of each row are distributed into a slave node,
based on the order denoted in the PARALLEL\_SECONDO\_SLAVES.

On each row, the data can be further divided into several column partition 
files, if the second key-attribute ~aj~ is given.
The ~aj~ must be different from ~ai~, to avoid producing empty partition files.
The number of columns is decided by the number of values of ~aj~,
and also can be indicated by the optional parameter ~scale2~.
~aj~ also will be removed by default after the operation, except the ~keepAJ~
 is set as true.


*/
struct SpreadInfo : OperatorInfo {

  SpreadInfo() : OperatorInfo()
  {
    name = "spread";
    signature = "stream(tuple(a1 ... ai ... aj ... an)) "
                " x string x text x [int] "
                " x ai x [int] x [bool] "
                " x [aj] x [int] x [bool] "
                " -> flist(tuple(a1 ... ai ... aj ... an))";
    syntax = "stream(tuple(a1 ... ai ... aj ... an)) "
            " x fileName x filePath x [dupTime] "
            " x ai x [scale] x [keepAI] "
            " x [aj] x [scale2] x [keepAJ] "
            "  -> flist(tuple(a1 ... ai ... aj ... an))";
    meaning = "This operator accepts a tuple stream, "
        "distributes its tuples into a matrix relation "
        "based on their values of a given attribute "
        "and returns a flist.";
  }

};

/*

5.1 Type Mapping

----
stream(tuple(a1 ... ai ... aj ... an))
x string x text x [int]
x ai x [int] x [bool]
x [aj] x [int] x [bool]
  -> flist(tuple(a1 ... ai ... aj ... an))
----

During the type mapping function, as we use several optional parameters,
hence it's better to divide these parameters into lists.
The specification of this operator is:

----
_ op[ list;list;list]
----

The first list denotes the fileName and filePath, and the optional ~dupTime~.
The second list denotes the first keyAttribute, together with its 
optional scale.
The third list denotes the optional second keyAttribute, together with 
its optional scale.

The type mapping function produces the text type file for the result files,
and duplicate it to every node's default pathlisted in 
PARALLEL\_SECONDO\_SLAVES\/MASTER.

21th Mar. 2012
Set the file name and path as optional parameters too,
they can be set by rules, without always set by users.

Also the type of the output flist becomes flist(stream(tuple(T))),
used to distinguish with the DLO flists for tuple relations,
which have the type of flist(rel(tuple(T)))
Now the map becomes:

----
stream(tuple(a1 ... ai ... aj ... an))
x string x text x [int]
x ai x [int] x [bool]
x [aj] x [int] x [bool]
  -> flist( stream ( tuple (a1 ... ai ... aj ... an)))
----

*/

ListExpr SpreadTypeMap(ListExpr args){
  try{
    NList l(args);
    string err[] = {
        // 0
        "ERROR! Operator expects 4 lists arguments.",
  
        "ERROR! Operator expects (stream(tuple(a1, a2, ..., an)))"
        "x ( [string] x [text] x [int] ) "
        "x ( ai x [int] x [bool] ) "
        "x ( [aj] x [int] x [bool] )",
  
        "ERROR! Infeasible evaluation in TM for attribute: ",
  
        "ERROR! Unavailable file name: ",
  
        "ERROR! Operator cannot find the dividing attribute: ",
  
        // 5
        "ERROR! Two keyAttributes must be different from each other.",
  
        "ERROR! The result stream tuple type cannot be empty.",
  
        "ERROR! Cannot create homonymous flists. ",
  
        "ERROR! Cannot open file at ",
  
        "ERROR! PARALLEL_SECONDO_SLAVES/MASTER is not set up. ",
  
        // 10
        "ERROR! Remote copy fails to path: ",
  
        "ERROR! This Secondo database is not listed inside "
        "PARALLEL_SECONDO_SLAVES/MASTER "
    };
    bool keepAI = false, keepAJ = false;
  
    if (l.length() != 4)
      return l.typeError(err[0]);
  
    NList pType, pValue;
  
    //First list, stream(tuple())
    string fileName = "", filePath = "";
    NList attrList;
    if (!l.first().first().checkStreamTuple(attrList)){
      return l.typeError(err[1]);
    }
    NList inStream = l.first().second();
  
    //Second list, ([string] [text] [int] )
    NList bpList = l.second();  //basic parameters
    pType = bpList.first();
    pValue = bpList.second();
    if (pType.length() > 3){
      return l.typeError(err[1]);
    }
  
    int len = pType.length();
    for (int i = 1 ; i <= len ; i++)
    {
      NList pp = pType.elem(i);
      NList pv = pValue.elem(i);
  
      if (pp.isSymbol(CcString::BasicType()))
      {
        //Set the file name
        NList fnList;
        if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
          return l.typeError(err[2] + "fileName");
        }
        fileName = fnList.str();
      }
      else if (pp.isSymbol(FText::BasicType()))
      {
        //Set the file path
        NList fpList;
        if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
          return l.typeError(err[2] + "filePath");
        }
        filePath = fpList.str();
      }
      else if (!pp.isSymbol(CcInt::BasicType())){
        return l.typeError(err[1]);
      }
    }
  
    //Third list, (keyAttr1 [int] [bool])
    NList scList1 = l.third(); //scale list 1
    pType = scList1.first();
    pValue= scList1.second();
    if (pType.length() < 1 || pType.length() > 3){
      return l.typeError(err[1]);
    }
    if (!pType.first().isSymbol()){
      return l.typeError(err[1]);
    }
    string keyAI = pType.first().convertToString();
    ListExpr attrType;
    int attrIndex1 =
      listutils::findAttribute(attrList.listExpr(), keyAI, attrType);
    if (attrIndex1 < 1){
      return l.typeError(err[4] + keyAI);
    }
  
    if (pType.length() == 2){
      if (pType.second().isSymbol(CcBool::BasicType())){
        keepAI = pValue.second().boolval();
      }
      else if (!pType.second().isSymbol(CcInt::BasicType())){
        return l.typeError(err[1]);
      }
    }
    else if (pType.length() == 3){
      if (pType.second().isSymbol(CcInt::BasicType())
          && pType.third().isSymbol(CcBool::BasicType())){
        keepAI = pValue.third().boolval();
      }
      else{
        return l.typeError(err[1]);
      }
    }
  
    //Fourth list, ([keyAttr2] [int] [bool])
    NList scList2 = l.fourth(); //scale list 1
    pType = scList2.first();
    pValue= scList2.second();
    string keyAJ = "";
    int attrIndex2 = -1;
    if (pType.length() > 3){
      return l.typeError(err[1]);
    }
    if (pType.length() > 0){
  
      if (!pType.first().isSymbol()){
        return l.typeError(err[1]);
      }
      keyAJ = pType.first().convertToString();
      attrIndex2 =
        listutils::findAttribute(attrList.listExpr(), keyAJ, attrType);
      if (attrIndex2 < 1){
        return l.typeError(err[4] + keyAJ);
      }
      else if (attrIndex2 == attrIndex1){
        return l.typeError(err[5]);
      }
  
      if (pType.length() == 2){
        if (pType.second().isSymbol(CcBool::BasicType())){
          keepAJ = pValue.second().boolval();
        }
        else if (!pType.second().isSymbol(CcInt::BasicType())){
          return l.typeError(err[1]);
        }
      }
      else if (pType.length() == 3){
        if (pType.second().isSymbol(CcInt::BasicType())
            && pType.third().isSymbol(CcBool::BasicType())){
          keepAJ = pValue.third().boolval();
        }
        else{
          return l.typeError(err[1]);
        }
      }
    }
  
    NList newAttrList;
    if (keepAI && keepAJ){
      newAttrList = attrList;
    }
    else{
      NList rest = attrList;
      while (!rest.isEmpty()){
        NList elem = rest.first();
        if (   ((elem.first().str() != keyAI) || (keepAI))
            && ((elem.first().str() != keyAJ) || (keepAJ)) ){
          newAttrList.append(elem);
        }
        rest.rest();
      }
    }
    if (newAttrList.length() == 0){
      return l.typeError(err[6]);
    }
    //Create the type file
    if (fileName.length() == 0)
      fileName = fList::tempName(false);
    NList resultList =
        NList(NList(fList::BasicType(),
              NList(NList(Stream<Tuple>::BasicType()),
              NList(NList(Tuple::BasicType()), newAttrList))));
    filePath = getLocalFilePath(filePath,
        (fileName + "_type"), "", true);
    if (FileSystem::FileOrFolderExists(filePath)){
      ListExpr exeType;
      bool ok = false;
      if (nl->ReadFromFile(filePath, exeType)){
        if (listutils::isTupleStream(exeType)){
          if (nl->Equal(exeType, nl->Second(resultList.listExpr()))){
            ok = true;
          }
        }
      }
      if (!ok)
        return l.typeError(err[7] + filePath);
    }
    else{
      ListExpr expList = nl->Second(resultList.listExpr());
      if (!nl->WriteToFile(filePath, expList)){
        return l.typeError(err[8] + filePath);
      }
    }
  
    //Duplicate the type to master and all slave nodes
    clusterInfo* ci = new clusterInfo();
    if (!ci->isOK()){
      return l.typeError(err[9]);
    }
    if (ci->getLocalNode() < 0){
      return l.typeError(err[11]);
    }
    string masterPath;
    if (!ci->isLocalTheMaster()){
      masterPath = ci->getRemotePath(0);
      if ( 0 != system(
          (scpCommand + filePath + " " + masterPath).c_str())){
        return l.typeError(err[10] + masterPath);
      }
    }
    for (size_t i = 1; i <= ci->getSlaveSize(); i++){
      //Copy the type file to every slave
      string rPath = ci->getRemotePath(i, false);
      if ( 0 != system(
          (scpCommand + filePath + " " + rPath).c_str())){
        return l.typeError(err[10] + rPath);
      }
    }
    return NList(NList(Symbol::APPEND()),
                 NList(NList(attrIndex1),
                       NList(attrIndex2),
                       NList(fileName,true, false)),
                 resultList).listExpr();
  } catch(...){
     return listutils::typeError("invalid input");
  }
}

/*
5.2 Value Mapping

Each partition inside the flist produced by ~spread~ operator
is also composed by two files, the type file and the data file.
All partition files in a same row, i.e. kept in a same slave node, 
share a same type file.

Both kinds files are as same as the files created by ~fconsume~ or 
~fdistribute~ files,
so that they can be read by using ~ffeed~ operator as normal.
The reason is that, during the parallel processing,
the slave nodes don't contain the flist object,
but only read files from their local or neighbors' disks.

The master node deonted by PARALLEL\_SECONDO\_MASTER must contain the type file,
in order to avoid dirty data by checking these type files' names on the 
master node.
Every slave node has one type file. If one slave node has several rows partition
 files, then all rows share a same type file.

where to put the data file? Partition files are produced at the disk where the
 spread operation is executed,
and then is copied to target after the production is finished.

*/
int SpreadValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  SpreadLocalInfo *lif = 0;

  if ( message <= CLOSE )
  {
    //Get the parameters
    result = qp->ResultStorage(s);
    Supplier bspList = args[1].addr,
             partList1 = args[2].addr,
             partList2 = args[3].addr;
    string fileName = "", filePath = "";
    int keyIdxI = -1, keyIdxJ = -1;
    int scaleI = 0, scaleJ = 0;
    bool keepAI = false, keepAJ = false;
    size_t dupTimes = 1;

    keyIdxI = ((CcInt*)args[4].addr)->GetValue() - 1;
    keyIdxJ = ((CcInt*)args[5].addr)->GetValue() - 1;

    int blLen = qp->GetNoSons(bspList);
    for (int i = 0; i < blLen; i++)
    {
      ListExpr pp = qp->GetType(qp->GetSupplierSon(bspList,i));
      if (nl->IsEqual(pp, CcString::BasicType())){
        //File Name is set
        fileName = ((CcString*)qp->Request(
            qp->GetSupplierSon(bspList, i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, FText::BasicType())){
        //File Path is set
        filePath = ((FText*)qp->Request(
            qp->GetSupplierSon(bspList, i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, CcInt::BasicType())){
        //duplicate time is set
        dupTimes = ((CcInt*)qp->Request(
            qp->GetSupplier(bspList,i)).addr)->GetValue();
      }
    }
    if (fileName.length() == 0){
      //The file name is not set
      fileName = ((CcString*)args[6].addr)->GetValue();
    }

    int plLen1 = qp->GetNoSons(partList1);
    if (plLen1 == 2){
      ListExpr argType = qp->GetType(qp->GetSupplier(partList1,1));
      if (nl->IsEqual(argType, CcBool::BasicType())){
        keepAI = ((CcBool*)qp->Request(
            qp->GetSupplier(partList1, 1)).addr)->GetValue();
      }
      else{
        scaleI = ((CcInt*)qp->Request(
            qp->GetSupplier(partList1, 1)).addr)->GetValue();
      }
    }
    else if (plLen1 == 3){
      scaleI = ((CcInt*)qp->Request(
          qp->GetSupplier(partList1, 1)).addr)->GetValue();
      keepAI = ((CcBool*)qp->Request(
          qp->GetSupplier(partList1, 2)).addr)->GetValue();
    }
    int plLen2 = qp->GetNoSons(partList2);
    if (plLen2 == 2){
      ListExpr argType = qp->GetType(qp->GetSupplier(partList2,1));
      if (nl->IsEqual(argType, CcBool::BasicType())){
        keepAJ = ((CcBool*)qp->Request(
            qp->GetSupplier(partList2, 1)).addr)->GetValue();
      }
      else{
        scaleJ = ((CcInt*)qp->Request(
            qp->GetSupplier(partList2, 1)).addr)->GetValue();
      }
    }
    else if (plLen2 == 3){
      scaleJ = ((CcInt*)qp->Request(
          qp->GetSupplier(partList2, 1)).addr)->GetValue();
      keepAJ = ((CcBool*)qp->Request(
          qp->GetSupplier(partList2, 2)).addr)->GetValue();
    }
      lif = (SpreadLocalInfo*)local.addr;
      if (lif) delete lif;
      lif = new SpreadLocalInfo(fileName, filePath, dupTimes,
         keyIdxI, scaleI, keepAI, keyIdxJ, scaleJ, keepAJ);
      if (!lif->isAvailable()){ return 0; }
      local.setAddr(lif);

      Word wTuple(Address(0));
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);
      while (qp->Received(args[0].addr))
      {
        if (!lif->insertTuple(wTuple)){
          cerr << "Inserting tuple to files fail. " << endl;
          break;
        }
        qp->Request(args[0].addr, wTuple);
      }

      if (lif->closeAllPartFiles()){
        result.addr = new fList(*(lif->getResultList()));
      }
      else{
        cerr << "Closing partition files fails" << endl;
      }

      delete lif;
      local.setAddr(0);
  }
//TODO add the progress estimation in the future
//  else if ( message == REQUESTPROGRESS )
//  else if ( message == CLOSEPROGRESS )


  return 0;
}

SpreadLocalInfo::SpreadLocalInfo(
    string fileName, string filePath, int _dp,
    int _ai1, int _rn, bool _kai,
    int _ai2, int _cn, bool _kaj):
    partFileName(fileName),
    attrIndex1(_ai1), attrIndex2(_ai2),
    rowAmount(_rn),colAmount(_cn),
    keepA1(_kai), keepA2(_kaj), done(false), tupleCounter(0),
    dupTimes(_dp)
{
  partFilePath = getLocalFilePath(filePath, fileName, "", false);

  // Read the schema from the type file created in type mapping
  string typeFilePath = getLocalFilePath(
      filePath, fileName, "_type");
  ListExpr resultTypeList;

  if (!nl->ReadFromFile(typeFilePath, resultTypeList)){
    cerr << "Reading result schema from the type file fails. "
        << endl;
    return;
  }

  ci = new clusterInfo();
  if (ci->isOK())
  {
    if (rowAmount == 0){
//By default, the row number of a flist is the number of slaves
      rowAmount = ci->getSlaveSize();
    }

    //This operator only creates DLF kind distributed relations
    resultList = new fList(fileName,
        NList(resultTypeList), ci, NList(),
        dupTimes, false, DLF, rowAmount, colAmount);

    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    resultTypeList = sc->NumericType(nl->Second(resultTypeList));
    exportTupleType = new TupleType(resultTypeList);
  }
}

bool SpreadLocalInfo::insertTuple(Word wTuple)
{
  // Insert a tuple into a proper data file.
  // row and column number are calculated based on
  // tuple's key attributes value.
  // If the file has not been created, then create it,
  // and insert the file pointer to the matrix
  // Or else, find the file pointer from the matrix,

  Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
  size_t row    = hashValue(tuple, attrIndex1, rowAmount) + 1;
  size_t column = 1;
  if ( attrIndex2 >= 0 ){
    column = hashValue(tuple, attrIndex2, colAmount) + 1;
  }

  fileInfo* fp = 0;
  map<size_t, rowFile*>::iterator mit = matrixRel.find(row);
  if (mit != matrixRel.end()){
    rowFile::iterator rit = mit->second->find(column);
    if (rit != mit->second->end()){
      fp = rit->second;
    }else{
      //create a new partition file.
      fp = new fileInfo(column, partFilePath, partFileName,
          exportTupleType->GetNoAttributes(), row);
      mit->second->insert(pair<size_t, fileInfo*>(column, fp));
    }
  }else
  {
    // Create a new rowFile
    fp = new fileInfo(column, partFilePath, partFileName,
      exportTupleType->GetNoAttributes(), row);
    rowFile *newRow = new rowFile();
    newRow->insert(pair<size_t, fileInfo*>(column, fp));
    matrixRel.insert(pair<size_t, rowFile*>(row, newRow));
  }

  bool ok = openFile(fp);

  if (ok)
  {
    if (!fp->writeTuple(tuple, tupleCounter, exportTupleType,
        attrIndex1, keepA1, attrIndex2, keepA2)){
      cerr << "Block file " << fp->getFilePath()
          << " write failes." << endl;
      ok = false;
    }
    else{
      tupleCounter++;
      tuple->DeleteIfAllowed();
    }
  }
  return ok;
}

bool SpreadLocalInfo::openFile(fileInfo *fp){
  //Control the amount of opening file handles

  if (fp->isFileOpen()){
    return true;
  }

  if (openFileList.size() >= MAX_OPENFILE_NUM)
  {
    //sort fileInfos according to their last tuples' indices
    sort(openFileList.begin(), openFileList.end(), compFileInfo);
    //The last one of the vector is the idler
    bool poped = false;
    //It's possible that fileInfos kept in the stack,
    //are closed from other functions.
    while(!poped && openFileList.size() > 0)
    {
      fileInfo* oldestFile = openFileList.back();
      if (oldestFile->isFileOpen())
      {
        oldestFile->closeFile();
        poped = true;
      }
      openFileList.pop_back();
    }
  }


  bool ok = fp->openFile();
  if (ok){
    openFileList.push_back(fp);
  }
  return ok;
}

bool SpreadLocalInfo::closeAllPartFiles()
{
  //traverse the whole matrix,
  //to add the last description list on all part files.
  //Then close and duplicate them.

  size_t lastRow = 0;

  map<size_t, rowFile*>::iterator mit = matrixRel.begin();
  while (mit != matrixRel.end()){
    size_t row = mit->first;
    if (row > (lastRow + 1)){
      //Insert empty rows
      NList emptyRowList = NList();
      for (size_t erow = (lastRow + 1); erow < row; erow++)
      {
        resultList->appendFileLocList(emptyRowList);
      }
    }
    lastRow = row;
    rowFile::iterator rit = mit->second->begin();
    NList columnList;
    string filePaths = "";
    while ( rit!= mit->second->end()){
      size_t column = rit->first;
      columnList.append(NList((int)column));
      fileInfo* fp = rit->second;
      if (openFile(fp))
      {
        fp->writeLastDscr();
        fp->closeFile();
      }
      else
      {
        cerr << "Part file " << fp->getFilePath()
            << " Cannot be correctly opened, "
                "when writing the last description list. " << endl;
        return false;
      }
      filePaths += (fp->getFilePath() + " ");
      rit++;
    }

    bool *copyList = new bool[ci->getClusterSize()];
    memset(copyList, 0, ci->getClusterSize());
    size_t startNode = row;
    for (size_t i = 0; i < dupTimes; i++){
      size_t dupNode = ci->getInterIndex(
          (startNode + i), false, true);
      copyList[dupNode] = true;
    }

    for(size_t i = 0; i < ci->getClusterSize(); i++){
      if (copyList[i]){
        string remotePath =
            ci->getRemotePath(i, false, true, true);
        int copyTime = MAX_COPYTIMES;
        while (copyTime-- > 0)
        {
          if (system(
              (scpCommand + filePaths + " " + remotePath).c_str())
              != 0 ){
            cerr << "Warning! Duplicate files "
                << filePaths << " to " << endl
                << remotePath << " fails. " << endl
                << strerror(error) << endl;
          }
          else
            break;
        }
        if (copyTime <= 0){
          cerr << "Error! Duplicate remote files fail." << endl;
          return false;
        }
      }
    }

    //add the first duplicate location to the fileLocList
    string remoteLocalPath =
        ci->getRemotePath(row, false, true, false);
    NList rowList = NList(
        NList((int)ci->getInterIndex(row, false, true)),
        columnList,
        NList(remoteLocalPath, true, true));
    resultList->appendFileLocList(rowList);

    mit++;
  }

  resultList->setDistributed();
  done = true;
  return true;
}

size_t SpreadLocalInfo::hashValue(
    Tuple *t, int attrIndex, int scale){
  size_t hashValue =
      ((Attribute*)t->GetAttribute(attrIndex))->HashValue();

  if (scale > 0){
    hashValue %= scale;
  }

  return hashValue;
}


Operator spreadOp(SpreadInfo(), SpreadValueMap, SpreadTypeMap);

/*
5 Operator ~spreadFiles~

This operator reads a set of sub-files, and spread them to slave Data Servers.
It is prepared to cooperate with the operators like ~divide\_osm~,
spreading a set of divided files to slave Data Servers.
Its signature is:

----
FileName: string x Path: text
x [Size: int]
  -> bool
----

It detects the files through the FileName and the Path argument.
All files must be named like: FileName\_No.
The ~No~ is an integer, starting from 0,
By default an empty path indicates the \$SECONDO\_BUILD\_DIR\/bin,
so that the implementers of the operators like ~divide\_osm~ don't need to
consider about the special path setting in Parallel SECONDO.
The Size is an optional argument, by default it is the cluster size,
i.e. the number of slave Data Servers.

*/
struct SpreadFilesInfo : OperatorInfo {

  SpreadFilesInfo() : OperatorInfo()
  {
    name = "spreadFiles";
    signature = "string x text x [int] -> bool";
    syntax = "FileName x Path x [Size] -> Success";
    meaning = "This operator reads a set of sub-files, "
        "and spread them to slave Data Servers.";
  }
};

/*
5.1 Type Mapping

This operator returns a boolean result,
which is true if all sub-files are copied to their respective target.

*/
ListExpr SpreadFilesTypeMap(ListExpr args){

  NList l(args);
  string err[] = {
      //0
      "ERROR!! Operator expects two or three arguments.",
      "ERROR!! Operator expects string x text x [int] as the input",
      ""
  };

  if (l.length() < 2 || l.length() > 3){
    return l.typeError(err[0]);
  }

  NList pType, pValue;

  //FileName
  if (!l.first().first().isSymbol(CcString::BasicType()))
    return l.typeError(err[1]);

  //Path
  if (!l.second().first().isSymbol(FText::BasicType()))
    return l.typeError(err[1]);

  //[Size]
  if (l.length() == 3){
    if (!l.third().first().isSymbol(CcInt::BasicType()))
      return l.typeError(err[1]);
  }

  return NList(NList(CcBool::BasicType())).listExpr();
}

/*
5.2 Value Mapping

Some checking are done in the value mappting stage,
also only give the warning message.
If the given Size is different from the current cluster size,
then a warning message will be prompted.
If the file postfix doesn't coherence, several files are missing,
the user will also be warned.
If the Size is larger than the cluster size,
then the operator stops at the last file within the cluster range.


Each sub-file is renamed after being copied to the destination Data Server,
by removing its numerical postfix.
Therefore, all sub-files in the Data Servers share the same file.
Files are all copied to Mini-Secondo\/bin directory,
for the same reason that uses \$SECONDO\_BUILD\_DIR\/bin as the default path.


*/
int SpreadFilesValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  result = qp->ResultStorage(s);
  string fileName, filePath;
  size_t  size;

  fileName = ((CcString*)args[0].addr)->GetValue();
  filePath = ((FText*)args[1].addr)->GetValue();
  clusterInfo* ci = new clusterInfo();
  if (qp->GetNoSons(s) == 3){
    size = (size_t)((CcInt*)args[2].addr)->GetValue();
  } else {
    size = (size_t)ci->getSlaveSize();
  }

  if (size > ci->getSlaveSize()){
    cerr << "Warning!! The Size parameter is larger than "
        "the current cluster size. " << endl;
    size = ci->getSlaveSize();
    cerr << "Only " << size << " files will be spread. " << endl;
  }
  else if (size < ci->getSlaveSize()){
    cerr << "Warning!! The Size parameter is less than "
        "the current cluster size. " << endl;
  }

  if (fileName.length() == 0){
    cerr << "Error!! The file name cannot be set empty. " << endl;
    ((CcBool*)(result.addr))->Set(true, false);
  }

  if (filePath.length() == 0)
    filePath = FileSystem::GetCurrentFolder();
  else if (filePath.find_first_of("/") != 0){
    cerr << "Error!! An absolute path is required for the Path argument."
        << endl;
    ((CcBool*)(result.addr))->Set(true, false);
    return 0;
  }

  const int PipeWidth = 10;
  pthread_t threadID[PipeWidth];
  SPF_LocalInfo *sli = new SPF_LocalInfo();
  pthread_mutex_init(&CLI_mutex, NULL);

  //Each thread transfer one file, and mark the result
  SPF_Thread* sts[size];
  for (int fi = 0; (size_t)fi < size;)
  {
    string file = filePath + "/" + fileName + "_" + int2string(fi);

    if ((size_t)fi >= size)
      break;

    //Files should be distributed on slaves only
    string rmsBin = ci->getMSECPath(fi + 1, true, false);
    FileSystem::AppendItem(rmsBin, "bin/"+fileName);

    for (int ti = 0; ti < PipeWidth; ti++)
    {
      if (!sli->getTokenPass(ti) || pthread_kill(threadID[ti], 0))
      {
        sli->setTokenPass(ti, true);
        sts[fi] = new SPF_Thread(sli, ti, fi, file, rmsBin);
        pthread_create(&threadID[ti], NULL, SPF_Thread::tCopyFile, sts[fi]);
        fi++;
        break;
      }
    }
  }

  for (int ti = 0; ti < PipeWidth; ti++)
  {
    if (!sli->getTokenPass(ti)){
      pthread_join(threadID[ti], NULL);
    }
  }

  for (int fi = 0; (size_t)fi < size; fi++)
  {
    if (!sts[fi]->getResult()){
      cerr << "Error!! File " << fi << " fails. " << endl;
      ((CcBool*)(result.addr))->Set(true, false);
    }
  }

  ((CcBool*)(result.addr))->Set(true, true);
  return 0;
}

void* SPF_Thread::tCopyFile(void* ptr)
{
  SPF_Thread* st = (SPF_Thread*)ptr;
  string local = st->source;

  if (!FileSystem::FileOrFolderExists(local)){
    pthread_mutex_lock(&CLI_mutex);
    cerr << "Warning!! Cannot locate the file " << local << endl;
    pthread_mutex_unlock(&CLI_mutex);
  } else if (FileSystem::IsDirectory(local)){
    pthread_mutex_lock(&CLI_mutex);
    cerr << "Warning!! File " << local
        << " should not be a directory." << endl;
    pthread_mutex_unlock(&CLI_mutex);
  }

  int copyTimes = MAX_COPYTIMES;
  bool ok = false;
  pthread_mutex_lock(&CLI_mutex);
  pthread_mutex_unlock(&CLI_mutex);
  while (copyTimes-- > 0){
    if (0 == system((scpCommand + local + " " + st->dest).c_str())){
      ok = true;
      break;
    }
  }

  st->setResult(ok);
  pthread_mutex_lock(&CLI_mutex);
  if (!ok){
    cerr << "Error!! Cannot transfer the file " << local << endl;
  } else {
    cerr << "Success!! Send file " << local << " to " << st->dest << endl;
  }
  pthread_mutex_unlock(&CLI_mutex);
  st->releaseToken();
  return NULL;
}

Operator spreadFilesOp(
    SpreadFilesInfo(), SpreadFilesValueMap, SpreadFilesTypeMap);

/*
5 Operator ~collect~

This operator is used to collect the data from the partition files denoted
 by the given flist.

----
flist(tuple) x [row] x [column] -> stream(tuple)
----

*/

struct CollectInfo : OperatorInfo {

  CollectInfo() : OperatorInfo()
  {
    name = "collect";
    signature = "flist(tuple) x [int] x [int] -> stream(tuple)";
    syntax = "flist(tuple) x [row] x [column] -> stream(tuple)";
    meaning = "This operator is used to collect the data "
        "from the partition files denoted by the given flist.";
  }

};

/*

5.1 Type Mapping

First ensure the distributed data in flist is a rel(tuple) type.

----
flist(tuple) x [int] x [int] -> stream(tuple)
----

If only one optional parameter is given, then it's viewed as a row number.
The optional parameters only accept non-negative integer numbers.

Any operators create new flist objects,
cannot be used before the ~collect~ operator,
or else the creation will be done twice since we use the
GetNLArgValueInTM function in query processor,


*/
ListExpr CollectTypeMap(ListExpr args)
{

  NList l(args);
  string err[] = {
      //0
      "ERROR! Operator expects flist x [int] x [int]. ",

      "ERROR! Unavailable optional parameters.",

      "ERROR! Operator expects row and column numbers "
      "are non-negative values.",
  };
  try{
    NList pType, pValue;
  
    if (l.length() != 2)
      return l.typeError(err[0]);
  
    //First flist
    pType = l.first().first();
    pValue = l.first().second();
    if (!isFListStreamDescription(pType)){
      return l.typeError(err[0]);
    }
    NList tupleType = pType.second().second();
  
    //Optional parameters
    pType = l.second().first();
    pValue = l.second().second();
    if (pType.length() > 2){
      return l.typeError(err[0]);
    }
    else if (pType.length() > 0){
      if (!pType.first().isSymbol(CcInt::BasicType())){
        return l.typeError(err[0]);
      }
  
      NList opVal;
      if (!qp->GetNLArgValueInTM(pValue.first(), opVal)){
        return l.typeError(err[1]);
      }else{
        int rowNum = opVal.intval();
        if (rowNum < 0){
          return l.typeError(err[2]);
        }
      }
  
      if (pType.length() > 1){
        if (!pType.first().isSymbol(CcInt::BasicType())){
          return l.typeError(err[0]);
        }
        if (!qp->GetNLArgValueInTM(pValue.second(), opVal)){
          return l.typeError(err[1]);
        }else{
          int columnNum = opVal.intval();
          if (columnNum < 0){
            return l.typeError(err[2]);
          }
        }
      }
    }
  
    NList streamType =
        NList(NList(Symbol::STREAM()),
                NList(tupleType));
  
    return streamType.listExpr();
  } catch(...){
      return listutils::typeError(err[0]);
  }
}

/*

5.2 Value Mapping

By default, it reads all partition files denoted in the given flist, and 
returns the tuples in a stream.
If the optional parameters are given, then this operator only reads 
part partition files.
For any partition files listed a flist, both it's row and column numbers are
 non-zero positive integer numbers.
If the row number is 0, then it denotes a complete row partition files, while
 a complete column part files are denoted when the column number is 0. E.g,

  * collect[1] or collect[1,0]  means to collect all partitions files 
    in the first row,

  * collect[0,2] means to collect all partition files in the second column,

  * collect[1,2] means to collect one partition file in the 1th row 
    and 2th column,

  * collect[0,0] means to collect all files inside the matrix.


*/
int CollectValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){
  CollectLocalInfo* cli = 0;

  switch(message){
    case OPEN: {

      fList* partitionFileList = (fList*)args[0].addr;

      NList currentCluster = clusterInfo().toNestedList();
      if (!partitionFileList->isCollectable(currentCluster)){
        cerr << "This flist is not collectable" << endl;
        return CANCEL;
      }

      size_t row = 0, column = 0;
      Supplier optList = args[1].addr;
      int optLen = qp->GetNoSons(optList);
      if (1 == optLen){
        row = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,0)).addr)->GetValue();
      }else if (2 == optLen){
        row = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,0)).addr)->GetValue();
        column = ((CcInt*)
          qp->Request(qp->GetSupplier(optList,1)).addr)->GetValue();
      }

      cli = (CollectLocalInfo*)local.addr;
      if (cli){
        delete cli;
        cli = 0;
      }
      cli = new CollectLocalInfo(partitionFileList, row, column);


#ifdef SEQCOPY
      if (cli->fetchAllPartFiles()){
        local.setAddr(cli);
      }
      else{
        delete cli;
        cli = 0;
        local.setAddr(Address(0));
      }
#endif

      local.setAddr(cli);
      return 0;
    }
    case REQUEST: {
      if (0 == local.addr)
        return CANCEL;

      cli = (CollectLocalInfo*)local.addr;

#ifdef PIPECOPY
      result.setAddr(cli->getNextTuple2());
#else
      result.setAddr(cli->getNextTuple());
#endif
      if ( 0 == result.addr)
      {
        return CANCEL;
      }
      else
      {
        return YIELD;
      }
    }
    case CLOSE: {
      cli = (CollectLocalInfo*)local.addr;
      if (!cli)
        return CANCEL;
      else{
        delete cli;
        cli = 0;
        local.setAddr(Address(0));
      }
      return 0;  //must return
    }
  }


  return 0;
}

#ifdef PIPECOPY
pthread_mutex_t  CollectLocalInfo::CLI_OWN_Mutex;
#endif

CollectLocalInfo::CollectLocalInfo(fList* _fl, size_t _r, size_t _c):
    fileList(_fl), row(_r), column(_c), inputFile(0)
{
  NList typeList = fileList->getInterTypeList();

  //Get the resultType
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ListExpr resultTypeList =
      sc->NumericType(typeList.second().listExpr());
  resultType = new TupleType(resultTypeList);

#ifdef PIPECOPY
  //Assign the tasks here.
  size_t rBegin = ( row == 0 ) ? 1 : row,
         rEnd   = ( row == 0 ) ? fileList->getMtxRowNum() : row;
  fileStatus = new bool[(rEnd - rBegin + 1)];
  memset(tokenPass, false, PipeWidth);
  pthread_mutex_init(&CLI_OWN_Mutex,NULL);

  for (size_t ri = rBegin; ri <= rEnd; ri++)
  {
    ListExpr columnList = fileList->getColumnList(ri);
    CLI_Thread* ct = new CLI_Thread(this, fileList->getSubName(),
        ri, columnList);
    int cand = fileList->getPartitionFileLoc(ri, *ct->remotePaths);

    if (cand > 0)
    {
      if (column > 0)
      {
        //Read one column only
        ListExpr rest = columnList;
        bool findColumn = false;
        while(!nl->IsEmpty(rest))
        {
          int elem = nl->IntValue(nl->First(rest));
          if ((int)column == elem){
            findColumn = true;
            break;
          }
          rest = nl->Rest(rest);
        }
        if (findColumn){
          ct->allColumns->push_back((int)column);
        }
        else{
          cerr << "The partition file (" << ri << "," << column << ") "
              "is unavailable. " << endl;
          fileStatus[ri - 1] = false;
          break;
        }
      }
      else
      {
        //Read all columns
        ListExpr rest = columnList;
        while(!nl->IsEmpty(rest))
        {
          ct->allColumns->push_back(nl->IntValue(nl->First(rest)));
          rest = nl->Rest(rest);
        }
      }
      fileTasks.push_back(ct);
    }
    else
    {
      delete ct;
    }
  }

  pthread_create(&faf_TID, NULL, fetchAllPartFiles2, this);
  fIdx = 0;
#endif
}

#ifdef SEQCOPY
bool CollectLocalInfo::fetchAllPartFiles()
{
  //According to the accepted row and column number,
  //fetch data files and fullfill their names to the partFiles vector

  size_t rBegin = ( row == 0 ) ? 1 : row,
         rEnd   = ( row == 0 ) ? fileList->getMtxRowNum() : row;

  clusterInfo* cluster = new clusterInfo();
  string localIP = cluster->getLocalIP();

  for ( size_t ri = rBegin; ri <= rEnd; ri++)
  {
    //Files of a same row are always kept together
    vector<string> remotePaths;
    size_t cand =
        fileList->getPartitionFileLoc(ri, remotePaths);
    if (cand > 0)
    {
      NList newColList = fileList->getColumnList(ri);
      if (column != 0)
      {
        //find one file of each row
        //check the column number is exist in the column list
        NList rest = newColList;
        bool find = false;
        while (!rest.isEmpty()){
          int elem = rest.first().intval();
          if ( (int)column == elem){
            find = true;
            break;
          }
          rest.rest();
        }
        if (find){
          newColList = NList();
          newColList.append(NList((int)column));
        }
        else{
          cerr << "The partition ("
              << ri << "," << column <<") is unavailable. " << endl;
          return false;
        }
      }

      NList rest = newColList;
      while (!rest.isEmpty()){
        size_t ci = rest.first().intval();
        string fileName = fileList->getSubName()
            + "_" + int2string(ri) + "_" + int2string(ci);
        string localPath = getLocalFilePath("", fileName, "", true);
        vector<string>::iterator pit = remotePaths.begin();
        while (pit != remotePaths.end()){
          string rPath = *pit;
          FileSystem::AppendItem(rPath, fileName);
          string remoteIP, remoteLocalPath;
          istringstream iss(rPath);
          getline(iss, remoteIP, ':');
          getline(iss, remoteLocalPath, ':');

          bool found = false;
          if (remoteIP.compare(localIP) == 0)
          {
            localPath = remoteLocalPath;
            found = true;
          }
          else
          {
            int copyTime = MAX_COPYTIMES;
            while (copyTime-- > 0){
              if (0 != system(
                  (scpCommand + rPath + " " + localPath).c_str()))
              {
                cerr << "Warning! Copy remote file from " << rPath
                    << " doesn't work yet." << endl;
              }else
              {
                break;
              }
            }

            if (copyTime <= 0){
              cerr << "ERROR! Copy remote file from " << rPath
                  << " fails" << endl;
              return false;
            }
            else{
              found = true;
            }
          }

          if (found)
          {
            partFiles.push_back(localPath);
            break;
          }
          pit++;
        }
        rest.rest();
      }
    }
  }
  return true;
}
#endif

#ifdef PIPECOPY
/*
With pipeline copy method, one thread starts when the valueMapping function
starts, to fetch all files over the cluster.
In this thread, it starts one independent thread for each row of the matrix,
each file is listed into ~partFiles~ when it is prepared.
When the first file in the ~partFiles~ is set, the operator starts to
read the tuples from it.

However, as the nestedList class in Secondo is not safe for thread processing,
all preparation work for each row have to be done inside the main process,
instead of done by every thread.

*/
void* CollectLocalInfo::fetchAllPartFiles2(void* ptr)
{
  CollectLocalInfo* cli = (CollectLocalInfo*)ptr;
  size_t row = cli->getRow();

  size_t rBegin = ( row == 0 ) ? 1 : row,
         rEnd   = ( row == 0 ) ? cli->getFileList()->getMtxRowNum() : row;

  pthread_t threadID[PipeWidth];

  for (vector<CLI_Thread*>::iterator task = cli->fileTasks.begin();
        task != cli->fileTasks.end();)
  {
    //For each task(row), start an independent thread to copy it from remote
    //computer, if it does not exist in the present one.
    for (int token = 0; token < PipeWidth; token++)
    {
      if (task == cli->fileTasks.end())
        break;

      if (!cli->tokenPass[token] || pthread_kill(threadID[token], 0))
      {
        cli->tokenPass[token] = true;
        (*task)->setToken(token);
        pthread_create(&threadID[token], NULL, tCopyFile, (*task));
        task++;
      }
    }
  }

  for (int token = 0; token < PipeWidth; token++)
  {
    //Wait until all copy threads finishes
    if (cli->tokenPass[token]){
      pthread_join(threadID[token], NULL);
    }
  }

  for (size_t ri = 0; ri <(rEnd - rBegin + 1); ri++){
    if (!cli->fileStatus[ri]){
      //It is possible that some rows are empty,
      //hence only the warning message instead of error message is given.
      cerr << "Warning! Row " << (rBegin + ri) << " is not prepared" << endl;
    }
  }

  return NULL;
}

void* CollectLocalInfo::tCopyFile(void* ptr)
{

  //Copy thread function

  CLI_Thread *ct = (CLI_Thread*)ptr;
  int token = ct->token;
  CollectLocalInfo* li = ct->cli;
  vector<string>* remotePaths = ct->remotePaths;
  int ri = ct->row;
  string localIP = clusterInfo().getLocalIP();

  string subName = ct->subName;
  vector<int>* allColumns = ct->allColumns;

  //Copy files of the current row, either copy one column or the whole row
  string fileName = subName + "_" + int2string(ri)
      + "_" + (li->column > 0 ? int2string(li->column) : "*");
  //Get only the file path, without attaching any file name after it.
  string localPath = getLocalFilePath("", "", "", false);
  vector<string>::iterator pit = remotePaths->begin();
  while (pit != remotePaths->end())
  {
    string rPath = *pit;
    string remoteIP, remoteLocalPath;
    istringstream iss(rPath);
    getline(iss, remoteIP, ':');
    getline(iss, remoteLocalPath, ':');

    bool getFile = false;
    if (remoteIP.compare(localIP) == 0)
    {
      localPath = remoteLocalPath;
      getFile = true;
    }
    else
    {
      FileSystem::AppendItem(rPath, fileName);

      //Copy from remote
      int copyTime = MAX_COPYTIMES;
      while (copyTime-- > 0){
        if (0 == system((scpCommand + rPath + " " + localPath).c_str())){
          getFile = true;
          break;
        }
      }

      if (!getFile){
        pthread_mutex_lock(&CLI_OWN_Mutex);
        cerr << "ERROR! Copy remote file from " << rPath << " fail." << endl;
        pthread_mutex_unlock(&CLI_OWN_Mutex);
      }
    }

    //Check all file is there
    bool findFile = true;
    if (getFile)
    {
      for (vector<int>::iterator ci = allColumns->begin();
          ci != allColumns->end(); ci++)
      {
        fileName = subName + "_" + int2string(ri)
            + "_" + int2string(*ci);
        string file = localPath;
        FileSystem::AppendItem(file, fileName);

        if (!FileSystem::FileOrFolderExists(file)){
          findFile = false;
          break;
        }
        else{
          pthread_mutex_lock(&CLI_OWN_Mutex);
          li->partFiles.push_back(file);
          pthread_mutex_unlock(&CLI_OWN_Mutex);
        }
      }
    }
    if (findFile){
      pthread_mutex_lock(&CLI_OWN_Mutex);
      li->fileStatus[ri - 1] = true;
      li->tokenPass[token] = false;
      pthread_mutex_unlock(&CLI_OWN_Mutex);
      break;
    }
    pit++;
  }

  return NULL;

}
#endif

#ifdef SEQCOPY
bool CollectLocalInfo::partFileOpened()
{
  if (0 == inputFile){
    if (partFiles.size() != 0){
      string partFileName = partFiles.back();
      //open the file to inputFiles
      inputFile = new ifstream(partFileName.c_str(), ios::binary);
      if (!inputFile->good()){
        inputFile = 0;
        return false;
      }

      //Read the tail description list
      u_int32_t descSize;
      size_t fileLength;
      inputFile->seekg(0, ios::end);
      fileLength = inputFile->tellg();
      inputFile->seekg(
          (fileLength - sizeof(descSize)), ios::beg);
      inputFile->read((char*)&descSize, sizeof(descSize));

      char descStr[descSize];
      inputFile->seekg(
          (fileLength - (descSize + sizeof(descSize))), ios::beg);
      inputFile->read(descStr, descSize);
      inputFile->seekg(0, ios::beg);

      NList descList = NList(binDecode(string(descStr)));
      if (descList.isEmpty())
      {
        cerr << "\nERROR! Tail description list read in "
            << partFileName << "fail." << endl;
        return false;
      }
      return true;
    }
    else{
      return false;  //No more file in the partFiles list
    }
  }
  else{
    return true;  //Exist an opened file
  }

}

Tuple* CollectLocalInfo::getNextTuple()
{
  Tuple* t = 0;

  //Read one tuple from the present opened file
  //if there is no more data in the current file,
  //then remove the file from the partFiles list,
  //and open the new one.
  while (partFileOpened()){
    u_int32_t blockSize = 0;
    inputFile->read( reinterpret_cast<char*>(&blockSize),
        sizeof(blockSize));
    if (blockSize > 0)
    {
      //read and return the tuple
      blockSize -= sizeof(blockSize);
      char *tupleBlock = new char[blockSize];
      inputFile->read(tupleBlock, blockSize);

      t = new Tuple(resultType);
      t->ReadFromBin(tupleBlock, blockSize);
      delete[] tupleBlock;
      break;
    }
    else
    {
      //close the opened file,
      //pop the file name from the file list.
      inputFile->close();
      delete inputFile;
      inputFile = 0;
      partFiles.pop_back();
    }
  }
  return t;
}
#endif

#ifdef PIPECOPY
Tuple* CollectLocalInfo::getNextTuple2()
{
  Tuple* t = 0;
  if (inputFile != 0)
  {
    t = readTupleFromFile(inputFile, resultType);
    if ( t == 0 ){
      inputFile->close();
      delete inputFile;
      inputFile = 0;
    }
    else
      return t;
  }

  string inputFileName = "";
  while(true)
  {
    if (inputFileName.length() != 0){
      inputFile = new ifstream(inputFileName.c_str(), ios::binary);

      if (!inputFile->good()){
        cerr << "ERROR! Read file " << inputFileName << " fail.\n\n";
        inputFile = 0;
        return 0;
      }

      //Read the description list
      u_int32_t descSize;
      size_t fileLength;
      inputFile->seekg(0, ios::end);
      fileLength = inputFile->tellg();
      inputFile->seekg((fileLength - sizeof(descSize)), ios::beg);
      inputFile->read((char*)&descSize, sizeof(descSize));

      char descStr[descSize];
      inputFile->seekg((fileLength - (descSize + sizeof(descSize))),
          ios::beg);
      inputFile->read(descStr, descSize);
      inputFile->seekg(0, ios::beg);
      NList descList = NList(binDecode(string(descStr)));
      if (descList.isEmpty()){
        cerr << "ERROR! Format error with fetched file "
            << inputFileName << endl;
        inputFile->close();
        delete inputFile;
        inputFile = 0;
        return 0;
      }

      t = readTupleFromFile(inputFile, resultType);
      if ( t == 0 ){
        cerr << "Waring! File " << inputFileName
            << " is empty! " << endl;
        inputFile->close();
        delete inputFile;
        inputFile = 0;
        inputFileName = "";
      }
      else
        return t;
    }
    else if (partFiles.size() > 0 && fIdx < partFiles.size()){
      inputFileName = partFiles[fIdx];
      fIdx++;
    }

    if (pthread_kill(faf_TID, 0)
        && (fIdx >= partFiles.size())
        && inputFileName.length() == 0){
      break;
    }
  }

  return 0;
}
#endif

Operator collectOp(CollectInfo(), CollectValueMap, CollectTypeMap);

/*
4. Type Operator ~PARA~

*/

struct ParaInfo : OperatorInfo
{
  ParaInfo()
  {
    name = "para";
    signature =
        "( flist(T) ) -> T \n"
        "T -> T, T in DELIVERABLE";
    syntax = "type operator";
    meaning = "Extract the data type from a flist object";
  }
};

ListExpr ParaTypeMapping( ListExpr args)
{
  NList l(args);
  string tpeErr = "Eexpect one flist or DELIVERABLE input";

  if (l.length() != 1)
    return l.typeError(tpeErr);

  NList inType;
  if (l.first().isAtom())
  {
    //non flist input
    if (!listutils::isKind(
        l.first().listExpr(), Kind::DELIVERABLE())){
      return l.typeError(tpeErr);
    }
    inType = l.first();
  }
  else
  {
    if (!l.first().first().isSymbol(fList::BasicType()))
      return l.typeError(tpeErr);
    inType = l.first().second();
  }

  return inType.listExpr();
}

int ParaValueMapping(Word* args, Word& result,
    int message, Word& local, Supplier s){

  cerr << "\nOps... It is not allowed to use para operator in "
      "any directly evaluable queries."
      << endl;
  return 0;
}

Operator paraOp(ParaInfo(), ParaValueMapping, ParaTypeMapping);

struct TParaInfo : OperatorInfo
{
  TParaInfo()
  {
    name = "TPARA";
    signature =
        "( flist(ANY) ) -> ANY";
    syntax = "type operator";
    meaning = "Extract the data type from the first flist object";
  }
};

ListExpr TParaTypeMapping( ListExpr args)
{
  try{
    NList l(args);

    if (l.length() < 1){
      return l.typeError("Expect at least one argument.");
    }
    NList ffListType = l.first();

    if (ffListType.isAtom()){
      return ffListType.listExpr();
    }
    else
    {
      if (ffListType.first().isSymbol(fList::BasicType())){
        return ffListType.second().listExpr();
      }
      else{
        return ffListType.listExpr();
      }
    }
  } catch(...){
     return listutils::typeError("invalid input");
  }

}


struct TPara2Info : OperatorInfo
{
  TPara2Info()
  {
    name = "TPARA2";
    signature =
        "( ANY x flist(ANY) ) -> ANY";
    syntax = "type operator";
    meaning = "Extract the data type from the second flist object";
  }
};

ListExpr TPara2TypeMapping( ListExpr args)
{
  try{
    NList l(args);

    if (l.length() < 2)
      return l.typeError("Expect at least two arguments.");
    NList sfListType = l.second();

    if (sfListType.isAtom()){
      return sfListType.listExpr();
    }
    else
    {
      if (sfListType.first().isSymbol(fList::BasicType())){
        return sfListType.second().listExpr();
      }
      else{
        return sfListType.listExpr();
      }
    }
  } catch(...){
    return listutils::typeError("invalid input");
  }

}

/*
3 Operator ~pffeed~

Created on 14th Jul, 2012
Jiamin Lu

This operator reads a set of data files from the cluster,
and returns the tuples from them. Files are copied to the current
node by pipeline. It maps

----
stream(tuple(aR:int, aC:int, aD:int, ...))
x aR x aC x aD
x fileName x [filePath] x [attemptTimes]
x [T1] x [T2]
\to stream(tuple())
----

The input tuple stream contains at least 3 attributes:

  * aR: The files must be contained inside a file-matrix,
and this attribute tells their row numbers.

  * aC: It tells the files' column numbers.

  * aD: It tells the files' first destination data server ID in the DS-catalog.

The following 3 symbols tells the names of above 3 attributes.
Also the fileName of the data files are given as the fourth parameter,
following with the optional file path.
All files must start with an identical fileName, and kept in the same
file path if the filePath parameter is set.
By default, they are kept in the pathes described in the DS-catalog.
All files can be copied from attemptTimes data servers,
if the copy fails in its first destination. The default value of the
attemptTimes is 1.

If the type file is not found in the current node, it can be fetched
from T1 or T2 nodes, if these two optional parameters are set.


*/

struct pffeedInfo : OperatorInfo
{
  pffeedInfo()
  {
    name = "pffeed";
    signature =
        "stream(tuple(aR:int, aC:int, aD:int, ...)) "
        "x aR x aC x aD x string x [text] x [int] "
        "x [int] x [int] -> stream(tuple())";
    meaning = "Reads a set of data files from the cluster,"
        "and returns the tuples from them.";
  }
};

ListExpr pffeedTypeMap(ListExpr args)
{
  try{
     NList l(args);
   
     string lenErr = "ERROR! Operator pffeed expects 3 argument lists. ";
     string inSTErr = "ERROR! Operator pffeed expects a "
         "stream(tuple()) as input";
     string firstPLErr = "ERROR! Operator pffeed expects first list as "
         "aR x aC x aD x fileName x [filePath] x [attemptTimes]";
     string rcdErr = "ERROR! aR, aC and aD do not exist or "
         "are not int attributes.";
     string secondPLErr = "ERROR! Operator pffeed expects second list as"
         " [int] x [int]";
     string atrErr = "ERROR! Required attribute is not found in the "
         "given tuple stream";
     string tpeErr = "ERROR! The type file is not found.";
     string ifeErr = "ERROR! Parameter cannot be evaluated: ";
     string ntfErr = "ERROR! No exist type file: ";
     string ntrErr = "ERROR! A tuple relation type list is "
           "NOT contained in file: ";
     string nslErr = "ERROR! The slave list file does not exist."
         "Is $PARALLEL_SECONDO_SLAVES correctly set up ?";
     string norErr = "ERROR! Remote node for type file is out of range.";
     string ctfErr = "ERROR! Copy remote type file fail.";
     string iatErr = "ERROR! Infeasible attempt times.";
   
     string fileName, filePath;
     NList pType, pValue;
     int attTimes = 1;
   
     if (l.length() != 3)
       return l.typeError(lenErr);
   
     //1th stream(tuple())
     pType = l.first().first();
     NList attrsList;
     if (!pType.checkStreamTuple(attrsList))
       return l.typeError(inSTErr);
   
     //2th aR x aC x aD x fileName x [filePath] x [attemptTimes]
     pType = l.second().first();
     pValue = l.second().second();
     string attrNam[3];
     int attrPos[3];
     for (int i = 1; i <= 3; i++)
     {
       if (!pType.elem(i).isSymbol())
         return l.typeError(firstPLErr);
       attrNam[(i-1)] = pType.elem(i).convertToString();
       ListExpr attrType;
       attrPos[(i-1)] = listutils::findAttribute(attrsList.listExpr(),
           attrNam[(i-1)], attrType);
       if (attrPos[(i-1)] < 1)
         return l.typeError(rcdErr);
       if (!NList(attrType).isSymbol(CcInt::BasicType()))
         return l.typeError(rcdErr);
     }
     if (!pType.fourth().isSymbol(CcString::BasicType()))
       return l.typeError(firstPLErr);
     NList fnList;
     if (!QueryProcessor::GetNLArgValueInTM(pValue.fourth(), fnList))
       return l.typeError(ifeErr + "fileName");
     fileName = fnList.str();
     // text, int, or text x int
     Cardinal opIdx = 5;
     while (opIdx <= pType.length())
     {
       if (pType.elem(opIdx).isSymbol(FText::BasicType()))
       {
         NList fpList;
         if (!QueryProcessor::GetNLArgValueInTM(
               pValue.elem(opIdx), fpList))
           return l.typeError(ifeErr + "filePath");
         filePath = fpList.str();
       }
       else if (pType.elem(opIdx).isSymbol(CcInt::BasicType()))
       {
         NList atList;
         if (!QueryProcessor::GetNLArgValueInTM(
               pValue.elem(opIdx), atList))
           return l.typeError(ifeErr + "attemptTimes");
         attTimes = atList.intval();
         if (attTimes < 1)
           return l.typeError(iatErr);
       }
       else
         return l.typeError(firstPLErr);
       opIdx++;
     }
     filePath = getLocalFilePath(filePath, fileName, "_type");
   
     //3th [int] x [int]
     pType = l.third().first();
     pValue = l.third().second();
     int typeNode[2] = { -1, -1};
     Cardinal pIdx = 0;
     while ( pIdx < pType.length())
     {
       if (!pType.elem(pIdx + 1).isSymbol(CcInt::BasicType()))
         return l.typeError(secondPLErr);
       NList tnList;
       if (!QueryProcessor::GetNLArgValueInTM(pValue.elem(pIdx + 1),
           tnList))
         return l.typeError(ifeErr + "type node");
       typeNode[pIdx] = tnList.intval();
       pIdx++;
     }
   
     for (int i = 0; i < 2; i++)
     {
       if (typeNode[i] >= 0)
       {
         clusterInfo *ci = new clusterInfo();
         if (!ci->isOK())
           return l.typeError(nslErr);
         if (typeNode[i] > (int)ci->getClusterSize())
         {
           ci->print();
           return l.typeError(norErr);
         }
   
         string rPath = ci->getRemotePath(typeNode[i], true, false, true,
             true, (fileName + "_type"));
         filePath = FileSystem::MakeTemp(filePath);
         cerr << "Copy the type file " << filePath
             << " from <-" << "\t" << rPath << endl;
         if (0 != system((scpCommand + rPath + " " + filePath).c_str()))
           return l.typeError(ctfErr);
         else
           break;
       }
     }
   
     ListExpr relType;
     if (!nl->ReadFromFile(filePath, relType))
       return l.typeError(ntfErr + filePath);
     if (!(listutils::isRelDescription(relType)
         || listutils::isTupleStream(relType)))
         return l.typeError(ntrErr + filePath);
   
     NList streamType =
           NList(NList(Symbol::STREAM()),
           NList(NList(relType).second()));
   
     //Remove the type file name
     filePath = filePath.substr(0, filePath.find_last_of("/"));
   
     return NList(NList(Symbol::APPEND()),
             NList(NList(attrPos[0]),
               NList(attrPos[1]),
               NList(attrPos[2]),
               NList(filePath, true, true),
               NList(attTimes)),
             streamType).listExpr();
  } catch(...){
     return listutils::typeError("invalid input");
  }
}

int pffeedValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s)
{

  PFFeedLocalInfo* pli = 0;

  switch(message)
  {
    case OPEN: {
      //Initialize the local info,
      //start the thread to copy the files
      int rp, cp, dp;
      rp = ((CcInt*)args[3].addr)->GetValue() -1;
      cp = ((CcInt*)args[4].addr)->GetValue() -1;
      dp = ((CcInt*)args[5].addr)->GetValue() -1;

      string fileName, filePath;
      fileName = ((CcString*)qp->Request(
          qp->GetSupplierSon(args[1].addr, 3)).addr)->GetValue();
      filePath = ((FText*)args[6].addr)->GetValue();
      int attTimes = ((CcInt*)args[7].addr)->GetValue();

      pli = (PFFeedLocalInfo*)local.addr;
      if (pli) delete pli;

      pli = new PFFeedLocalInfo(s, args[0],
          rp, cp, dp, fileName, filePath, attTimes);
      local.setAddr(pli);

      return 0;
    }
    case REQUEST: {
      pli = static_cast<PFFeedLocalInfo*>(local.addr);
      if (!pli)
        return CANCEL;

      Tuple* tuple = pli->getNextTuple();
      if (tuple)
      {
        result.setAddr(tuple);
        return YIELD;
      }
      return CANCEL;
    }
    case CLOSE: {
      pli = static_cast<PFFeedLocalInfo*>(local.addr);
      if (pli)
        delete pli;
      local.setAddr(0);
      return 0;
    }
  }
  return 0;
}

Operator pffeedOp(pffeedInfo(), pffeedValueMap, pffeedTypeMap);

PFFeedLocalInfo::PFFeedLocalInfo(Supplier s, Word inputStream,
    int rp, int cp, int dp, string _fn, string _fp, int _at)
  : fileName(_fn), localFilePath(_fp), attTimes(_at)
{
  ListExpr streamTypeList = qp->GetType(s);
  resultType = new TupleType(SecondoSystem::GetCatalog()
    ->NumericType(nl->Second(streamTypeList)));
  interCluster = new clusterInfo();

  PLI_FAF_Thread* ft = new PLI_FAF_Thread(
      this, inputStream, rp, cp, dp);
  pthread_create(&faf_TID, NULL, fetchAllFiles, ft);

  fIdx = 0;
  curFileName = "";
  curFilePt = 0;
}

void* PFFeedLocalInfo::fetchAllFiles(void* ptr)
{
  PLI_FAF_Thread* pft = (PLI_FAF_Thread*)ptr;
  PFFeedLocalInfo* pli = pft->pli;
  Word inputStream = pft->inputStream;
  int rp = pft->attrPos[0],
      cp = pft->attrPos[1],
      dp = pft->attrPos[2];

  pthread_t threadID[PipeWidth];
  memset(pli->tokenPass, false, PipeWidth);
  pthread_mutex_init(&CLI_mutex, NULL);

  Word tupleWord;
  qp->Open(inputStream.addr);
  qp->Request(inputStream.addr, tupleWord);
  while(qp->Received(inputStream.addr))
  {
    Tuple * nextTuple = (Tuple*)tupleWord.addr;
    int row = ((CcInt*)nextTuple->GetAttribute(rp))->GetValue(),
        column = ((CcInt*)nextTuple->GetAttribute(cp))->GetValue(),
        dest = ((CcInt*)nextTuple->GetAttribute(dp))->GetValue();

    //Copy files over the cluster
    for (int token = 0; token < PipeWidth; token++)
    {
      if (!pli->tokenPass[token] || pthread_kill(threadID[token],0))
      {
        pli->tokenPass[token] = true;
        PLI_CF_Thread* ct =
            new PLI_CF_Thread(pli, row, column, dest,token);
        pthread_create(&threadID[token], NULL, tCopyFile, ct);

        nextTuple->DeleteIfAllowed();
        qp->Request(inputStream.addr, tupleWord);
        break;
      }
    }
  }
  qp->Close(inputStream.addr);

  for (int token = 0; token < PipeWidth; token++)
  {
    if (pli->tokenPass[token]){
      pthread_join(threadID[token], NULL);
    }
  }

  return NULL;
}

void* PFFeedLocalInfo::tCopyFile(void* ptr)
{
  PLI_CF_Thread* ct = (PLI_CF_Thread*)ptr;
  PFFeedLocalInfo* pli = (PFFeedLocalInfo*)ct->pli;
  int row   = ct->row,  column  = ct->column,
      dest  = ct->dest, token   = ct->token;

  int localNodeID = pli->interCluster->getLocalNode();
  string localIP = pli->interCluster->getLocalIP();
  int cand = pli->getAttemptTimes();
  string fileName = pli->getFilePrefixName() + "_" + int2string(row)
      + "_" + int2string(column);
  string filePath = pli->getLocalFilePath();
  bool fileFound = false;

  if ( (row < 0) || (column < 0) || (dest < 0))
  {
    pthread_mutex_lock(&CLI_mutex);
    cerr << "Cannot get partition file with triple: "
        << "(" << row << "," << column << "," << dest << ")" << endl;
    pthread_mutex_unlock(&CLI_mutex);
  }

  string lPath = filePath;
  for (int att = 0; att < cand; att++)
  {
    //Copy the file from one node,
    //traverse the next one if the copy fails
    int nodeID = dest + att;
    bool api = (nodeID !=
        getRoundRobinIndex(row, pli->interCluster->getSlaveSize()))
        ? true : false;

    //Attach Producer IP
    string producerIP = pli->interCluster->getIP(row, true);
    string targetIP = pli->interCluster->getIP(nodeID, true);
    //remote and local file path
    string rPath = pli->interCluster->getRemotePath(
        nodeID, true, true, true, true,
        fileName, api, producerIP);
    FileSystem::AppendItem(lPath, fileName);

    if (localIP.compare(targetIP) == 0){
      if ( localNodeID != nodeID )
        lPath = rPath.substr(rPath.find(":") + 1);
    }
    else{
      //Copy file from the remote node
      int copyTimes = MAX_COPYTIMES;
//      pthread_mutex_lock(&CLI_mutex);
//      cerr << "WANT file " << lPath << " from " << rPath << endl;
//      pthread_mutex_unlock(&CLI_mutex);
      while (copyTimes-- > 0){
        if (0 == system((scpCommand + rPath + " " + lPath).c_str())){
          break;
        }
      }
    }

    if (FileSystem::FileOrFolderExists(lPath)){
      fileFound = !FileSystem::IsDirectory(lPath);
    }

    if (fileFound){
      break;
    }
    else{
      pthread_mutex_lock(&CLI_mutex);
      cerr << "Warning! Cannot fetch file at : "
          << lPath  << " from " << rPath << endl;
      pthread_mutex_unlock(&CLI_mutex);
    }
  }

  pthread_mutex_lock(&CLI_mutex);
  if (!fileFound){
    cerr << "ERROR! Get file " << fileName << " from node " << dest
        << " in thread " << token << " fails! " << endl;
  }
  else{
    pli->partFiles.push_back(lPath);
    pli->tokenPass[token] = false;
  }
  pthread_mutex_unlock(&CLI_mutex);

  return NULL;
}

Tuple* PFFeedLocalInfo::getNextTuple()
{
  Tuple* t = 0;

  if (curFilePt != 0)
  {
    t = readTupleFromFile(curFilePt, resultType);
    if (t == 0){
      //Read the next file
      curFilePt->close();
      delete curFilePt;
      curFilePt = 0;
      curFileName = "";
    }
    else
      return t;
  }

  while (true)
  {
    if (curFileName.length() != 0){
      curFilePt = new ifstream(curFileName.c_str(), ios::binary);
      if (!curFilePt->good()){
        cerr << "ERROR! Read file " << curFileName << " fail.\n\n";
        curFilePt = 0;
        return 0;
      }

      //Read the description list
      u_int32_t descSize;
      size_t fileLength;
      curFilePt->seekg(0, ios::end);
      fileLength = curFilePt->tellg();
      curFilePt->seekg((fileLength - sizeof(descSize)), ios::beg);
      curFilePt->read((char*)&descSize, sizeof(descSize));

      char descStr[descSize];
      curFilePt->seekg((fileLength - (descSize + sizeof(descSize))),
          ios::beg);
      curFilePt->read(descStr, descSize);
      curFilePt->seekg(0, ios::beg);
      NList descList = NList(binDecode(string(descStr)));
      if (descList.isEmpty()){
        cerr << "ERROR! Format error with fetched file "
            << curFileName << endl;
        curFilePt->close();
        delete curFilePt;
        curFilePt = 0;
        return 0;
      }

      t = readTupleFromFile(curFilePt, resultType);
      if ( t == 0 ){
        cerr << "Waring! File " << curFileName
            << " is empty! " << endl;
        curFilePt->close();
        delete curFilePt;
        curFilePt = 0;
        return 0;
      }
      return t;
    }
    else if (partFiles.size() > 0 && fIdx < partFiles.size()){
      curFileName = partFiles[fIdx];
      fIdx++;
    }

    if (pthread_kill(faf_TID, 0)
        && (fIdx >= partFiles.size())
        && curFileName.length() == 0){
      break;
    }
  }

  return 0;
}


/*
4 Operator ~hadoopMap~

Create DGO DLO kind flist after being queried
within the map step of the embedded Hadoop job.

Update at 22th Mar. 2012
Jiamin

Remove the DGO kind, but it can create either DLO or DLF kind flist.
The operator name is changed to ~hadoopMap~,
which means it creates a new flist only through the map step of
the preset Hadoop job.

The map step in MR works similar as a ~filter~ operation,
hence this operator only accepts one flist as the input,
but in its internal function, several other flists can be used too.

It is not necessary to indicate name to created sub-objects or sub-files,
which can be set by the system.
However, for DLF kind flist, it is better to keep the tridition.
Therefore, file name and file path is prepared as optional argument.
Also, an optional symbol is prepared, used to indicate the kind of the
 result flist.
By default, it is DLO, but also can be set as DLF.
I didn't use an integer or a bool for this value,
since it is possible for us to create more kinds for the flist.

*/
struct hadoopMapInfo : OperatorInfo
{
  hadoopMapInfo()
  {
    name = "hadoopMap";
    signature =
        "flist(T) x [string] x [text] x [(DLO):DLF] x [bool] "
        "x ( map T T1 ) -> flist(T1)";
    meaning = "Create DLO or DLF kind flist after the map step";
  }
};

/*
4.1 Type Mapping of ~hadoopMap~

This operator maps

----
T x string -> flist
----

The T is the data created by the prepositive query.
If this query is only an exist Secondo object name,
then this operator creates a DGO flist data.
Or else if the query contains at least one ~para~ operator,
then this operator creates a DLO flist data,
and creates local Secondo objects in slave databases through
delivering the prepositive query by a generic Hadoop job.

The string is the object name created on every slave database,
and it must be start with a capital character.
If the object name is given as an empty string,
then it creates a DGO kind flist by default.


Update at 21th Mar. 2012
Jiamin

Now this operator maps

----
flist(T) x [string] x [text] x [(DLO):DLF]
  x ( map ( T -> T1) ) -> flist(T1)
----

Update at 14th May. 2012 by Jiamin

Adds the optional parameter mapTaskNum, now the operator maps

----
flist(T) x [string] x [text] x [(DLO):DLF] x [mapTaskNum]
  x ( map ( T -> T1) ) -> flist(T1)
----

If the produced flist kind is DLO, then the mapTaskNum must be less than
the scale of slaves, as each slave database can only keep one sub-object.

Update at 21th Aug. 2012 by Jiamin

Adds an optional parameter executed:bool, to indicate whether the
Hadoop job practically process. If it is false, then this operator creates
an intermediate flist object. An intermediate flist contains an umempty
UEMapQuery (UnExecuted Map Query), describing the process in the Map stage.
It is composed by four elements(MapQuery DLO\_List DLF\_List mapTaskNum),
both hadoopReduce and hadoopReduce2 operators set their Map tasks based on
this UEMapQuery.

By default this value is true. Now this operator maps

----
flist(T) x [string] x [text] x [(DLO):DLF] x [int] x [bool]
  x ( map ( T -> T1) ) -> flist(T1)
----


*/

ListExpr hadoopMapTypeMap(ListExpr args){
  try{
     NList l(args);
   
     string lenErr = "ERROR! Operator hadoopMap expects 3 argument lists. ";
     string typErr = "ERROR! Operator hadoopMap expects "
         "flist(T) x [string] x [text] x [(DLO):DLF] x [int] x [bool] "
         "x (map T T1))";
   
     string ifaErr = "ERROR! Infeasible evaluation in TM of attribute:";
     string mtnErr = "ERROR! Expect a positive map task number. ";
     string umnErr = "ERROR! It is not allowed to produce a DLO kind flist, "
         "with a row number that is larger than the slave scale.";
     string nprErr = "ERROR! Operator hadoopMap expects "
         "creating a new DLO or DLF kind flist.";
     string onmErr = "ERROR! Operator hadoopMap expects the created "
         "object name starts with upper case. ";
     string uafErr = "ERROR!! The internal function is unavailable.";
     string hnmErr = "ERROR! Exists homonymous flist type file in: ";
     string fwtErr = "ERROR! Failed writing type into file: ";
     string expErr = "ERROR! Improper output type for DLF flist";
     string udnErr = "ERROR! Long database name is set.";
     string uewErr = "ERROR! An unexecuted flist must be DLF type.";
   
     string objName, filePath, qStr;
     fListKind kind = DLO;
     bool executed = true;
   
     if (l.length() != 3)
       return l.typeError(lenErr);
   
     //The input must be a flist type data.
     NList inputType = l.first().first();
     if (inputType.isSymbol("typeerror") || inputType.length() != 2 ){
       return l.typeError(typErr);
     }
   
     if (!inputType.first().isSymbol(fList::BasicType())){
       return l.typeError(typErr);
     }
   
     //Optional parameters
     int len = l.second().first().length();
     clusterInfo ci;
     size_t mapTaskNum = ci.getSlaveSize();
     if (len > 0)
     {
       if ( len > 3 )
         return l.typeError(typErr);
   
       for (int i = 1; i <= len; i++)
       {
         NList pp = l.second().first().elem(i);
         NList pv = l.second().second().elem(i);
   
         if (pp.isSymbol(CcString::BasicType()))
         {
           //ObjectName defined.
           NList fnList;
           if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
             return l.typeError(ifaErr + "objName");
           }
           objName = fnList.str();
         }
         else if (pp.isSymbol(FText::BasicType()))
         {
           //FilePath defined
           NList fpList;
           if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
             return l.typeError(ifaErr + "filePath");
           }
           filePath = fpList.str();
         }
         else if (pp.isSymbol(CcInt::BasicType()))
         {
           //mapTaskNum
           NList mtnList;
           if (!QueryProcessor::GetNLArgValueInTM(pv, mtnList)){
             return l.typeError(ifaErr + "mapTaskNum");
           }
           mapTaskNum = mtnList.intval();
         }
         else if (pp.isSymbol("DLF")){
           kind = DLF;
         }
         else if (pp.isSymbol("DLO")){
           kind = DLO;
         }
         else if (pp.isSymbol(CcBool::BasicType())){
           NList etList;
           if (!QueryProcessor::GetNLArgValueInTM(pv, etList)){
             return l.typeError(ifaErr + "executed");
           }
           executed = etList.boolval();
         }
         else{
           return l.typeError(typErr);
         }
       }
     }
   
     if (!executed && (kind != DLF)){
       return l.typeError(uewErr);
     }
   
     if (mapTaskNum <= 0){
       return l.typeError(mtnErr);
     } else if ((kind == DLO) && (mapTaskNum > ci.getSlaveSize())){
       return l.typeError(umnErr);
     }
   
   
     //Encapsulated object type
     NList coType = l.third().first().third();   //Create Type
     NList coQuery = l.third().second().third(); //Create Query
     NList coBType;                              //Create Base Type
     if (coType.isAtom()){
       coBType = coType;
     }
     else{
       coBType = coType.first();
     }
     if (coBType.isEqual("typeerror"))
       return l.typeError(uafErr);
     qStr = coQuery.convertToString();
   
     //Query Parameter
     string coParaName = l.third().second().second().first().str();
   
     //If it is a DLF, then the output type must be a tuple stream
     if (kind == DLF && !listutils::isTupleStream(coType.listExpr())){
       return l.typeError(expErr);
     }
   
     NList resultType = NList(NList(fList::BasicType()), NList(coType));
     if (objName.length() == 0)
       objName = fList::tempName(false);
     else
     {
       char f = objName[0];
       if (f<'A' || f>'Z'){
         return l.typeError(onmErr);
       }
       //If the name of sub-objects or sub-files are denoted by users,
       //Then it must be kept in a text type file,
       //in case of the homonymous problem
       //also this file must be kept in the default file path.
   
       string filePath =
           getLocalFilePath("", (objName + "_type"),"", true);
       if (FileSystem::FileOrFolderExists(filePath)){
         ListExpr exeType;
         bool ok = false;
         if (nl->ReadFromFile(filePath, exeType)){
           //TODO Need to be more compatible with file-related operators
             if (nl->Equal(nl->Second(exeType),
                 resultType.second().second().listExpr())){
               ok = true;
             }
         }
         if (!ok)
           return l.typeError(hnmErr + filePath);
       }
       else{
         ListExpr expList = nl->Second(resultType.listExpr());
         if (nl->IsAtom(expList)){
           expList = nl->OneElemList(expList);
         }
         if (!nl->WriteToFile(filePath, expList)){
           return l.typeError(fwtErr + filePath);
         }
       }
     }
   
     //Check the length of the database name
     string dbName = fList::tempName(true);
     if (dbName.length() >= 16){
       //16 is the maximum length that a database name can be
       return l.typeError(udnErr);
     }
   
   
     return NList(NList(Symbol::APPEND()),
         NList(NList(qStr, true, true),
               NList(objName, true, false),
               NList((int)kind),
               NList(coParaName, true, false),
               NList(dbName, true, false)),
         resultType).listExpr();
  } catch(...){
    return listutils::typeError("invalid input");
  }
}

int hadoopMapValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  if ( message <= CLOSE )
  {
    //Appended parameters
    //Query string
    string CreateQuery =
        ((FText*)args[3].addr)->GetValue();
    //Get the object name.
    string CreateObjectName =
        ((CcString*)args[4].addr)->GetValue();
    //Get the export file type
    fListKind kind =
        (fListKind)((CcInt*)args[5].addr)->GetValue();
    //Get the object name of the input flist
    string inParaName =
        ((CcString*)args[6].addr)->GetValue();
    //Get the database name
    string dbName =
        ((CcString*)args[7].addr)->GetValue();
    //Optional parameters
    string CreateFilePath = "";
    Supplier oppList = args[1].addr;
    int opLen = qp->GetNoSons(oppList);
    int mapTaskNum = -1;
    bool executed = true;
    for (int i = 0; i < opLen; i++)
    {
      ListExpr pp = qp->GetType(qp->GetSupplierSon(oppList,i));
      if (nl->IsEqual(pp, FText::BasicType())){
        CreateFilePath = ((FText*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, CcInt::BasicType())){
        mapTaskNum = ((CcInt*)
          qp->Request(qp->GetSupplier(oppList, i)).addr)->GetValue();
      }
      else if (nl->IsEqual(pp, CcBool::BasicType())){
        executed =
          ((CcBool*)qp->Request(qp->GetSupplier(oppList, i)).addr)->GetValue();
      }
    }

    //Result type
    clusterInfo *ci = new clusterInfo();
    ListExpr resultType = qp->GetType(s);
    fList* resultFList = new fList(CreateObjectName, NList(resultType),
        ci, NList(), 1, false, kind);
    fList* inputFList = (fList*)args[0].addr;
    int dupTimes = inputFList->getDupTimes();
    //Parameters required by the Hadoop job are:
    ListExpr CreateQueryList;
    nl->ReadFromString(CreateQuery, CreateQueryList);
    vector<string> flistParaList;
    vector<fList*> flistObjList;
    flistParaList.push_back(inParaName); //The input parameter
    flistObjList.push_back(inputFList);
    vector<string> DLF_NameList, DLF_fileLocList;
    vector<string> DLO_NameList, DLO_locList;

    //Scan para operations in the internal function list
    //If the ~para~ operator contains a DELIEVERABLE object,
    //then extract its value in nested-list format,
    //and embed inside the query.
    //Or else, replace the para operation with the flist name,
    //then left to the next function.
    bool ok = true;
    CreateQueryList = replaceParaOp(CreateQueryList, flistParaList,
        flistObjList, ok);
    if (!ok){
      cerr << "Replace para operation fails" << endl;
    }
    else{
      CreateQueryList = replaceSecObj(CreateQueryList);
      //Replace parameter value according to their flist value
      for (size_t i = 0; i < flistParaList.size(); i++)
      {
        int argIndex = (i == 0 ? 1 : 0);
        CreateQueryList =
            replaceDLOF(CreateQueryList,
                flistParaList[i], flistObjList[i],
                DLF_NameList, DLF_fileLocList,
                DLO_NameList, DLO_locList, false, ok, argIndex);
        if (!ok){
          cerr << "Replace DLF flist fails" << endl;
          break;
        }
      }
    }

    NList dlfNameList, dlfLocList;
    NList dloNameList, dloLocList;
    ListExpr sidList;
    if (!ok){
      cerr << "Preparing Hadoop job parameters fails." << endl;
    }
    else
    {
      for (size_t i = 0; i < DLF_NameList.size(); i++)
      {
        dlfNameList.append(NList(DLF_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLF_fileLocList[i], locList);
        dlfLocList.append(NList(locList));
      }

      for (size_t i = 0; i < DLO_NameList.size(); i++)
      {
        dloNameList.append(NList(DLO_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLO_locList[i], locList);
        dloLocList.append(NList(locList));
      }

      if (mapTaskNum < 0)
        mapTaskNum = ci->getSlaveSize();

      if (executed)
      {
        //Call the Hadoop job
        stringstream queryStr;
        queryStr
          << "hadoop jar ParallelSecondo.jar "
              "ParallelSecondo.PS_HadoopMap \\\n"
          << dbName << " " << CreateObjectName << " "
          << " \"" << tranStr(nl->ToString(CreateQueryList),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dlfNameList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dlfLocList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dloNameList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(dloLocList.convertToString(),
              "\"", "\\\"") << "\" \\\n"
          << dupTimes  << " " << kind << " "
          << " \"" << CreateFilePath << "\" "
          << mapTaskNum << endl;
        int rtn = -1;
        cout << queryStr.str() << endl;
        rtn = system(queryStr.str().c_str());
        ok = (rtn == 0);

        if (ok)
        {
          FILE *fs;
          char buf[MAX_STRINGSIZE];
          fs = popen("hadoop dfs -cat OUTPUT/part*", "r");
          if (NULL != fs)
          {
            stringstream ss;
            while (!feof(fs) && fgets(buf,sizeof(buf),fs))
            {
                ss << buf;
            }
            string locListStr = ss.str();
            locListStr = locListStr.substr(locListStr.find_first_of(' '));
            nl->ReadFromString(locListStr, sidList);
          }
          else
            ok = false;
        }

        if (ok)
        {
          NList fileLocList;

          if (kind == DLO)
            CreateFilePath = dbLoc;

          //Create file location list
          ListExpr rest = sidList;
          int lastRow = 0;
          NList emptyRow = NList();
          map<int, int> resultMap;
          while(!nl->IsEmpty(rest))
          {
            ListExpr rowInfo = nl->First(rest);
            int rowNum = nl->IntValue(nl->First(rowInfo));
            int slaveIdx = nl->IntValue(nl->Second(rowInfo));
            resultMap.insert(pair<int, int>(rowNum, slaveIdx));
            rest = nl->Rest(rest);
          }

          //Sort according to the row number automatically
          map<int,int>::iterator it;
          for (it = resultMap.begin(); it != resultMap.end(); it++)
          {
            int rowNum = it->first;
            int slaveIdx = it->second;

            //Insert empty rows
            for (lastRow++; lastRow < rowNum; lastRow++)
            {
              if (fileLocList.isEmpty())
              {
                fileLocList.makeHead(emptyRow);
              }
              else
              {
                fileLocList.append(emptyRow);
              }
            }

            //Insert the current row
            NList newRow = NList(NList(slaveIdx), NList(1).enclose(), NList(
                CreateFilePath, true, true));
            if (fileLocList.isEmpty())
            {
              fileLocList.makeHead(newRow);
            }
            else
            {
              fileLocList.append(newRow);
            }
          }
          resultFList = new fList(CreateObjectName, NList(resultType), ci,
              fileLocList, 1, true, kind);
        }
      }
      else
      {//executed == false

        //Create an intermediate flist
        NList UEMapQuery(
           NList(CreateQueryList),
           NList(dlfNameList, dlfLocList),
           NList(dloNameList, dloLocList),
           NList(mapTaskNum));

        resultFList = new fList(CreateObjectName, NList(resultType),
          ci, NList(), 1, false, kind, 0, 0, UEMapQuery);
      }
    }

    result.setAddr(resultFList);
  }
  //TODO add the progress estimation in the future
  //  else if ( message == REQUESTPROGRESS )
  //  else if ( message == CLOSEPROGRESS )

    return 0;

}

Operator hadoopMapOP(
    hadoopMapInfo(), hadoopMapValueMap, hadoopMapTypeMap);


/*
6 Operator ~hadoopMapAll~

Create at 10th Jul. 2013
Jiamin

This operator works very similar as the ~hadoopMap~,
except it doesn't ask for an input flist object.
Instead it starts a map task on every slave Data Server,
and process the argument function in it.

It would be quite difficult to re-use the codes in ~hadoopMap~ here,
therefore, it is better to completely create this operator,
although most codes can be copied.

Its signature is:

----
[string] x [text] x [(DLO):DLF] x [int] x [bool]
  x ( map (  -> T1) ) -> flist(T1)
----

*/


struct hadoopMapAllInfo : OperatorInfo
{
  hadoopMapAllInfo()
  {
    name = "hadoopMapAll";
    signature =
        "[string] x [text] x [(DLO):DLF] x [bool] "
        "x ( map T1 ) -> flist(T1)";
    meaning = "Create DLO or DLF kind flist after the map step";
  }
};


/*
5 Operator ~hadoopReduce~

Create at 11th Apr. 2012
Jiamin

As the hadoop operator needs us to change the QueryProcessor, which 
may takes a while.
I started to create another new operator, named ~hadoopReduce~.
This is mainly used to process the reduce step of the precast Hadoop job.

It also creates both DLO and DLF kind flist.
This one provides the unary operation, while the binary operator will be 
implemented later.

Update 22th Aug. 2012 by Jiamin
Make it possible to accept intermediate flist object as the input parameter.
All flist objects used in its argument functions cannot be in intermediate 
status.

*/
struct hadoopReduceInfo : OperatorInfo
{
  hadoopReduceInfo()
  {
    name = "hadoopReduce";
    signature =
        "flist(T) x partAttr x [string] x [text] x [(DLO):DLF] x [int] "
        "x ( map T T1 ) -> flist(T1)";
    meaning = "Create DLO or DLF kind flist after the reduce step";
  }
};


/*
5.1 Type Mapping of ~hadoopReduce~

For unary operation, the mapping is:

----
flist(T) x partAttribute
  x [string] x [text] x [(DLO):DLF] x [int]
  x ( map ( T -> T1) )
\to flist(T1)

T is rel(tuple) or stream(tuple)

----

Comparing with the ~hadoopMap~ operator, an additional argument partAttribute
is required to redistribute the input flist over the cluster.
Therefore, the input flist must be a DLF kind, or a DLO kind contains relation
 type.

The syntax is:

----
flist(T) x partAttribute
  x [objectName: string] x [objectPath: text] x [(DLO):DLF] x
  [reduceTaskNum: int]
  x ( interFunc: map ( T \to T1) )
\to flist(T1)

T is rel(tuple) or stream(tuple)

----

The reduceTaskNum is used to denote the scale of the parallelism.
The number of map tasks is denoted by the input flist.
If the operator creates a DLO kind flist, then the reduceTaskNum 
cannot be larger
than the amount of data servers, since one data server can only keep at most
one sub-object of a DLO flist.
However, if the created flist belong to DLF kind, then the reduceTaskNum can be
set to arbitrary values.


*/

ListExpr hadoopReduceTypeMap(ListExpr args){
  try{
     NList l(args);
   
     string lenErr = "ERROR! Operator hadoopReduce expects 3 argument lists. ";
     string typErr = "ERROR! Operator hadoopReduce expects "
         "flist(T) x partAttr x [string] x [text] x [(DLO):DLF] x [int] "
         "x (map T T1))";
     string ifsErr = "ERROR! Operator hadoopReduce expects the input flist "
         "contains either a tuple stream or a tuple relation. ";
     string upaErr = "ERROR! The partition attribute "
         "is not found in the input flist";
     string ernErr = "ERROR! The reduceTaskNum cannot be larger than the"
                     " cluster scale while producing a DLO flist object.";
     string onmErr = "ERROR! Operator hadoopMap expects the created "
         "object name starts with upper case. ";
     string ifaErr = "ERROR! Infeasible evaluation in TM of attribute:";
     string nprErr = "ERROR! Operator hadoopReduce expects "
         "creating a new DLO or DLF kind flist.";
     string uafErr = "ERROR!! The internal function is unavailable.";
     string expErr = "ERROR! Improper output type for DLF flist";
     string fwtErr = "ERROR! Failed writing type into file: ";
     string hnmErr = "ERROR! Exists homonymous flist type file in: ";
     string udnErr = "ERROR! Long database name is set.";
   
     string objName, filePath, qStr;
     fListKind kind = DLO;
     clusterInfo ci;
     int reduceTaskNum = ci.getSlaveSize();
   
     if (l.length() != 3)
       return l.typeError(lenErr);
   
     //The first flist type data.
     NList inputType = l.first().first();
     if (inputType.isSymbol("typeerror") || inputType.length() != 2 ){
       return l.typeError(typErr);
     }
     if (!inputType.first().isSymbol(fList::BasicType())){
       return l.typeError(typErr);
     }
     if (!listutils::isRelDescription(inputType.second().listExpr()) &&
         !listutils::isTupleStream(inputType.second().listExpr())){
       return l.typeError(ifsErr);
     }
   
   
     //The second list: partAttribute x [string] x [text] x [(DLO):DLF]
     int len = l.second().first().length();
     string PAName = "";
     int PAIndex = 0;
     ListExpr PAType;
     if (len < 1 || len > 5 )
       return l.typeError(typErr);
     else
     {
       //The first argument in this list must be the partition attribute name
       NList pType = l.second().first();
       NList pValue = l.second().second();
       PAName = pType.first().str();
   
       ListExpr AttrList = inputType.second().second().second().listExpr();
       PAIndex = listutils::findAttribute(AttrList, PAName, PAType);
       if (PAIndex <= 0)
         return l.typeError(upaErr);
   
       if (len > 1)
       {
         for (int i = 2; i <= len; i++)
         {
           NList pp = pType.elem(i);
           NList pv = pValue.elem(i);
   
           if (pp.isSymbol(CcString::BasicType()))
           {
             //ObjectName defined.
             NList fnList;
             if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
               return l.typeError(ifaErr + "objName");
             }
             objName = fnList.str();
           }
           else if (pp.isSymbol(FText::BasicType()))
           {
             //FilePath defined
             NList fpList;
             if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
               return l.typeError(ifaErr + "filePath");
             }
             filePath = fpList.str();
           }
           else if (pp.isSymbol(CcInt::BasicType())){
             NList rnList;
             if (!QueryProcessor::GetNLArgValueInTM(pv, rnList)){
               return l.typeError(ifaErr + "objName");
             }
             reduceTaskNum = rnList.intval();
           }
           else if (pp.isSymbol("DLF")){
             kind = DLF;
           }
           else if (!pp.isSymbol("DLO")){
             return l.typeError(nprErr);
           }
         }
       }
     }
     if (kind == DLO && reduceTaskNum > (int)ci.getSlaveSize()){
       return l.typeError(ernErr);
     }
   
     //The third map list
     NList mapType = l.third().first();
     NList mapValue = l.third().second();
     NList coType = mapType.third();             //Create Type
     NList coQuery = mapValue.third();           //Create Query
     NList coBType;                              //Create Base Type
     if (coType.isAtom()){
       coBType = coType;
     }
     else{
       coBType = coType.first();
     }
     if (coBType.isEqual("typeerror"))
       return l.typeError(uafErr);
     qStr = coQuery.convertToString();
   
     //Query Parameter
     string coParaName = mapValue.second().first().str();
   
     //Check the type for output flist
     //If it is a DLF, then the output type must be a tuple stream
     if (kind == DLF && !listutils::isTupleStream(coType.listExpr())){
       return l.typeError(expErr);
     }
   
     NList resultType = NList(NList(fList::BasicType()), NList(coType));
     if (objName.length() == 0)
       objName = fList::tempName(false);
     else
     {
       char f = objName[0];
       if (f<'A' || f>'Z'){
         return l.typeError(onmErr);
       }
       //If the name of sub-objects or sub-files are denoted by users,
       //Then it must be kept in a text type file,
       //in case of the homonymous problem
       //also this file must be kept in the default file path.
   
       string filePath =
           getLocalFilePath("", (objName + "_type"),"", true);
       if (FileSystem::FileOrFolderExists(filePath)){
         ListExpr exeType;
         bool ok = false;
         if (nl->ReadFromFile(filePath, exeType)){
           //TODO Need to be more compatible with file-related operators
             if (nl->Equal(nl->Second(exeType),
                 resultType.second().second().listExpr())){
               ok = true;
             }
         }
         if (!ok)
           return l.typeError(hnmErr + filePath);
       }
       else{
         ListExpr expList = nl->Second(resultType.listExpr());
         if (nl->IsAtom(expList)){
           expList = nl->OneElemList(expList);
         }
         if (!nl->WriteToFile(filePath, expList)){
           return l.typeError(fwtErr + filePath);
         }
       }
     }
   
     //Check the length of the database name
     string dbName = fList::tempName(true);
     if (dbName.length() >= 16){
       //16 is the maximum length that a database name can be
       return l.typeError(udnErr);
     }
   
     return NList(NList(Symbol::APPEND()),
         NList(NList(qStr, true, true),
               NList(objName, true, false),
               NList((int)kind),
               NList(coParaName, true, false),
               NList(dbName, true, false)),
         resultType).listExpr();
  } catch(...){
    return listutils::typeError("invalid input");
  }

}


int hadoopReduceValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  if ( message <= CLOSE )
  {
    //Appended parameters
    //Query string
    string CreateQuery =
        ((FText*)args[3].addr)->GetValue();
    //Get the object name.
    string CreateObjectName =
        ((CcString*)args[4].addr)->GetValue();
    //Get the export file type
    fListKind kind =
        (fListKind)((CcInt*)args[5].addr)->GetValue();
    //Get the object name of the input flist
    string inParaName =
        ((CcString*)args[6].addr)->GetValue();
    //Get the database name
    string dbName =
        ((CcString*)args[7].addr)->GetValue();

    clusterInfo *ci = new clusterInfo();
    //Get the partition attribute name, since the partition is done by mappers
    Supplier oppList = args[1].addr;
    string PAName =
        nl->SymbolValue(qp->GetType(qp->GetSupplierSon(oppList, 0)));
    //Optional parameters
    string CreateFilePath = "";
    int reduceTaskNum = ci->getSlaveSize();
    int opLen = qp->GetNoSons(oppList);
    for (int i = 1; i < opLen; i++)
    {
      //Only the file path need to be get
      ListExpr pp = qp->GetType(qp->GetSupplierSon(oppList,i));
      if (nl->IsEqual(pp, FText::BasicType())){
        CreateFilePath = ((FText*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
      else if(nl->IsEqual(pp, CcInt::BasicType())){
        reduceTaskNum = ((CcInt*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
    }

    //Result type
    ListExpr resultType = qp->GetType(s);
    fList* resultFList =  new fList(CreateObjectName, NList(resultType),
        ci, NList(), 1, false, kind);

    fList* inputFList = (fList*)args[0].addr;
    int dupTimes = inputFList->getDupTimes();

    //Prepare the reduce query
    ListExpr CreateQueryList;
    nl->ReadFromString(CreateQuery, CreateQueryList);
    vector<string> flistParaList;
    vector<fList*> flistObjList;
    flistParaList.push_back(inParaName); //The input parameter
    flistObjList.push_back(inputFList);
    vector<string> DLF_NameList, DLF_fileLocList;
    vector<string> DLO_NameList, DLO_locList;

    bool ok = true;
    CreateQueryList = replaceParaOp(CreateQueryList, flistParaList,
        flistObjList, ok);
    if (!ok){
      cerr << "Replace para operation fails" << endl;
    }
    else{
      //Replace parameter value according to their flist value
      CreateQueryList = replaceSecObj(CreateQueryList);
      for (size_t i = 0; i < flistParaList.size(); i++)
      {
        int argIndex = (i == 0 ? 1 : 0);
        CreateQueryList =
            replaceDLOF(CreateQueryList,
                flistParaList[i], flistObjList[i],
                DLF_NameList, DLF_fileLocList,
                DLO_NameList, DLO_locList, true, ok, argIndex);
        if (!ok)
          break;
      }
    }

    NList dlfNameList, dlfLocList;
    NList dloNameList, dloLocList;
    vector<pair<int, pair<int, int> > > hpResult;
    if (!ok){
      cerr << "Reading flist data fails." << endl;
    }
    else
    {
      for (size_t i = 0; i < DLF_NameList.size(); i++)
      {
        dlfNameList.append(NList(DLF_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLF_fileLocList[i], locList);
        dlfLocList.append(NList(locList));
      }

      for (size_t i = 0; i < DLO_NameList.size(); i++)
      {
        dloNameList.append(NList(DLO_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLO_locList[i], locList);
        dloLocList.append(NList(locList));
      }

      //The recognition of the function argument
      string inputRcg = "";
      if (inputFList->getKind() == DLO)
        inputRcg = inputFList->getSubName();
      else
        inputRcg = "<DLFMark:Arg1:" + inputFList->getSubName() + "/>";

      bool mapStage = false;
      //Name and Location lists for DLF and DLO flist in the map stage
      NList mq, mfn, mfl, mon, mol;
      int mapTaskNum;
      if (!inputFList->getUEMapQuery().isEmpty()){
        mapStage = true;
        mq = inputFList->getUEMapQuery().first();
        mfn = inputFList->getUEMapQuery().second().first();
        mfl  = inputFList->getUEMapQuery().second().second();
        mon = inputFList->getUEMapQuery().third().first();
        mol  = inputFList->getUEMapQuery().third().second();
        mapTaskNum = inputFList->getUEMapQuery().fourth().intval();
      }


      //Call the Hadoop job
      stringstream queryStr;
      queryStr
      << "hadoop jar ParallelSecondo.jar ParallelSecondo.PS_HadoopReduce \\\n"
      << dbName << " " << CreateObjectName << " "
    << " \"" << tranStr(nl->ToString(CreateQueryList),"\"", "\\\"") << "\" \\\n"
    << " \"" << tranStr(dlfNameList.convertToString(),"\"", "\\\"") << "\" \\\n"
    << " \"" << tranStr(dlfLocList.convertToString(), "\"", "\\\"") << "\" \\\n"
    << " \"" << tranStr(dloNameList.convertToString(),"\"", "\\\"") << "\" \\\n"
    << " \"" << tranStr(dloLocList.convertToString(), "\"", "\\\"") << "\" \\\n"
    << dupTimes  << " " << kind << " " << " \"" << CreateFilePath << "\" "
    << " \"" << inputRcg << "\" ";

      if (mapStage)
      {
        //Describe the map tasks
        queryStr
          << " \"" << tranStr(mq.convertToString(), "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(mfn.convertToString(), "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(mfl.convertToString(), "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(mon.convertToString(), "\"", "\\\"") << "\" \\\n"
          << " \"" << tranStr(mol.convertToString(), "\"", "\\\"") << "\" \\\n"
          << mapTaskNum << " ";
      }

      //The input flist sub-object, which will be partitioned in the map step
      queryStr << " \"" << PAName << "\" "<< reduceTaskNum
        << endl;
      int rtn = -1;
      cout << queryStr.str() << endl;
      rtn = system(queryStr.str().c_str());
      ok = (rtn == 0);

      if (ok)
      {
        FILE *fs;
        char buf[MAX_STRINGSIZE];
        fs = popen("hadoop dfs -cat OUTPUT/part*", "r");
        if (NULL != fs)
        {
          while(fgets(buf, sizeof(buf), fs))
          {
            stringstream ss;
            ss << buf;
            stringstream iss(ss.str());
            int rowIndex, columnIndex, slaveIndex;
            iss >> rowIndex >> columnIndex >> slaveIndex;

            hpResult.push_back(pair<int, pair<int, int> >
              (rowIndex, pair<int, int>(columnIndex, slaveIndex)));
          }
        }
        else
          ok = false;
        pclose(fs);
      }
    }

    if (ok)
    {
      NList fileLocList;
      if (kind == DLO)
         CreateFilePath = dbLoc;

      //Create file location list
      vector<pair<int, pair<int, int> > >::iterator rit;
      int exRowIndex = 1;  //expected row number
      for ( rit = hpResult.begin(); rit != hpResult.end(); rit++)
      {
        int rowIndex = rit->first;
        for (; exRowIndex <= rowIndex; exRowIndex++)
        {
          NList newRow = NList();
          if (rowIndex == exRowIndex){
            int columnIndex = rit->second.first;
            int slaveIndex = rit->second.second;
            newRow = NList(NList(slaveIndex), NList(columnIndex).enclose(),
                NList(CreateFilePath, true, true));
          }
          if (fileLocList.isEmpty()){
            fileLocList.makeHead(newRow);
          }
          else{
            fileLocList.append(newRow);
          }
        }
      }

      resultFList = new fList(CreateObjectName, NList(resultType),
          ci, fileLocList,
          1,         //dup time
          true,      //is distributed
          kind);     //kind

    }

    result.setAddr(resultFList);

  }
  //TODO add the progress estimation in the future
  //  else if ( message == REQUESTPROGRESS )
  //  else if ( message == CLOSEPROGRESS )

    return 0;
}

Operator hadoopReduceOP(
    hadoopReduceInfo(), hadoopReduceValueMap, hadoopReduceTypeMap);

/*
5 Operator ~hadoopReduce2~

Create at 18th Apr. 2012
Jiamin

This is also mainly used to process the reduce step of the precast Hadoop job.
It provides the binary operation, and creates a flist belong to either
DLO or DLF kind.


*/
struct hadoopReduce2Info : OperatorInfo
{
  hadoopReduce2Info()
  {
    name = "hadoopReduce2";
    signature =
      "flist(T1) x flist(T2) x partAttr1 x partAttr2 "
      "x [string] x [text] x [(DLO):DLF] x [int] x [bool]"
      "x ( map (T1 T2) T3 ) -> flist(T3)";
    meaning = "Create DLO or DLF kind flist after the binary reduce step";
  }
};

/*
5.1 Type Mapping of ~hadoopReduce2~

For unary operation, the mapping is:

----
flist(T1) x flist(T2)
  x partAttr1 x partAttr2
  x [string] x [text] x [(DLO):DLF] x [int]
  x ( map (T1 T2) T3 )
\to flist(T3)

T1 and T2 are either rel(tuple) or stream(tuple)

----

Similar as the above unary hadoopReduce operator, this one expects two 
input flists, also two partition attributes, each one corresponding to 
one input flist, respectively.

The syntax is:

----
flist(T1) x flist(T2)
  x partAttr1 x partAttr2
  x [objectName: string] x [objectPath: text] x [(DLO):DLF] 
  x [reduceTaskNum: int]
  x ( interFunc: map ( T1 x T2 \to T1) )
\to flist(T3)

T is rel(tuple) or stream(tuple)

----

Update at 11th Sept. 2012.

Allow hadoopReduce2 to describe Hadoop-based parallel queries,
intermediate results are shuffled with HDFS.
Here no DLO flist is allowed being used in the argument function.
It maps:

----
flist(T1) x flist(T2)
  x partAttr1 x partAttr2
  x [objectName: string] x [objectPath: text] x [(DLO):DLF]
  x [reduceTaskNum: int] x [isHDJ: bool]
  x ( interFunc: map ( T1 x T2 \to T1) )
\to flist(T3)

T is rel(tuple) or stream(tuple)

----

By default, the ~isHDJ~ parameter is false.

*/

ListExpr hadoopReduce2TypeMap(ListExpr args){
  try{
     NList l(args);
   
     string lenErr = "ERROR! Operator hadoopReduce expects 4 argument lists. ";
     string typErr = "ERROR! Operator hadoopReduce expects "
         "flist(T1) x flist(T2) "
         "x partAttr1 x partAttr2 x [string] x [text] x [(DLO):DLF] "
         "x [int] x [bool] x ( map (T1 T2) T3 ) -> flist(T3)";
     string ifsErr = "ERROR! Operator hadoopReduce expects the input flist "
         "contains either a tuple stream or a tuple relation. ";
     string upaErr = "ERROR! The partition attribute "
         "is not found in the corresponding input flist";
     string ifaErr = "ERROR! Infeasible evaluation in TM of attribute:";
     string nprErr = "ERROR! Operator hadoopReduce expects "
         "creating a new DLO or DLF kind flist.";
     string ernErr = "ERROR! The reduceTaskNum cannot be larger than the"
                    " cluster scale while producing a DLO flist object.";
     string onmErr = "ERROR! Operator hadoopMap expects the created "
         "object name starts with upper case. ";
     string uafErr = "ERROR!! The internal function is unavailable.";
     string expErr = "ERROR! Improper output type for DLF flist";
     string fwtErr = "ERROR! Failed writing type into file: ";
     string hnmErr = "ERROR! Exists homonymous flist type file in: ";
     string udnErr = "ERROR! Long database name is set.";
   
   
     string objName, filePath, qStr;
     fListKind kind = DLO;
     clusterInfo ci;
     int reduceTaskNum = ci.getSlaveSize();
   
     int argIndex = 1;
   
     if (l.length() != 4)
       return l.typeError(lenErr);
   
     //The first two flist arguments
     NList inputType[2];
     for (; argIndex <= 2; argIndex++){
       int lc = argIndex - 1;
       inputType[lc] = l.elem(argIndex).first();
       if (inputType[lc].isSymbol("typeerror") || inputType[lc].length() != 2 ){
         return l.typeError(typErr);
       }
       if (!inputType[lc].first().isSymbol(fList::BasicType())){
         return l.typeError(typErr);
       }
       if (!listutils::isRelDescription(inputType[lc].second().listExpr()) &&
           !listutils::isTupleStream(inputType[lc].second().listExpr())){
         return l.typeError(ifsErr);
       }
     }
   
     //The third argument list
     NList pType   = l.elem(argIndex).first();
     NList pValue  = l.elem(argIndex).second();
     int len = pType.length();
     if (len < 2 || len > 6)
       return l.typeError(typErr);
   
     string    PAName[2] = { pType.first().str(), pType.second().str()};
     int       PAIndex[2]= {0,0};
     ListExpr  PAType[2];
     bool      isHDJ = false;
     for (int inc = 0; inc < 2; inc++){
       ListExpr attrList = inputType[inc].second().second().second().listExpr();
       PAIndex[inc] = listutils::findAttribute(attrList, 
                                               PAName[inc], PAType[inc]);
       if (PAIndex[inc] <= 0)
         return l.typeError(upaErr);
     }
     for (int oac = 3; oac <= len; oac++)
     {
       //Check other optional arguments in this list
       NList pp = pType.elem(oac);
       NList pv = pValue.elem(oac);
   
       if (pp.isSymbol(CcString::BasicType()))
       {
         //ObjectName defined.
         NList fnList;
         if (!QueryProcessor::GetNLArgValueInTM(pv, fnList)){
           return l.typeError(ifaErr + "objName");
         }
         objName = fnList.str();
       }
       else if (pp.isSymbol(FText::BasicType()))
       {
         //FilePath defined
         NList fpList;
         if (!QueryProcessor::GetNLArgValueInTM(pv, fpList)){
           return l.typeError(ifaErr + "filePath");
         }
         filePath = fpList.str();
       }
       else if (pp.isSymbol(CcInt::BasicType())){
         NList rnList;
         if (!QueryProcessor::GetNLArgValueInTM(pv, rnList)){
           return l.typeError(ifaErr + "objName");
         }
         reduceTaskNum = rnList.intval();
       }
       else if (pp.isSymbol(CcBool::BasicType())){
         NList ihList;
         if (!QueryProcessor::GetNLArgValueInTM(pv, ihList)){
           return l.typeError(ifaErr + "isHDJ");
         }
         isHDJ = ihList.boolval();
       }
       else if (pp.isSymbol("DLF")){
         kind = DLF;
       }
       else if (!pp.isSymbol("DLO")){
         return l.typeError(nprErr);
       }
     }
     if (kind == DLO && reduceTaskNum > (int)ci.getSlaveSize()){
       return l.typeError(ernErr);
     }
   
     argIndex++;
     //The fourth inter-query function
     NList mapType = l.elem(argIndex).first();
     NList mapValue = l.elem(argIndex).second();
     if (mapType.length() != 4)
       return l.typeError(uafErr);
   
     NList coType = mapType.fourth();             //Create Type
     NList coQuery = mapValue.fourth();          //Create Query
     NList coBType;                              //Create Base Type
     if (coType.isAtom()){
       coBType = coType;
     }
     else{
       coBType = coType.first();
     }
     if (coBType.isEqual("typeerror"))
       return l.typeError(uafErr);
     qStr = coQuery.convertToString();
   
     //Query Parameter
     string coParaName[2] = {
         mapValue.second().first().str(),
         mapValue.third().first().str()};
   
     //Check the type for output flist
     //If it is a DLF, then the output type must be a tuple stream
     if (kind == DLF && !listutils::isTupleStream(coType.listExpr())){
       return l.typeError(expErr);
     }
     NList resultType = NList(NList(fList::BasicType()), NList(coType));
   
     if (objName.length() == 0)
       objName = fList::tempName(false);
     else
     {
       char f = objName[0];
       if (f<'A' || f>'Z'){
         return l.typeError(onmErr);
       }
       //If the name of sub-objects or sub-files are denoted by users,
       //Then it must be kept in a text type file,
       //in case of the homonymous problem
       //also this file must be kept in the default file path.
   
       string filePath =
           getLocalFilePath("", (objName + "_type"),"", true);
       if (FileSystem::FileOrFolderExists(filePath)){
         ListExpr exeType;
         bool ok = false;
         if (nl->ReadFromFile(filePath, exeType)){
           //TODO Need to be more compatible with file-related operators
             if (nl->Equal(nl->Second(exeType),
                 resultType.second().second().listExpr())){
               ok = true;
             }
         }
         if (!ok)
           return l.typeError(hnmErr + filePath);
       }
       else{
         ListExpr expList = nl->Second(resultType.listExpr());
         if (nl->IsAtom(expList)){
           expList = nl->OneElemList(expList);
         }
         if (!nl->WriteToFile(filePath, expList)){
           return l.typeError(fwtErr + filePath);
         }
       }
     }
   
     //Check the length of the database name
     string dbName = fList::tempName(true);
     if (dbName.length() >= 16){
       //16 is the maximum length that a database name can be
       return l.typeError(udnErr);
     }
   
     NList appendList;
     appendList.append(NList(qStr, true, true));
     appendList.append(NList(objName, true, false));
     appendList.append(NList((int)kind));
     appendList.append(NList(coParaName[0], true, false));
     appendList.append(NList(coParaName[1], true, false));
     appendList.append(NList(dbName, true, false));
     appendList.append(NList(isHDJ));
   
     return NList(NList(Symbol::APPEND()),
         appendList,
         resultType).listExpr();
  } catch(...){
    return listutils::typeError("invalid input");
  }
}


int hadoopReduce2ValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){
  if ( message <= CLOSE )
  {
    //Get Arguments
    //Query string
    string CreateQuery =
        ((FText*)args[4].addr)->GetValue();
    //Get the object name.
    string CreateObjectName =
        ((CcString*)args[5].addr)->GetValue();
    //Get the export file type
    fListKind kind =
        (fListKind)((CcInt*)args[6].addr)->GetValue();
    //Get the object name of the input flist
    string inParaName[2] = {
        ((CcString*)args[7].addr)->GetValue(),
        ((CcString*)args[8].addr)->GetValue()};
    //Get the database name
    string dbName =
        ((CcString*)args[9].addr)->GetValue();
    //Get isHDJ
    bool isHDJ = ((CcBool*)args[10].addr)->GetValue();

    clusterInfo *ci = new clusterInfo();
    //Get the partition attribute name, since the partition is done by mappers
    Supplier oppList = args[2].addr;
    string PAName[2] = {
        nl->SymbolValue(qp->GetType(qp->GetSupplierSon(oppList, 0))),
        nl->SymbolValue(qp->GetType(qp->GetSupplierSon(oppList, 1)))};
    //Optional parameters
    string CreateFilePath = "";
    int reduceTaskNum = ci->getSlaveSize();
    int opLen = qp->GetNoSons(oppList);
    for (int i = 2; i < opLen; i++)
    {
      //Only the file path need to be get
      ListExpr pp = qp->GetType(qp->GetSupplierSon(oppList,i));
      if (nl->IsEqual(pp, FText::BasicType())){
        CreateFilePath = ((FText*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
      else if(nl->IsEqual(pp, CcInt::BasicType())){
        reduceTaskNum = ((CcInt*)
          qp->Request(qp->GetSupplier(oppList,i)).addr)->GetValue();
      }
    }

    ListExpr resultType = qp->GetType(s);
    fList* resultFList =  new fList(CreateObjectName, NList(resultType),
        ci, NList(), 1, false, kind);


    fList* inputFList[2] = {(fList*)args[0].addr, (fList*)args[1].addr};
    int dupTimes[2] = { inputFList[0]->getDupTimes(),
                        inputFList[1]->getDupTimes()};

    //Prepare the reduce query
    ListExpr CreateQueryList;
    nl->ReadFromString(CreateQuery, CreateQueryList);
    vector<string> flistParaList;
    vector<fList*> flistObjList;
    flistParaList.push_back(inParaName[0]); //The input parameter name
    flistParaList.push_back(inParaName[1]);
    flistObjList.push_back(inputFList[0]);  //The input parameter value
    flistObjList.push_back(inputFList[1]);
    vector<string> DLF_NameList, DLF_fileLocList;
    vector<string> DLO_NameList, DLO_locList;

    size_t dloNumber = 0;
    if (inputFList[0]->getKind() == DLO) dloNumber++;
    if (inputFList[1]->getKind() == DLO) dloNumber++;

    bool ok = true;
    //Process all para operators inside the reduce query
    CreateQueryList = replaceParaOp(
        CreateQueryList, flistParaList, flistObjList, ok);
    if (!ok){
      cerr << "Replace para operation fails" << endl;
    }
    else{
      CreateQueryList = replaceSecObj(CreateQueryList);
      //Replace parameter value according to their flist value
      for (size_t i = 0; i < flistParaList.size(); i++)
      {
        //The top two flists are input arguments for the inter-query
        //they are re-partitioned in the map step.
        int argIndex = ((i == 0 || i == 1)? (i+1) : 0);
        CreateQueryList =
            replaceDLOF(CreateQueryList,
                flistParaList[i], flistObjList[i],
                DLF_NameList, DLF_fileLocList,
                DLO_NameList, DLO_locList, true, ok, argIndex);
        if (!ok)
          break;
      }
    }

    if (ok && isHDJ && (DLO_NameList.size() > dloNumber)){
      cerr << "For Hadoop-based queries, it is not allowed "
          "to use DLO flist in the argument function." << endl;
      ok = false;
    }

    NList dlfNameList, dlfLocList;
    NList dloNameList, dloLocList;
    vector<pair<int, pair<int, int> > > hpResult;
    if (!ok){
      cerr << "Reading flist data fails." << endl;
    }
    else
    {
      for (size_t i = 0; i < DLF_NameList.size(); i++)
      {
        dlfNameList.append(NList(DLF_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLF_fileLocList[i], locList);
        dlfLocList.append(NList(locList));
      }

      for (size_t i = 0; i < DLO_NameList.size(); i++)
      {
        dloNameList.append(NList(DLO_NameList[i], true, false));
        ListExpr locList;
        nl->ReadFromString(DLO_locList[i], locList);
        dloLocList.append(NList(locList));
      }


      //The recognition of the function argument
      string inputRcg[2] = {"", ""};
      for (int i = 0; i < 2; i++)
      {
        if (inputFList[i]->getKind() == DLO)
          inputRcg[i] = inputFList[i]->getSubName();
        else
          inputRcg[i] = "<DLFMark:Arg" + int2string(i+1) + ":"
            + inputFList[i]->getSubName() + "/>";
      }

      NList IUEMapQuery = NList(
          inputFList[0]->getUEMapQuery(),
          inputFList[1]->getUEMapQuery());

      //Call the Hadoop job
      stringstream queryStr;
      queryStr
        << "hadoop jar ParallelSecondo.jar "
            "ParallelSecondo.PS_HadoopReduce2 \\\n"
  << dbName << " " << CreateObjectName << " \\\n"
  << " \"" << tranStr(nl->ToString(CreateQueryList), "\"", "\\\"") << "\" \\\n"
  << " \"" << tranStr(dlfNameList.convertToString(), "\"", "\\\"") << "\" \\\n"
  << " \"" << tranStr(dlfLocList.convertToString(), "\"", "\\\"") << "\" \\\n"
  << " \"" << tranStr(dloNameList.convertToString(), "\"", "\\\"") << "\" \\\n"
  << " \"" << tranStr(dloLocList.convertToString(), "\"", "\\\"") << "\" \\\n"
  << " \"" << CreateFilePath << "\" \\\n"
  << " \"" << inputRcg[0] << "\" " << dupTimes[0] << " \"" << PAName[0]<< "\" "
  << " \"" << inputRcg[1] << "\" "<< dupTimes[1] << " \"" << PAName[1]<< "\" "
  << reduceTaskNum << " " << kind << " \\\n"
  << " \"" << tranStr(IUEMapQuery.convertToString(), "\"", "\\\"") << "\" "
  << boolalpha << isHDJ
  << endl;
      int rtn = -1;
      cout << queryStr.str() << endl;
      rtn = system(queryStr.str().c_str());
      ok = (rtn == 0);

      if (ok)
      {
        FILE *fs;
        char buf[MAX_STRINGSIZE];
        fs = popen("hadoop dfs -cat OUTPUT/part*", "r");
        if (NULL != fs)
        {
          while(fgets(buf, sizeof(buf), fs))
          {
            stringstream ss;
            ss << buf;
            stringstream iss(ss.str());
            int rowIndex, columnIndex, slaveIndex;
            iss >> rowIndex >> columnIndex >> slaveIndex;

            hpResult.push_back(pair<int, pair<int, int> >
              (rowIndex, pair<int, int>(columnIndex, slaveIndex)));
          }
        }
        else
          ok = false;
        pclose(fs);
      }
    }

    if (ok)
    {
      NList fileLocList;
      if (kind == DLO)
         CreateFilePath = dbLoc;

      //Create file location list
      vector<pair<int, pair<int, int> > >::iterator rit;
      int exRowIndex = 1;  //expected row number
      for ( rit = hpResult.begin(); rit != hpResult.end(); rit++)
      {
        int rowIndex = rit->first;
        for (; exRowIndex <= rowIndex; exRowIndex++)
        {
          NList newRow = NList();
          if (rowIndex == exRowIndex){
            int columnIndex = rit->second.first;
            int slaveIndex = rit->second.second;
            newRow = NList(NList(slaveIndex), NList(columnIndex).enclose(),
                NList(CreateFilePath, true, true));
          }
          if (fileLocList.isEmpty()){
            fileLocList.makeHead(newRow);
          }
          else{
            fileLocList.append(newRow);
          }
        }
      }

      resultFList = new fList(CreateObjectName, NList(resultType),
          ci, fileLocList,
          1,         //dup time
          true,      //is distributed
          kind);     //kind

    }

    result.setAddr(resultFList);
  }
  //TODO add the progress estimation in the future
  //  else if ( message == REQUESTPROGRESS )
  //  else if ( message == CLOSEPROGRESS )

    return 0;

}

Operator hadoopReduce2OP(
    hadoopReduce2Info(), hadoopReduce2ValueMap, hadoopReduce2TypeMap);


/*
4 Operator ~createFList~

Create an undistributed flist,
used to debug the implementation of the flist operator.

T -> flist

I disable the In and Out function of the flist,
like what BTree does, because we abandon the DGO kind object,
hence it is impossible to create a flist without running a hadoop job.

Update on 15th May 2013

In some cases, like in the parallel generation of BerlinMOD data,
data are created independently on all Data Servers.
In order to access all data on the cluster,
the ~createFList~ operator is extended.

It accepts a schema relation, to indicate the type of the distributed data.
Besides, it accepts several arguments to indicate the location of distributed data,
and returns the flist at last.

For DLF flist, this operator maps:

----
stream(tuple(T))
  x ObjectName  : string
  x LocRel      : rel(tuple((Row:int)(DS:int)(Column:int)(FilePath:text))))
  x type        : DLF
  x Distributed : bool
\to flist(stream(tuple(T)))
----

For DLO flist, this operator maps:

----
T
  x ObjectName  : string
  x LocRel      : rel(tuple((Row:int)(DS:int)(Column:int)(FilePath:text))))
  x type        : DLO
  x Distributed : bool
\to flist(T)
----


*/
struct createFListInfo : OperatorInfo
{
  createFListInfo()
  {
    name = "createFList";
    signature = "T x string x rel x DLO(DLF) x bool -> flist(T)";
    meaning = "Create a flist object, "
        "in case the data are already distributed on the cluster without "
        "using the spread operator.";
  }
};

/*
4.1 Type Mapping

*/
ListExpr createFListTypeMap(ListExpr args){

  NList l(args);
  string tpeErr = "ERROR! createFList expects "
      "T x string x rel x DLO(DLF) x bool";
  string lorErr = "ERROR! The location relation excepts "
      "rel(tuple((Row:int) (DS:int) (Column:int) (FilePath:text)))";
  string ifaErr = "ERROR! Infeasible evaluation in TM of attribute:";
  string onmErr = "ERROR! Operator createFList expects the created "
      "object name starts with upper case. ";
  string hnmErr = "ERROR! Unmatched type "
      "or Exists homonymous flist type file in: ";
  string fwtErr = "ERROR! Failed writing type into file: ";


  if (l.length() != 5){
    return l.typeError(tpeErr);
  }

  try{
    NList pType, pValue;
    fListKind kind = DLO;

    //First argument
    NList inputType = l.first().first();

    //Second argument : ObjectName
    //If it is empty, then use the default temporal value
    pType = l.second().first();
    if (!pType.isSymbol(CcString::BasicType()))
      return l.typeError(tpeErr);

    //Third argument : location relation
    pType = l.third().first();
    if (!listutils::isRelDescription(pType.listExpr()))
      return l.typeError(tpeErr);
    NList attrList = pType.second().second();
    if (attrList.length() != 4)
      return l.typeError(lorErr);
    for (int i = 1; i < 4; i++)
    {
      if (!attrList.elem(i).second().isEqual(CcInt::BasicType()))
        return l.typeError(lorErr);
    }
    if (!attrList.fourth().second().isEqual(FText::BasicType()))
      return l.typeError(lorErr);

    //Fourth argument
    pType = l.fourth().first();
    if (pType.isSymbol("DLF")){
      kind = DLF;
    }
    else if (pType.isSymbol("DLO")){
      kind = DLO;
    }
    else
      return l.typeError(tpeErr);

    //Fifth argument
    pType = l.fifth().first();
    if (!pType.isSymbol(CcBool::BasicType()))
      return l.typeError(tpeErr);

    if ( kind == DLF )
    {
      if (!listutils::isTupleStream(inputType.listExpr()))
        return l.typeError(tpeErr);
    }

    //Create the type file
    NList onList;
    if (!QueryProcessor::GetNLArgValueInTM(l.second().second(), onList)){
      return l.typeError(ifaErr + "ObjectName");
    }
    string objName = onList.str();
    if (objName.length() == 0)
      objName = fList::tempName(false);
    else
    {
      //Prepare the type file if the ObjectName is indicated by the user
      char f = objName[0];
      if ( f < 'A' || f > 'Z'){
        return l.typeError(onmErr);
      }

      string filePath = getLocalFilePath("", (objName + "_type"), "", true);
      if (FileSystem::FileOrFolderExists(filePath))
      {
        ListExpr exeType; //The exist type
        bool ok = false;
        if (nl->ReadFromFile(filePath, exeType))
        {
          if (nl->Equal(exeType, inputType.listExpr())){
            ok = true;
          }
          else
          {
            if (kind == DLF
                && (listutils::isRelDescription(exeType)
                  || listutils::isTupleStream(exeType))){
              //Be more compatible with file-related operators,
              //checks the stream tuple only
              if (nl->Equal(nl->Second(inputType.listExpr()),
                    nl->Second(exeType)))
                ok = true;
            }
          }
        }
        if (!ok)
          return l.typeError(hnmErr + filePath);
      }
      else
      {
        ListExpr expList = inputType.listExpr();
        if (nl->IsAtom(expList)){
          expList = nl->OneElemList(expList);
        }
        if (!nl->WriteToFile(filePath, expList)){
          return l.typeError(fwtErr + filePath);
        }
      }
    }

    NList resultType = NList(NList(fList::BasicType()), inputType);

    return NList(
            NList(Symbol::APPEND()),
            NList(NList(objName, true, false),
                  NList((int)kind)),
            resultType).listExpr();
  } catch(...){
    return l.typeError(tpeErr);
  }

}

/*
4.2 Value Mapping

*/
int createFListValueMap(Word* args, Word& result,
    int message, Word& local, Supplier s){

  string dbName = fList::tempName(true);
  string objName = ((CcString*)args[5].addr)->GetValue();
  fListKind kind = (fListKind)((CcInt*)args[6].addr)->GetValue();
  NList resultType = NList(qp->GetType(s));
  clusterInfo* ci = new clusterInfo();
  //TODO
  size_t dupTime = 1;
  bool distributed = ((CcBool*)args[4].addr)->GetValue();
  NList fileLocList;

  fList* emptyFlist =
      new fList(objName, resultType, ci, fileLocList, dupTime);

  vector<pair<int, rowInLocRel> > locList;
  GenericRelation* locRel = (GenericRelation*)(args[2].addr);
  GenericRelationIterator* iter = locRel->MakeScan();
  Tuple* nextTuple = iter->GetNextTuple();
  while (!iter->EndOfScan()){
    int row    = ((CcInt*)nextTuple->GetAttribute(0))->GetValue();
    rowInLocRel r;
    r.dsIndex = ((CcInt*)nextTuple->GetAttribute(1))->GetValue();
    r.column  = ((CcInt*)nextTuple->GetAttribute(2))->GetValue();
    r.filePath = ((FText*)nextTuple->GetAttribute(3))->GetValue();
    locList.push_back(pair<int, rowInLocRel>(row, r));

    nextTuple->DeleteIfAllowed();
    nextTuple = iter->GetNextTuple();
  }
  sort(locList.begin(), locList.end(),rowRelInfo);

  vector<pair<int, rowInLocRel> >::iterator llit = locList.begin();
  int crow = 1;
  while (llit != locList.end())
  {
    int row = llit->first;

    if (crow < row){
      for (; crow < row; crow++){
        fileLocList.append(NList());
      }
    }
    else if ( crow == row)
    {
      int dsIndex = llit->second.dsIndex;
      string filePath = llit->second.filePath;
      NList columnsList;
      while (llit != locList.end())
      {
        if (llit->first != crow){
          crow++;
          break;
        }
        if (llit->second.dsIndex != dsIndex ||
            llit->second.filePath.compare(filePath) != 0){
          cerr <<
              "ERROR! The format of the location relation is wrong." << endl;
          result.setAddr(emptyFlist);
          return 0;
        }
        columnsList.append(NList(llit->second.column));
        llit++;
      }
      fileLocList.append(NList(NList(dsIndex), columnsList,
          NList(filePath, true, true)));
    }
    else
    {
      // crow > crow
      cerr << "ERROR! The format of the location relation is wrong." << endl;
      result.setAddr(emptyFlist);
      return 0;
    }
  }

  fList* rl = new fList(objName, resultType, ci, fileLocList,
      dupTime, distributed, kind);

  result.setAddr(rl);
  return 0;
}


Operator cflOp(
    createFListInfo(), createFListValueMap, createFListTypeMap);




/*
6 Auxiliary functions

*/

/*
Checks for valid description of a flist

*/
bool isFListStreamDescription(const NList& typeInfo)
{
  if (typeInfo.length() != 2){
    return false;
  }
  if (!( typeInfo.first().isSymbol(fList::BasicType()) &&
      listutils::isTupleStream(typeInfo.second().listExpr()))){
    return false;
  }

  return true;
}

/*
Scans the query list, and replace parameter values by fList data.
DGO kind data replace the parameter with their value list,
while DLO kind data replace with object name.
If it is a DLF type list, then the file name is appended into the DLF\_NameList,
while the fileLocList is appended into the DLF\_fileLocList

Update at 13th Apr. Jiamin
Add a boolean parameter isArg with the default value of false.
This is used to denote whether the input parameter listName is
an argument of the query list.
If it is, then in the operator like ~hadoopReduce~,
this value should be re-distributed and be replaced in the map step.

Update at 30th Apr. Jiamin
Rename to replaceDLOF, mark on not only DLF, but also DLO flist,
and returns location information of both kinds flists.
Therefore, if a sub-object of a DLO flist doesn't exist on one slave data 
server, no map task is deployed.

Update at 22th Aug. Jiamin
Add the boolean parameter ua (Unavailable Allowed).
If it is set as true, then for function argument (argIndex > 0),
they can be unavailable

*/
ListExpr replaceDLOF(ListExpr createQuery, string listName, fList* listObject,
    vector<string>& DLF_NameList, vector<string>& DLF_fileLocList,
    vector<string>& DLO_NameList, vector<string>& DLO_locList,
    bool ua, bool& ok, int argIndex/* = 0*/)
{ //Replace DLO and DLF
  if (!ok)
  {
    return nl->OneElemList(nl->SymbolAtom("error"));
  }

  if (nl->IsEmpty(createQuery))
    return createQuery;

  if (nl->IsAtom(createQuery))
  {
    if ((nl->AtomType(createQuery) == SymbolType) &&
        (nl->SymbolValue(createQuery) == listName))
    {
      if (listObject->isAvailable() || (ua && argIndex > 0))
      {
        string objectName = listObject->getSubName();
        switch (listObject->getKind())
        {
          case DLO:{

            string paraName = objectName;
            if (argIndex > 0){
              stringstream ss;
              ss << "<DLOMark:Arg" << argIndex << ":" << objectName << "/>";
              paraName = ss.str();
            }

            DLO_NameList.push_back(paraName);
            DLO_locList.push_back(listObject->getLocList().convertToString());

            return nl->SymbolAtom(objectName);
          }
          case DLF:{
            stringstream ss;
            ss << "<DLFMark:";
            if (argIndex > 0){
              ss << "Arg" << argIndex << ":";
            }
            ss << objectName << "/>";
            DLF_NameList.push_back(ss.str());

            DLF_fileLocList.push_back(
                listObject->getLocList().convertToString());
            return nl->StringAtom(ss.str(),true);
          }
          default:{
            ok = false;
            return nl->OneElemList(nl->SymbolAtom("error"));
          }
        }
      }

      ok = false;
      return nl->OneElemList(nl->SymbolAtom("error"));
    }
    else
      return createQuery;
  }
  else
  {
    return (nl->Cons(replaceDLOF(nl->First(createQuery),
          listName, listObject, DLF_NameList, DLF_fileLocList,
          DLO_NameList, DLO_locList, ua, ok, argIndex),
        replaceDLOF(nl->Rest(createQuery),
          listName, listObject, DLF_NameList, DLF_fileLocList,
          DLO_NameList, DLO_locList, ua, ok, argIndex)));
  }
}


/*
Find all nested lists of ~para~ operations,

If the operator contains a T type data, T [INSET] DATA,
then replace it with the data's value directly.
It it contains a flist(T) data,
then add its name and value to these two vectors.

*/
ListExpr replaceParaOp(
    ListExpr createQuery, vector<string>& flistParaList,
    vector<fList*>& flistObjList, bool& ok)
{
  if (!ok){
    return nl->OneElemList(nl->SymbolAtom("error"));
  }

  if (nl->IsEmpty(createQuery))
    return createQuery;

  if (nl->ListLength(createQuery) == 2)
  {
    ListExpr first = nl->First(createQuery);
    if (nl->IsAtom(first))
    {
      if (nl->IsEqual(first, "para"))
      {
        ListExpr second = nl->Second(createQuery);
        string paraName = nl->ToString(second);
        ListExpr paraType =
            SecondoSystem::GetCatalog()->GetObjectTypeExpr(paraName);

        if (nl->ListLength(paraType) > 1){
          if(nl->IsEqual(nl->First(paraType), fList::BasicType())){
            flistParaList.push_back(paraName);
            Word listValue;
            bool defined;
            ok = SecondoSystem::GetCatalog()->
                GetObject(paraName, listValue, defined);
            if (ok){
              flistObjList.push_back((fList*)listValue.addr);
              return nl->SymbolAtom(paraName);
            }
            else
              return nl->OneElemList(nl->SymbolAtom("error"));
          }
        }
        else{
          ListExpr DGOValue =
              SecondoSystem::GetCatalog()->GetObjectValue(paraName);

          ListExpr DGOType =
              SecondoSystem::GetCatalog()->GetObjectTypeExpr(paraName);
          return nl->TwoElemList(DGOType, DGOValue);
        }
      }
    }
  }

  if (nl->ListLength(createQuery) > 0){
    return (nl->Cons(
      replaceParaOp(nl->First(createQuery),
           flistParaList, flistObjList, ok),
      replaceParaOp(nl->Rest(createQuery),
           flistParaList, flistObjList, ok) ));
  }
  else{
    return createQuery;
  }
}

/*
Find all DELIEVERABLE Secondo objects, and substitute it with its nested-list
expression instead of its name.
With this function, there is no need to add ~para~ operator for these symbol 
objects, which was designed as DGO flist.

*/
ListExpr replaceSecObj(ListExpr createQuery)
{
  if (nl->IsEmpty(createQuery))
    return createQuery;

  if (nl->IsAtom(createQuery))
  {
    string atomName = nl->ToString(createQuery);
    bool isObject = SecondoSystem::GetCatalog()->IsObjectName(atomName);
    if (isObject)
    {
      ListExpr paraType =
                SecondoSystem::GetCatalog()->GetObjectTypeExpr(atomName);
      if (listutils::isKind(paraType, Kind::DELIVERABLE())){
        ListExpr DGOValue =
            SecondoSystem::GetCatalog()->GetObjectValue(atomName);

        ListExpr DGOType =
            SecondoSystem::GetCatalog()->GetObjectTypeExpr(atomName);
        return nl->TwoElemList(DGOType, DGOValue);
      }
    }
  }

  if (nl->ListLength(createQuery) > 0)
  {
    return nl->Cons(
        replaceSecObj(nl->First(createQuery)),
        replaceSecObj(nl->Rest(createQuery)));
  }
  else
    return createQuery;
}


/*
3 Class ~HadoopAlgebra~

A new subclass ~HadoopAlgebra~ of class ~Algebra~ is declared.
The only specialization with respect to class ~Algebra~ takes place within
the constructor: all type constructors and operators are registered at the
actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~
is defined.

*/

class HadoopAlgebra: public Algebra
{
public:
  HadoopAlgebra() :
    Algebra()
  {
    AddTypeConstructor(&flTC);


    AddOperator(&spreadOp);
    spreadOp.SetUsesArgsInTypeMapping();

    AddOperator(&spreadFilesOp);
    spreadFilesOp.SetUsesArgsInTypeMapping();

    AddOperator(&collectOp);
    collectOp.SetUsesArgsInTypeMapping();

    AddOperator(&paraOp);

    AddOperator(TParaInfo(), 0, TParaTypeMapping);
    AddOperator(TPara2Info(), 0, TPara2TypeMapping);

    AddOperator(&hadoopMapOP);
    hadoopMapOP.SetUsesArgsInTypeMapping();

    AddOperator(&hadoopReduceOP);
    hadoopReduceOP.SetUsesArgsInTypeMapping();

    AddOperator(&hadoopReduce2OP);
    hadoopReduce2OP.SetUsesArgsInTypeMapping();

    AddOperator(&cflOp);
    cflOp.SetUsesArgsInTypeMapping();

    AddOperator(&pffeedOp);
    pffeedOp.SetUsesArgsInTypeMapping();

  }
  ~HadoopAlgebra()
  {
  }
  ;
};


/*
4 Initialization

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

extern "C" Algebra*
InitializeHadoopAlgebra(
    NestedList* nlRef, QueryProcessor* qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new HadoopAlgebra());
}

/*
[newpage]

*/


