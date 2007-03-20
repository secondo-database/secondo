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

package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import gui.SecondoObject;

/**
 * A Displayclass for an graph from the graph-algebra
 */
public class Dsplgraph extends DisplayGraph implements DsplSimple, DisplayComplex
{
	/** The internal datatype representation */
	String label;
	Vector verticies;
	Vector edges;	
  Rectangle2D.Double bounds=null;


private int comparePoints(Point2D.Double p1, Point2D.Double p2){
  if(p1.x<p2.x) return -1;
  if(p1.x>p2.x) return 1;
  if(p1.y<p2.y) return -1;
  if(p1.y>p2.y) return 1;
  return 0;
}


	
/**
 * Find out, if the graph is directed. If it is, then not only true is given in the edges hash is also every double edge removed. 
 * 
 */
	
	private boolean isUndirected()
{
    if(edges==null){
       return true;
    }
    int size = edges.size();

    TreeSet tmpedges= new TreeSet(edges);
    Vector newEdges = new Vector(size/2);

    for(int i=0;i<size;i++){
      Dspledge edge = (Dspledge)edges.get(i);
      Dspledge sEdge = edge.getSymm();
      if(comparePoints(edge.point1,edge.point2)<=0){
         if(!tmpedges.contains(sEdge)){
             return false;
         } else {
             newEdges.add(edge);
         }
      }
    }

    edges = newEdges;
    return true;

/*


		Dspledge tmpe,tmpe2;
		{
			Vector tmpedges=(Vector)edges.clone();  			//a temporary hashset, where the double edges will be removed
			for (Iterator i = edges.iterator(); i.hasNext();)
			{
				tmpe = ((Dspledge) i.next());
				tmpe2=new Dspledge(tmpe.point2.x,tmpe.point2.y,tmpe.point1.x,tmpe.point1.y,tmpe.cost);	
				if(edges.contains(tmpe2)||(tmpe.point1.x==tmpe.point2.x&&tmpe.point1.y==tmpe.point2.y))
				{					
					if(tmpedges.contains(tmpe2)&&tmpedges.contains(tmpe)&&!(tmpe.point1.x==tmpe.point2.x&&tmpe.point1.y==tmpe.point2.y))
						//remove no revective edges
						tmpedges.remove(tmpe2);
				}
				else
				{
					return(false);
				}			
			}
			edges=new Vector(tmpedges);
			return(true);			
		}
*/

}




  /** returns always vertices and edges.
    **/
   public Shape getRenderObject(int no, AffineTransform af){
       if(no<verticies.size()){
           return ((Dsplvertex) verticies.get(no)).getRenderObject(0,af);
       } else {
           no = no -verticies.size();
           return ((Dspledge)edges.get(no)).getRenderObject(0,af);
       } 
   }

   /** draw all Labels
   **/
   public void draw(Graphics g, double time, AffineTransform af){

     double maxd = Math.min(Math.abs(af.getScaleX()),Math.abs(af.getScaleY())); 
     
     Rectangle2D rect = getBounds();
     if(rect==null){
       return;
     } 
     double max1 = (maxd*rect.getWidth()/120.0) * (maxd*rect.getHeight())/30;
     max1 = max1/4.0;

     int max = (int)(max1);
     if(max<verticies.size()){
       return;
     }

     Color oldC = g.getColor();

     for(int i=0;i<verticies.size();i++){
        Dsplvertex v = (Dsplvertex) verticies.get(i);
        Layer.drawLabel(v,g,v.getBounds(),time,af,Color.BLUE);
     }

     max = max - verticies.size();
     if(max<verticies.size()+edges.size()){
        return;
     }
     for(int i=0;i<edges.size();i++){
        Dspledge e = (Dspledge) edges.get(i);
        Layer.drawLabel(e,g,e.getBounds(),time,af,Color.RED);
     }
     g.setColor(oldC);

   }



   /** Returns 0. */
   public int numberOfShapes(){
      int shapes = verticies.size() + edges.size();
      return shapes;
   }

   public boolean isPointType(int no){
       return no<verticies.size();
   }

   public boolean isLineType(int no){
      return (no>=verticies.size());
   }



	/**
	 * Scans the numeric representation of a point datatype
	 * 
	 * @param v
	 *            the nestedlist representation of the graph
	 */

