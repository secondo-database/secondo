package movingregion;
import java.awt.*;
import java.awt.geom.*;
import java.util.*;

/**
 * This class contains a number of static utility methods used by the
 * other classes.
 *
 * @author Erlend Tøssebro
 */
public class TriRepUtil
{
    public static boolean debugging=false;
    public static boolean debuggingWarnings=true;
    
    /**
     * Checks whether the points p1 and p2 are on the same side of the line
     * represented by line1 and line2.
     *
     * @param line1 The starting point of the line given as a LineWA
     * @param line2 The ending point of the line given as a LineWA
     * @param p1 The first point
     * @param p2 The second point
     *
     * @return A value indicating whether the two points are on the same side
     *         or not:<br>
     *         - less than 0: Points on different sides.<br>
     *         - equal to 0: At least one of the points is on the line.<br>
     *         - greater than 0: Points on the same side.
     */
    public static int sameSide(LineWA line1, LineWA line2, LineWA p1, LineWA p2)
    {
        int dx,dy,dx1,dx2,dy1,dy2,same;
        dx = line2.x-line1.x;
        dy = line2.y-line1.y;
        dx1 = p1.x-line1.x;
        dy1 = p1.y-line1.y;
        dx2 = p2.x-line2.x;
        dy2 = p2.y-line2.y;
        same = (dx*dy1 - dy*dx1)*(dx*dy2 - dy*dx2);
        return(same);
    }
    
    /**
     * Checks whether the points p1 and p2 are on the same side of the line
     * represented by line1 and line2.
     *
     * @param line1 The starting point of the line given as a LineWA
     * @param line2 The ending point of the line given as a LineWA
     * @param p1 The first point
     *
     * @return A value indicating whether the two points are on the same side
     *         or not:<br>
     *         - less than 0: Points on different sides.<br>
     *         - equal to 0: At least one of the points is on the line.<br>
     *         - greater than 0: Points on the same side.
     */
    public static int sameSide(LineWA line1, LineWA line2, LineWA p1)
    {
        int same;
        same = (line2.x-line1.x)*(p1.y-line1.y)-(p1.x-line1.x)*(line2.y-line1.y);
        return(same);
    }
    
    /**
     * Removes duplicate objects from the given list. It does this by first
     * sorting the list and then going through the list testing whether two
     * consecutive elements are equal using the equals() method.
     * Only works if the natural ordering is consistent with
     * equals! (That is, it doens't work with LineWA objects)
     *
     * @param list The list of object from which the function should remove
     *             duplicates.
     */
    public static void removeDuplicates(java.util.List list)
    {
        int size;
        Object[] listasarray;
        Collections.sort(list);
        listasarray = list.toArray();
        for (int a=1;a<listasarray.length;a++)
        {
            if (listasarray[a-1].equals(listasarray[a]))
            {
                list.remove(listasarray[a-1]);
            }
        }
    }
    
    /**
     * Removes duplicated without sorting. It therefore works with all
     * vectors, even if the element's natural order is not consistent with
     * equals. Its running time is O(n*n) instead of O(n*log(n)).
     *
     * @param list The list of object from which the function should remove
     *             duplicates.
     */
    public static void removeDuplicatesR(Vector list)
    {
        int a,b;
        Object element,element2;
        for (a=0;a<list.size();a++)
        {
            element = list.elementAt(a);
            for (b=a+1;b<list.size();b++)
            {
                element2 = list.elementAt(b);
                if (element2.equals(element))
                {
                    list.removeElementAt(b);
                    b--;
                }
            }
        }
    }
    
