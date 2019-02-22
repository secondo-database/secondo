/*
1 Utils class

*/

#include "Utils.h"
#include <ostream>
#include <sstream>

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

} // end of namespace cdacspatialjoin