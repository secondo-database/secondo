package viewer.hoese.algebras.periodic;

import sj.lang.ListExpr;
public interface  Move{
  RelInterval getInterval();
  Object getObjectAt(Time T);
  boolean readFrom(ListExpr LE,Class linearClass);
  BBox getBoundingBox();
}