    /**
     * Computes the convex hull of a list of lines and places this at the
     * beginning of the array given in the input. Returns a new array containing
     * only the lines of the convex hull.This function uses the graham scan
     * algrithm.<p>
     * Warning: The angle stored in the elements in the input list as well as
     * the ordering of the elements in the list is changed by this function!
     * </p>
     *
     * @param lt The polygon represented as a list of lines
     *
     * @return A list of lines containing the convex hull of the input
     *         in counterclockwise order.
     */
    public static LineWA[] convexHull(LineWA[] lt)
    {
        if(TriRepUtil.debugging)
        {
            System.out.println("convexHull bekommt:");
            for(int i=0;i<lt.length;i++)
            {
                   System.out.println(lt[i]);
            }
        }
        
        int minpoint, miny, minx, tmpx, tmpy;
        Stack unfinishedhull;
        int a, index,side, counter,i;
        double hyp;
        LineWA point1, point2, tmp;
        LineWA[] finishedhull;
        
        // Test length. Any object with three or fewer points is guaranteed
        // to be convex
        if (lt.length <= 3) return(lt);
        
        // Find the point with the lowest y coordinate.
        // If several points have the minimum y-coordinate, pick the one with the
        // lowest x-coordinate.
        minpoint = 0;
        index = 0;
        miny = Integer.MAX_VALUE;
        minx = Integer.MAX_VALUE;
        unfinishedhull = new Stack();
        for (a=0;a<lt.length;a++)
        {
            if ((lt[a].y < miny) || ((lt[a].y == miny) && (lt[a].x < minx)))
            {
                miny = lt[a].y;
                minx = lt[a].x;
                index = a;
            }
        }
        // Swaps the minimum point with the first point.
        tmp = lt[0];
        lt[0] = lt[index];
        lt[index] = tmp;
        
        // Find the angles to the other points with respect to the first point.
        lt[0].angle = -1.0;// To make sure it is the least when the array is sorted
        for (a=1;a<lt.length;a++)
        {
            tmpx = lt[a].x - lt[0].x;
            tmpy = lt[a].y - lt[0].y;
            hyp = Math.sqrt(tmpx*tmpx + tmpy*tmpy);
            lt[a].angle = Math.toDegrees(Math.acos(tmpx/hyp));
        }
        
        // Sort the points with respect to the angle
        Arrays.sort(lt);
        if(TriRepUtil.debugging)
        {
            System.out.println("Sorted:");
            for(int j=0;j<lt.length;j++)
            {
                System.out.println(lt[j]);
            }
        }
        Vector tmpvec = new Vector();
        tmpvec.add(lt[0]);
        for (i = 1; i < lt.length; i++) {
            if (Math.abs(((LineWA) tmpvec.elementAt(tmpvec.size() - 1)).angle - lt[i].angle) > 0.01) {
                tmpvec.add(lt[i]);
            } else {
                tmpx = lt[0].x - lt[i].x;
                tmpy = lt[0].y - lt[i].y;
                double distli = Math.sqrt(tmpx * tmpx + tmpy * tmpy);
                tmpx = lt[0].x - ((LineWA) tmpvec.elementAt(tmpvec.size() - 1)).x;
                tmpy = lt[0].y - ((LineWA) tmpvec.elementAt(tmpvec.size() - 1)).y;
                double disttmp = Math.sqrt(tmpx * tmpx + tmpy * tmpy);
                if (distli > disttmp) {
                    tmpvec.remove(tmpvec.size() - 1);
                    tmpvec.add(lt[i]);
                }
            }
        }
        if(tmpvec.size()<3)
            return(lt);
        lt=new LineWA[tmpvec.size()];
        for (i = 0; i < tmpvec.size(); i++) {
            lt[i] = (LineWA) tmpvec.elementAt(i);
        }      
        
        if(TriRepUtil.debugging)
        {
            System.out.println("nach Löschen:");
            for(int j=0;j<lt.length;j++)
            {
                System.out.println(lt[j]);
            }
        }
        Arrays.sort(lt);
        if(TriRepUtil.debugging)
        {
            System.out.println("Sorted2:");
            for(int j=0;j<lt.length;j++)
            {
                System.out.println(lt[j]);
            }
        }
        // Use graham scan to create convex hull
        unfinishedhull = new Stack();
        // The point with the lowest y-coordinate is on the hull
        unfinishedhull.push(lt[0]);
        // The point with the lowest angle with respect to the x-axis is on the
        // hull
        unfinishedhull.push(lt[1]);
        i=2;
        int N=lt.length;
        while (i<N)
        {
            
            point1 = (LineWA)unfinishedhull.peek();
            point2 = (LineWA)unfinishedhull.elementAt(unfinishedhull.size()-2);
            if(sameSide(point1,point2,lt[i])<=0)
            {
                unfinishedhull.push(lt[i]);
                i++;
            }
            else
                unfinishedhull.pop();
            
        }
        
    /*
     
    // Inserting the next point, which might not be on the hull
    unfinishedhull.push(lt[2]);
     
    // Beginning graham scan
    for (a=3;a<lt.length;a++) {
      point1 = (LineWA)unfinishedhull.peek();
      point2 = (LineWA)unfinishedhull.elementAt(unfinishedhull.size()-2);
      i = 2;
      while (point1.equals(point2)) {
        i++;
        point2 = (LineWA)unfinishedhull.elementAt(unfinishedhull.size()-i);
      }
      side = sameSide(point1, point2, lt[0], lt[a]);
      while (side<=0) {
        counter = 1;
        while ((side == 0) && (counter < a)) {
          side = sameSide(point1, point2, lt[counter], lt[a]);
          counter++;
        }
        if (side < 0) {
          unfinishedhull.pop();
          point1 = (LineWA)unfinishedhull.peek();
          point2 = (LineWA)unfinishedhull.elementAt(unfinishedhull.size()-2);
          i = 2;
          while (point1.equals(point2)) {
            i++;
            point2 = (LineWA)unfinishedhull.elementAt(unfinishedhull.size()-i);
          }
        } else break;
        side = sameSide(point1, point2, lt[0], lt[a]);
      }
      unfinishedhull.push(lt[a]);
    }*/
        finishedhull = new LineWA[unfinishedhull.size()];
        unfinishedhull.toArray(finishedhull);
        if(TriRepUtil.debugging)
        {
            System.out.println("convexHull erzeugt:");
            for( i=0;i<finishedhull.length;i++)
            {
                System.out.println(finishedhull[i]);
            }
        }
        return(finishedhull);
    }
    
    /**
     * Finds where in an array a given object is.
     * Unlike binary search, this function does not assume that the array
     * is sorted. Uses the equals() method to test for equality unless the
     * object is null.
     *
     * @param array The array to be searched
     * @param obj The object to be searched for
     *
     * @return The index of the object in the array. If the array does not
     *         contain the object, the function returns -1.
     */
    public static int indexOf(Object[] array, Object obj)
    {
        int a;
        if (obj == null)
        {
            for (a=0;a<array.length;a++)
            {
                if (array[a] == null) return(a);
            }
        }
        else
        {
            for (a=0;a<array.length;a++)
            {
                if (obj.equals(array[a])) return(a);
            }
        }
        return(-1);
    }
    
