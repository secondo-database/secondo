
//This file is part of SECONDO.

//Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science, 
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

package tools;


import java.io.*;
import viewer.hoese.LEUtils;

/*
This class can be used to write the units of a moving real
by adding measurements points. The measurements must be ordered
by time. To use this class, the following code example may be
useful:

<pre>
  UnitWriter uw = new UnitWriter(System.out);
  uw.add(3.0, 7.2);
  uw.add(2.0, 7.4);
  // ...
  uw.finish();
</pre>

Between the single values, a linear approximation is performed. 

*/

public class UnitWriter{

private PrintStream out;
private boolean first;
// is true if the first point in time is stored
private boolean initialized = false;
private boolean isComplete = false;
private double lastTime=0.0;
private double v1 = 0.0;
private double t1 = 0.0;
private double v2 = 0.0;
private double t2 = 0.0;
private double dV = 0.0;
private boolean written = false;

static double EPSILON=0.00000;

private static long writtenUnits = 0;
private static long skippedPoints = 0;



/** Creates a new UnitWriter instance which writes to the given
 * PrintStream.
 **/
public UnitWriter(PrintStream out){
   this.out = out;
   first = true;
}



/** Sets this UnitWriter to its initial state.
 **/

private void reset(){
   first = true;
   initialized = false;
   isComplete = false;
   lastTime=0.0;
   v1 = 0.0;
   t1 = 0.0;
   v2 = 0.0;
   t2 = 0.0;
   dV = 0.0;
   written = false;
   writtenUnits = 0;
   skippedPoints = 0;
}

/** Appends a value.
  * Note that in a sequence of add calls, the argument t must be 
  * ordered. 
  **/
public boolean add(double v, double t){


  if(!initialized){ // the first point
    v1 = v;
    t1 = t;
    initialized = true;
    return true;
  }

  if(t<=t1){ // time must be more than before
     return false;
  }

  if(!isComplete){ // only one point is stored
    v2 = v;
    t2 = t;
    double dt = (t2-t1);
    // we compute the change of the coordinates within 1000 milliseconds
    dV = (v2-v1)/dt;
    isComplete=true;
    return true;
  }
  // check whether the new points are a extension
  // of the existing unit
  double dt = (t-t1);
  double ev = v1 + dV*dt;

  if(Math.abs(ev-v)<=EPSILON){
     // the expected points a near enough to ignore
     // intermediate points
     t2 = t;
     v2 = v;
     skippedPoints++;
     return true;
  }
  // the new point is not a extension of the movement
  // write the old unit
  write();
  // and build a new one
  v1 = v2;
  t1 = t2;
  v2 = v;
  t2 = t;
  written = false;
  dt = (t2-t1);
  dV = (v2-v1)/dt;
  isComplete=true;
  return true;
}
 
/** Tells this UnitWriter, that the moving real creation is finished.
  * The UnitWriter writes the last unit and goes into its initial state.
  **/
public void finish(){
   write();
   out.println(")");
   reset();
}

/** Writes the currently stored  unit. **/
private boolean write(){
   
    double a,b,c;
    if(first){
       out.println("(");
       first=false;
    }
    if(!isComplete)
      return false;
    
    out.print("    ("); // open unit
    out.print("("); // open interval
    String T1 = "\""+LEUtils.convertTimeToString(t1)+"\"";
    String T2 = "\""+LEUtils.convertTimeToString(t2)+"\"";
    out.print(T1+" "+T2);
    out.print(" TRUE FALSE"); // closeness of interval
    out.print(")"); // close interval
    out.print("("); // open map

    // we assume a linear change of the altitude
    a=0.0;  // no quadratic part
   /* old version */
   //    b=(v1-v2)/(t1-t2);
   // c=v1-t1*b;
    /* new version */
    c = v1;
    b = (v2-c)/(t2-t1);
    out.print(a+" "+b+" "+c+" ");
    out.print(" FALSE"); // no square root

    out.print(")"); // close map
    out.println(")"); // close unit
    written = true;
    writtenUnits++;
    return true;
}

/** Writes some statistical information to cerr.
 **/
public static void printStatistic(){
   System.err.println("written Units  : " + writtenUnits );
   System.err.println("skipped Points : " + skippedPoints);
}

}


