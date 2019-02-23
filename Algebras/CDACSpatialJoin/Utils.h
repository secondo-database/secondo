/*
1 Utils class

*/

#pragma once

#include <string>

namespace cdacspatialjoin {
   /* Returns a formatted string representation of the given num, inserting
   thousands separators. */
   std::string formatInt(long num);

   /* returns the separator char used for paths on the current operating
    * system */
   inline char getPathSeparator();

   /* returns the string combined from the given directory (path1) and the
    * given subdirectory or file (path2) */
   std::string pathCombine(const std::string& path1, const std::string& path2);

   /* returns true if a directory with the given path exists in the file
    * system */
   bool directoryExists(const std::string& dir);
}
