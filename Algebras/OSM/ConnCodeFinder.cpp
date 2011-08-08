/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the OSM Algebra

June-November, 2011. Thomas Uchdorf

[TOC]

1 Overview

This implementation file contains the implementation of the class
~ConnCodeFinder~.

For more detailed information see ConnCodeFinder.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "ConnCodeFinder.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cassert>
#include <cstring>

#define is_bit_set(number,n) \
   ((((number) & (1 << ((n)-1))) != 0)? 1 : 0)

// --- Constructors
// Default-Constructor
ConnCodeFinder::ConnCodeFinder ()
{
   // empty
}

// Destructor
ConnCodeFinder::~ConnCodeFinder ()
{
    // empty
}

// --- Methods
int ConnCodeFinder::getConnectivityCode (
   int dir1, int dir2, int dir3, int dir4, 
   bool ow1, bool ow2, bool ow3, bool ow4)
{
   assert (dir1 >= 0 && dir1 <= 2);
   assert (dir2 >= 0 && dir2 <= 2);
   assert (dir3 >= 0 && dir3 <= 2);
   assert (dir4 >= 0 && dir4 <= 2);
   // if oneway == true or dir == 0 then do nothing
   // if oneway == false and dir != 0
   //    then 00000001 -> 00000011 or 00000010 -> 00000011
   int iWest = (!ow1 && dir1 != 0)? 3 : dir1;
   int iNorth = (!ow2 && dir2 != 0)? 3 : dir2;
   int iEast = (!ow3 && dir3 != 0)? 3 : dir3;
   int iSouth = (!ow4 && dir4 != 0)? 3 : dir4;
   int help [4][4] = {{is_bit_set(iWest,1)&is_bit_set(iEast,1),
                      is_bit_set(iWest,1)&is_bit_set(iWest,2),
                      is_bit_set(iWest,1)&is_bit_set(iNorth,1),
                      is_bit_set(iWest,1)&is_bit_set(iSouth,2)},
                     {is_bit_set(iEast,2)&is_bit_set(iEast,1),
                      is_bit_set(iEast,2)&is_bit_set(iWest,2),
                      is_bit_set(iEast,2)&is_bit_set(iNorth,1),
                      is_bit_set(iEast,2)&is_bit_set(iSouth,2)},
                     {is_bit_set(iSouth,1)&is_bit_set(iEast,1),
                      is_bit_set(iSouth,1)&is_bit_set(iWest,2),
                      is_bit_set(iSouth,1)&is_bit_set(iNorth,1),
                      is_bit_set(iSouth,1)&is_bit_set(iSouth,2)},
                     {is_bit_set(iNorth,2)&is_bit_set(iEast,1),
                      is_bit_set(iNorth,2)&is_bit_set(iWest,2),
                      is_bit_set(iNorth,2)&is_bit_set(iNorth,1),
                      is_bit_set(iNorth,2)&is_bit_set(iSouth,2)}}; 

   // --- Transforming the matrix (Up in horizontal direction actually
   //     means to the left and down in horizontal direction means to the
   //     right. So, the 3rd row and the 4th row have to be swapped. The
   //     same applies to the according columns.)
   // ------------------------------------------
   // |        |A_{up}|A_{down}|B_{up}|B_{down}|
   // ------------------------------------------
   // |A_{up}  |      |        |      |        |
   // ------------------------------------------
   // |A_{down}|      |        |      |        |
   // ------------------------------------------
   // |B_{up}  |      |        |      |        |
   // ------------------------------------------
   // |B_{down}|      |        |      |        |
   // ------------------------------------------
   //
   //                          |
   //                          |
   //                          V
   //
   // ------------------------------------------
   // |        |A_{up}|A_{down}|B_{down}|B_{up}|
   // ------------------------------------------
   // |A_{up}  |      |        |        |      |
   // ------------------------------------------
   // |A_{down}|      |        |        |      |
   // ------------------------------------------
   // |B_{down}|      |        |        |      |
   // ------------------------------------------
   // |B_{up}  |      |        |        |      |
   // ------------------------------------------
   int mat [4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
   int iRow = 0; 
   int iCol = 0; 
   for (iRow = 0; iRow < 4; ++iRow)  {
      for (iCol = 0; iCol < 4; ++iCol)  {
         if (iRow == 2 && iCol == 2)  {
            mat[3][3] = help[2][2];
         } else if (iRow == 2 && iCol == 3)  {
            mat[3][2] = help[2][3];
         } else if (iRow == 3 && iCol == 2)  {
            mat[2][3] = help[3][2];
         } else if (iRow == 3 && iCol == 3)  {
            mat[2][2] = help[3][3];
         } else if (iRow == 2)  {
            mat[3][iCol] = help[2][iCol];
         } else if (iRow == 3)  {
            mat[2][iCol] = help[3][iCol];
         } else if (iCol == 2) {
            mat[iRow][3] = help[iRow][2];           
         } else if (iCol == 3) {
            mat[iRow][2] = help[iRow][3];           
         } else  {
            mat[iRow][iCol] = help[iRow][iCol];
         }
      }
   }

   int sum = 0;
   int i = 15;
   int elem = 0;
   //std::ostringstream calc;
   //std::cout << "Übergangscode-Matrix:" << std::endl;
   for (iRow = 0; iRow < 4; ++iRow)  {
      for (iCol = 0; iCol < 4; ++iCol)  {
         //std::cout << mat[iRow][iCol];
         //if (mat[iRow][iCol] == 1)  {
         //   calc << "2^{" << i << "} + ";
         //}
         elem = (mat[iRow][iCol]<<i);
         sum += elem;
         --i;
      } 
      //std::cout << std::endl;
   } 
   //calc << "0";
   //std::cout << "Rechnung: " << calc.str () << std::endl;
   //std::cout << "Dezimalwert: " << sum << std::endl;
   return sum;
}

void ConnCodeFinder::printAllConnectivityCodes ()
{
   //std::cout <<  "256 Fälle - 13 Fälle, in denen sich keine
   //  2 Straßen kreuzen" << std::endl;
   for (int iWest = 0; iWest < 4; ++iWest)  {
      for (int iNorth = 0; iNorth < 4; ++iNorth)  {
         for (int iEast = 0; iEast < 4; ++iEast)  {
            for (int iSouth = 0; iSouth < 4; ++iSouth)  {
                  if (!((iWest == 0 && iNorth == 0 && 
                         iEast == 0 && iSouth == 0) ||
                      (iWest == 0 && iNorth == 0 && iEast == 0) ||
                      (iWest == 0 && iNorth == 0 && iSouth == 0) ||
                      (iWest == 0 && iEast == 0 && iSouth == 0) ||
                      (iNorth == 0 && iEast == 0 && iSouth == 0))) {
                        switch (iNorth)  {
                           case (0):
                               std::cout << " ×" << std::endl; break;
                           case (1):
                               std::cout << " ↑" << std::endl; break;
                           case (2):
                               std::cout << " ↓" << std::endl; break;
                           case (3):
                               std::cout << " ↕" << std::endl; break;
                           default:
                               assert (false);
                               break;
                        }
                        switch (iWest)  {
                           case (0):
                               std::cout << "×"; break;
                           case (1):
                               std::cout << "→"; break;
                           case (2):
                               std::cout << "←"; break;
                           case (3):
                               std::cout << "↔"; break;
                           default:
                               assert (false);
                               break;
                        }
                       switch (iEast)  {
                           case (0):
                               std::cout << "  ×" << std::endl; break;
                           case (1):
                               std::cout << "  →" << std::endl; break;
                           case (2):
                               std::cout << "  ←" << std::endl; break;
                           case (3):
                               std::cout << "  ↔" << std::endl; break;
                           default:
                               assert (false);
                               break;
                        }
                        switch (iSouth)  {
                           case (0):
                               std::cout << " ×"; break;
                           case (1):
                               std::cout << " ↑"; break;
                           case (2):
                               std::cout << " ↓"; break;
                           case (3):
                               std::cout << " ↕"; break;
                           default:
                               assert (false);
                               break;
                        }
                     std::cout << std::endl;
                     int mat [4][4] = {{is_bit_set(iWest,1)&
                                        is_bit_set(iEast,1),
                                        is_bit_set(iWest,1)&
                                        is_bit_set(iWest,2),
                                        is_bit_set(iWest,1)&
                                        is_bit_set(iNorth,1),
                                        is_bit_set(iWest,1)&
                                        is_bit_set(iSouth,2)},
                                       {is_bit_set(iEast,2)&
                                        is_bit_set(iEast,1),
                                        is_bit_set(iEast,2)&
                                        is_bit_set(iWest,2),
                                        is_bit_set(iEast,2)&
                                        is_bit_set(iNorth,1),
                                        is_bit_set(iEast,2)&
                                        is_bit_set(iSouth,2)},
                                       {is_bit_set(iSouth,1)&
                                        is_bit_set(iEast,1),
                                        is_bit_set(iSouth,1)&
                                        is_bit_set(iWest,2),
                                        is_bit_set(iSouth,1)&
                                        is_bit_set(iNorth,1),
                                        is_bit_set(iSouth,1)&
                                        is_bit_set(iSouth,2)},
                                       {is_bit_set(iNorth,2)&
                                        is_bit_set(iEast,1),
                                        is_bit_set(iNorth,2)&
                                        is_bit_set(iWest,2),
                                        is_bit_set(iNorth,2)&
                                        is_bit_set(iNorth,1),
                                        is_bit_set(iNorth,2)&
                                        is_bit_set(iSouth,2)}}; 
                     int sum = 0;
                     int i = 15;
                     int elem = 0;
                     std::ostringstream calc;
                     std::cout << "Übergangscode-Matrix:" << std::endl;
                     for (int iRow = 0; iRow < 4; ++iRow)  {
                        for (int iCol = 0; iCol < 4; ++iCol)  {
                           std::cout << mat[iRow][iCol];
                           if (mat[iRow][iCol] == 1)  {
                              calc << "2^{" << i << "} + ";
                           }
                           elem = (mat[iRow][iCol]<<i);
                           sum += elem;
                           --i;
                        } 
                        std::cout << std::endl;
                     } 
                     calc << "0";
                     std::cout << "Rechnung: " << calc.str () << std::endl;
                     std::cout << "Dezimalwert: " << sum << std::endl;
                }
            }
         }
      }
   }
   return;
}