    /**
     * Finds where in an array a given object is.
     * Unlike binary search, this function does not assume that the array
     * is sorted. Uses the equals() method to test for equality unless the
     * object is null.
     *
     * @param array The array to be searched
     * @param obj The object to be searched for
     * @param startindex The index at which the function should start
     *                   searching.
     *
     * @return The index of the object in the array. If the array does not
     *         contain the object, the function returns -1.
     */
    public static int indexOf(Object[] array, Object obj, int startindex)
    {
        int a;
        if (obj == null)
        {
            for (a=startindex;a<array.length;a++)
            {
                if (array[a] == null) return(a);
            }
        }
        for (a=startindex;a<array.length;a++)
        {
            if (obj.equals(array[a])) return(a);
        }
        return(-1);
    }
    
    /**
     * Finds the area of a polygon represented by a point list.
     *
     * @param obj The polygon represented as a LineWA array.
     *
     * @return The area of the object.
     */
    public static int findArea(LineWA[] obj)
    {
        int area;
        area = 0;
        for (int a=1;a<obj.length;a++)
        {
            area += ((obj[a-1].y + obj[a].y)/2)*(obj[a-1].x - obj[a].x);
        }
        area += ((obj[obj.length-1].y + obj[0].y)/2)*(obj[obj.length-1].x - obj[0].x);
        area = Math.abs(area);
        return(area);
    }
    
    public static double getSingleHausdorffDistance(LineWA obj1,LineWA[] obj2)
    {
        double res=(obj1.x-obj2[0].x)*(obj1.x-obj2[0].x)+(obj1.y-obj2[0].y)*(obj1.y-obj2[0].y);
        for(int i=1;i<obj2.length;i++)
        {
            res=Math.min(res,(obj1.x-obj2[i].x)*(obj1.x-obj2[i].x)+(obj1.y-obj2[i].y)*(obj1.y-obj2[i].y));
        }
        return(Math.sqrt(res));
    }
    
    public static double getHausdorfDistance(LineWA[] obj1, LineWA[] obj2)
    {
        double res=0;
        for(int i=0;i<obj1.length;i++)
        {
            res=Math.max(res,getSingleHausdorffDistance(obj1[i],obj2));
        }
        return(res);
    }
    
    public static double getSingleOverlap(LineWA[] obj1, LineWA[] obj2)
    {
        
        double areaovr;
        PathIterator path;
        Area area1, area2;
        Polygon p1, p2;
        int a, type;
        double[] coords;
        
        double initx, inity, prevx, prevy;
        
        // Creating the polygons
        p1 = new Polygon();
        for (a = 0;a<obj1.length;a++)
        {
            p1.addPoint(obj1[a].x, obj1[a].y);
        }
        p2 = new Polygon();
        for (a = 0;a<obj2.length;a++)
        {
            p2.addPoint(obj2[a].x, obj2[a].y);
        }
        
        // creating the areas from the polygons
        area1 = new Area(p1);
        area2 = new Area(p2);
        
        // Intersecting the areas
        area1.intersect(area2);
        
        // Calculating the size from the extracted shape
        if (!area1.isPolygonal())
        {
            throw(new TriRepCreationException("TriRepUtil 237: The border of the intersection area somehow does not consist only of straight lines!"));
        }
        path = area1.getPathIterator(null);
        coords = new double[6];
        areaovr = 0.0;
        initx = 0.0;
        inity = 0.0;
        prevx = 0.0;
        prevy = 0.0;
        while (!path.isDone())
        {
            type = path.currentSegment(coords);
            if (type == PathIterator.SEG_MOVETO)
            {
                initx = coords[0];
                inity = coords[1];
                prevx = initx;
                prevy = inity;
            }
            if (type == PathIterator.SEG_LINETO)
            {
                areaovr += ((prevy + coords[1])/2)*(prevx - coords[0]);
                prevx = coords[0];
                prevy = coords[1];
            }
            if (type == PathIterator.SEG_CLOSE)
            {
                areaovr += ((prevy + inity)/2)*(prevx - initx);
            }
            path.next();
        }
        areaovr = Math.abs(areaovr);
        return(areaovr);
    }
    
    /**
     * Finds the overlap between two polygons represented by point lists.
     * Uses the java.awt.geom.Area class to compute intersection
     * between two area features.
     *
     * @param obj1 The first polygon represented as a LineWA array.
     * @param obj2 The second polygon represented as a LineWA array.
     *
     * @return An array containing two <code>double</code>values. The first
     *         is the percentage of <i>obj1</i> that <i>obj2</i> overlaps,
     *         and the second is the percentage of <i>obj2</i> that
     *         <i>obj1</i> overlaps.
     */
    public static double[] findOverlap(LineWA[] obj1, LineWA[] obj2)
    {
        double areaovr=getSingleOverlap(obj1,obj2);
        double a1 = (double)findArea(obj1);
        double a2 = (double)findArea(obj2);
        double[] overlaps = new double[2];
        overlaps[0]=(areaovr/a1)*100;
        overlaps[1]=(areaovr/a2)*100;
        return(overlaps);
    }
    
