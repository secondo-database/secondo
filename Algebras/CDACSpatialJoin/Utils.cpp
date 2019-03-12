/*
1 Utils class

*/

#include "Utils.h"
#include <ostream>
#include <sstream>
#include <sys/stat.h>
#include <cmath>

using namespace std;

namespace cdacspatialjoin {

std::string formatInt(const long num) {
   // there is no manipulator for grouping of thousands, really?
   std::stringstream result;
   size_t n;
   if (num < 0) {
      result << "-";
      n = static_cast<size_t>(-num);
   } else {
      n = static_cast<size_t>(num);
   }

   size_t div = 1;
   while (n / div >= 1000)
      div *= 1000;

   bool leadingZeros = false;
   do {
      size_t part = (n / div) % 1000;
      if (leadingZeros) {
         if (part < 10)
            result << "00";
         else if (part < 100)
            result << "0";
      }
      result << part;
      if (div == 1)
         break;
      result << "'";
      div /= 1000;
      leadingZeros = true;
   } while (true);
   return result.str();
}

std::string formatMillis(const clock_t duration) {
   stringstream result;
   const double durationSec = duration / static_cast<double>(CLOCKS_PER_SEC);
   const double durationMillis = std::round(durationSec * 1000.0);
   result << formatInt(static_cast<long>(durationMillis)) << " ms";
   return result.str();
}

inline char getPathSeparator() {
#ifdef _WIN32
   return '\\';
#else
   return '/';
#endif
}


std::string pathCombine(const std::string& path1, const std::string& path2) {
   const char separator = getPathSeparator();
   stringstream combined;
   combined << path1;
   if (   !path1.empty() && path1[path1.size() - 1] != separator
       && !path2.empty() && path2[0] != separator) {
      combined << getPathSeparator();
   }
   combined << path2;
   return combined.str();
}

bool directoryExists(const std::string& dir) {
   struct stat buffer {};
   return (stat(dir.c_str(), &buffer) == 0);
}
} // end of namespace cdacspatialjoin