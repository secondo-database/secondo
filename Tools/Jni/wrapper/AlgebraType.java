package wrapper;

import sj.lang.ListExpr;
import java.io.Serializable;

public interface AlgebraType extends Serializable,Comparable{

   // additionally we need an constructor without any arguments
  /** returns a depth copy of this objects */
  Object copy();
  /** returns a hashValue depending of the value of this object */
  int getHashValue();
  /** Reads the value of this from  instance */
  boolean loadFrom(ListExpr typeInfo,ListExpr instance);    
  /** Returns the value of this as a nested list */
  ListExpr toListExpr(ListExpr typeInfo);
  /** Checks the type of this. */
  boolean checkType(ListExpr LE);
  
}
