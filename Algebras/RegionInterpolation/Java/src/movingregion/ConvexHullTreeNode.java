package movingregion;

import java.util.*;
import java.io.*;


/**
 * This class represents a node in the convex hull tree. The node contains
 * a polygon representation of an object or concavity, and links to child
 * nodes representing concavities in the object. The polygon represented
 * in a convex hull tree node is convex.
 *
 * @author Erlend Tøssebro
 */
public class ConvexHullTreeNode implements RegionTreeNode,Serializable
{
    static final long serialVersionUID = -7832017217383957662l;
    
    /**
     * This class represents one line in the polygon stored in this
     * convex hull tree node in the
     * same was as a <code>LineWA</code> represents a line in a normal
     * polygon. The name is short for Convex Hull Line. The only difference
     * from a normal <code>LineWA</code> is that an object of this class may
     * also store a link to a child of this convex hull tree node.
     *
     * @author Erlend Tøssebro
     */
    private class CHLine extends LineWA implements Serializable
    {
        public ConvexHullTreeNode child;
         static final long serialVersionUID =3477774149506989314l;
        /**
         * Creates a new <code>CHLine</code> object from a <code>LineWA</code>
         * object.
         *
         * @param line The <code>LineWA</code> object.
         */
        public CHLine(LineWA line)
        {
            x = line.x;
            y = line.y;
            angle = line.angle;
            child = null;
        }
    }
    
    private class doublePoint
    {
        double x;
        double y;
        public doublePoint(double x, double y)
        {
            this.x=x;
            this.y=y;
        }
        public String toString()
        {
            return("("+x+";"+y+")");
        }
    }
    
    private class LineDist implements Comparable
    {
        public int x;
        public int y;
        public double distance;
        public LineDist(LineWA p,double distance)
        {
            x=p.x;
            y=p.y;
            this.distance=distance;
        }
        public LineDist(int x,int y)
        {
            this.x=x;
            this.y=y;
            distance=0.0;
        }
        public int compareTo(Object o)
        {
            double tmp=distance-((LineDist)o).distance;
            if (tmp>0)
                return(1);
            if(tmp<0)
                return(-1);
            return(0);
        }
    }
    
    
    // The list of lines
    private Vector linelist;
    // The list of regions which overlap this region
    //private Vector overlaplist;
    // The point in the line list with the smallest y-coordinate
    private int smallestpoint;
    private int smallesty;
    private int smallestx;
    private boolean isHole=false;
    RegionTreeNode myParent;
    private int sourceOrTarget=0;
    /**
     * Creates an empty convex hull tree node
     */
    public ConvexHullTreeNode()
    {
        linelist = new Vector();        
        smallesty = Integer.MAX_VALUE;
        smallestx = Integer.MAX_VALUE;
        smallestpoint = -1;
        myParent=null;
    }
    
    /**
     * Creates a convex hull tree from the given polygon. If the region
     * defined by the polygon is not convex, this constructor
     * will also create the necessary subnodes. This constructor assumes
     * that this node will be the root of the convex hull tree.
     *
     * @param linelist The polygon represented as a list of <code>LineWA</code>
     *                 objects.
     * @param min The minimum overlap required for an edge to be added to the
     *            overlap graph
     * @param minmatch The minimum overlap required for two convex hull tree
     *                 nodes to match.
     */
    public ConvexHullTreeNode(LineWA[] linelist,RegionTreeNode myParent, int sourceOrTarget)
    {
        this(linelist,0,myParent, sourceOrTarget);
    }
    
    
    public ConvexHullTreeNode(LineWA[] linelist,boolean isHole,RegionTreeNode myParent, int sourceOrTarget)
    {
        this(linelist,0, isHole,myParent, sourceOrTarget);
    }
    
    public ConvexHullTreeNode(LineWA[] linelist, int level,RegionTreeNode myParent, int sourceOrTarget)
    {
        this(linelist,level,false,myParent, sourceOrTarget);
    }
    
