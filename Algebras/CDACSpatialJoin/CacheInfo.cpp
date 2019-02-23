/*
1 CacheInfo class

*/

#include <iostream>
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
        int base /* = 10 */) {
   try {
      string line = readString(path, file);
      size_t pos;
      return static_cast<unsigned>(stoi(line, &pos, base));
   } catch(invalid_argument& ex) {
      return 0;
   }
}

unsigned CacheInfo::readSizeInBytes(const string& path, const string file) {
   try {
      string line = readString(path, file);
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
   string type = readString(path, file);
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
      auto info = new CacheInfo(path);
      infos.push_back(info);
      ++index;
   }
}

CacheInfos& CacheInfos::getOnlyInstance() {
   static CacheInfos onlyInstance;
   return onlyInstance;
}

const CacheInfo* CacheInfos::getCacheInfo(const CacheType type,
        const unsigned level) {
   CacheInfos& cacheInfos  = getOnlyInstance();
   for (const CacheInfo* info : cacheInfos.infos) {
      if (info->level == level &&
              (info->type == type || info->type == CacheType::Unified))
         return info;
   }
   return nullptr;
}

void CacheInfos::report(std::ostream& out) {
   out << "Caches available:" << endl;
   CacheInfos& cacheInfos  = getOnlyInstance();
   for (const CacheInfo* info : cacheInfos.infos)
      cacheInfos.report(info, out);
   out << endl;
}

void CacheInfos::report(const CacheInfo* info, std::ostream& out) const {
   // example output, cp. http://www.cpu-world.com/CPUs/Core_i7/
   //                     Intel-Core%20i7-3630QM%20Mobile%20processor.html :
   // L1         Data: 4 x   32 KB,   64 sets,  8-way, 64 bytes line size, *
   // L1  Instruction: 4 x   32 KB,   64 sets,  8-way, 64 bytes line size, *
   // L2      Unified: 4 x  256 KB,  512 sets,  8-way, 64 bytes line size, *
   // L3      Unified: 1 x 6144 KB, 8192 sets, 12-way, 64 bytes line size, *
   // (* = "physical line partition 1" for all)

   out << "* L" << info->level << " "
         << setw(12) << CacheInfo::toString(info->type) << ": "
         << cpuCount / info->sharedCpuCount << " x "
         << setw(4) << info->getSizeInKiB() << " KB, "
         << setw(4) << info->numberOfSets << " sets, "
         << setw(2) << info->waysOfAssociativity << "-way, "
         << info->coherencyLineSize << " bytes line size, "
         << "physical line partition " << info->physicalLinePartition << endl;
}
