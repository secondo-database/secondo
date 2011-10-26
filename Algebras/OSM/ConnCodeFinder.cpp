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
#include "BitOperations.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cassert>
#include <cstring>

// --- Constructors
// Default-Constructor
ConnCodeFinder::ConnCodeFinder ()
  : m_lowerA (0), m_higherB (0), m_higherA (0), m_lowerB (0), m_connCode (0),
  m_matrix ()
{
   // empty
}

ConnCodeFinder::ConnCodeFinder (int lowerA, int higherB, int higherA,
                                int lowerB)
  : m_lowerA (lowerA), m_higherB (higherB), m_higherA (higherA),
  m_lowerB (lowerB), m_connCode (0), m_matrix ()
{
   computeConnCode ();
}

// Destructor
ConnCodeFinder::~ConnCodeFinder ()
{
    // empty
}

// --- Methods
void ConnCodeFinder::assignSections (int sec1, int sec2, bool ow1, bool ow2,
   int *pLowerDir, int *pHigherDir, bool *pLowerOw, bool *pHigherOw)
{
   // 1: incoming, 2: outgoing
   static int INCOMING = 1;
   static int OUTGOING = 2;
   static int UNDEFINED = 0;
   int &lowerDir = (*pLowerDir);
   int &higherDir = (*pHigherDir);
   bool &lowerOw = (*pLowerOw);
   bool &higherOw = (*pHigherOw);
   if (sec1 == INCOMING && sec2 == OUTGOING)  {
      lowerDir = 1;
      higherDir = 1;
      lowerOw = ow1;
      higherOw = ow2;
   } else if (sec1 == OUTGOING && sec2 == INCOMING)  {
      lowerDir = 1;
      higherDir = 1;
      lowerOw = ow2;
      higherOw = ow1;
   } else if (sec1 == INCOMING && sec2 == UNDEFINED)  {
      lowerDir = 1;
      higherDir = 0;
      lowerOw = ow1;
      higherOw = false;
   } else if (sec1 == OUTGOING && sec2 == UNDEFINED)  {
      lowerDir = 0;
      higherDir = 1;
      lowerOw = false;
      higherOw = ow1;
   } else if (sec1 == UNDEFINED && sec2 == INCOMING)  {
      lowerDir = 1;
      higherDir = 0;
      lowerOw = ow2;
      higherOw = false;
   } else if (sec1 == UNDEFINED && sec2 == OUTGOING)  {
      lowerDir = 0;
      higherDir = 1;
      lowerOw = false;
      higherOw = ow2;
   } else {
      assert (false);
   } 
}

int ConnCodeFinder::getConnectivityCode (
   int inout1, int inout2, int inout3, int inout4, 
   bool ow1, bool ow2, bool ow3, bool ow4)
{
   assert (inout1 >= 0 && inout1 <= 2);
   assert (inout2 >= 0 && inout2 <= 2);
   assert (inout3 >= 0 && inout3 <= 2);
   assert (inout4 >= 0 && inout4 <= 2);
   // Testing if the street direction is opposite to the section direction
   // (the opposing sections can neither both be incoming nor outgoing)
   assert (!(((inout1 == 1 || inout1 == 2) && inout1 == inout3))
           && !(((inout2 == 1 || inout2 == 2) && inout2 == inout4)));

   // --- Mapping the section values to a fix coordinate system
   int dir1(0), dir2(0), dir3(0), dir4(0);
   bool oneway1(false), oneway2(false), oneway3(false), oneway4(false);
   assignSections (inout1, inout3, ow1, ow3, &dir1, &dir3, &oneway1, &oneway3);
   assignSections (inout2, inout4, ow2, ow4, &dir2, &dir4, &oneway2, &oneway4);

   // if oneway == true or dir == 0 then do nothing
   // if oneway == false and dir != 0
   //    then 00000001 -> 00000011 or 00000010 -> 00000011
   ConnCodeFinder ccFinder =
      ConnCodeFinder ((!oneway1 && dir1 != 0)? 3 : dir1,
                      (!oneway2 && dir2 != 0)? 3 : dir2,
                      (!oneway3 && dir3 != 0)? 3 : dir3,
                      (!oneway4 && dir4 != 0)? 3 : dir4);
   return ccFinder.getConnCode ();
}