    /**
     * Creates a convex hull tree from the given polygon. If the region
     * defined by the polygon is not convex, this constructor
     * will also create the necessary subnodes.
     *
     * @param linelist The polygon represented as a list of <code>LineWA</code>
     *                 objects.
     * @param level Which level of the convex hull tree this node is on
     * @param min The minimum overlap required for an edge to be added to the
     *            overlap graph
     * @param minmatch The minimum overlap required for two convex hull tree
     *                 nodes to match.
     */
    public ConvexHullTreeNode(LineWA[] linelist, int level, boolean isHole,RegionTreeNode myParent, int sourceOrTarget)
    {
        LineWA[] tmplist, convhull, childlist;
        this.myParent=myParent;
        this.isHole=isHole;
        this.sourceOrTarget=sourceOrTarget;
        int node;
        int index1, index2, length, lastindex, noiterations;
        int indexll1, indexll2;
        this.linelist = new Vector();
        //  overlaplist = new Vector();
        smallesty = Integer.MAX_VALUE;
        smallestx = Integer.MAX_VALUE;
        smallestpoint = -1;
        //matched = false;
        //On the root, look if the polygone is ordered CCW
        //if not change the order
        if (level==0)
        {
            double area=TriRepUtil.getArea(linelist);
            if(TriRepUtil.debugging)
                System.out.println("Area="+area);
            if(area<0)
            {
                if(TriRepUtil.debugging)
                    System.out.println("Falsche Drehrichtung");
                LineWA[] tmplistrev = new LineWA[linelist.length];
                for (int i=0;i<linelist.length;i++)
                {
                    tmplistrev[linelist.length-i-1]=linelist[i];
                }
                linelist=tmplistrev;
            }
        }
        // Create temporary list so that convexHull() won't change the order of
        // points in the original linelist.
        Vector tmplistVector=new Vector();
        HashSet doubleDet=new HashSet();
        
        if(TriRepUtil.debugging)
           System.out.println("Konstruiere Node Level: "+level);
        for (int a=0;a<linelist.length;a++)
        {
            if(doubleDet.contains(linelist[a]))
            {
                if(TriRepUtil.debuggingWarnings)
                    System.out.println("Doppelten Punkt: "+linelist[a]+" in Linelist gelöscht");
            }
            else
            {
                tmplistVector.add(linelist[a]);            
            }
        }
        tmplist = new LineWA[tmplistVector.size()];
        for(int i=0;i<tmplistVector.size();i++)
        {
            tmplist[i] = (LineWA) tmplistVector.elementAt(i);
        }
        // Find the convex hull
        convhull = TriRepUtil.convexHull(tmplist);
        // Find where the first node in linelist is in the convex hull
        // (Or the earliest possible if the first node is not on the hull)
        // This is to preserve the order in the linelist in the ordering of
        // points in the convex hull tree node.
        index1 = 0;
        lastindex = convhull.length;
        for (int a=0;a<linelist.length;a++)
        {
            index1 = TriRepUtil.indexOf(convhull, linelist[a]);
            if (index1 != -1) break;
        }
        for (int a=linelist.length-1;a>=0;a--)
        {
            lastindex = TriRepUtil.indexOf(convhull, linelist[a]);
            if (lastindex != -1) break;
        }
        if (lastindex > index1)
        {
            for (int a=index1;a<=lastindex;a++)
            {
                insertLine(convhull[a]);
            }
        }
        else
        {
            for (int a=index1;a<convhull.length;a++)
            {
                insertLine(convhull[a]);
            }
            for (int a=0;a<=lastindex;a++)
            {
                insertLine(convhull[a]);
            }
        }
        // Check lines in convex hull with lines in line list. Whenever two points
        // which are neighbours in the convex hull are not neighbours in the line
        // list, create a child node from the points between them.
        
        noiterations = numberOfLines();
        index1 = 0;
        indexll1 = 0;
        while (!(linelist[indexll1].equals(getLine(index1))))
        {
            indexll1++;
        }
        for (int a=0;a<noiterations;a++)
        {
            index2 = index1+1;
            indexll2 = indexll1+1;
            while (!(linelist[indexll2 % linelist.length].equals(getLine(index2 % noiterations))))
            {
                indexll2++;
            }
            
            if ((indexll2 != indexll1+1) && ((level == 0) ||
                    ((indexll2 != linelist.length) && (indexll1 != linelist.length-1)))
                    && (indexll2 != indexll1-1))
            {
                // create child node
                length = indexll2-indexll1+1;
                childlist = new LineWA[length];
                for (int b=0;b<length;b++)
                {
                    if ((b+indexll1) < linelist.length)
                    {
                        childlist[length-b-1] = linelist[b+indexll1];
                    }
                    else
                    {
                        childlist[length-b-1] = linelist[b+indexll1-linelist.length];
                    }
                }
                insertChild(index1, new ConvexHullTreeNode(childlist, level+1,this.isHole,this, sourceOrTarget));
            }
            index1 = index2;
            indexll1 = indexll2;
        }
    }
    
//    public ConvexHullTreeNode addChild(LineWA[] newChild)
//    {
//        ConvexHullTreeNode newNode=new ConvexHullTreeNode(newChild,1,this.isHole(),this);       //@TODO bloody Hack
//        LineWA[] thisOutline=this.getOrderedOutLine();
//        LineWA[] newOutline=newNode.getOrderedOutLine();
//        int insertIndex;
//        for(int i=0;i< thisOutline.length;i++)
//        {
//            for(int j=0;j<newOutline.length;j++)
//            {
//                if(thisOutline[i].equals(newOutline[j]))
//                {
//                    if(thisOutline[(i+1)%thisOutline.length].equals(newOutline[(j+1)%newOutline.length]))
//                    {
//                        ((CHLine)this.linelist.elementAt(i)).child=newNode;
//                        return(newNode);
//                    }
//                    if(TriRepUtil.PointOnLine(newOutline[(j+1)%newOutline.length],thisOutline[i],thisOutline[(i+1)%thisOutline.length]))
//                    {
//                        ((CHLine)this.linelist.elementAt(i)).child=newNode;
//                        this.linelist.insertElementAt(new CHLine(newOutline[(j+1)%newOutline.length]),i+1);                        
//                        return(newNode);
//                    }
//                    if(TriRepUtil.PointOnLine(newOutline[(j-1+newOutline.length)%newOutline.length],thisOutline[i],thisOutline[(i-1+thisOutline.length)%thisOutline.length]))
//                    {
//                        ((CHLine)this.linelist.elementAt((i-1+thisOutline.length)%thisOutline.length)).child=newNode;
//                        this.linelist.insertElementAt(new CHLine(newOutline[(j-1+newOutline.length)%newOutline.length]),(i-1+thisOutline.length)%thisOutline.length);                        
//                        return(newNode);
//                    }
//                    insertIndex=i;
//                }
//            }
//        }
//        return(null);
//    }
    
    public int hashCode()
    {
        int start=5132;
        int modu=7536;
        int res=start;
        LineWA[] tmp=this.getLines();
        for(int i=0;i<tmp.length;i++)
        {
            res=res+tmp[i].x+tmp[i].y;
        }
        res=res*(this.sourceOrTarget + 1);
        res=res%modu;
        return (res);
    }
    
