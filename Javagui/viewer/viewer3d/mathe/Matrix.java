//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package viewer.viewer3d.mathe;

/*****************************************
*  Matrix
*
*  Autor   : Thomas Behr
*  Version : 1.0
*  Datum   : 22.6.1999
*
*******************************************/

public class Matrix {

/** the entries of the matrix */
private double Content[][];
/** number of rows */
private int rows;
/** number of columns */
private int columns;


/**
 * creates a new matrix
 * @param x : number of rows
 * @param y : number of columns
 */
public Matrix(int x,int y) {
    Content = new double[x][y];
    rows = x;
    columns = y;
    int i,j;

    for(i=0;i<rows;i++)
      for(j=0;j<columns;j++)
        setValue(i,j,0.0);
   }

/** returns the entry on (x,y) */
 public final double getValue(int row, int column) {
   return Content[row][column];
 }

/** set the entry [x,y] to value */
 public final void setValue(int row, int column, double value) {
   // Fehlerbehandlung
   Content[row][column] = value;
 }

/**
 * equalize this to M
 */
 public final void equalize(Matrix M) {
   int i;
   int j;
   if ((rows!=M.rows) | (columns!=M.columns))
      { Content = new double[M.rows][M.columns];
      }


   for(i=0;i<M.rows;i++)
     for(j=0;j<M.columns;j++)
       setValue(i,j,M.getValue(i,j));
  }


/** returns a copy of this */
public Matrix duplicate() {
  Matrix newM;
  newM = new Matrix(rows,columns);
  newM.equalize(this);
  return newM;
  }


/** check for equality from this and M */
public boolean equals(Matrix M) {
   boolean Test;
   Test = (rows==M.rows) & (columns==M.columns);
   if (Test){
     int i;
     int j;
     for(i=0;i<rows;i++)
       for(j=0;j<columns;j++)
         Test &= getValue(i,j)==M.getValue(i,j);
    }
    return Test;
  }



/**
  * add this to Summand
  * number of rows and cols from this and Summand must be equal
  */
public final Matrix add(Matrix Summand) {
   int i;
   int j;
   Matrix Sum = new Matrix(rows,columns);

   for(i=0;i<rows;i++)
    for(j=0;j<columns;j++)
      Sum.setValue(i,j, getValue(i,j) + Summand.getValue(i,j));

   return Sum;
}

/**
 * computes the difference between this and Subtrahend
 * number of cols and rows from this and Subtrahend must be equal
 */
 public final Matrix difference(Matrix Subtrahent) {
   int i;
   int j;
   Matrix Difference = new Matrix(rows,columns);

   for(i=0;i<rows;i++)
    for(j=0;j<columns;j++)
      Difference.setValue(i,j, getValue(i,j) + Subtrahent.getValue(i,j));

   return Difference;
 }


/**
 * multiple this with K per element
 */
 public Matrix mul(double K) {
  Matrix M = new Matrix(rows,columns);
  int i;
  int j;
  for(i=0;i<rows;i++)
    for(j=0;j<columns;j++)
      M.setValue(i,j, getValue(i,j)*K);
  return M;
  }


/**
 * computes product from this and M;
 * number of columns of this must be equal to the number
 * of rows of M and vice versa
 */
public Matrix mul(Matrix M) {
   // Fehler abfangen
   // this.columns <> M.rows
   Matrix Product = new Matrix(rows,M.columns);
   int i;
   int k;
   int j;
   double Sum;


      for(i=0;i<rows;i++)
        for(k=0;k<M.columns; k++) {
           Sum=0.0;
           for(j=0;j<columns;j++) {
             Sum += this.getValue(i,j)*M.getValue(j,k);
            }
           Product.setValue(i,k,Sum);
         }

    return Product;
  }


/** returns a readable representation of this matrix */
public String toString() {
     int i;
     int j;
     String result = new String();
     result = "";

     for(i=0;i<rows;i++) {
       for(j=0;j<columns;j++){
         result += getValue(i,j)+"  ";
       }
       result += "\n";
     }  
     return result;
 }


/**
  * computes the determinante of this matrix;
  * number of cols and rows must be equal
  */
public double Determinante() {
  int     dia,index,gz,index2;
  double  l,det;
  boolean swap,ok;

  ok     = true;
  swap = false;

  for(dia=0;dia<rows-1;dia++) {
    if (ok) {
        gz = dia;
        for(index=dia+1;index<columns;index++) {
           if (Math.abs(getValue(index,dia))>Math.abs(getValue(gz,dia))) {
              gz = index;
           }
         }

      // gz ist jetzt rowsnr. mit Groeátem Pivotelement *)

          if (gz!=dia) {
            swapRows(dia,gz);
            swap=!(swap);
          }

          if (getValue(dia,dia)==0.0 )   {
             ok = false;
          }

          if (ok) {

              for(index=dia+1;index<columns;index++) {
                 l = getValue(index,dia)/getValue(dia,dia);
                 for(index2=dia;index2<columns;index2++) {
                    setValue(index,index2,getValue(index,index2)-l*getValue(dia,index2));
                 } // for
              } // for
           } // if
       } // if
   } // for

   det = 1.0;
   for(index=0;index<columns;index++) {
       det = det * getValue(index,index);
   }

   if (swap) {
      det = -det;
   }

   return det;

}


/**
  * swap comumn s1 whith column s2
  */
private final void swapColumns(int s1, int s2) {
  // Fehler abfangen
  // s1<0 oder s2<0 oder s1>=columns oder s2>= columns
  double hilf;

  for(int i=0;i<rows;i++) {
      hilf =getValue(i,s1);
      setValue(i,s1,getValue(i,s2));
      setValue(i,s2,hilf);
    }
  }


/** swap row z1 with z2 */
private final void swapRows(int z1, int z2) {
   // Fehler abfangen
   // z1|z1 < 0 oder z1|z2 >=rows

   double hilf;
   for(int i=0; i<columns; i++) {
      hilf = getValue(z1,i);
      setValue(z1,i,getValue(z2,i));
      setValue(z2,i,hilf);
   }
 }


}
