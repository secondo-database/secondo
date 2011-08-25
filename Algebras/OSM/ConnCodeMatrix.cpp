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
~ConnCodeMatrix~.

For more detailed information see ConnCodeMatrix.h.

2 Defines and Includes

*/

#undef __TRACE__
//#define __TRACE__ cout <<  __FILE__ << "::" << __LINE__;
#define __TRACE__

// --- Including header-files
#include "ConnCodeMatrix.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>

// --- Constructors
// Default-Constructor
ConnCodeMatrix::ConnCodeMatrix () : m_connCode (0), m_values (NULL)
{
   initialize ();
}

// Destructor
ConnCodeMatrix::~ConnCodeMatrix ()
{
   for (int iRow = 0; iRow < 4; ++iRow)  {
      delete [] m_values[iRow];
   }
   delete [] m_values;
}

// --- Methods
void ConnCodeMatrix::initialize ()
{
   m_values = new int *[4];
   for (int iRow = 0; iRow < 4; ++iRow)  {
      m_values[iRow] = new int [4];
      for (int iCol = 0; iCol < 4; ++iCol)  {
         m_values[iRow][iCol] = 0;
      }
   }
}

void ConnCodeMatrix::setValues (const int (&values) [4][4])
{
   assert (m_values);
   for (int iRow = 0; iRow < 4; ++iRow)  {
      for (int iCol = 0; iCol < 4; ++iCol)  {
         m_values[iRow][iCol] = values[iRow][iCol];
      }
   }
}

void ConnCodeMatrix::transform ()
{
   assert (m_values);
   int result [4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
   int iRow = 0; 
   int iCol = 0; 
   for (iRow = 0; iRow < 4; ++iRow)  {
      for (iCol = 0; iCol < 4; ++iCol)  {
         if (iRow == 2 && iCol == 2)  {
            result[3][3] = m_values[2][2];
         } else if (iRow == 2 && iCol == 3)  {
            result[3][2] = m_values[2][3];
         } else if (iRow == 3 && iCol == 2)  {
            result[2][3] = m_values[3][2];
         } else if (iRow == 3 && iCol == 3)  {
            result[2][2] = m_values[3][3];
         } else if (iRow == 2)  {
            result[3][iCol] = m_values[2][iCol];
         } else if (iRow == 3)  {
            result[2][iCol] = m_values[3][iCol];
         } else if (iCol == 2) {
            result[iRow][3] = m_values[iRow][2];           
         } else if (iCol == 3) {
            result[iRow][2] = m_values[iRow][3];           
         } else  {
            result[iRow][iCol] = m_values[iRow][iCol];
         }
      }
   }
   setValues (result);
}

void ConnCodeMatrix::shrinkTo3x3Mat (int missing)
{
   assert (m_values);
   assert (missing >= 0 && missing <= 3);
   int iRow = 0; 
   int iCol = 0;
   int help = 0;
   // Moving the relevant elements to the lower right
   for (iRow = missing; iRow > 0; --iRow)  {
      for (iCol = missing; iCol > 0; --iCol)  {
         help = m_values[iRow-1][iCol-1];
         m_values[iRow-1][iCol-1] = m_values[iRow][iCol];
         m_values[iRow][iCol] = help;
      }
   }
   // Ignoring the first row and the first column
   for (iCol = 0; iCol < 4; ++iCol)  {
      m_values[0][iCol] = 0;
   }
   for (iRow = 0; iRow < 4; ++iRow)  {
      m_values[iRow][0] = 0;
   }
}

void ConnCodeMatrix::computeConnCode ()
{
   assert (m_values);
   int sum = 0;
   int i = 15;
   int elem = 0;
   for (int iRow = 0; iRow < 4; ++iRow)  {
      for (int iCol = 0; iCol < 4; ++iCol)  {
         elem = (m_values[iRow][iCol]<<i);
         sum += elem;
         --i;
      } 
   } 
   m_connCode = sum;
}

int ConnCodeMatrix::getConnCode () const
{
   //return m_connCode;
   // Temporarily returning a fix connectivity code
   return 65535;
}

void ConnCodeMatrix::print () const
{
   std::ostringstream calc;
   std::cout << "Übergangscode-Matrix:" << std::endl;
   int i = 15;
   for (int iRow = 0; iRow < 4; ++iRow)  {
      for (int iCol = 0; iCol < 4; ++iCol)  {
         std::cout << m_values[iRow][iCol];
         if (m_values[iRow][iCol] == 1)  {
            calc << "2^{" << i << "} + ";
         }
         --i;
      } 
      std::cout << std::endl;
   } 
   calc << "0";
   std::cout << "Rechnung: " << calc.str () << std::endl;
}