    public LineWA[]getSplitLine(ConvexHullTreeNode ref1,ConvexHullTreeNode ref2)
    {
        System.out.println(this);
        System.out.println(ref1);
        System.out.println(ref2);        
        LineWA p1=ref1.getCenter();
        LineWA p2=ref2.getCenter();
        System.out.println(p1);        
        System.out.println(p2);        
        Vector pdist1=new Vector();
        Vector pdist2=new Vector();
        Vector resv=new Vector();
        LineWA[] ref1lines=ref1.getLines();
        LineWA[] ref2lines=ref2.getLines();
        for (int i=0; i<ref1lines.length;i++)
        {
            double dist=TriRepUtil.getRectangularDistance(p1,p2,ref1lines[i]);
            if(!Double.isNaN(dist))
                pdist1.add(new LineDist(ref1lines[i],dist));
        }
        
        for (int i=0; i<ref2lines.length;i++)
        {
            double dist=TriRepUtil.getRectangularDistance(p1,p2,ref2lines[i]);
            if(!Double.isNaN(dist))
                pdist2.add(new LineDist(ref2lines[i],dist));
        }
        Collections.sort(pdist1);
        Collections.sort(pdist2);
        
        
        while(pdist1.size()>1||pdist2.size()>1)
        {
            if(pdist1.size()==0||pdist2.size()==0)
            {
                if(TriRepUtil.debuggingWarnings)
                    System.out.println("keine Splitline gefunden ");
                return(null);
            }
            LineDist tmp1=(LineDist)pdist1.elementAt(0);
            LineDist tmp2=(LineDist)pdist2.elementAt(0);
            if(((TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref1lines).length)<=2)
            &&(TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref2lines).length)<=2)
                resv.add(new doublePoint((tmp1.x+tmp2.x)/2.0,(tmp1.y+tmp2.y)/2.0));
            
