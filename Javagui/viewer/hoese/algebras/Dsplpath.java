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

/**
 * A Displayclass for a path from the graph-algebra
 */

public class Dsplpath extends DisplayGraph implements DsplSimple
{
	/** The internal datatype representation */
	private Vector verticies;
	private Vector edges;
  private boolean defined;

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
        
    if(isUndefined(v)){
        err = false;
        defined = false;
        return;
    }
    defined = true;

		verticies = new Vector(v.listLength());
		float cost;
		edges = new Vector(v.listLength());
		Dsplvertex tmpVertex;
		if(v.first().second().isAtom())
		{
			Reporter.writeError("Error: It is not posible to view a path without goegraphical context!");
			verticies=null;
			edges=null;				
			err=true;
			return;
		}
		Dsplvertex lastVertex = new Dsplvertex((float) v.first().second()
				.first().realValue(), (float) v.first().second().second()
				.realValue(), v.first().first().intValue());
		verticies.add(lastVertex);
		ListExpr rest = v.rest();
		while (!rest.isEmpty())
		{
			if(rest.second().second().isAtom())
			{
				Reporter.writeError("Error: It is not posible to view a path without goegraphical context!");
				verticies=null;
				edges=null;				
				err=true;
				return;
			}
			cost = (float) rest.first().realValue();
			tmpVertex = new Dsplvertex((float) rest.second().second().first()
					.realValue(), (float) rest.second().second().second()
					.realValue(), rest.second().first().intValue());
			verticies.add(tmpVertex);
			if(tmpVertex.getLabelText(0.0).length()>maxLabelLen) romanNumbers=false;
			if(tmpVertex.getKey()<1)romanNumbers=false;
			if(tmpVertex.getKey()>3999)romanNumbers=false;
			edges.add(new Dspledge(lastVertex.getX(), lastVertex.getY(),
					tmpVertex.getX(), tmpVertex.getY(), cost));
			lastVertex = tmpVertex;
			rest = rest.rest().rest();
		}
		if (!romanNumbers)
		{
		     Iterator it = verticies.iterator();
			    while (it.hasNext()) {
			       ((Dsplvertex)it.next()).normLabel();
			     }
		}
	}

  
  public Shape getRenderObject(int i, AffineTransform af){
     if(i<verticies.size()){
        return ((Dsplvertex)verticies.get(i)).getRenderObject(0,af);
     } else {
         i = i - verticies.size();
         return ((Dspledge) edges.get(i)).getRenderObject(0,af);
     }
  }

  public int numberOfShapes(){
     return verticies.size() + edges.size();
  }


	/**
	 * Gets the bound rectangle of the path by creating a union of the bounds of
	 * all verticies and edges
	 * 
	 * @return The bound rectangle of the path
	 */

	public Rectangle2D.Double getBounds()
	{
		Rectangle2D.Double res = null;
		for (Iterator i = verticies.iterator(); i.hasNext();)
		{

			if (res == null)
				res = ((Dsplvertex) i.next()).getBounds();
			res = (Rectangle2D.Double) res.createUnion(((Dsplvertex) i.next())
					.getBounds());
		}
		for (Iterator i = edges.iterator(); i.hasNext();)
		{
			res = (Rectangle2D.Double) res.createUnion(((Dspledge) i.next())
					.getBounds());
		}
		return res;
	}


	/**
	 * Init. the Dsplpath instance.
	 * 
	 * @param type
	 *            The symbol path
	 * @param value
	 *            the nestedlist representation of the path
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
		} else if(!defined){
      qr.addEntry("undefined");
    } else {
			qr.addEntry(this);
    }
	}

}
