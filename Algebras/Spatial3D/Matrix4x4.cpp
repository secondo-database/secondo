/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[NP] [\newpage]
//[ue] [\"u]
//[e] [\'e]

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

Jens Breit, Joachim Dechow, Daniel Fuchs, Simon Jacobi, G[ue]nther Milosits, 
Daijun Nagamine, Hans-Joachim Klauke.

Betreuer: Dr. Thomas Behr, Fabio Vald[e]s


[1] Implementation of an auxiliary class Matrix4x4

[TOC]

[NP]

1 Includes and Defines

*/


#include <cmath>
#include <iostream> 
#include "Matrix4x4.h"

using namespace std;

namespace spatial3DTransformations {

/*
2 Some auxiliary functions

2.1 ~CoutMatrix~

Writes the values of the matrix on the screen.
 
*/
    
  void Matrix4x4::coutMatrix(){
    cout << endl;
    cout << this->values[0][0];
    cout << " ";
    cout << this->values[0][1];
    cout << " ";
    cout << this->values[0][2];
    cout << " ";
    cout << this->values[0][3];
    cout << endl;
    cout << this->values[1][0];
    cout << " ";
    cout << this->values[1][1];
    cout << " ";
    cout << this->values[1][2];
    cout << " ";
    cout << this->values[1][3];
    cout << endl;
    cout << this->values[2][0];
    cout << " ";
    cout << this->values[2][1];
    cout << " ";
    cout << this->values[2][2];
    cout << " ";
    cout << this->values[2][3];
    cout << endl;
    cout << this->values[3][0];
    cout << " ";
    cout << this->values[3][1];
    cout << " ";
    cout << this->values[3][2];
    cout << " ";
    cout << this->values[3][3];
    cout << endl;
    cout << endl;
  };
  
/*
2.1 ~SetTestMatrix~

Creates a matrix for testing.
 
*/
  void Matrix4x4::SetTestMatrix(Matrix4x4* matrix){
    matrix->values[0][0] = 1;
    matrix->values[0][1] = 2;
    matrix->values[0][2] = 3;
    matrix->values[0][3] = 4;
    matrix->values[1][0] = 5;
    matrix->values[1][1] = 6;
    matrix->values[1][2] = 7;
    matrix->values[1][3] = 8;
    matrix->values[2][0] = 9;
    matrix->values[2][1] = 10;
    matrix->values[2][2] = 11;
    matrix->values[2][3] = 12;
    matrix->values[3][0] = 13;
    matrix->values[3][1] = 14;
    matrix->values[3][2] = 15;
    matrix->values[3][3] = 16;
  };
  
/*
2.1 ~Multiply~

Multiplys a matrix with an other matrix.
 
*/
  void Matrix4x4::Multiply(Matrix4x4* matrix)
  {
    for(int i = 0; i < 4; i++){
      double raw0 =  values[i][0] * matrix->values[0][0]
          + values[i][1] * matrix->values[1][0]
          + values[i][2] * matrix->values[2][0]
          + values[i][3] * matrix->values[3][0];
      double raw1 =  values[i][0] * matrix->values[0][1]
          + values[i][1] * matrix->values[1][1]
          + values[i][2] * matrix->values[2][1]
          + values[i][3] * matrix->values[3][1];
      double raw2 =  values[i][0] * matrix->values[0][2]
          + values[i][1] * matrix->values[1][2]
          + values[i][2] * matrix->values[2][2]
          + values[i][3] * matrix->values[3][2];
      double raw3 =  values[i][0] * matrix->values[0][3]
          + values[i][1] * matrix->values[1][3]
          + values[i][2] * matrix->values[2][3]
          + values[i][3] * matrix->values[3][3];
      values[i][0] = raw0;
      values[i][1] = raw1;
      values[i][2] = raw2;
      values[i][3] = raw3;
    }
  };
  
/*
3 Functions for the affine transformations

3.1 ~GetShiftMatrix~

Creates a matrix for shifting with a vector.
 
*/
  Matrix4x4* Matrix4x4::GetShiftMatrix(
    double shiftX, double shiftY, double shiftZ)
  {
    Matrix4x4* returnMatrix = new Matrix4x4();
    
    returnMatrix->values[0][0] = 1;
    returnMatrix->values[1][1] = 1;
    returnMatrix->values[2][2] = 1;
    returnMatrix->values[3][3] = 1;

    returnMatrix->values[0][3] = shiftX;
    returnMatrix->values[1][3] = shiftY;
    returnMatrix->values[2][3] = shiftZ;
    return returnMatrix;
  };  
  
/*
3.2 ~GetScaleMatrix~

Creates a matrix for scaling with a vector.
 
*/
  Matrix4x4* Matrix4x4::GetScaleMatrix(
    double scaleX, double scaleY, double scaleZ)
  {
    Matrix4x4* returnMatrix = new Matrix4x4();
    
    returnMatrix->values[0][0] = scaleX;
    returnMatrix->values[1][1] = scaleY;
    returnMatrix->values[2][2] = scaleZ;
    returnMatrix->values[3][3] = 1;
    return returnMatrix;
  };
  
/*
3.3 ~GetXRotationMatrix~, ~GetYRotationMatrix~,~GetYRotationMatrix~
~GetRotationInOriginWithUnitVectorMatrix~

Some auxiliary function for rotation.
 
*/
  Matrix4x4* Matrix4x4::GetXRotationMatrix(double phi)
  {
    Matrix4x4* returnMatrix = new Matrix4x4();
    
    returnMatrix->values[0][0] = 1;
    returnMatrix->values[1][1] = cos(phi);
    returnMatrix->values[1][2] = -sin(phi);
    returnMatrix->values[2][1] = sin(phi);
    returnMatrix->values[2][2] = cos(phi);
    returnMatrix->values[3][3] = 1;
    return returnMatrix;
  };