            if(tmp1.distance>tmp2.distance)
            {
                if(pdist2.size()>1)
                {
                    pdist2.remove(0);
                }
                else
                {
                    while(pdist1.size()>1)
                    {
                        tmp1=(LineDist)pdist1.elementAt(0);
                        tmp2=(LineDist)pdist2.elementAt(0);
                        if(((TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref1lines).length)<=2)
                        &&(TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref2lines).length)<=2)
                            resv.add(new doublePoint((tmp1.x+tmp2.x)/2.0,(tmp1.y+tmp2.y)/2.0));
                        pdist1.remove(0);
                    }
                }
            }
            else
            {
                
                if(pdist1.size()>1)
                {
                    pdist1.remove(0);
                }
                else
                {
                    while(pdist2.size()>1)
                    {
                        tmp1=(LineDist)pdist1.elementAt(0);
                        tmp2=(LineDist)pdist2.elementAt(0);
                        if(((TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref1lines).length)<=2)
                        &&(TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref2lines).length)<=2)
                            resv.add(new doublePoint((tmp1.x+tmp2.x)/2.0,(tmp1.y+tmp2.y)/2.0));
                        pdist2.remove(0);
                    }
                }
            }
            
        }
        if(pdist1.size()==0||pdist2.size()==0)
        {
            if(TriRepUtil.debuggingWarnings)
                System.out.println("Problem bei der Erstellung einer Splitline");
            return(null);
        }
        LineDist tmp1=(LineDist)pdist1.elementAt(0);
        LineDist tmp2=(LineDist)pdist2.elementAt(0);
        if(((TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref1lines).length)<=2)
        &&(TriRepUtil.getIntersections(new LineWA(tmp1.x,tmp1.y),new LineWA(tmp2.x,tmp2.y),ref2lines).length)<=2)
            resv.add(new doublePoint((tmp1.x+tmp2.x)/2.0,(tmp1.y+tmp2.y)/2.0));
        for(int i=0;i< resv.size();i++)
        {
            System.out.println((doublePoint)resv.elementAt(i));
        }
        double sumLinex=0;
        double sumLiney=0;
        for(int i=0;i<resv.size();i++)
        {
            sumLinex+=((doublePoint)resv.elementAt(i)).x;
            sumLiney+=((doublePoint)resv.elementAt(i)).y;
        }
        doublePoint centerLine=new doublePoint(sumLinex/(resv.size()),sumLiney/(resv.size()));
        int sumRefx=0;
        int sumRefy=0;
        for(int i=0;i<ref1lines.length;i++)
        {
            sumRefx+=ref1lines[i].x;
            sumRefy+=ref1lines[i].y;
        }
        for(int i=0;i<ref2lines.length;i++)
        {
            sumRefx+=ref2lines[i].x;
            sumRefy+=ref2lines[i].y;
        }
        doublePoint centerRef=new doublePoint(sumRefx*1.0/(ref1lines.length+ref2lines.length),sumRefy*1.0/(ref1lines.length+ref2lines.length));        
        double scaleVector=Math.abs(TriRepUtil.getArea(this.getLines()))/(Math.abs(TriRepUtil.getArea(ref1lines))+Math.abs(TriRepUtil.getArea(ref2lines)));        
        double distLine=Math.sqrt((centerLine.x-((doublePoint)resv.elementAt(0)).x)*(centerLine.x-((doublePoint)resv.elementAt(0)).x)+(centerLine.y-((doublePoint)resv.elementAt(0)).y)*(centerLine.y-((doublePoint)resv.elementAt(0)).y));
        distLine=Math.max(distLine,Math.sqrt((centerLine.x-((doublePoint)resv.elementAt(resv.size()-1)).x)*(centerLine.x-((doublePoint)resv.elementAt(resv.size()-1)).x)+(centerLine.y-((doublePoint)resv.elementAt(resv.size()-1)).y)*(centerLine.y-((doublePoint)resv.elementAt(resv.size()-1)).y)));
        double thismaxdist =0;
        LineWA centerThis=this.getCenter();
        LineWA[] thisLines=this.getLines();
        for(int i=0;i<thisLines.length;i++)
        {
            thismaxdist=Math.max(thismaxdist,Math.sqrt((thisLines[i].x-centerThis.x)*(thisLines[i].x-centerThis.x)+(thisLines[i].y-centerThis.y)*(thisLines[i].y-centerThis.y)));
        }
        double scale=thismaxdist/distLine*1.15;
        for(int i=0;i<resv.size();i++)
        {
            doublePoint tmp=((doublePoint)resv.elementAt(i));
            tmp.x=centerThis.x+(centerLine.x-centerRef.x)*scaleVector+(tmp.x-centerLine.x)*scale;
                    //tmp.x+(centerThis.x-centerLine.x)+(centerLine.x-centerRef.x)*scaleVector;
            tmp.y=centerThis.y+(centerLine.y-centerRef.y)*scaleVector+(tmp.y-centerLine.y)*scale;
        }
        
        LineWA[] res=new LineWA[resv.size()];
        System.out.println("RES");
        for(int i=0;i<resv.size();i++)
        {
            doublePoint tmp=((doublePoint)resv.elementAt(i));
            System.out.println(tmp);
            res[i]=new LineWA((int)Math.round(tmp.x),(int)Math.round(tmp.y));
        }
        
        return res;
    }
    
    
    public LineWA[][] getSplitNodes(LineWA[] splitLine)
    {
        if(splitLine==null||splitLine.length==0)
            return(null);
        LineWA[][] res=new LineWA[2][];
        int lowIndexLine,highIndexLine,lowIndexPoly,highIndexPoly;
        Vector IntersectionPoints=new Vector();
        Vector IntersectionIndexPoly=new Vector();
        Vector IntersectionIndexLine=new Vector();
        LineWA[] polyLine=this.getLines();
        for(int i=0;i<polyLine.length;i++)
        {
            for(int j=0;j<(splitLine.length-1);j++)
            {
                LineWA inters=TriRepUtil.getIntersection(polyLine[i],polyLine[(i+1)%polyLine.length],splitLine[j],splitLine[j+1]);
                if(inters!=null)
                {
                    IntersectionPoints.add(inters);
                    IntersectionIndexPoly.add(new Integer(i));
                    IntersectionIndexLine.add(new Integer(j));
                }
            }
        }
        if(IntersectionPoints.size()!=2)
        {
            return(null);
        }
        else
        {
            
            if(((Integer)IntersectionIndexPoly.elementAt(0)).intValue()>((Integer)IntersectionIndexPoly.elementAt(1)).intValue())
            {
                lowIndexPoly=((Integer)IntersectionIndexPoly.elementAt(1)).intValue();
                highIndexPoly=((Integer)IntersectionIndexPoly.elementAt(0)).intValue();
            }
            else
            {
                lowIndexPoly=((Integer)IntersectionIndexPoly.elementAt(0)).intValue();
                highIndexPoly=((Integer)IntersectionIndexPoly.elementAt(1)).intValue();
            }
            if(((Integer)IntersectionIndexLine.elementAt(0)).intValue()>((Integer)IntersectionIndexLine.elementAt(1)).intValue())
            {
                lowIndexLine=((Integer)IntersectionIndexLine.elementAt(1)).intValue();
                highIndexLine=((Integer)IntersectionIndexLine.elementAt(0)).intValue();
            }
            else
            {
                lowIndexLine=((Integer)IntersectionIndexLine.elementAt(0)).intValue();
                highIndexLine=((Integer)IntersectionIndexLine.elementAt(1)).intValue();
            }
            if(TriRepUtil.debugging)
            {
                System.out.println("lowHIg"+lowIndexPoly+highIndexPoly);
                System.out.println("lowHIg"+lowIndexLine+highIndexLine);
            }
            res[0]=new LineWA[polyLine.length-highIndexPoly+lowIndexPoly+highIndexLine-lowIndexLine+2];
            res[1]=new LineWA[highIndexPoly-lowIndexPoly+highIndexLine-lowIndexLine+2];
            int index1=0;
            int index2=0;
            for(int i=0;i<=lowIndexPoly;i++ )
            {
                res[0][index1++]=polyLine[i];
            }
            for(int i=highIndexPoly;i>lowIndexPoly;i--)
            {
                res[1][index2++]=polyLine[i];
            }
            int indexindex=IntersectionIndexPoly.indexOf(new Integer(lowIndexPoly));  //toDo Sonderfall
            
            if(lowIndexPoly==highIndexPoly && TriRepUtil.debuggingWarnings)System.out.println("Problem: highIndex == lowIndex");
            
            res[0][index1++]=(LineWA)IntersectionPoints.elementAt(indexindex);
            res[1][index2++]=(LineWA)IntersectionPoints.elementAt(indexindex);
            if(IntersectionIndexLine.elementAt(indexindex).equals(new Integer(lowIndexLine)))
            {
                for(int i=lowIndexLine+1;i<=highIndexLine;i++)
                {
                    res[0][index1++]=splitLine[i];
                    res[1][index2++]=splitLine[i];
                }
            }
            else
            {
                
                for(int i=highIndexLine;i>lowIndexLine;i--)
                {
                    res[0][index1++]=splitLine[i];
                    res[1][index2++]=splitLine[i];
                }
            }
            //indexindex=IntersectionIndexPoly.indexOf(new Integer(highIndexPoly));
            res[0][index1++]=(LineWA)IntersectionPoints.elementAt((indexindex-1)*-1);
            res[1][index2++]=(LineWA)IntersectionPoints.elementAt((indexindex-1)*-1);
            for(int i=highIndexPoly+1;i<polyLine.length;i++)
            {
                res[0][index1++]=polyLine[i];
            }
        }
        if(res[0][0].equals(res[0][res[0].length-1]))
            return(null);
        if(res[1][0].equals(res[1][res[1].length-1]))
            return(null);
        
        return(res);
    }
    
    public boolean equals(Object o)
    {
        if(o instanceof ConvexHullTreeNode)
        {
            boolean res=true;
            ConvexHullTreeNode tmp=(ConvexHullTreeNode)o;
            if(this.sourceOrTarget!=tmp.getsourceOrTarget())
                return(false);
            LineWA[]tmp1=this.getLines();
            LineWA[]tmp2=tmp.getLines();            
            if(tmp1.length!=tmp2.length)
                return false;
            for(int i=0;i<tmp1.length;i++)
            {
                if(tmp1[i].x!=tmp2[i].x)
                    res=false;
                if(tmp1[i].y!=tmp2[i].y)
                    res=false;
            }
            return(res);
        }
        else
        {
            if (o instanceof SingleMatch)
            {
                SingleMatch tmp=(SingleMatch)o;
                return(this.equals(tmp.getSource()));
            }
            else
            {
                return false;
            }
        }
    }
    
    /**
     * This function inserts a new line at the end of the list of lines
     * defining the polygon represented by this object.
     *
     * @param line The line represented by a <code>LineWA</code> object.
     *
     * @return The index of the new line in the list of lines.
     */
    public int insertLine(LineWA line)
    {
        CHLine newline = new CHLine(line);
        linelist.addElement(newline);
        if ((line.y < smallesty) || ((line.y == smallesty) && (line.x < smallestx)))
        {
            smallestpoint = linelist.size()-1;
            smallesty = line.y;
            smallestx = line.x;
        }
        return(linelist.size()-1);
    }
    
    public boolean isHole()
    {
        return(isHole);
    }
    
    /**
     * This function computes which point has the "smallest" location, and
     * stores this in the object variables <code>smallestpoint</code>,
     * <code>smallestx</code> and <code>smallesty</code>. The smallest
     * location is the one with the lowest y-coordinate. If two points have
     * the same y-coordinate, the one with the smallest x-coordinate is
     * considered to be the smallest.
     */
    private void computeSmallestPoint()
    {
        CHLine line;
        smallesty = Integer.MAX_VALUE;
        smallestx = Integer.MAX_VALUE;
        for (int a=0;a<linelist.size();a++)
        {
            line = (CHLine)linelist.elementAt(a);
            if ((line.y < smallesty) || ((line.y == smallesty) && (line.x < smallestx)))
            {
                smallestpoint = a;
                smallesty = line.y;
                smallestx = line.x;
            }
        }
    }
    
    /**
     * This function removes a line from the polygon.
     *
     * @param index The line represented by its index in the list of lines.
     */
    public void removeLine(int index)
    {
        linelist.removeElementAt(index);
        if (smallestpoint > index) smallestpoint--;
        if (smallestpoint == index)
        {
            computeSmallestPoint();
        }
    }
    
    /**
     * This function returns the line at a given position in the line list.
     *
     * @param index The line represented by its index in the list of lines.
     *
     * @return The line represented by a <code>LineWA</code> object.
     *         Modifications to this <code>LineWA</code> object are also
     *         reflected in the line stored in this object!
     */
    public LineWA getLine(int index)
    {
        return((LineWA)linelist.elementAt(index));
    }
    
    /**
     * This function creates an empty convex hull tree node and associates
     * it with a given line in this object.
     *
     * @param lineindex The line represented by its index in the list of lines.
     *
     * @return The newly created convex hull tree node.
     */
    public ConvexHullTreeNode createChild(int lineindex)
    {
        CHLine line;
        line = (CHLine)linelist.elementAt(lineindex);
        line.child = new ConvexHullTreeNode();
        return(line.child);
    }
    
    /**
     * This function links a particular line in this object to a given
     * convex hull tree node, thereby making it a child of this node.
     *
     * @param lineindex The line represented by its index in the list of lines.
     * @param child The convex hull tree node which should be linked to
     *              the line.
     */
    public void insertChild(int lineindex, ConvexHullTreeNode child)
    {
        CHLine line;
        line = (CHLine)linelist.elementAt(lineindex);
        line.child = child;
    }
    
