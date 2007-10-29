package movingregion;

/**
  * This class represents an edge in the overlap graph. The overlap graph
  * is a collection of edges between convex hull tree nodes on the same
  * level of two convex hull trees. One edge represents the fact that the
  * first node overlaps the second. Overlap graph edges do not have their
  * own constructor. They are created as a result of function calls in
  * <code>ConvexHullTreeNode</code>
  *
  * @author Erlend Tøssebro
  */
public class OverlapGraphEdge implements Comparable{
  public double overlap;
  public ConvexHullTreeNode overlapswith;

  /**
    * This function compares two overlap graph edges with respect to how
    * high the overlap is. It is the implementation
    * of the <code>comparable</code> interface.
    *
    * @param o The object to compare this object to.
    *
    * @return -1 if this object is smaller, 0 if the two are equal and 1
    *         if this object is larger.
    */
  public int compareTo(Object o) {
    OverlapGraphEdge oge;
    oge = (OverlapGraphEdge)o;
    if (overlap < oge.overlap) return(-1);
    if (overlap == oge.overlap) return(0);
    return(1);
  }
}
