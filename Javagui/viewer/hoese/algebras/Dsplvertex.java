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
 * A Displayclass for a vertex from the graph-algebra
 * 
 */

public class Dsplvertex extends Dsplpoint implements Comparable
{

	/** The internal datatype representation */
	private int keyint;

	private static String replaceAll(String source, String search,
			String replace)
	{
		if (search.equals(replace))
		{
			return source; // kann ja sein, dass wir nichts tun müssen
		}

		StringBuffer result = new StringBuffer();
		int len = search.length();
		if (len == 0)
		{
			return source; // verhindert Endlosschleife bei search.equals("");
		}

		int pos = 0; // position
		int nPos; // next position
		do
		{
			nPos = source.indexOf(search, pos);
			if (nPos != -1)
			{ // gefunden
				result.append(source.substring(pos, nPos));
				result.append(replace);
				pos = nPos + len;
			} else
			{ // nicht gefunden
				result.append(source.substring(pos)); // letzter abschnitt
			}
		} while (nPos != -1);

		return result.toString();
	}
	/**
	 * standard constructor.
	 */

	public Dsplvertex()
	{
		super();
	}

	/**
	 * constructor to set the point and the key of the new vertex
	 * 
	 * @param x
	 *            The x of the vertexpoint
	 * @param y
	 *            The y of the vertexpoint
	 * @param keyin
	 *            the key of the vertex
	 */

	public Dsplvertex(float x, float y, int keyin)
	{
		super();
		this.keyint = keyint;
		String tmpstr = "(" + keyin + "(" + x + " " + y + "))";
		ListExpr tmp = ListExpr.theEmptyList();
		tmp.readFromString(tmpstr);
		ScanValue(tmp);
	}

	/**
	 * Returns the key as label
	 * 
	 * @return The label of the vertex
	 */

	public String getLabelText(double time)
	{
		return label;
	}



	
	public void normLabel()
	{
		label = keyint+"";
	}

	/**
	 * This method sets the Layer and the Category of the vertex.
	 * 
	 * @param dg
	 *            DisplayGraph dg The object to which this vertex belongs
	 */
	public void setDG(DisplayGraph dg)
	{
		RefLayer = dg.RefLayer;
		selected = dg.getSelected();
		Cat = dg.getCategory();
	}

	private String getRomanNumber(int Nr)
	{
		int tmpi=Nr;
		String tmp="";
		while (tmpi>=1000)
		{
			tmpi-=1000;
			tmp=tmp+"M";
		}
		while (tmpi>=500)
		{
			tmpi-=500;
			tmp=tmp+"D";
		}
		while (tmpi>=100)
		{
			tmpi-=100;
			tmp=tmp+"C";
		}
		while (tmpi>=50)
		{
			tmpi-=50;
			tmp=tmp+"L";
		}
		while (tmpi>=10)
		{
			tmpi-=10;
			tmp=tmp+"X";
		}

		while (tmpi>=5)
		{
			tmpi-=5;
			tmp=tmp+"V";
		}
		while (tmpi>=1)
		{
			tmpi-=1;
			tmp=tmp+"I";
		}		
		tmp=replaceAll(tmp,"DCCCC","CM");
		tmp=replaceAll(tmp,"CCCC","CD");
		tmp=replaceAll(tmp,"LXXXX","XC");
		tmp=replaceAll(tmp,"XXXX","XL");
		tmp=replaceAll(tmp,"VIIII","IX");
		tmp=replaceAll(tmp,"IIII","IV");
		return(tmp);
	}
	/**
	 * Get the x of the vertex
	 * 
	 * @return The x
	 */
	public float getX()
	{
		return ((float) point.x);
	}

	/**
	 * Get the y of the vertex
	 * 
	 * @return The y
	 */

	public float getY()
	{
		return ((float) point.y);
	}

	public int getKey()
	{
		return(keyint);
	}
	/**
	 * Scans the nestedlist representation of a vertex
	 * 
	 * @param v
	 *            the nestedlist representation of a vertex
	 */

	protected void ScanValue(ListExpr v)
	{
		if (v.listLength() != 2)
		{
			Reporter
					.writeError("Error: No correct vertex expression: 2 elements needed");
			err = true;
			return;
		}
		super.ScanValue(v.second());
		Double Key;
		if ((v.first().atomType() != ListExpr.INT_ATOM))
		{
			Reporter
					.writeError("Error: No correct vertex : need list of (INT (REAL REAL)) form");
			err = true;
			return;
		}
		Key = LEUtils.readNumeric(v.first());
		if (Key == null)
		{
			err = true;
			label = null;
			return;
		}
		keyint = (int) (Key.doubleValue());
		if(keyint<4000 && keyint>0)
		{
			label = getRomanNumber(keyint);
		}
		else{
			label = keyint+"";
		}
	}

	/**
	 * This is used to use a Dsplvertex in a HashSet
	 * 
	 * @return A Hash code for a vertex
	 */

	public int hashCode()
	{
		long tmp;
		int hc = 17;
		int hashMultiplier = 59;
		hc = hc * hashMultiplier + keyint;
		tmp = ((point.x == 0.0) ? 0L : Double.doubleToLongBits(point.x));
		hc = hc * hashMultiplier + (int) (tmp >>> 32);
		hc = hc * hashMultiplier + (int) (tmp & 0xFFFFFFFF);
		tmp = ((point.x == 0.0) ? 0L : Double.doubleToLongBits(point.y));
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
		if(!(o instanceof Dsplvertex))
			return false;
		Dsplvertex that = (Dsplvertex) o;
		return this.keyint == that.keyint && this.point.x == that.point.x
				&& this.point.y == that.point.y;
	}

	/**
	 * This is used to use a Dsplvertex in a TreeSet
	 * 
	 * @param o
	 *            The object to compare to
	 * @return A number greater than 0 if this is greater then o
	 */

	public int compareTo(Object o)
	{
		Dsplvertex other;
		try
		{
			other = (Dsplvertex) o;
		} catch (Exception e)
		{
			return (0);
		}
		return (this.keyint - other.keyint);
	}

	/**
	 * Init. the Dsplvertex instance.
	 * 
	 * @param type
	 *            The symbol vertex
	 * @param value
	 *            the nestedlist representation of the vertex
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
			qr.addEntry(new String("(" + AttrName + ": GA(vertex))"));
			return;
		} else
			qr.addEntry(this);
	}

}
