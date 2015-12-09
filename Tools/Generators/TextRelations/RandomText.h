/*
November 2009, S.Jungnickel

This file contains functions for random text generation.

*/   

#include "math.h"


/*
Creates a random string of length ~l~. Possible characters are
[0-9][A-Z][a-z] 

*/

string randomText(int l)
{
  string s;

  for (int j = 0; j < l; j++) 
  { 
    int c;

    switch ( rand() % 3 )
    {
      case 0:
  c = rand() % 10 + 48; // 0-9
  break;
      case 1:
  c = rand() % 26 + 65; // A-Z
  break;
      case 2:
  c = rand() % 26 + 97; // a-z
  break;
    }

    s.push_back(c);
  }

  return s;
}


