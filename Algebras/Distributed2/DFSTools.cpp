/*
----
This file is part of SECONDO.

Copyright (C) 2018,
Faculty of Mathematics and Computer Science,
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


*/

#include "DFSTools.h"
#include "SecondoSystem.h"
#include <string>
#include <libgen.h>
#include "FileSystem.h"
#include "StringUtils.h"


namespace distributed2{
  extern DFSType* filesystem;
}

namespace dfstools{

   std::string getRemoteName(const std::string localName){
     std::string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
     std::string basename = FileSystem::Basename(localName.c_str()); 
     return dbname+"::"+basename;
   }

   std::string getCategory(){
     return SecondoSystem::GetInstance()->GetDatabaseName();
   }

   bool getRemoteFile(const std::string& localfilename){
      if(!distributed2::filesystem){
        return false;
      }
      std::string rname = getRemoteName(localfilename);
      try{
         distributed2::filesystem->receiveFileToLocal(rname.c_str(), 
                                                      localfilename.c_str());
         return true;
      } catch(...){
          return false;
      }
   }

   bool storeRemoteFile(const std::string& localFileName){
      if(!distributed2::filesystem){
        return false;
      }
      std::string rname = getRemoteName(localFileName);
      std::string cat = getCategory();
      try{
         distributed2::filesystem->storeFileFromLocal(rname.c_str(),
                                       localFileName.c_str(),
                                       cat.c_str());
         return true;
      } catch(...){
          return false;
      }

   }

   bool deleteAllFilesInDB(){
      if(!distributed2::filesystem){
        return false;
      }
      try{   
        distributed2::filesystem->deleteAllFilesOfCategory(
                                               getCategory().c_str());
        return true;
      } catch(...){
        return false;
      }
   }   
   
   bool deleteAllFiles(){
      if(!distributed2::filesystem){
        return false;
      }
      try{   
        distributed2::filesystem->deleteAllFiles();
        return true;
      } catch(...){
        return false;
      }
   }   

   UI64 removeTempFiles(){
      if(!distributed2::filesystem){
        return 0;
      }
      try{   
        std::vector<std::string> allFiles = 
                               distributed2::filesystem->listFileNames();
        std::string start = getRemoteName("TMP_");
        int count = 0;
        for(size_t i=0;i<allFiles.size();i++){
            std::string& file = allFiles[i];
            if(stringutils::startsWith(file, start)){
              distributed2::filesystem->deleteFile(file.c_str());
              count++;
            }
        }
        return count;
      } catch(...){
         return 0;
      }
   }



}




