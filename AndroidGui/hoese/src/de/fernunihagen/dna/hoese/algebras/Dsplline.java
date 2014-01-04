package de.fernunihagen.dna.hoese.algebras;

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

import sj.lang.ListExpr;

import javamini.awt.geom.Rectangle2D;
import java.util.*;

import javamini.awt.geom.AffineTransform;
import javamini.awt.geom.GeneralPath;
import de.fernunihagen.dna.hoese.LEUtils;
import javamini.awt.geom.Point2D;
import de.fernunihagen.dna.hoese.ProjectionManager;
import de.fernunihagen.dna.hoese.QueryResult;
import javamini.awt.Shape;
import tools.Reporter;

/**
 * The displayclass of the line datatype (Rose algebra).
 */
public class Dsplline extends DisplayGraph {
	/** The bounding-box rectangle */
	Rectangle2D.Double bounds;
	/** The shape representing this line */
	GeneralPath GP;
	/** boolean flag indicating the defined state */
	boolean defined;
	/** The textual representation of this line **/
	String entry;

	/** returns true because this type is a line **/
	public boolean isLineType(int num) {
		return true;
	}

	public int numberOfShapes() {
		return 1;
	}

	public Shape getRenderObject(int num, AffineTransform at) {
		if (num != 0) {
			return null;
		} else {
			return GP;
		}
	}

	/** returns the string for the textual representation of this **/
	public String toString() {
		if (err || !defined) {
			return entry;

		} else {
			return entry + " (" + Cat.getName() + ")";
		}

	}

	/**
	 * Scans the representation of the line datatype and constucts the lines
	 * Vector.
	 * 
	 * @param v
	 *            A list of segments
	 * @see sj.lang.ListExpr
	 * @see <a href="Dspllinesrc.html#ScanValue">Source</a>
	 */
	public boolean ScanValue(ListExpr value, Point2D.Double last1,
			Point2D.Double last2, boolean useProjection) {
		if (isUndefined(value)) {
			defined = false;
			GP = null;
			return false;
		}
		defined = true;
		double koord[] = new double[4];
		double x1, y1, x2, y2;
		double lastX = 0, lastY = 0;
		boolean first = true;
		GP = new GeneralPath();
		while (!value.isEmpty()) {
			ListExpr v = value.first();
			if (v.listLength() != 4) {
				Reporter.writeError("Error: No correct segment expression: 4 elements needed");
				GP = null;
				err = true;
				return false;
			}
			for (int koordindex = 0; koordindex < 4; koordindex++) {
				Double d = LEUtils.readNumeric(v.first());
				if (d == null) { // error detected
					err = true;
					GP = null;
					return false;
				}
				koord[koordindex] = d.doubleValue();
				v = v.rest();
			}
			try {
				if (useProjection) {
					if (!ProjectionManager.project(koord[0], koord[1], aPoint)) {
						err = true;
						Reporter.debug("error in project segment (" + koord[0]
								+ "," + koord[1] + ")->" + koord[2] + ","
								+ koord[3] + ")");
						GP = null;
						return false;
					} else {
						x1 = aPoint.x;
						y1 = aPoint.y;
						if (!ProjectionManager.project(koord[2], koord[3],
								aPoint)) {
							err = true;
							GP = null;
							Reporter.debug("error in project segment ("
									+ koord[0] + "," + koord[1] + ")->"
									+ koord[2] + "," + koord[3] + ")");
							return false;
						} else {
							x2 = aPoint.x;
							y2 = aPoint.y;
						}
					}
				} else {
					x1 = koord[0];
					y1 = koord[1];
					x2 = koord[2];
					y2 = koord[3];
				}

				last1.setLocation(x1, y1);
				last2.setLocation(x2, y2);

				if (first) {
					GP.moveTo((float) x1, (float) y1);
					GP.lineTo((float) x2, (float) y2);
					first = false;
					lastX = x2;
					lastY = y2;
				} else {
					if ((lastX == x1) && (lastY == y1)) {
						GP.lineTo((float) x2, (float) y2);
						lastX = x2;
						lastY = y2;
					} else if ((lastX == x2) && (lastY == y2)) {
						GP.lineTo((float) x1, (float) y1);
						lastX = x1;
						lastY = y1;
					} else { // not connected
						GP.moveTo((float) x1, (float) y1);
						GP.lineTo((float) x2, (float) y2);
						lastX = x2;
						lastY = y2;
					}
				}
			} catch (Exception e) {
				Reporter.debug(e);
			}
			value = value.rest();
		}
		if (first) { // empty line
			GP = null;
			return false;
		}
		return true;
	}

	public boolean ScanValue(ListExpr value, Point2D.Double last1,
			Point2D.Double last2) {
		return ScanValue(value, last1, last2, true);
	}

	public void ScanValue(ListExpr value) {

		Point2D.Double p1 = new Point2D.Double();
		Point2D.Double p2 = new Point2D.Double();

		ScanValue(value, p1, p2);
	}

	public void ScanValue(ListExpr value, boolean useProjection) {

		Point2D.Double p1 = new Point2D.Double();
		Point2D.Double p2 = new Point2D.Double();

		ScanValue(value, p1, p2, useProjection);
	}

	public void init(String name, int nameWidth, int indent, ListExpr type,
			ListExpr value, QueryResult qr) {
		AttrName = extendString(name, nameWidth, indent);
		ScanValue(value);
		if (err) {
			Reporter.writeError("Error in ListExpr :parsing aborted");
			entry = AttrName + " : <error>";
			qr.addEntry(entry);
			bounds = null;
			GP = null;
			return;
		} else if (!defined) {
			entry = AttrName + " : undefined";
			qr.addEntry(entry);
			return;
		}
		// normal case-> defined line
		entry = AttrName + " : line";
		defined = GP != null;
		err = false;
		qr.addEntry(this);
		if (GP == null)
			bounds = null;
		else {
			bounds = new Rectangle2D.Double();
			bounds.setRect(GP.getBounds2D());
		}
	}

	/**
	 * @return The boundingbox of this line-object
	 * @see <a href="Dspllinesrc.html#getBounds">Source</a>
	 */
	public Rectangle2D.Double getBounds() {
		return bounds;
	}

	/**
	 * Tests if a given position is near (10pxs) of this line, by iterating over
	 * all segments.
	 * 
	 * @param xpos
	 *            The x-Position to test.
	 * @param ypos
	 *            The y-Position to test.
	 * @param scalex
	 *            The actual x-zoomfactor
	 * @param scaley
	 *            The actual y-zoomfactor
	 * @return true if x-, ypos is contained in this points type
	 */
	public boolean contains(double xpos, double ypos, double scalex,
			double scaley) {
		if (bounds == null)
			return false; // an empty line
		// bounding box test
		if ((bounds.getWidth() * bounds.getHeight() != 0)
				&& (!bounds.intersects(xpos - 5.0 * scalex,
						ypos - 5.0 * scaley, 10.0 * scalex, 10.0 * scaley)))
			return false;
		Rectangle2D.Double r = new Rectangle2D.Double(xpos - 5.0 * scalex, ypos
				- 5.0 * scaley, 10.0 * scalex, 10.0 * scaley);

		return GP.intersects(r);
	}

}
