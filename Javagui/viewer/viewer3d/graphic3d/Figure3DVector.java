package viewer.viewer3d.graphic3d;

import java.util.*;

/*****************************
*
*  Autor   : Thomas Behr
*  Version : 1.1
*  Datum   : 16.5.2000
*
*******************************/


class Figure3DVector {

/** the intern store */
 private Vector V;

/** creates a new vector */
 public Figure3DVector() { V = new Vector(); }

/** insert a new figure at end of this vector */
 public void append(Figure3D IPoly) { V.add(IPoly); }
 
/** get the number of containing figures */
 public int getSize() { return V.size(); }


/** get the figure on position i */
 public Figure3D getFigure3DAt(int i) throws NoSuchElementException {
   try {
       return (Figure3D) V.get(i);
       }
   catch (Exception e ) { throw new NoSuchElementException(); }
 }

/** removes all figures from this vector */
 public void empty() { V = new Vector(); }

/** get the position of given figure */
 public int getIndexOf(Figure3D Poly) throws NoSuchElementException {
    return V.indexOf(Poly);
 }

/** remove given figure from this vector */
 public void delete(Figure3D Poly) throws NoSuchElementException {
     V.remove( V.indexOf(Poly));
 }

/** check for emptyness */
 public boolean isEmpty() { return V.size()==0; }


/** check for equality */
public boolean equals(Figure3DVector P) { return V.equals(P.V); }



} // class