	protected void ScanValue(ListExpr v)
	{
   
		int maxLabelLen=5;
		boolean romanNumbers=true;
		Dsplvertex tmp;
		
    if(v.listLength()!=2){
        Reporter.writeError("ListLength of the graph type must be two.");
        err=true; 
        verticies = null;
        edges = null;
        return;
    }

		ListExpr VList, EList;
		VList = v.first();
		EList = v.second();
		ListExpr rest = VList;

	
    int vlen = VList.listLength();
    verticies = new Vector(vlen);
    HashMap map = new HashMap(vlen*3);
      
    while (!rest.isEmpty())
		{
      // each vertex has to be format (id (xpos, ypos))
      ListExpr V = rest.first();
      if(V.listLength()!=2){
        Reporter.writeError("ListLength of a vertex must be two.");
        err=true; 
        verticies = null;
        edges = null;
        return;
      }
      if(V.first().atomType()!=ListExpr.INT_ATOM ||
         V.second().atomType()!=ListExpr.NO_ATOM){
        Reporter.writeError("invalid representation of a vertex found");
        err=true; 
        verticies = null;
        edges = null;
        return;
      }
      ListExpr posList = V.second();
      if(posList.listLength()!=2 || posList.first().atomType()!=ListExpr.REAL_ATOM ||
         posList.second().atomType()!=ListExpr.REAL_ATOM){
        Reporter.writeError("invalid representation of a position found");
        err=true; 
        verticies = null;
        edges = null;
        return;
      } 
      double x = posList.first().realValue();
      double y = posList.second().realValue();
      int id = V.first().intValue();

			tmp=new Dsplvertex((float)x, (float)y, id);
			verticies.add(tmp);

			if(tmp.getLabelText(0.0).length()>maxLabelLen) romanNumbers=false;
			if(tmp.getKey()<1)romanNumbers=false;
			if(tmp.getKey()>3999)romanNumbers=false;			
      map.put(new Integer(id),new Integer(verticies.size()-1));
			rest = rest.rest();
		}
		if (!romanNumbers)
		{
			     Iterator it = verticies.iterator();
			    while (it.hasNext()) {
			       ((Dsplvertex)it.next()).normLabel();
			     }
		}

		rest = EList;
    edges = new Vector(EList.listLength());

		while (!rest.isEmpty())
		{
      ListExpr edgeList = rest.first();
      if(edgeList.listLength()!=3 ||
         edgeList.first().atomType()!=ListExpr.INT_ATOM ||
         edgeList.second().atomType()!=ListExpr.INT_ATOM ||
         edgeList.third().atomType()!=ListExpr.REAL_ATOM){
        
        Reporter.writeError("invalid representation of an egde found");
        err=true; 
        verticies = null;
        edges = null;
        return;
      }
      int v1 = edgeList.first().intValue();
      int v2 = edgeList.second().intValue();
      double cost = edgeList.third().realValue();
      Integer V1pos  = (Integer)map.get(new Integer(v1));
      Integer V2pos = (Integer)map.get(new Integer(v2));
      if(V1pos==null || V2pos==null){
        Reporter.writeError("edge with invalid node numbers found ("+v1+", "+v2+")");
        err=true; 
        verticies = null;
        edges = null;
        return;
       }
       Dsplvertex vert1 = (Dsplvertex)verticies.get(V1pos.intValue());
       Dsplvertex vert2 = (Dsplvertex)verticies.get(V2pos.intValue());
       edges.add(new Dspledge(vert1.getX(),vert1.getY(),vert2.getX(),vert2.getY(),cost)); 
			rest = rest.rest();
		} 
    
    if(isUndirected())
		{			
			Dspledge tmpe;
			  Iterator it = edges.iterator();
			    while (it.hasNext()) {
			    	tmpe=(Dspledge)it.next();		
			    	tmpe.noArrow();
			     }
		}
	  bounds = computeBounds();	
	}


private Rectangle2D.Double computeBounds(){
		Rectangle2D.Double res = null;
    if(verticies==null){
         return res;
    }
		for(int i=0; i< verticies.size();i++)
		{
      Rectangle2D.Double rect = ((Dsplvertex)verticies.get(i)).getBounds();
      if(rect!=null){
        if(res==null){
           res = rect;
        } else{
           res = (Rectangle2D.Double)res.createUnion(rect);
        }
      }
  
		}
		return res;
}

	/**
	 * Gets the bound rectangle of the graph by creating a union of the bounds
	 * of all verticies and edges
	 * 
	 * @return The bound rectangle of the graph
	 */

	public Rectangle2D.Double getBounds()
	{
	  return bounds;	
	}


	/**
	 * Init. the Dsplgraph instance.
	 * 
	 * @param type
	 *            The symbol graph
	 * @param value
	 *            the nestedlist representation of the graph
	 * @param qr
	 *            queryresult to display output.
	 */

	public void init(ListExpr type, ListExpr value, QueryResult qr)
	{
		AttrName = type.symbolValue();
		ScanValue(value);
		if (err)
		{
			Reporter.writeError("Error in ListExpr :parsing aborted");
			qr.addEntry(new String("(" + AttrName + ": GA(graph))"));
			return;
		} else
			qr.addEntry(this);
	}

}
