/*
1 Cache information retrieval

*/

#pragma once

#include <memory>
#include <vector>
#include <ostream>

namespace cdacspatialjoin {

/*
1.1 CacheType enum

Determines whether a cache is used for Data, for Instructions,
or both (Unified).

*/

enum CacheType {
   Data,
   Instruction,
   Unified,
   Other // for unexpected values
};


/*
1.2 CacheInfo struct

Provides information on a single cache, e.g. a L1 Data Cache that is shared
by the logical CPUs 0 and 1. Use CacheInfos::getCacheInfo(...) to get access
to the CacheInfo instances.

*/
struct CacheInfo {
   /* the cache line size in bytes */
   const unsigned coherencyLineSize;

   /* the cache level, typically 1, 2, or 3 */
   const unsigned level;

   /* the number of sets (e.g., 64 for L1, 512 for L2, 8192 for L3) */
   const unsigned numberOfSets;

   /* the physical line partition */
   const unsigned physicalLinePartition;

   /* a list of the logical CPUs that share this cache, e.g. "0-1" for a
    * cache that is shared by logical CPUs 0 and 1 */
   const std::string sharedCpuList;

   /* a bitmap of the CPUs that share this cache, e.g. 0b00000011 */
   const unsigned sharedCpuMap;

   /* the size of the cache in bytes (e.g. 32768 for a 32 KB L1 cache) */
   const unsigned sizeInBytes;

   /* the cache type (Data, Instruction or Unified) */
   const CacheType type;

   /* the associativity, e.g. 8 for L1 and L2, 12-way for L3 */
   const unsigned waysOfAssociativity;

   /* the number of CPUs that share this cache (derived from sharedCpuMap) */
   unsigned sharedCpuCount;

   // -----------------------------------------------------

   /* constructor expects the path under which cache information is found */
   explicit CacheInfo(const std::string& path);

   ~CacheInfo() = default;

   /* returns the cache size in KiB, e.g. 32 for L1, 256 for L2, 6144 for L3 */
   unsigned getSizeInKiB() const;

   /* returns a string representation of the given cache type, e.g. "Data",
    * "Instruction" or "Unified" (for both data and instruction) */
   static std::string toString(CacheType type);

private:
   /* returns the first line from the given text file */
   static std::string readString(const std::string& path, std::string file);

   /* reads the first line of the given text file and returns the unsigned
    * int value that is assumed to be at the beginning of this line */
   static unsigned readUnsigned(const std::string& path, std::string file,
           int base = 10);

   /* reads the first line of the given text file and returns the memory size
    * value that is assumed to be at the beginning of this line. If the line
    * contains "32K", 32768 is returned etc.  */
   static unsigned readSizeInBytes(const std::string& path, std::string file);

   /* reads the first line of the given text file, compares it with the
    * known cache types "Data", "Instruction" and "Unified", and returns the
    * corresponding enumeration value */
   static CacheType readCacheType(const std::string& path, std::string file);
};

typedef std::shared_ptr<CacheInfo> CacheInfoPtr;

/*
1.3 CacheInfos class

Provides information on the various caches on this computer.

*/
class CacheInfos {
   // -----------------------------------------------------
   // static members and functions

   static CacheInfos& getOnlyInstance();

public:
   /* returns the CacheInfo for the cache of the given type and level.
    * If "Data" or "Instruction" is passed as the type, a "Unified" cache
    * may be returned (check the "type" field of the returned instance).
    * Typically, level can be 1, 2, or 3. If no cache with the given parameters
    * exists, nullptr is returned. */
   static const CacheInfoPtr getCacheInfo(CacheType type, unsigned level);

   /* reports a summary of all caches on this computer to the given output
    * stream */
   static void report(std::ostream& out);

   // -----------------------------------------------------
   // instance members and functions

private:
   /* the number of logical(!) cpus on this machine */
   unsigned cpuCount;

   /* the cache information for cpu0. It is assumed that all other cpus have
    * similar information */
   std::vector<CacheInfoPtr> infos;

   CacheInfos();

public:
   ~CacheInfos() = default;

private:
   /* reports a summary of the given cache to the given output stream */
   void report(const CacheInfoPtr& info, std::ostream& out) const;
};

} // end of namespace cdacspatialjoin