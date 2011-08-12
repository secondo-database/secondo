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

// --- Constructors
// Default-Constructor
ConnCodeFinder::ConnCodeFinder ()
  : m_west (0), m_north (0), m_east (0), m_south (0), m_connCode (0),
  m_matrix ()
{
   // empty
}

ConnCodeFinder::ConnCodeFinder (int west, int north, int east, int south)
  : m_west (west), m_north (north), m_east (east), m_south (south),
  m_connCode (0), m_matrix ()
{
   computeConnCode ();
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

   // Swapping vertical directions since networks require them in opposite
   // order
   //if (dir2 == 1) { dir2 = 2; } else if (dir2 == 2) { dir2 = 1; }
   //if (dir4 == 1) { dir4 = 2; } else if (dir4 == 2) { dir4 = 1; }

   // if oneway == true or dir == 0 then do nothing
   // if oneway == false and dir != 0
   //    then 00000001 -> 00000011 or 00000010 -> 00000011
   ConnCodeFinder ccFinder =
      ConnCodeFinder ((!ow1 && dir1 != 0)? 3 : dir1,
                      (!ow2 && dir2 != 0)? 3 : dir2,
                      (!ow3 && dir3 != 0)? 3 : dir3,
                      (!ow4 && dir4 != 0)? 3 : dir4);
   return ccFinder.getConnCode ();
}

void ConnCodeFinder::computeConnCode ()
{
   int matrix [4][4] =
      {{is_bit_set(getWest (),1)&is_bit_set(getEast (),1),     //AupAup
        is_bit_set(getWest (),1)&is_bit_set(getWest (),2),     //AupAdown
        is_bit_set(getWest (),1)&is_bit_set(getNorth (),1),    //AupBup
        is_bit_set(getWest (),1)&is_bit_set(getSouth (),2)},   //AupBdown
       {is_bit_set(getEast (),2)&is_bit_set(getEast (),1),     //AdownAup
        is_bit_set(getEast (),2)&is_bit_set(getWest (),2),     //AdownAdown
        is_bit_set(getEast (),2)&is_bit_set(getNorth (),1),    //AdownBup
        is_bit_set(getEast (),2)&is_bit_set(getSouth (),2)},   //AdownBdown
       {is_bit_set(getSouth (),1)&is_bit_set(getEast (),1),    //BupAup
        is_bit_set(getSouth (),1)&is_bit_set(getWest (),2),    //BupAdown
        is_bit_set(getSouth (),1)&is_bit_set(getNorth (),1),   //BupBup
        is_bit_set(getSouth (),1)&is_bit_set(getSouth (),2)},  //BupBdown
       {is_bit_set(getNorth (),2)&is_bit_set(getEast (),1),    //BdownAup
        is_bit_set(getNorth (),2)&is_bit_set(getWest (),2),    //BdownAdown
        is_bit_set(getNorth (),2)&is_bit_set(getNorth (),1),   //BdownBup
        is_bit_set(getNorth (),2)&is_bit_set(getSouth (),2)}}; //BdownBdown
   m_matrix.setValues (matrix);
   // Transforming the matrix
   //m_matrix.transform ();

   // Checking whether the connectivity code is searched for a dual and a 
   // simple street
   if ((getWest () == 3 && getEast () == 3) &&
       (getSouth () == 1 && getNorth () == 1))  {
      m_matrix.shrinkTo3x3Mat (3);
   } else if ((getWest () == 3 && getEast () == 3) &&
              (getSouth () == 2 && getNorth () == 2))  {
      m_matrix.shrinkTo3x3Mat (2);
   } else if ((getWest () == 1 && getEast () == 1) && 
              (getSouth () == 3 && getNorth () == 3))  {
      m_matrix.shrinkTo3x3Mat (1);
   } else if ((getWest () == 2 && getEast () == 2) && 
              (getSouth () == 3 && getNorth () == 3))  {
      m_matrix.shrinkTo3x3Mat (0);
   }
   // if the junction lies between two simple streets the code is ignored
   // anyway
   m_matrix.computeConnCode ();
   
   m_connCode = m_matrix.getConnCode (); 
}

int ConnCodeFinder::getConnCode () const
{
   return m_connCode;
}

void ConnCodeFinder::printVerticalArrows (int iNorthSouth)
{
   switch (iNorthSouth)  {
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
}

void ConnCodeFinder::printHorizontalArrows (int iWestEast)
{
   switch (iWestEast)  {
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
}

void ConnCodeFinder::printAllConnectivityCodes ()
{
   ConnCodeFinder *ccFinder = NULL;
   //std::cout <<  "256 Fälle - 13 Fälle, in denen sich keine
   //  2 Straßen kreuzen" << std::endl;
   for (int iWest = 0; iWest < 4; ++iWest)  {
      for (int iNorth = 0; iNorth < 4; ++iNorth)  {
         for (int iEast = 0; iEast < 4; ++iEast)  {
            for (int iSouth = 0; iSouth < 4; ++iSouth)  {
               ccFinder = new ConnCodeFinder (iWest, iNorth, iEast, iSouth);
                  if (ccFinder->isJunctionBtwAtLeastTwoSections ()) {
                       ccFinder->print (); 
                  }
               delete ccFinder;
            }
         }
      }
   }
}

// --- Methods
bool ConnCodeFinder::isJunctionBtwAtLeastTwoSections () const
{
   return !((getWest () == 0 && getNorth () == 0 && 
             getEast () == 0 && getSouth () == 0) ||
            (getWest () == 0 && getNorth () == 0 && getEast () == 0) ||
            (getWest () == 0 && getNorth () == 0 && getSouth () == 0) ||
            (getWest () == 0 && getEast () == 0 && getSouth () == 0) ||
            (getNorth () == 0 && getEast () == 0 && getSouth () == 0));
}

int ConnCodeFinder::getWest () const
{
   return m_west;
}

int ConnCodeFinder::getNorth () const
{
   return m_north;
}

int ConnCodeFinder::getEast () const
{
   return m_east;
}

int ConnCodeFinder::getSouth () const
{
   return m_south;
}

void ConnCodeFinder::print () const
{
   printVerticalArrows (getNorth ()); 
   printHorizontalArrows (getWest ());
   std::cout << "  ";
   printHorizontalArrows (getEast ());
   std::cout << std::endl;
   printVerticalArrows (getSouth ());
   std::cout << std::endl;
   m_matrix.print ();
   std::cout << "Dezimalwert: " << getConnCode () << std::endl; 
}