    /** double[] overlaps;
     * This function computes the distance between two polygons by computing
     * the smallest convex hull containing both polygons and then computing
     * how much each of the polygons overlap with this convex hull.
     *
     * @param obj1 The first polygon represented as a LineWA array.
     * @param obj2 The second polygon represented as a LineWA array.
     *
     * @return An array containing two <code>double</code>values. The first
     *         is the percentage of the convex hull that <i>obj1</i> overlaps,
     *         and the second is the percentage of the convex hull that
     *         <i>obj2</i> overlaps.
     */
    public static double[] convexHullDistance(LineWA[] obj1, LineWA[] obj2)
    {
        int a;
        LineWA[] allpoints;
        LineWA[] convexhull;
        double area1, area2, areach;
        double[] overlaps;
        // Computing combined convex hull
        allpoints = new LineWA[obj1.length + obj2.length];
        for (a = 0;a<obj1.length;a++)
        {
            allpoints[a] = obj1[a];
        }
        for (a = 0;a<obj2.length;a++)
        {
            allpoints[a + obj1.length] = obj2[a];
        }
        
        convexhull = convexHull(allpoints);
        
        area1 = (double)findArea(obj1);
        area2 = (double)findArea(obj2);
        areach = (double)findArea(convexhull);
        overlaps = new double[2];
        overlaps[0] = (area1/areach)*100;
        overlaps[1] = (area2/areach)*100;
        return(overlaps);
    }
    
    public static LineWA getClosestBoundaryPoint(LineWA lineA,LineWA lineB, LineWA[] poly)
    {
        Vector intersections=new Vector();
        for(int i=0;i< poly.length;i++)
        {
            if(TriRepUtil.PointOnLine(poly[i],lineA,lineB))
                intersections.add(poly[i]);
            //LineWA inter=TriRepUtil.getIntersection(lineA,lineB,poly[i],poly[(i+1)%poly.length]);
            //if(inter!=null)
//                intersections.add(inter);
        }
        if(intersections.size()==0)
            return(null);
        if(intersections.size()==1)
            return((LineWA)intersections.elementAt(0));
        int minSquareDist=Integer.MAX_VALUE;
        int minIndex=0;
        for(int i=0;i<intersections.size();i++)
        {
            LineWA tmp=(LineWA)intersections.elementAt(i);
            int squareDist=(tmp.x-lineA.x)*(tmp.x-lineA.x)+(tmp.y-lineA.y)*(tmp.y-lineA.y);
            if(squareDist<minSquareDist)
            {
                minSquareDist=squareDist;
                minIndex=i;
            }
        }
        return((LineWA)intersections.elementAt(minIndex));
    }
    
    public static LineWA getclosestIntersection(LineWA lineA,LineWA lineB, LineWA[] poly)
    {
        Vector intersections=new Vector();
        for(int i=0;i< poly.length;i++)
        {
            LineWA inter=TriRepUtil.getIntersection(lineA,lineB,poly[i],poly[(i+1)%poly.length]);
            if(inter!=null)
                intersections.add(inter);
        }
        if(intersections.size()==0)
            return(null);
        if(intersections.size()==1)
            return((LineWA)intersections.elementAt(0));
        int minSquareDist=Integer.MAX_VALUE;
        int minIndex=0;
        for(int i=0;i<intersections.size();i++)
        {
            LineWA tmp=(LineWA)intersections.elementAt(i);
            int squareDist=(tmp.x-lineA.x)*(tmp.x-lineA.x)+(tmp.y-lineA.y)*(tmp.y-lineA.y);
            if(squareDist<minSquareDist)
            {
                minSquareDist=squareDist;
                minIndex=i;
            }
        }
        return((LineWA)intersections.elementAt(minIndex));
    }
    
    public static LineWA[] getIntersections(LineWA lineA,LineWA lineB, LineWA[] poly)
    {
        Vector intersections=new Vector();
        for(int i=0;i< poly.length;i++)
        {
            LineWA inter=TriRepUtil.getIntersection(lineA,lineB,poly[i],poly[(i+1)%poly.length]);
            if(inter!=null)
                intersections.add(inter);
        }
        LineWA[] res=new LineWA[intersections.size()];
        for(int i=0;i<res.length;i++)
        {
            res[i]=(LineWA)intersections.elementAt(i);
        }
        return(res);
    }
    
    
    public static LineWA getIntersection(LineWA line1A,LineWA line1B,LineWA line2A,LineWA line2B)
    {
        int minx1,maxx1,miny1,maxy1;
        if(line1A.x>line1B.x)
        {
            minx1=line1B.x;
            maxx1=line1A.x;
        }
        else
        {
            minx1=line1A.x;
            maxx1=line1B.x;
        }
        if(line1A.y>line1B.y)
        {
            miny1=line1B.y;
            maxy1=line1A.y;
        }
        else
        {
            miny1=line1A.y;
            maxy1=line1B.y;
        }
        if(line2A.x<minx1&&line2B.x<minx1)
            return(null);
        if(line2A.x>maxx1&&line2B.x>maxx1)
            return(null);
        if(line2A.y<miny1&&line2B.y<miny1)
            return(null);
        if(line2A.y>maxy1&&line2B.y>maxy1)
            return(null);
        LineWA res=null;
        int xlk= line1B.x-line1A.x;
        int ylk= line1B.y-line1A.y;
        int xnm= line2B.x-line2A.x;
        int ynm= line2B.y-line2A.y;
        int xmk= line2A.x-line1A.x;
        int ymk= line2A.y-line1A.y;
        int det=xnm*ylk-ynm*xlk;
        if(det==0)
        {
            return(null);
        }
        else
        {
            double detinv =1.0/det;
            double s=(xnm*ymk-ynm*xmk)*detinv;
            double t=(xlk*ymk-ylk*xmk)*detinv;
            if(s<-0||s>1||t<-0||t>1)
            {
                return(null);
            }
            else
            {
                int x=(int)Math.round(line1A.x+xlk*s);
                int y=(int)Math.round(line1A.y+ylk*s);
                res=new LineWA(x,y);
            }
        }
        return(res);
    }
    public static boolean PointsOnLine(LineWA[] points,LineWA lineA,LineWA lineB)
    {
        boolean res=false;
        for(int i=0;i<points.length;i++)
        {
            if(PointOnLine(points[i],lineA,lineB))
            {
                res=true;
                break;
            }
        }
        return(res);
    }
    
