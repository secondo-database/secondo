package viewer.viewer3d.graphic2d;

import gui.idmanager.*;


/*******************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
*********************************/


import java.util.* ;

/** this class provides a vector to store Figure2D-Objects */

class Figure2DVector {

/** the intern store */
private Vector V = new Vector();


/** add a new Figure2D */
public void append (Figure2D P) { V.add(P); }

/** insert P on position i */
public void insertAt(Figure2D P,int i) { V.add(i,P); }


/** insert P on position given by sort-value of P */
public void Include( Figure2D P) {
  // insert by Sort from P
  int min = 0;
  int max = V.size();
  int middle;
  double PSort = P.getSort();

  while ( (max-min) > 1 )  {
           middle = (max+min) / 2;
           if ( ((Figure2D) V.get(middle) ).getSort() > PSort )
              max = middle;
           else
              min = middle;
    } // while

  V.add(min,P);
}

/** returns number of containing figures */
public int getSize() { return V.size(); }

/** get the figure on position i */
public Figure2D getFigure2DAt(int i) throws NoSuchElementException {
  return (Figure2D) V.get(i);
}

/** deletes all containing figures */
public void empty() { V = new Vector(); }

/** check for emptyness */
public boolean isEmpty() { return V.size()==0; }


/** search the figure with given ID from begin */
private int SearchID(ID SID, int Begin ) {
  if (Begin >= V.size() ) return -1;
  for (int i=Begin; i<V.size(); i++) {
    if (SID.equals (( (Figure2D) V.get(i)).getID() )  )
        return i;
  }
  return -1;
}


/** deletes all figures with given ID */
public void deleteFiguresWithID(ID PID)  {

   // removes all Figures whith PID

  int i = 0;
  int max = V.size();

  while (i<max) {
    if ( ( (Figure2D) V.get(i) ).getID().equals(PID) ) {
       V.remove(i);
       max --;
      }
      else i++;
  }
}
          


} // class