//    /**
//     * This function inserts an overlap graph edge between this node and
//     * another node.
//     *
//     * @param amount How much the two nodes overlap, given as the percentage
//     *               of this object that is overlapped by the other object.
//     * @param node The convex hull tree node that overlaps this node.
//     */
//    public void insertOverlap(double amount, ConvexHullTreeNode node)
//    {
//        OverlapGraphEdge og = new OverlapGraphEdge();
//        og.overlap = amount;
//        og.overlapswith = node;
//        overlaplist.addElement(og);
//    }
//
//    /**
//     * This function removes an overlap graph edge between this node and
//     * another node.
//     *
//     * @param node The other node.
//     */
//    public void removeOverlap(ConvexHullTreeNode node)
//    {
//        OverlapGraphEdge og;
//        for (int a=0;a<overlaplist.size();a++)
//        {
//            og = (OverlapGraphEdge)overlaplist.elementAt(a);
//            if (og.overlapswith == node)
//            {
//                overlaplist.removeElementAt(a);
//                a--;
//            }
//        }
//    }
//
//    /**
//     * Removes all overlap edges going out from this node.
//     */
//    public void removeAllOverlaps()
//    {
//        overlaplist.clear();
//    }
    
    /**
     * Finds the number of lines that makes op the polygon stored in this
     * node.
     *
     * @return The number of lines.
     */
    public int numberOfLines()
    {
        return(linelist.size());
    }
    
    /**
     * Returns all children of this node.
     *
     * @return The children of this node given as a
     *         <code>ConvexHullTreeNode</code> array.
     */
    public ConvexHullTreeNode[] getChildren()
    {
        Vector tmp;
        int length;
        CHLine line;
        ConvexHullTreeNode[] result;
        tmp = new Vector();
        length = linelist.size();
        for (int a=0;a<length;a++)
        {
            line = (CHLine)linelist.elementAt(a);
            if (line.child != null)
            {
                tmp.addElement(line.child);
            }
        }
        result = new ConvexHullTreeNode[tmp.size()];
        tmp.toArray(result);
        return(result);
    }
    