    public static boolean PointOnLine(LineWA point,LineWA lineA,LineWA lineB)
    {
        double failureFactor=4.5;
        if(point.y<Math.min(lineA.y,lineB.y)-failureFactor)
            return (false);
        if(point.y>Math.max(lineA.y,lineB.y)+failureFactor)
            return(false);
        if(lineA.x==lineB.x)    //vertical Line
        {
            if(Math.abs(point.x-lineA.x)>failureFactor)
                return(false);
            else
            {
                return(true);
            }
        }
        if(point.x<Math.min(lineA.x,lineB.x)-failureFactor)
            return(false);
        if(point.x>Math.max(lineA.x,lineB.x)+failureFactor)
            return(false);
        double m=(lineB.y-lineA.y)/(1.0*(lineB.x-lineA.x));   //<inf, cause Line not vertical
        double b=lineA.y-m*lineA.x;                         //line has the form: y=m*x+b
        boolean res=false;
        if(Math.abs(point.y-(point.x*m+b))<failureFactor)
            res=true;
        return(res);
    }
    
    public static boolean PointOnBoundary(LineWA[] poly,LineWA point)
    {
        for(int i=0;i<poly.length;i++)
        {
            if(TriRepUtil.PointOnLine(point,poly[i],poly[(i+1)%poly.length]))
                return(true);
        }
        return(false);
    }
    
    public static double getRectangularDistance(LineWA lineA,LineWA lineB,LineWA point)
    {
        int xkj=lineA.x-point.x;
        int ykj=lineA.y-point.y;
        int xlk=lineB.x-lineA.x;
        int ylk=lineB.y-lineA.y;
        int denom=xlk*xlk+ylk*ylk;
        if(denom==0)
        {
            return(Math.sqrt(xkj*xkj+ykj*ykj));
        }
        double t=(-1.0*(xkj*xlk+ykj*ylk))/(denom*1.0);
        if(t<0||t>1)
            return(Double.NaN);
        double xfac=xkj+t*xlk;
        double yfac=ykj+t*ylk;
        double res=Math.sqrt(xfac*xfac+yfac*yfac);
        
        if((lineB.x-lineA.x)*(point.y-lineA.y)-(point.x-lineA.x)*(lineB.y-lineA.y)>0)
            res=res*-1;
        return res;
    }
    
    
    public static LineWA[] joinLinelists(LineWA[] first,LineWA[]second)
    {
        if(TriRepUtil.debugging)
        {
            System.out.println("JoinLininList");
            for(int i=0;i<first.length;i++)
            {
                System.out.println(first[i]);
            }
            System.out.println();

            for(int i=0;i<second.length;i++)
            {
                System.out.println(second[i]);
            }
        }
        try
        {
            double area=TriRepUtil.getArea(first);
            if(area<0)
            {
                LineWA[] tmplistrev = new LineWA[first.length];
                for (int i=0;i<first.length;i++)
                {
                    tmplistrev[first.length-i-1]=first[i];
                }
                first=tmplistrev;
            }
            area=TriRepUtil.getArea(second);
            if(area>0)
            {
                LineWA[] tmplistrev = new LineWA[second.length];
                for (int i=0;i<second.length;i++)
                {
                    tmplistrev[second.length-i-1]=second[i];
                }
                second=tmplistrev;
            }
            Vector resV=new Vector();
            int startIndex=0;
            while(TriRepUtil.PointOnBoundary(second,first[startIndex]))
                startIndex++;
            int i=startIndex;
            while(!TriRepUtil.PointsOnLine(second,first[i],first[i+1]))
            {
                resV.add(first[i]);
                i++;
            }
            LineWA inter=TriRepUtil.getClosestBoundaryPoint(first[i-1],first[i],second);
            int j=0;
            while(!second[j].equals(inter))
                j++;
            resV.add(second[j]);
            j=(j+1)%second.length;
            // inter=TriRepUtil.getClosestBoundaryPoint(second[j],second[(j+1)%second.length],first);
            while(!TriRepUtil.PointOnBoundary(first,second[j]))
            {
                resV.add(second[j]);
                j=(j+1)%second.length;
            }
            resV.add(second[j]);
            while(TriRepUtil.PointOnBoundary(second,first[i]))
            {
                i++;
            }
            for(;i<first.length;i++)
            {
                resV.add(first[i]);
            }
            for(i=0;i<startIndex;i++)
            {
                resV.add(first[i]);
            }
            LineWA[] res=new LineWA[resV.size()];
            for(i=0;i<resV.size();i++)
            {
                res[i]=(LineWA)resV.elementAt(i);
            }
            return(res);
        }
        catch(NullPointerException e)
        {
            return(first);
        }
    }
    