void ConnCodeFinder::computeConnCode ()
{
   int matrix [4][4] =
      {{is_bit_set(getLowerA (),1)&is_bit_set(getHigherA (),1),   //AupAup
        is_bit_set(getLowerA (),1)&is_bit_set(getLowerA (),2),    //AupAdown
        is_bit_set(getLowerA (),1)&is_bit_set(getHigherB (),1),   //AupBup
        is_bit_set(getLowerA (),1)&is_bit_set(getLowerB (),2)},   //AupBdown
       {is_bit_set(getHigherA (),2)&is_bit_set(getHigherA (),1),  //AdownAup
        is_bit_set(getHigherA (),2)&is_bit_set(getLowerA (),2),   //AdownAdown
        is_bit_set(getHigherA (),2)&is_bit_set(getHigherB (),1),  //AdownBup
        is_bit_set(getHigherA (),2)&is_bit_set(getLowerB (),2)},  //AdownBdown
       {is_bit_set(getLowerB (),1)&is_bit_set(getHigherA (),1),   //BupAup
        is_bit_set(getLowerB (),1)&is_bit_set(getLowerA (),2),    //BupAdown
        is_bit_set(getLowerB (),1)&is_bit_set(getHigherB (),1),   //BupBup
        is_bit_set(getLowerB (),1)&is_bit_set(getLowerB (),2)},   //BupBdown
       {is_bit_set(getHigherB (),2)&is_bit_set(getHigherA (),1),  //BdownAup
        is_bit_set(getHigherB (),2)&is_bit_set(getLowerA (),2),   //BdownAdown
        is_bit_set(getHigherB (),2)&is_bit_set(getHigherB (),1),  //BdownBup
        is_bit_set(getHigherB (),2)&is_bit_set(getLowerB (),2)}}; //BdownBdown
   m_matrix.setValues (matrix);
   // Transforming the matrix
   //m_matrix.transform ();

   // Checking whether the connectivity code is searched for a dual and a 
   // simple street
   /*if ((getLowerA () == 3 && getHigherA () == 3) &&
       (getLowerB () == 1 && getHigherB () == 1))  {
      m_matrix.shrinkTo3x3Mat (3);
   } else if ((getLowerA () == 3 && getHigherA () == 3) &&
              (getLowerB () == 2 && getHigherB () == 2))  {
      m_matrix.shrinkTo3x3Mat (2);
   } else if ((getLowerA () == 1 && getHigherA () == 1) && 
              (getLowerB () == 3 && getHigherB () == 3))  {
      m_matrix.shrinkTo3x3Mat (1);
   } else if ((getLowerA () == 2 && getHigherA () == 2) && 
              (getLowerB () == 3 && getHigherB () == 3))  {
      m_matrix.shrinkTo3x3Mat (0);
   }*/
   // if the junction lies between two simple streets the code is ignored
   // anyway
   m_matrix.computeConnCode ();
   
   m_connCode = m_matrix.getConnCode (); 
}

int ConnCodeFinder::getConnCode () const
{
   return m_connCode;
}

void ConnCodeFinder::printVerticalArrows (int b)
{
   switch (b)  {
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

void ConnCodeFinder::printHorizontalArrows (int a)
{
   switch (a)  {
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
   for (int iLowerA = 0; iLowerA < 4; ++iLowerA)  {
      for (int iHigherB = 0; iHigherB < 4; ++iHigherB)  {
         for (int iHigherA = 0; iHigherA < 4; ++iHigherA)  {
            for (int iLowerB = 0; iLowerB < 4; ++iLowerB)  {
               ccFinder = new ConnCodeFinder (iLowerA, iHigherB, iHigherA,
                  iLowerB);
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
   return !((getLowerA () == 0 && getHigherB () == 0 && 
             getHigherA () == 0 && getLowerB () == 0) ||
            (getLowerA () == 0 && getHigherB () == 0 && getHigherA () == 0) ||
            (getLowerA () == 0 && getHigherB () == 0 && getLowerB () == 0) ||
            (getLowerA () == 0 && getHigherA () == 0 && getLowerB () == 0) ||
            (getHigherB () == 0 && getHigherA () == 0 && getLowerB () == 0));
}

int ConnCodeFinder::getLowerA () const
{
   return m_lowerA;
}

int ConnCodeFinder::getHigherB () const
{
   return m_higherB;
}

int ConnCodeFinder::getHigherA () const
{
   return m_higherA;
}

int ConnCodeFinder::getLowerB () const
{
   return m_lowerB;
}

void ConnCodeFinder::print () const
{
   printVerticalArrows (getHigherB ()); 
   printHorizontalArrows (getLowerA ());
   std::cout << "  ";
   printHorizontalArrows (getHigherA ());
   std::cout << std::endl;
   printVerticalArrows (getLowerB ());
   std::cout << std::endl;
   m_matrix.print ();
   std::cout << "Dezimalwert: " << getConnCode () << std::endl; 
}
