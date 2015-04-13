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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

<our names here>

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of a Spatial3D algebra

[TOC]

1 Includes and Defines

*/


#ifndef _MATRIX4X4_H
#define _MATRIX4X4_H

using namespace std;

namespace spatial3DTransformations {
  /* Some helper functions for the transformations:
  * 
  */


  class Matrix4x4
  {
  public:
    double  values[4][4];
    Matrix4x4(){
      for(int i = 0; i < 4; i++){
        for(int j =0; j < 4; j++){
          values[i][j] = 0;
        }
      }
    };

    static Matrix4x4* GetRotationMatrix(
      double pX, double pY,double pZ,double vX,double vY,double vZ,double phi);

    static Matrix4x4* GetMirrorMatrix(
      double pX, double pY,double pZ,double vX,double vY,double vZ);
    
    static Matrix4x4* GetTranslateMatrix(double vX,double vY,double vZ);
    
    static Matrix4x4* GetScaleDirMatrix(
      double pX,double pY,double pZ,double vX,double vY,double vZ);
    
    static Matrix4x4* GetScaleMatrix(
      double pX,double pY,double pZ, double scale);
    void coutMatrix();

  private:  
    void Multiply(Matrix4x4* matrix);
    static Matrix4x4* GetShiftMatrix(
      double shiftX, double shiftY, double shiftZ);
    static Matrix4x4* GetScaleMatrix(
      double scaleX, double scaleY, double scaleZ);
    static Matrix4x4* GetXRotationMatrix(double phi);
    static Matrix4x4* GetYRotationMatrix(double phi);
    static Matrix4x4* GetZRotationMatrix(double phi);
    static Matrix4x4* GetRotationInOriginWithUnitVectorMatrix(
      double nX, double nY,double nZ,double phi);
    void SetTestMatrix(Matrix4x4* matrix);
  };
}
 
#endif  