    public  static LineWA getIntersection(LineWA[] line1,LineWA[] line2)
    {
        return(getIntersection(line1[0],line1[1],line2[0],line2[1]));
    }
    
//    /**
//     * Computes the overlaps between objects at the same level of the line
//     * tree. Tries to find matching objects. The computed overlaps are stored
//     * in the convex hull tree nodes.
//     *
//     * @param obj1 The first object represented as a convex hull tree.
//     * @param obj2 The second object represented as a convex hull tree.
//     * @param min_overlap_match The amount of overlap required to consider the
//     *                          two objects to match.
//     */
//    public static void computeOverlapGraph(ConvexHullTreeNode obj1,
//            ConvexHullTreeNode obj2,
//            double min_overlap_match)
//    {
//        computeOverlapGraphIter(obj1, obj2, min_overlap_match);
//        obj1.sortOverlapEdges();
//        obj2.sortOverlapEdges();
//    }
//
//    /**
//     * Iterator function used by <code>computeOverlapGraph</code>.
//     *
//     * @param obj1 The first object represented as a convex hull tree.
//     * @param obj2 The second object represented as a convex hull tree.
//     * @param min_overlap_match The amount of overlap required to consider the
//     *                          two objects to match.
//     */
//    public static void computeOverlapGraphIter(ConvexHullTreeNode obj1,
//            ConvexHullTreeNode obj2,
//            double min_overlap_match)
//    {
//        double[] overlap;
//        int a,b;
//        ConvexHullTreeNode[] children1, children2;
//        overlap = findOverlap(obj1.getOutLine(), obj2.getOutLine());
//        if ((overlap[0]>min_overlap_match) || (overlap[1]>min_overlap_match))
//        {
//            // Insert line in overlap graph, and check the subnodes
//            obj1.insertOverlap(overlap[0], obj2);
//            obj2.insertOverlap(overlap[1], obj1);
//            children1 = obj1.getChildren();
//            children2 = obj2.getChildren();
//            for (a=0;a<children1.length;a++)
//            {
//                for (b=0;b<children2.length;b++)
//                {
//                    computeOverlapGraphIter(children1[a], children2[b], min_overlap_match);
//                }
//            }
//        }
//    }
//
//    /**
//     * Computes the overlaps between objects at the same level of the line
//     * tree. Tries to find matching objects. The difference between this and
//     * <code>computeOverlapGraph</code> is that this one does not insert
//     * overlap graph edges for the nodes themselves, only for the children.
//     *
//     * @param obj1 The first object represented as a convex hull tree.
//     * @param obj2 The second object represented as a convex hull tree.
//     * @param min_overlap The amount of overlap required to consider the
//     *                    two objects to match.
//     */
//    public static void recomputeOverlapGraph(ConvexHullTreeNode obj1,
//            ConvexHullTreeNode obj2,
//            double min_overlap)
//    {
//        double[] overlap;
//        int a,b;
//        ConvexHullTreeNode[] children1, children2;
//        overlap = findOverlap(obj1.getOutLine(), obj2.getOutLine());
//        if ((overlap[0]>min_overlap) && (overlap[1]>min_overlap))
//        {
//            // Check the subnodes
//            children1 = obj1.getChildren();
//            children2 = obj2.getChildren();
//            for (a=0;a<children1.length;a++)
//            {
//                for (b=0;b<children2.length;b++)
//                {
//                    computeOverlapGraph(children1[a], children2[b], min_overlap);
//                }
//            }
//        }
//    }
//
//    /**
//     * Removes all overlap references to a given convex hull tree node and
//     * all it's children from the convex hull tree. (Used when deleting
//     * nodes from the convex hull tree)
//     *
//     * @param obj The comvex hull tree node to be removed.
//     */
//    public static void removeAllOverlaps(ConvexHullTreeNode obj)
//    {
//        OverlapGraphEdge[] overlaplist;
//        ConvexHullTreeNode[] children;
//        overlaplist = obj.getOverlapEdges();
//        for (int a=0;a<overlaplist.length;a++)
//        {
//            overlaplist[a].overlapswith.removeOverlap(obj);
//        }
//        obj.removeAllOverlaps();
//        children = obj.getChildren();
//        for (int a=0;a<children.length;a++)
//        {
//            removeAllOverlaps(children[a]);
//        }
//    }
    
