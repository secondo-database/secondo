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
import java.text.DecimalFormat;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;

/**
 * A Displayclass for an edge from the graph-algebra
 * 
 */

public class Dspledge extends Dsplline implements Comparable
{

	/** The internal datatype representation */
	Point2D.Double point1;
	Point2D.Double point2;
	double cost;
	boolean isArrow = true;
	static DecimalFormat format = new DecimalFormat(",##0.00");	
		
	public String toString()
	{
		return ("(" + point1.x + "/" + point1.y + ")" + "--->(" + point2.x
				+ "/" + point2.y + ") " + cost);
	}

  public Dspledge getSymm(){
    return new Dspledge(point2, point1, cost, isArrow);
  }

  public Dspledge(Point2D.Double p1, Point2D.Double p2, double cost, boolean isArrow){
     this.point1 = p1;
     this.point2 = p2;
     this.cost = cost;
     this.isArrow = isArrow;
  } 

  public int compareTo(Object o){
    if(!(o instanceof Dspledge)){
        return -1;
    }
    Dspledge e = (Dspledge) o;
    if(point1.x < e.point1.x) return -1;
    if(point1.x > e.point1.x) return 1;
    if(point1.y < e.point1.y) return -1;
    if(point1.y > e.point1.y) return 1;
    if(cost < e.cost) return -1;
    if(cost > e.cost) return 1;
    // ignore isArrow
    return 0;

  }



	/**
	 * This Method adds an arrowhead at the end of the edge
	 */

	private void arrowhead()
	{
		double headlen = .05;
		double headwidth = .6;
		double headform = .5;
		double rRef = 1.5;
		double abw = .2;
		if (RefLayer != null)
		{

			double ps = Cat.getPointSize(renderAttribute,
					CurrentState.ActualTime);
			double pixy = Math.abs(ps / RefLayer.getProjection().getScaleY());
			double pix = Math.abs(ps / RefLayer.getProjection().getScaleX());
			if (point1.equals(point2))
			{
				headlen = headlen * 3;
				double r1 = pix / 2.0;
				double r2 = rRef * pix / 2.0;
				double Mx = point2.x;
				double My = point2.y + pixy * abw - r2;
				double d = point2.y - My;
				double a = ((r2 * r2) - (r1 * r1) + (d * d)) / (2.0 * d);
				double y2 = My + a;
				double h = Math.sqrt((r2 * r2) - (a * a));
				double xi = Mx - h;
				double xi_prime = Mx + h;
				double alpha = Math.toDegrees(Math.asin(h / r2));
				double winkelPfeil = 270 - alpha - (360 - 2 * alpha) * headlen;
				double xP = Mx + r2 * Math.cos(Math.toRadians(winkelPfeil));
				double yP = My - r2 * Math.sin(Math.toRadians(winkelPfeil));
				double winkelPfeil2 = 270 - alpha - (360 - 2 * alpha) * headlen
						+ (360 - 2 * alpha) * headlen * headform;
				double xP2 = Mx + r2 * Math.cos(Math.toRadians(winkelPfeil2));
				double yP2 = My - r2 * Math.sin(Math.toRadians(winkelPfeil2));
				double hl = Math.sqrt((xi - xP) * (xi - xP) + (y2 - yP)
						* (y2 - yP));
				double xP3 = Mx + (r2 + hl * headwidth)
						* Math.cos(Math.toRadians(winkelPfeil));
				double yP3 = My - (r2 + hl * headwidth)
						* Math.sin(Math.toRadians(winkelPfeil));
				double xP4 = Mx + (r2 - hl * headwidth)
						* Math.cos(Math.toRadians(winkelPfeil));
				double yP4 = My - (r2 - hl * headwidth)
						* Math.sin(Math.toRadians(winkelPfeil));
				GP.reset();
				if (this.isArrow)
				{
					GP.moveTo((float) xi, (float) y2);
					GP.lineTo((float) xP3, (float) (yP3));
					GP.lineTo((float) xP2, (float) (yP2));
					GP.lineTo((float) xP4, (float) (yP4));
					GP.lineTo((float) xi, (float) (y2));
				}
				GP.append(new Arc2D.Double(Mx - r2, My - r2, r2 * 2.0,
						r2 * 2.0, 270 + alpha, 360 - 2 * alpha, Arc2D.OPEN),
						false);
			} else
			{
				double endpx = point2.x
						- (point2.x - point1.x)
						/ (Math.sqrt((point2.x - point1.x)
								* (point2.x - point1.x) + (point2.y - point1.y)
								* (point2.y - point1.y))) * pix / 2;
				double endpy = point2.y
						- (point2.y - point1.y)
						/ (Math.sqrt((point2.x - point1.x)
								* (point2.x - point1.x) + (point2.y - point1.y)
								* (point2.y - point1.y))) * pixy / 2;

				double edgevecx = ((endpx - point1.x));
				double edgevecy = ((endpy - point1.y));
				double edgelen = (Math.sqrt(edgevecx * edgevecx + edgevecy
						* edgevecy));
				double edgevecxnorm = edgevecx / edgelen;
				double edgevecynorm = edgevecy / edgelen;

				GP.reset();// Delete old GP to avoid multiple heads
				GP.moveTo((float) point1.x, (float) point1.y);// Redraw the
																// Line
				GP.lineTo((float) endpx, (float) endpy);// Redraw the Line
				if (this.isArrow)
				{
					GP.moveTo((float) endpx, (float) endpy);
					GP.lineTo(
							(float) (endpx - edgevecx * headlen + edgevecynorm
									* headlen * headwidth * edgelen),
							(float) (endpy - edgevecy * headlen - edgevecxnorm
									* headlen * headwidth * edgelen));
					GP.lineTo((float) (endpx - edgevecx * headlen * headform),
							(float) (endpy - edgevecy * headlen * headform));
					GP
							.lineTo(
									(float) (endpx - edgevecx * (headlen) - edgevecynorm
											* headlen * headwidth * edgelen),
									(float) (endpy - edgevecy * headlen + edgevecxnorm
											* headlen * headwidth * edgelen));
					GP.lineTo((float) endpx, (float) endpy);
				}
			}
		}
	}

