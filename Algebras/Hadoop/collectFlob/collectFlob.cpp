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

using namespace std;

const int MAX_COPYTIMES = 50;
const string scpCommand = "scp -q ";

string int2string(const int& number)
{
  ostringstream oss;
  oss << number;
  return oss.str();
}

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
  string resultFilePath = PSFSNode + "/" + resultFileName;
  ofstream resultFile(resultFilePath.c_str(), ios::binary);
  
  string flobOrder;
  u_int32_t lastFileId = 0;
  while (getline(sheetFile, flobOrder))
  {
    stringstream ss(flobOrder);
    u_int32_t fileId;
    int sourceDS, mode;
    size_t offset, size;
    ss >> fileId >> sourceDS >> offset >> mode >> size;

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