    /**
     * Creates a <code>LineWA</code> list from two sets of coordinate values.
     * This function also computes the angles between two consecutive lines
     * and stores these in the <code>LineWA</code> objects.
     *
     * @param xlist A list of the x-coordinates of the points.
     * @param ylist A list of the y-coordinates of the points.
     *
     * @return A list of <code>LineWA</code> objects containing the points
     *         given.
     */
    public static LineWA[] createLineList(int[] xlist, int[] ylist)
    {
        int a;                  // Tellervariabel
        LineWA[] linelist;      // the unfinished list of lines
        int nextpoint, lengthx, lengthy;
        double lengthhyp,angle;
        int length = xlist.length;
        linelist = new LineWA[length];
        for (a=0;a<(length-1);a++)
        {
            nextpoint = a+1;
            lengthx = xlist[nextpoint]-xlist[a];
            lengthy = ylist[nextpoint]-ylist[a];
            lengthhyp = Math.sqrt((double)((lengthx*lengthx)+(lengthy*lengthy)));
            angle = Math.acos(lengthx/lengthhyp);
            if (lengthy < 0) angle = (Math.PI*2)-angle;
            linelist[a] = new LineWA();
            linelist[a].angle = angle;
            linelist[a].x = xlist[a]*20;
            linelist[a].y = ylist[a]*20;
        }
        a = length-1;
        nextpoint = 0;
        lengthx = xlist[nextpoint]-xlist[a];
        lengthy = ylist[nextpoint]-ylist[a];
        lengthhyp = Math.sqrt((double)((lengthx*lengthx)+(lengthy*lengthy)));
        angle = Math.acos(lengthx/lengthhyp);
        if (lengthy < 0) angle = (Math.PI*2)-angle;
        linelist[a] = new LineWA();
        linelist[a].angle = angle;
        linelist[a].x = xlist[a]*20;
        linelist[a].y = ylist[a]*20;
        return(linelist);
    }
    
    /**
     * This function turns a <code>vector</code> of <code>LineWA</code>
     * values into an array of <code>LineWA</code>. The function also
     * computes the angles between two consecutive lines and stores this
     * in the <code>LineWA</code> objects.
     *
     * @param pointList The list of points given as <code>LineWA</code>'s.
     *
     * @return The array of <code>LineWA</code>'s
     */
    public static LineWA[] createLineList(Vector pointList)
    {
        int a;                  // Tellervariabel
        LineWA[] linelist;      // The unfinished list of lines
        int nextpoint, lengthx, lengthy;
        LineWA thisLineWA, nextLineWA;
        double lengthhyp,angle;
        int length = pointList.size();
        linelist = new LineWA[length];
        for (a=0;a<length;a++)
        {
            thisLineWA = (LineWA)pointList.elementAt(a);
            nextLineWA = (LineWA)pointList.elementAt((a+1) % length);
            lengthx = nextLineWA.x - thisLineWA.x;
            lengthy = nextLineWA.y - thisLineWA.y;
            lengthhyp =
                    Math.sqrt((double)((lengthx*lengthx)+(lengthy*lengthy)));
            angle = Math.acos(lengthx/lengthhyp);
            if (lengthy < 0) angle = (Math.PI*2)-angle;
            linelist[a] = new LineWA(thisLineWA.x, thisLineWA.y, angle);
        }
        return(linelist);
    }
    
    /**
     * Computes the angle between two consecutive lines for all the lines
     * given, and stores this in the <code>LineWA</code> objects.
     *
     * @param lines The list of lines.
     */
    public static void computeLineAngles(LineWA[] lines)
    {
        int listlength, lengthx, lengthy, nextpoint, a;
        double lengthhyp, angle;
        listlength = lines.length-1;
        for (a=0;a<listlength;a++)
        {
            nextpoint = a+1;
            lengthx = lines[nextpoint].x - lines[a].x;
            lengthy = lines[nextpoint].y - lines[a].y;
            lengthhyp = Math.sqrt((double)((lengthx*lengthx)+(lengthy*lengthy)));
            angle = Math.acos(lengthx/lengthhyp);
            if (lengthy < 0) angle = (Math.PI*2)-angle;
            lines[a].angle = angle;
        }
        a = listlength;
        nextpoint = 0;
        lengthx = lines[nextpoint].x - lines[a].x;
        lengthy = lines[nextpoint].y - lines[a].y;
        lengthhyp = Math.sqrt((double)((lengthx*lengthx)+(lengthy*lengthy)));
        angle = Math.acos(lengthx/lengthhyp);
        if (lengthy < 0) angle = (Math.PI*2)-angle;
        lines[a].angle = angle;
    }
    
    /**
     * For debugging purposes: prints the coordinate values of the points in
     * a line list to the screen.
     *
     * @param lines The list of <code>LineWA</code> objects to be printed
     */
    public static void printLineList(LineWA[] lines)
    {
        for (int a=0;a<lines.length;a++)
        {
            System.out.println(lines[a]);
        }
    }
    
    /**
     * This function rotates the given cycle around its center.
     *
     * @param shape The cycle to be rotated given as a <code>LineWA</code>
     *              array.
     * @param angle The angle in radians that the shape should be rotated.
     */
    public static void rotateCycle(LineWA[] shape, double angle)
    {
        int dx, dy, x1,y1,x2,y2;
        double r, theta;
        int newx, newy;
        Rectangle boundingBox;
        x1 = Integer.MAX_VALUE;
        y1 = Integer.MAX_VALUE;
        x2 = 0;
        y2 = 0;
        for (int a=0;a<shape.length;a++)
        {
            if (shape[a].x < x1) x1 = shape[a].x;
            if (shape[a].y < y1) y1 = shape[a].y;
            if (shape[a].x > x2) x2 = shape[a].x;
            if (shape[a].y > y2) y2 = shape[a].y;
        }
        boundingBox = new Rectangle(x1,y1,x2-x1,y2-y1);
        dx = (int)boundingBox.getCenterX();
        dy = (int)boundingBox.getCenterY();
        translateCycle(shape, -dx, -dy);
        for (int a=0;a<shape.length;a++)
        {
            r = Math.sqrt(shape[a].x*shape[a].x + shape[a].y*shape[a].y);
            theta = Math.acos(shape[a].x/r);
            if (shape[a].y < 0) theta = -theta;
            theta += angle;
            newx = (int)(Math.cos(theta)*r);
            newy = (int)(Math.sin(theta)*r);
            shape[a].x = newx;
            shape[a].y = newy;
        }
        translateCycle(shape, dx, dy);
    }
    
