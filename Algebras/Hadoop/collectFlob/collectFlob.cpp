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

[1] Implementation of CollectFlob

Dec 2013 Jiamin Lu

[TOC]

[newpage]

1 Abstract

This is a program completely separated from Parallel SECONDO system. 
It reads a sheet of Flob orders, read the required data from a Flob file, 
then export them into a binary file. 

Its arguments include: 

----
string: flobSheetName
string: PSFSNodePath
string: ResultFileName
string: TargetPath
----

All the management about the underlying cluster is processed within SECONDO, 
hence here it only carries out the basic data access. 

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <utility>
#include <sys/types.h> //declaration for u_int32_t
#include <cstdlib>     //invoking system commands
#include<string.h>

using namespace std;

const int MAX_COPYTIMES = 50;
const string scpCommand = "scp -q ";

string int2string(const int& number)
{
  ostringstream oss;
  oss << number;
  return oss.str();
}

template<typename T1, typename T2>
struct classCompPair
{
  bool operator() (const pair<T1,T2>& pair_1,
      const pair<T1,T2>& pair_2) const
  {
    return (pair_1.first < pair_2.first)
        || (!(pair_1.first > pair_2.first && pair_1.first < pair_2.first)
          && (pair_1.second < pair_2.second));
  }
};


int main(int argc, char** argv)
{
  if (argc != 5)
  {
    cerr << "Usage: " << argv[0] << " needs the following parameters" << endl;
    cerr << "  flobSheetName :\tstring " << endl;
    cerr << "  PSFSNodePath :\tstring " << endl;
    cerr << "  ResultFileName :\tstring " << endl;
    cerr << "  TargetPath :\tstring " << endl;
    return -1;
  }

  string sheetName = argv[1];
  ifstream sheetFile(sheetName.c_str());
  if (!sheetFile.good()){
    cerr << "The sheet file is not correctly opened." << endl;
    return -1;
  }
  string PSFSNode = argv[2];
  if (access(PSFSNode.c_str(), 0) == -1){
    cerr << "The PSFS node path: " << PSFSNode << " does not exist. " << endl;
    return -1;
  }
  string resultFileName = argv[3];
  string targetPath = argv[4];       //Path on the remote computer. 
  
  map< u_int32_t, ifstream* > flobFiles;
  map< u_int32_t, ifstream* >::iterator it;
  ifstream* flobFile = 0;
  string resultFilePath = PSFSNode + "/tmp_" + resultFileName;
  targetPath += ("/" + resultFileName);
  ofstream resultFile(resultFilePath.c_str(), ios::binary);
  
  //Cached Flob markers, avoid extract the same Flob
  set<pair<u_int32_t, size_t>, classCompPair<u_int32_t, size_t> > lobMarkers;
  //Empty lobs are prepared for Flobs that have been created in another sheet
  set<pair<u_int32_t, size_t>, classCompPair<u_int32_t, size_t> > emptyLobs;

  string flobOrder;
  u_int32_t lastFileId = 0;
//  size_t outputSize = 0;
//  size_t totalSize = 0;
  while (getline(sheetFile, flobOrder))
  {
    stringstream ss(flobOrder);
    u_int32_t fileId;
    int sourceDS, mode;
    size_t offset, size;
    ss >> fileId >> sourceDS >> offset >> mode >> size;
//    totalSize += size;    

    if ( mode != 3) {
      //This Flob may already have been fetched by another thread. 
      size_t recId = sourceDS;
      pair<u_int32_t, size_t> mlob(fileId, recId);
      if (emptyLobs.find(mlob) != emptyLobs.end()){
        continue;
      } else {
        emptyLobs.insert(mlob);
      }

      char block[size];
      memset(block, 0, size);
      resultFile.write(block, size);
//    outputSize += size;
      continue;
    } 

    //The flob is never mentioned in the current sheet
    pair<u_int32_t, size_t> mlob(fileId, offset);
    if (lobMarkers.find(mlob) != lobMarkers.end()){
      continue;
    } else {
      lobMarkers.insert(mlob);
    }

    if (lastFileId != fileId)
    {
      it = flobFiles.find(fileId);
      if ( it == flobFiles.end()){
        string flobFileName = PSFSNode + "/flobFile_" + int2string(fileId);
        flobFile = new ifstream(flobFileName.c_str(), ios::binary);
        flobFiles[fileId] = flobFile;
        it = flobFiles.find(fileId);
      }
      lastFileId = fileId;
    }

    char block[size];
    flobFile = it->second;
    flobFile->seekg(offset, ios_base::beg);
    flobFile->read(block, size);
    resultFile.write(block, size);
//    outputSize += size;
  }
 
  for (it = flobFiles.begin(); it != flobFiles.end(); it++)
  {
    flobFile = it->second;
    flobFile->close();
    delete flobFile;
    flobFile = 0;
  }
  flobFiles.clear();
  resultFile.close();
  sheetFile.close();
  
  //Send the result file back
  int atimes = MAX_COPYTIMES;
  while ( atimes-- > 0){
    if (0 == system((scpCommand + resultFilePath + " " + targetPath).c_str())){
      if (::unlink(resultFilePath.c_str()) != 0){
        cerr << "Warning! Deleting the result file fails. " << endl;
      }

      if (::unlink(sheetName.c_str()) != 0){
        cerr << "Warning! Deleting the sheet file fails. " << endl;
      }
      break;
    }
    else{
      sleep(1);
    }
  }
}