//    /**
//     * Returns all overlap graph edges going from this node.
//     *
//     * @return The overlap graph edges given as an
//     *         <code>OverlapGraphEdge</code> array.
//     */
//    public OverlapGraphEdge[] getOverlapEdges()
//    {
//        OverlapGraphEdge[] result;
//        result = new OverlapGraphEdge[overlaplist.size()];
//        overlaplist.toArray(result);
//        return(result);
//    }
//
//    /**
//     * Returns all convex hull tree nodes that overlaps with this node.
//     *
//     * @return The overlapping nodes given as a
//     *         <code>ConvexHullTreeNode</code> array.
//     */
//    public ConvexHullTreeNode[] getOverlappingNodes()
//    {
//        ConvexHullTreeNode[] result;
//        Vector tmplist;
//        int nooverl;
//        tmplist = new Vector();
//        nooverl = overlaplist.size();
//        for (int a=0;a<nooverl;a++)
//        {
//            tmplist.addElement(((OverlapGraphEdge)overlaplist.elementAt(a)).overlapswith);
//        }
//        result = new ConvexHullTreeNode[tmplist.size()];
//        tmplist.toArray(result);
//        return(result);
//    }
    
//    /**
//     * This function sorts the overlap graph edges of this node and its children
//     * by the amount of overlap.
//     */
//    public void sortOverlapEdges()
//    {
//        OverlapGraphEdge[] oe;
//        ConvexHullTreeNode[] children;
//        if (overlaplist.size() > 0)
//        {
//            oe = new OverlapGraphEdge[overlaplist.size()];
//            overlaplist.toArray(oe);
//            Arrays.sort(oe);
//            overlaplist = new Vector();
//            overlaplist.addAll(Arrays.asList(oe));
//        }
//        children = getChildren();
//        for (int a=0;a<children.length;a++)
//        {
//            children[a].sortOverlapEdges();
//        }
//    }
    
    private int computex(int x, int y)
    {
        return(x+(y/4));
    }
    
    private int computey(int y)
    {
        return(y/2);
    }
    public void removeChild(ConvexHullTreeNode toDelete)
    {
        int length;
        CHLine line;
        length = linelist.size();
        for (int a=0;a<length;a++)
        {
            line = (CHLine)linelist.elementAt(a);
            if (line.child!=null&&line.child.equals(toDelete))
            {
                line.child=null;
            }
        }
        
    }
    
    public void setHole(boolean isHole)
    {
        this.isHole=isHole;
        ConvexHullTreeNode[] children=this.getChildren();
        for(int i=0;i<children.length;i++)
        {
            children[i].setHole(isHole);
        }
    }
    
    public void setSourceOrTarget(int sourceOrTarget)
    {
        this.sourceOrTarget=sourceOrTarget;
        ConvexHullTreeNode[] children=this.getChildren();
        for(int i=0;i<children.length;i++)
        {
            children[i].setSourceOrTarget(sourceOrTarget);
        }        
    }
    
    /**
     * This function finds the line a given child of this node is associated
     * with.
     *
     * @param child The child node.
     *
     * @return A list of two <code>LineWA</code> objects, the one associated
     *         with the child node and the next one. If the convex hull tree
     *         node given as parameter to this function is not a child
     *         of this node, this function returns NULL.
     */
    public LineWA[] getLineForChild(ConvexHullTreeNode child)
    {
        int length;
        CHLine line;
        LineWA[] result;
        length = linelist.size();
        for (int a=0;a<length;a++)
        {
            line = (CHLine)linelist.elementAt(a);
            if (line.child!=null&&line.child.equals(child))
            {
                result = new LineWA[2];
                result[0] = (LineWA)line;
                if (a != length-1)
                {
                    result[1] = (LineWA)linelist.elementAt(a+1);
                }
                else
                {
                    result[1] = (LineWA)linelist.elementAt(0);
                }
                return(result);
            }
        }
        return(null);
    }
    
    /**
     * Gets the smallest point.
     *
     * @return The smallest point, which is the point with the smallest
     *         y-coordinate, represented by its integer index in the point
     *         list in this object.
     */
    public int getSmallestPoint()
    {
        return(smallestpoint);
    }
    
    
    /**
     * Gets the polygon stored in the node in the order in which it is stored
     * internally.
     *
     * @return The polygon stored in this node represented as a
     *         <code>LineWA</code> array.
     */
    public LineWA[] getOutLine()
    {
        LineWA[] result = new LineWA[linelist.size()];
        linelist.toArray(result);
        return(result);
    }
    
    /**
     * Gets the polygon stored in the node with the point with the smallest
     * y-coordinate first.
     *
     * @return The polygon stored in this node represented as a
     *         <code>LineWA</code> array.
     */
    public LineWA[] getOrderedOutLine()
    {
        int a, iterlength, length;
        length = linelist.size();
        LineWA[] result = new LineWA[length];
        iterlength = length-smallestpoint;
        a = -2;
        for (a=0;a<iterlength;a++)
        {
            result[a] = (LineWA)linelist.elementAt(a+smallestpoint);
        }
        for (a=0;a<smallestpoint;a++)
        {
            result[a+iterlength] = (LineWA)linelist.elementAt(a);
        }
        return(result);
    }
    
    public LineWA getSteinerPoint()
    {
        LineWA[]lines=this.getOutLine();
        TriRepUtil.computeLineAngles(lines);
        int resx=0;
        int resy=0;
        for(int i=0;i<lines.length;i++)
        {
            double angle=ConVertex.getAngleRad(lines[i].x,lines[i].y,lines[(i-1+lines.length)%lines.length].x,lines[(i-1+lines.length)%lines.length].y,lines[(i+1)%lines.length].x,lines[(i+1)%lines.length].y);
            double weight=.5-(angle/2/Math.PI);
            resx+=(int)(lines[i].x*weight);
            resy+=(int)(lines[i].y*weight);
        }
//        resx=resx/lines.length;
//        resy=resy/lines.length;
        return(new LineWA(resx,resy));
    }
    
    public int getsourceOrTarget()
    {
        return(this.sourceOrTarget);
    }
    
    public LineWA getCenter()
    {
        LineWA[]lines=this.getOutLine();
        int resx=0;
        int resy=0;
        for(int i=0;i<lines.length;i++)
        {
            resx+=lines[i].x;
            resy+=lines[i].y;
        }
        resx=resx/(lines.length);
        resy=resy/(lines.length);
        return(new LineWA(resx,resy));
    }
    
    /**
     * Gets the polygon stored in the convex hull tree that has this node as
     * its root.
     *
     * @return The polygon stored in this node represented as a
     *         <code>LineWA</code> array.
     */
    public LineWA[] getLines()
    {
        LineWA[] result;
        Vector listoflines;
        int nolines;
        listoflines = getLinesAsVector();
        result = new LineWA[listoflines.size()];
        listoflines.toArray(result);
        return(result);
    }
    
