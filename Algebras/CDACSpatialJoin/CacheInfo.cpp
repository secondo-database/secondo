/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{2}
\tableofcontents


1 CacheInfo class

*/

#include <fstream>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string/predicate.hpp>

#include "CacheInfo.h"
#include "Utils.h"

using namespace cdacspatialjoin;
using namespace std;


/*
1.2 CacheInfo struct

*/
CacheInfo::CacheInfo(const std::string& path) :
   coherencyLineSize(readUnsigned(path, "coherency_line_size")),
   level(readUnsigned(path, "level")),
   numberOfSets(readUnsigned(path, "number_of_sets")),
   physicalLinePartition(readUnsigned(path, "physical_line_partition")),
   sharedCpuList(readString(path, "shared_cpu_list")),
   sharedCpuMap(readUnsigned(path, "shared_cpu_map", 16)),
   sizeInBytes(readSizeInBytes(path, "size")),
   type(readCacheType(path, "type")),
   waysOfAssociativity(readUnsigned(path, "ways_of_associativity")),
   sharedCpuCount(0) {

   // count the number of cpus that share this cache
   unsigned cpuBits = sharedCpuMap;
   while (cpuBits != 0) {
      if (cpuBits & 1)
         ++sharedCpuCount;
      cpuBits >>= 1;
   }
   sharedCpuCount = std::max(sharedCpuCount, 1U); // at least one
}

std::string CacheInfo::readString(const string& path, const string file) {
   const string pathFile = pathCombine(path, file);
   ifstream inputFile(pathFile);
   if (inputFile.is_open()) {
      string line;
      if (!getline(inputFile, line))
         line = "";
      inputFile.close();
      return line;
   } else {
      return "";
   }
}

unsigned CacheInfo::readUnsigned(const string& path, const string file,
                                 const int base /* = 10 */) {
   try {
      const string line = readString(path, file);
      size_t pos;
      return static_cast<unsigned>(stoi(line, &pos, base));
   } catch(invalid_argument& ex) {
      return 0;
   }
}

unsigned CacheInfo::readSizeInBytes(const string& path, const string file) {
   try {
      const string line = readString(path, file);
      size_t pos;
      auto value = static_cast<unsigned>(stoi(line, &pos));
      if (pos < line.size()) {
         if (line[pos] == 'K')
            value *= 1024;
         else if (line[pos] == 'M')
            value *= 1024 * 1024;
      }
      return value;
   } catch(invalid_argument& ex) {
      return 0;
   }
}

CacheType CacheInfo::readCacheType(const string& path, const string file) {
   const string type = readString(path, file);
   if (boost::iequals(type, "Data"))
      return CacheType::Data;
   else if (boost::iequals(type, "Instruction"))
      return CacheType::Instruction;
   else if (boost::iequals(type, "Unified"))
      return CacheType::Unified;
   else
      return CacheType::Other;
}

std::string CacheInfo::toString(CacheType type) {
    switch(type) {
       case Data:
          return "Data";
       case Instruction:
          return "Instruction";
       case Unified:
          return "Unified";
       case Other:
          return "Other";
       default:
          assert(false);
          return "";
    }
}

unsigned CacheInfo::getSizeInKiB() const {
   return sizeInBytes / 1024;
}


/*
1.3 CacheInfos class

*/
CacheInfos::CacheInfos() {
   const string INFO_PATH_CPU_ ="/sys/devices/system/cpu/cpu";
   const string INFO_PATH_CPU0_CACHE_ = INFO_PATH_CPU_ + "0/cache/index";

   cpuCount = 0;
   while (directoryExists(INFO_PATH_CPU_ + to_string(cpuCount)))
       ++cpuCount;

   int index = 0;
   while (directoryExists(INFO_PATH_CPU0_CACHE_ + to_string(index))) {
      const string path = INFO_PATH_CPU0_CACHE_ + to_string(index);
      infos.push_back(make_shared<CacheInfo>(path));
      ++index;
   }
}

CacheInfos& CacheInfos::getOnlyInstance() {
   static CacheInfos onlyInstance;
   return onlyInstance;
}

const CacheInfoPtr CacheInfos::getCacheInfo(const CacheType type,
        const unsigned level) {
   const CacheInfos& cacheInfos = getOnlyInstance();
   for (const CacheInfoPtr& info : cacheInfos.infos) {
      if (info->level == level &&
              (info->type == type || info->type == CacheType::Unified))
         return info;
   }
   return nullptr;
}

void CacheInfos::report(std::ostream& out) {
   out << setfill(' ');
   out << endl << "Caches:" << endl;
   const CacheInfos& cacheInfos  = getOnlyInstance();
   for (const CacheInfoPtr& info : cacheInfos.infos)
      cacheInfos.report(info, out);
}

void CacheInfos::report(const CacheInfoPtr& info, std::ostream& out) const {
   // example output, cp. http://www.cpu-world.com/CPUs/Core_i7/
   //                     Intel-Core%20i7-3630QM%20Mobile%20processor.html :
   // L1         Data: 4 x   32 KB,   64 sets,  8-way, 64 bytes line size, *
   // L1  Instruction: 4 x   32 KB,   64 sets,  8-way, 64 bytes line size, *
   // L2      Unified: 4 x  256 KB,  512 sets,  8-way, 64 bytes line size, *
   // L3      Unified: 1 x 6144 KB, 8192 sets, 12-way, 64 bytes line size, *
   // (* = "physical line partition 1" for all)

   out << "* L" << info->level << " "
       << setw(12) << CacheInfo::toString(info->type) << ": "
       << cpuCount / std::max(info->sharedCpuCount, 1U) << " x "
       << setw(4) << info->getSizeInKiB() << " KB, "
       << setw(4) << info->numberOfSets << " sets, "
       << setw(2) << info->waysOfAssociativity << "-way, "
       << info->coherencyLineSize << " bytes line size, "
       << "physical line partition " << info->physicalLinePartition << endl;
}