	public void noArrow()
	{
		this.isArrow = false;
	}
	/**
	 * Returns the formated costs of the edge as label of the edge
	 * 
	 * @return The label of the edge
	 */

	public String getLabelText(double time)
	{
		return format.format(cost);
	}

	/**
	 * standard constructor.
	 */

	public Dspledge()
	{
		super();
	}

	public Rectangle2D.Double getBounds()
	{
		arrowhead();
		Rectangle2D.Double tmp = new Rectangle2D.Double();
		tmp.setRect(GP.getBounds2D());
		return tmp;
	}

	/**
	 * Constructor used by graphs and pathes
	 * 
	 * @param x1
	 *            The x of the startpoint
	 * @param y1
	 *            The y of the startpoint
	 * @param x2
	 *            The x of the endpoint
	 * @param y2
	 *            The y of the endpoint
	 * @param cost
	 *            The costs of the edge
	 */

	public Dspledge(double x1, double y1, double x2, double y2, double cost)
	{
		super();	
		point1 = new Point2D.Double(x1, y1);
		point2 = new Point2D.Double(x2, y2);
		this.cost = cost;
		String tmpstr = "((" + x1 + " " + y1 + " " + x2 + " " + y2 + "))";
		ListExpr tmp = ListExpr.theEmptyList();
		tmp.readFromString(tmpstr);
		super.ScanValue(tmp);
		arrowhead();
		if (err)
		{
			Reporter.writeError("Error in ListExpr :parsing aborted");
			return;
		}
		entry = AttrName + " : line";
		defined = GP != null;
		err = false;
	}

	/**
	 * This method sets the Layer and the Category of the vertex.
	 * 
	 * @param dg
	 *            The object to which this edge belongs
	 */

	public void setDG(DisplayGraph dg)
	{
		RefLayer = dg.RefLayer;
		selected = dg.getSelected();
		Cat = dg.getCategory();
		arrowhead();
	}

	/**
	 * This Method does nothing but throwing an error. It is not possible to
	 * view a single edge.
	 * 
	 * @param value
	 */

	public void ScanValue(ListExpr value)
	{
		Reporter.writeError("Error: It is not posible to view a single Edge. Please view an Edge as part of a Path or Graph! ");
		GP = null;
		err = true;

	}

	/**
	 * This is used to use a Dspledge in a HashSet
	 * 
	 * @return A Hash code for an edge
	 */

	public int hashCode()
	{
		long tmp;
		int hc = 9;
		int hashMultiplier = 53; // nächste Primzahlen: 61 ;67
		tmp = ((point1.x == 0.0) ? 0L : Double.doubleToLongBits(point1.x));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		tmp = ((point1.y == 0.0) ? 0L : Double.doubleToLongBits(point1.y));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		tmp = ((point2.x == 0.0) ? 0L : Double.doubleToLongBits(point2.x));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		tmp = ((point2.y == 0.0) ? 0L : Double.doubleToLongBits(point2.y));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		tmp = ((this.cost == 0.0) ? 0L : Double.doubleToLongBits(this.cost));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		return (hc);
	}

	public boolean equals(Object o)
	{
		if (o == null)
			return false;
		if (o == this)
			return true;
		if(!(o instanceof Dspledge))
			return false;
		Dspledge that = (Dspledge) o;
		return this.cost == that.cost && this.point1.x == that.point1.x
				&& this.point1.y == that.point1.y
				&& this.point2.x == that.point2.x
				&& this.point2.y == that.point2.y;
	}

	/**
	 * Init. the Dspledge instance.
	 * 
	 * @param type
	 *            The symbol edge
	 * @param value
	 *            the nestedlist representation of the edge
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
			qr.addEntry(new String("(" + AttrName + ": GA(edge))"));
			return;
		} else
			qr.addEntry(this);
	}

}
