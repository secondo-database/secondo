package viewer.viewer3d.graphic3d;

import java.util.*;

/************************
*
* Autor   : Thomas Behr
* Version : 1.1
* Datum   : 16.5.2000
*
***************************/



public class Triangle3DVector {

/** the intern store */
private Vector V = new Vector();

/** add a new triangle to this vector */
public void append(Triangle3D Q) { V.add(Q); }

/** removes all triangles from this vector */
public void empty() { V = new Vector(); }

/** check for emptyness */
public boolean isEmpty() { return V.size() == 0; }

/** get the number of containing triangles */
public int getSize() { return V.size(); }

/** get the triangle on position i */
public Triangle3D getTriangle3DAt( int i) throws IndexOutOfBoundsException {

  if ( (i<0) || (i>=V.size()) ) throw new IndexOutOfBoundsException();

  return (Triangle3D) V.get(i); 

}

public Triangle3D get(int i){
  return getTriangle3DAt(i);
}

/** remove the triangle on position i */
public void remove(int i) throws IndexOutOfBoundsException {
   if ( (i<0) || (i>=V.size()) ) throw new IndexOutOfBoundsException();  
   V.remove(i);
}


}