  Matrix4x4* Matrix4x4::GetYRotationMatrix(double phi)
  {
    Matrix4x4* returnMatrix = new Matrix4x4();
    
    returnMatrix->values[0][0] = cos(phi);
    returnMatrix->values[1][1] = 1;
    returnMatrix->values[2][2] = cos(phi);
    returnMatrix->values[3][3] = 1;
    returnMatrix->values[2][0] = -sin(phi);
    returnMatrix->values[0][2] = sin(phi);
    return returnMatrix;
  };

  Matrix4x4* Matrix4x4::GetZRotationMatrix(double phi)
  {
    Matrix4x4* returnMatrix = new Matrix4x4();
    
    returnMatrix->values[0][0] = cos(phi);
    returnMatrix->values[1][1] = cos(phi);
    returnMatrix->values[2][2] = 1;
    returnMatrix->values[3][3] = 1;
    returnMatrix->values[0][1] = -sin(phi);
    returnMatrix->values[1][0] = sin(phi);
    return returnMatrix;
  };
    
  Matrix4x4* Matrix4x4::GetRotationInOriginWithUnitVectorMatrix(
    double nX, double nY,double nZ,double phi)
  {
    
    Matrix4x4* returnMatrix = new Matrix4x4();
   
    returnMatrix->values[0][0] = nX * nX * (1-cos(phi)) + cos(phi);
    returnMatrix->values[0][1] = nX * nY * (1-cos(phi)) - nZ * sin(phi);
    returnMatrix->values[0][2] = nX * nZ * (1-cos(phi)) + nY * sin(phi);
    returnMatrix->values[1][0] = nX * nY * (1-cos(phi)) + nZ * sin(phi);
    returnMatrix->values[1][1] = nY * nY * (1-cos(phi)) + cos(phi);
    returnMatrix->values[1][2] = nY * nZ * (1-cos(phi)) - nX * sin(phi);
    returnMatrix->values[2][0] = nX * nZ * (1-cos(phi)) - nY * sin(phi);
    returnMatrix->values[2][1] = nY * nZ * (1-cos(phi)) + nX * sin(phi);
    returnMatrix->values[2][2] = nZ * nZ * (1-cos(phi)) + cos(phi);
    returnMatrix->values[3][3] = 1;

    return returnMatrix;
  };
  
/*
3.4 ~GetRotationMatrix~

Creates a matrix for rotation on a straight lines with an angle.
 
*/
  Matrix4x4* Matrix4x4::GetRotationMatrix(
    double pX, double pY,double pZ,double vX,double vY,double vZ,double phi)
  {
    Matrix4x4* shiftToOrigin = GetShiftMatrix(-pX, -pY, -pZ);

    double lenghOfVector = sqrt(vX * vX + vY * vY + vZ * vZ);
    Matrix4x4* rotate = GetRotationInOriginWithUnitVectorMatrix(
      vX / lenghOfVector,vY / lenghOfVector,vZ / lenghOfVector,phi);

    Matrix4x4* shiftToPoint = GetShiftMatrix(pX, pY, pZ);

    Matrix4x4* returnMatrix = shiftToPoint;
    returnMatrix->Multiply(rotate);
    returnMatrix->Multiply(shiftToOrigin);
    
    delete rotate;
    delete shiftToOrigin;
    
    return returnMatrix;
  };
      
/*
3.5 ~GetMirrorMatrix~

Creates a matrix for mirroring on a plane.
 
*/
  Matrix4x4* Matrix4x4::GetMirrorMatrix(
    double pX, double pY,double pZ,double vX,double vY,double vZ)
  {
    Matrix4x4* rotate1 = 0;
    Matrix4x4* rotate2 = 0;
    Matrix4x4* rotateBack2 = 0;
    Matrix4x4* rotateBack1 = 0;

    
    //build the the used matrices:
    Matrix4x4* shiftToOrigin = GetShiftMatrix(-pX, -pY, -pZ);

    //The plane is not the z=0 Plane
    if((vX * vX + vY * vY) > 0){
      double cosPhi1 = vX / sqrt(vX * vX + vY * vY);
      double phi1 = acos(cosPhi1);
      if((vX > 0.0 && vY > 0.0) || (vX < 0.0 && vX < 0.0)){
        phi1 = -phi1;
      }
      rotate1 = GetZRotationMatrix(phi1);

      //now the normalvector of the plane has y = 0 
      double cosPhi2 = vZ / sqrt(vX * vX + vY * vY + vZ * vZ);
      double phi2 = acos(cosPhi2);
      if((vZ > 0.0 && vX > 0.0) || (vZ < 0.0 && vX < 0.0)){
        phi2 = -phi2;
      }
      rotate2 = GetYRotationMatrix(phi2);
      //now the normalvector of the plane has x = 0 

      rotateBack2 = GetYRotationMatrix(-phi2);
      rotateBack1 = GetZRotationMatrix(-phi1);
    }
    //mirror on the z=0 plane
    Matrix4x4* scale = GetScaleMatrix(1,1,-1);
    Matrix4x4* shiftBack = GetShiftMatrix(pX, pY, pZ);

    //Build the Transformation:
    //move the plane to the orgin:
    Matrix4x4* returnMatrix = shiftBack;
    //when the plane is not the z=0 Plane rotate it:
    if((vX * vX + vY * vY) > 0){
      returnMatrix->Multiply(rotateBack1);
      returnMatrix->Multiply(rotateBack2);
    }
    //mirror on the z=0 plane
    returnMatrix->Multiply(scale);
    //rotate back:
    if((vX * vX + vY * vY) > 0){
      returnMatrix->Multiply(rotate2);
      returnMatrix->Multiply(rotate1);
    }
    //shift back:
    returnMatrix->Multiply(shiftToOrigin);

    if(rotate1 != 0){
      delete rotate1;
    }
    if(rotate2 != 0){
      delete rotate2;
    }
    if(rotateBack1 != 0){
      delete rotateBack1;
    }
    if(rotateBack2 != 0){
      delete rotateBack2;
    }
    delete scale;
    delete shiftToOrigin;
    
    return returnMatrix;
  };    
    
/*
3.6 ~GetTranslateMatrix~

Creates a matrix for translation with an vector.
 
*/
  Matrix4x4* Matrix4x4::GetTranslateMatrix(
    double vX,double vY,double vZ)
  {
    Matrix4x4* returnMatrix = GetShiftMatrix(vX, vY, vZ);
    return returnMatrix;
  };
    
/*
3.7 ~GetScaleDirMatrix~

Creates a matrix for scaling with a vector from a point.
 
*/
  Matrix4x4* Matrix4x4::GetScaleDirMatrix(
    double pX,double pY,double pZ,double vX,double vY,double vZ)
  {
    Matrix4x4* shiftToOrigin = GetShiftMatrix(-pX, -pY, -pZ);
    Matrix4x4* scale = GetScaleMatrix(vX, vY, vZ);
    Matrix4x4* shiftToPoint = GetShiftMatrix(pX, pY, pZ);

    Matrix4x4* returnMatrix = shiftToPoint;
    returnMatrix->Multiply(scale);
    returnMatrix->Multiply(shiftToOrigin);
    delete scale;
    delete shiftToOrigin;
    
    return returnMatrix;
  };

    
/*
3.7 ~GetScaleMatrix~

Creates a matrix for scaling with a factor from a point.
 
*/
  Matrix4x4* Matrix4x4::GetScaleMatrix(
    double pX,double pY,double pZ, double scaleFactor)
  {
    Matrix4x4* shiftToOrigin = GetShiftMatrix(-pX, -pY, -pZ);
    Matrix4x4* scale = GetScaleMatrix(scaleFactor,scaleFactor,scaleFactor);
    Matrix4x4* shiftToPoint = GetShiftMatrix(pX, pY, pZ);

    Matrix4x4* returnMatrix = shiftToPoint;
    returnMatrix->Multiply(scale);
    returnMatrix->Multiply(shiftToOrigin);
    delete scale;
    delete shiftToOrigin;
    
    return returnMatrix;
  };
} 