    public static double getArea(LineWA[] linelist)
    {
        double res=0.0;
        for(int i=0;i<linelist.length;i++)
        {
            int ip=(i+1)%linelist.length;
            res=res+((linelist[i].x-linelist[ip].x)*(linelist[i].y+linelist[ip].y))/2.0;
        }
        return(res);
    }
    /**
     * Moves the given cycle by dx and dy.
     *
     * @param outline The cycle to be moved given as a <code>LineWA</code>
     *              array.
     * @param dx How far along the x-axis the cycle should be moved.
     * @param dy How far along the y-axis the cycle should be moved.
     */
    public static void translateCycle(LineWA[] outline, int dx, int dy)
    {
        for (int n=0;n<outline.length;n++)
        {
            outline[n].x+=dx;
            outline[n].y+=dy;
        }
    }
    
    public static double getDiameter(LineWA[] Poly)
    {
        LineWA[][]tmp= new LineWA[1][];
        tmp[0]=Poly;
        return(getMaxDistance2(tmp));
    }
    public static double getMaxDistance2(LineWA[][] Polys)
    {
        long sTime=System.currentTimeMillis();
        int count=0;
        double res=0.0;
        for(int i=0;i<Polys.length;i++)
        {
            count+=Polys[i].length;
        }
        LineWA[] tmp=new LineWA[count];
        int counter=0;
        for(int i=0;i<Polys.length;i++)
        {
            for(int j=0;j<Polys[i].length;j++)
            {
                tmp[counter++]=Polys[i][j];
            }
        }
        LineWA[] conTmp=TriRepUtil.convexHull(tmp);
        int tmpdist=0;
        int pos=conTmp.length/2;
        for(int i=0;i<conTmp.length;i++)
        {
            pos=getLongestDistFromPoint(conTmp[i],conTmp,pos);
            int x=conTmp[i].x-conTmp[pos].x;
            int y=conTmp[i].y-conTmp[pos].y;
            if((x*x+y*y)>tmpdist)
                tmpdist=x*x+y*y;
            /*for(int j=0;j<conTmp.length;j++)
            {
                int x=conTmp[i].x-conTmp[j].x;
                int y=conTmp[i].y-conTmp[j].y;
                if((x*x+y*y)>tmpdist)
                    tmpdist=x*x+y*y;
            }*/
        }
        if(TriRepUtil.debugging)
            System.out.println("Besser brauchte"+(System.currentTimeMillis()-sTime)+"ms");
        return(Math.sqrt(tmpdist));
    }
    
    private static int getLongestDistFromPoint(LineWA from, LineWA[]list,int pos)
    {
        int d=(from.x+list[pos].x)*(from.x+list[pos].x)+(from.y+list[pos].y)*(from.y+list[pos].y);
        int dp=(from.x+list[(pos+1)%list.length].x)*(from.x+list[(pos+1)%list.length].x)+(from.y+list[(pos+1)%list.length].y)*(from.y+list[(pos+1)%list.length].y);
        int dm=(from.x+list[(pos-1+list.length)%list.length].x)*(from.x+list[(pos-1+list.length)%list.length].x)+(from.y+list[(pos-1+list.length)%list.length].y)*(from.y+list[(pos-1+list.length)%list.length].y);
        if(d>=dp&&d>=dm)
        {
            return(pos);
        }
        else
        {
            if(dp>d)
            {
                return(getLongestDistFromPoint(from,list,(pos+1)%list.length));
            }
            else
            {
                return(getLongestDistFromPoint(from,list,(pos-1+list.length)%list.length));
            }
        }
    }
    
    public static double getMaxDistance(LineWA[][] Polys)
    {
        long sTime=System.currentTimeMillis();
        int count=0;
        double res=0.0;
        for(int i=0;i<Polys.length;i++)
        {
            count+=Polys[i].length;
        }
        LineWA[] tmp=new LineWA[count];
        int counter=0;
        for(int i=0;i<Polys.length;i++)
        {
            for(int j=0;j<Polys[i].length;j++)
            {
                tmp[counter++]=Polys[i][j];
            }
        }
        int tmpdist=0;
        for(int i=0;i<tmp.length;i++)
        {
            for(int j=0;j<tmp.length;j++)
            {
                int x=tmp[i].x-tmp[j].x;
                int y=tmp[i].y-tmp[j].y;
                if((x*x+y*y)>tmpdist)
                    tmpdist=x*x+y*y;
            }
        }
        if(TriRepUtil.debugging)
            System.out.println("Einfach brauchte"+(System.currentTimeMillis()-sTime)+"ms");
        return(Math.sqrt(tmpdist));
    }
    
}