//    /**
//     * Sets the variable indicating whether this object has been matched to
//     * another or not.
//     *
//     * @param val TRUE if this object has been matched, FALSE otherwise.
//     */
//    public void setMatched(boolean val)
//    {
//        matched = val;
//    }
    
    public RegionTreeNode getParentNode()
    {
        return(this.myParent);
    }
    
//    /**
//     * Finds whether this object has already been matched to another convex hull
//     * tree node or not.
//     *
//     * @return TRUE if it has been matched, FALSE otherwise.
//     */
//    public boolean isMatched()
//    {
//        return(matched);
//    }
    
    /**
     * Iterator function used by <code>getLines</code>.
     *
     * @return A vector of the lines it has retrieved.
     */
    private Vector getLinesAsVector()
    {
        Vector listoflines, tmplist;
        CHLine tmpline;
        int nolines, nolines2;
        listoflines = new Vector();
        nolines = linelist.size();
        for (int a=0;a<nolines;a++)
        {
            tmpline = (CHLine)linelist.elementAt(a);
            listoflines.insertElementAt(tmpline, 0);
            if (tmpline.child != null)
            {
                tmplist = tmpline.child.getLinesAsVector();
                nolines2 = tmplist.size()-1;
                for (int b=1;b<nolines2;b++)
                {
                    listoflines.insertElementAt(tmplist.elementAt(b), 0);
                }
            }
        }
        return(listoflines);
    }
    
    public void setParent(RegionTreeNode myParent)
    {
        this.myParent=myParent;
    }
    
    /**
     * This function takes the given nodes, which must be children of this node,
     * and creates a single convex hull tree node out of them. This includes
     * rearranging the subtree which was under these nodes. This function
     * is called when the matching algorithm discovers that one concavity
     * in one object should be matched to more than one in the other object.
     *
     * @param tojoin The convex hull tree nodes which should be joinen.
     *
     * @return The coordinates of the points which have been deleted from
     *         this node represented as <code>LineWA</code> objects.
     *         This function will return null if there is an error.
     */
    public LineWA[] joinChildren(ConvexHullTreeNode[] tojoin)
    {
        Vector concvector, linevector, tmpvector, childvector;
        ConvexHullTreeNode[] childlist;
        LineWA[] lines, convhull, cv1, cv2, childlinelist, deletelist;
        int a, begindex, nopoints, tojoinindex, next, now, index1, index2, diff;
        CHLine tmpline;
        concvector = new Vector();
        linevector = new Vector();
        nopoints = 0;
        // Test whether all the members of the tojoin list are children of this
        //node:
        childlist = getChildren();
        for (a=0;a<tojoin.length;a++)
        {
            if (TriRepUtil.indexOf(childlist, tojoin[a]) == -1)
            {
                if(TriRepUtil.debugging)
                    System.out.println("WARNING ConvexHullTreeNode 469: joinChildren called with non-child node!");
                return(null);
            }
            if (tojoin[a] == null)
            {
                if(TriRepUtil.debugging)
                    System.out.println("WARNING CHTN 473: Tojoin element is null!");
            }
        }
        // Remove overlap between this node and other nodes (including children).
        //TriRepUtil.removeAllOverlaps(this);
        // Get the points in concavities as lists of lines inserted in counter-
        // clockwise order.
        for (a=0;a<linelist.size();a++)
        {
            tmpline = (CHLine)linelist.elementAt(a);
            tojoinindex = TriRepUtil.indexOf(tojoin, tmpline.child);
            if ((tmpline.child != null) && (tojoinindex != -1))
            {
                concvector.add(tojoin[tojoinindex].getLines());
                linevector.addAll(Arrays.asList(tojoin[tojoinindex].getOutLine()));
                nopoints += tojoin[tojoinindex].numberOfLines();
            }
        }
        // Construct the convex hull of the set of concavities.
        TriRepUtil.removeDuplicatesR(linevector);
        lines = new LineWA[linevector.size()];
        linevector.toArray(lines);
        convhull = TriRepUtil.convexHull(lines);
        // find the distance between the old concavities on the convex hull.
        // If one is more than 1, this is the "end" of the concavity list.
        begindex = -1;
        cv1 = null;
        cv2 = null;
        for (a=0;a<concvector.size();a++)
        {
            next = (a+1) % concvector.size();
            now = a % concvector.size();
            cv1 = (LineWA[])concvector.elementAt(now);
            cv2 = (LineWA[])concvector.elementAt(next);
            index1 = TriRepUtil.indexOf(convhull, cv1[cv1.length-1]);
            index2 = TriRepUtil.indexOf(convhull, cv2[0]);
            if ((index1 == -1) || (index2 == -1))
            {
                throw(new TriRepCreationException("ConvexHullTreeNode 505: Error: couldn't find end line on convex hull!"));
            }
            if (index2 < index1) index1 -= convhull.length;
            if ((index2-index1) > 1)
            {
                if (begindex != -1)
                {
                    if(TriRepUtil.debugging)
                    {
                        System.out.println("WARNING ConvexHullTreeNode 510: more than one end point: using last!");
                        System.out.print("Number of concavities to join: ");
                        System.out.println(tojoin.length);
                        System.out.println("Length of concavities:");
                        for (int l=0;l<tojoin.length;l++)
                        {
                            System.out.println(tojoin[l].numberOfLines());
                        }
                    }
                }
                begindex = next;
            }
        }
        if (begindex == -1)
        {
            if(TriRepUtil.debugging)
                System.out.println("WARNING ConvexHullTreeNode 516: no begin point found!");
            begindex = 0;
        }
        childvector = new Vector();
        // Create the line list for the joined concavity
        for (a=begindex;a<concvector.size();a++)
        {
            cv1 = (LineWA[])concvector.elementAt(a);
            // If a different from begindex, insert lines between concavities
            if (a != begindex)
            {
                index1 = linelist.indexOf(cv2[cv2.length-1]);
                index2 = linelist.indexOf(cv1[0]);
                diff = index2-index1;
                if (diff < 0)
                {
                    diff +=linelist.size();
                    index2 += linelist.size();
                }
                if (diff > 1)
                {
                    for (int b=index1;b<index2;b++)
                    {
                        childvector.add(linelist.elementAt(b % linelist.size()));
                    }
                }
            }
            // Insert concavity
            childvector.addAll(Arrays.asList(cv1));
            cv2 = cv1;
        }
        for (a=0;a<begindex;a++)
        {
            cv1 = (LineWA[])concvector.elementAt(a);
            // If a different from begindex, insert lines between concavities
            if (a != begindex)
            {
                index1 = linelist.indexOf(cv2[cv2.length-1]);
                index2 = linelist.indexOf(cv1[0]);
                diff = index2-index1;
                if (diff < 0)
                {
                    diff +=linelist.size();
                    index2 += linelist.size();
                }
                if (diff > 1)
                {
                    for (int b=index1;b<index2;b++)
                    {
                        childvector.add(linelist.elementAt(b % linelist.size()));
                    }
                }
            }
            // Insert concavity
            childvector.addAll(Arrays.asList(cv1));
            cv2 = cv1;
        }
        TriRepUtil.removeDuplicatesR(childvector);
        // Remove all lines between the first and last concavities
        index1 = linelist.indexOf(childvector.firstElement());
        index2 = linelist.indexOf(childvector.lastElement());
        tmpvector = new Vector();
        if (index2 < index1)
        {
            nopoints = linelist.size();
            for (a=index1+1;a<nopoints;a++)
            {
                tmpvector.add(linelist.elementAt(index1+1));
                removeLine(index1+1);
            }
            for (a=0;a<index2;a++)
            {
                tmpvector.add(linelist.elementAt(0));
                removeLine(0);
            }
        }
        else
        {
            for (a=index1+1;a<index2;a++)
            {
                tmpvector.add(linelist.elementAt(index1+1));
                removeLine(index1+1);
            }
        }
        index1 = linelist.indexOf(childvector.firstElement());
        index2 = linelist.indexOf(childvector.lastElement());
        // Build new subtree and assign this to child of single line.
        tmpline = (CHLine)linelist.elementAt(index1);
        childlinelist = new LineWA[childvector.size()];
        nopoints = childlinelist.length;
        for (a=0;a<nopoints;a++)
        {
            childlinelist[nopoints-a-1] = (LineWA)childvector.elementAt(a);
        }
        //TriRepUtil.printLineList(childlinelist);
        tmpline.child = new ConvexHullTreeNode(childlinelist,1,this, sourceOrTarget);
        ConvexHullTreeNode[] childChildren=tmpline.child.getChildren();
        for (int i=0;i<childChildren.length;i++)
        {
            childChildren[i].setParent(tmpline.child);
        }
        for(int i=0;i<tojoin.length;i++)
        {
            tojoin[i].setParent(tmpline.child);
        }
        deletelist = new LineWA[tmpvector.size()];
        tmpvector.toArray(deletelist);
        return(deletelist);
    }
    
    public String toString()
    {
        LineWA[] tmp=this.getLines();
        String res="[ "+tmp[0];
        for (int i=1;i< tmp.length;i++)
        {
            res=res+'\n'+"   "+tmp[i];
        }
        res=res+"]"+'\n';
        return(res);
    }
    
}
