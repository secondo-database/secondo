#ifndef CHAR_TRANSFORM_H
#define CHAR_TRANSFORM_H

#include <cstdlib>

/*

The functions in this file are necessary because in GCC 3.2
~toupper~ and ~tolower~ are templates. This makes it impossible
to use ~toupper~ or ~tolower~ in calls to the STL function
~transform~. Instead, the functions defined below can be used.

*/

inline char
ToUpperProperFunction(char c)
{
  return toupper(c);
};

inline char
ToLowerProperFunction(char c)
{
  return tolower(c);
}


#endif /* CHAR_TRANSFORM_H */
