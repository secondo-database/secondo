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

//2012, June Simone Jandt

package viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.awt.geom.Rectangle2D.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JNetwork
 * For the graphical interface we limit us almost to the spatial informations
 * about the jnet.
 */
public class JNetwork{

   private Vector junctions = new Vector<JJunction>();
   private Vector sections = new Vector<JSection>();
   private Vector routes = new Vector<JRoute>();
   private String name = "";

  public JNetwork(ListExpr value)
  {
    if (value.listLength() == 5){
      name = value.first().stringValue();
      fillJunctions(value.second());
      fillSections(value.third());
      fillRoutes(value.fourth());
      JNetworkManager.getInstance().addNetwork(this);
    }
    else {
      name = "undefined";
    }
  }

  public int numOfShapes() {
    return junctions.size() + 2*sections.size();
  }

  public boolean isPointType(int no) {
    return ((no < junctions.size()) || isArrowType(no));
  }

  public boolean isLineType(int no) {
    return (no >= junctions.size() && no < (junctions.size() + sections.size()));
  }

  public String toString(){
    return name;
  }

  public Rectangle2D.Double getBounds(){
    if (sections.size()>0){
      Rectangle2D.Double res = ((JSection)sections.get(0)).getBounds();
      for (int i = 1; i < sections.size(); i++){
        Rectangle2D.Double next = ((JSection)sections.get(i)).getBounds();
        if (next != null)
          res.add(next);
      }
      return res;
    } else if (junctions.size() > 0){
      Rectangle2D.Double res = ((JJunction)junctions.get(0)).getBounds();
      for (int i = 1; i < junctions.size(); i++){
        Rectangle2D.Double next = ((JJunction)junctions.get(i)).getBounds();
        if (next != null)
          res.add(next);
      }
      return res;
    } else {
      return null;
    }
  }

  public Shape getRenderObject(int no, AffineTransform af, double pointSize,
                               boolean asRect){
    if (isPointType(no) && !isArrowType(no)){
      return ((JJunction)junctions.get(no)).getRenderObject(af, pointSize, asRect);
    }
    if (isLineType(no)){
      return ((JSection)sections.get(no - junctions.size())).getRenderObject(0,af,pointSize);
    }
    if (isArrowType(no)){
        return ((JSection)sections.get(no-(junctions.size()+sections.size()))).getRenderObject(1, af, pointSize);
    }
    return null;
  }

  public Point2D.Double getPosition(RouteLocation rloc){
    Vector<Integer> secIds = getSids(rloc.getRid());
    if (secIds != null){
      int i = 0;
      while (i < secIds.size()){
        int sid = secIds.get(i++);
        JSection curSect = (JSection)sections.get(sid-1);
        int pos = curSect.contains(rloc);
        if (pos > -1)
          return curSect.getPosition(rloc, pos);
      }
    }
    return null;
  }

  public JSection getSection(JRouteInterval rint){
    Vector<Integer> secIds = getSids(rint.getRid());
    GeneralPath curSectRenderedPath = null;
    GeneralPath curSectPath = null;
    GeneralPath curveRendered = new GeneralPath();
    GeneralPath curve = new GeneralPath();
    Rectangle2D.Double bounds = null;
    Point2D.Double p1 = new Point2D.Double(0.0,0.0);
    Point2D.Double p2 = new Point2D.Double(0.0,0.0);
    Point2D.Double startpoint = new Point2D.Double(0.0,0.0);
    Point2D.Double endpoint =  new Point2D.Double(0.0,0.0);
    boolean hasArrow = (rint.getDir().toString().compareTo("Both") != 0);
    boolean drawArrow = false;
    boolean up = (rint.getDir().toString().compareTo("Up") == 0);
    boolean first = true;
    if (secIds != null) {
      int i = 0;
      while (i < secIds.size()){
        int sid = secIds.get(i++);
        JSection curSect = (JSection) sections.get(sid-1);
        boolean addResult = false;
        int pos = curSect.isCompletelyInside(rint);
        if (pos > -1){
          curSectRenderedPath = curSect.getCurve(true);
          curSectPath = curSect.getCurve(false);
          addResult = true;
        } else {
          pos = curSect.contains(rint);
          if (pos > -1){
            curSectRenderedPath = curSect.getCurve(rint, pos, true);
            curSectPath = curSect.getCurve(rint, pos, false);
            addResult = true;
            startpoint = curSect.getPosition(rint.getStartRLoc(), pos);
            endpoint = curSect.getPosition(rint.getEndRLoc(), pos);
            if (hasArrow) drawArrow = true;
          } else {
            pos = curSect.contains(rint.getStartRLoc());
            if (pos > -1) {
              curSectRenderedPath = curSect.getCurveFrom(rint.getStartRLoc(),
                                                         pos,
                                                         true);
              curSectPath = curSect.getCurveFrom(rint.getStartRLoc(), pos,
                                                 false);
              addResult = true;
              startpoint = curSect.getPosition(rint.getStartRLoc(), pos);
              if (hasArrow && !up) drawArrow = true;
            } else {
              pos = curSect.contains(rint.getEndRLoc());
              if (pos > -1) {
                curSectRenderedPath = curSect.getCurveTo(rint.getEndRLoc(), pos,
                                                         true);
                curSectPath = curSect.getCurveTo(rint.getEndRLoc(),pos, false);
                addResult = true;
                endpoint = curSect.getPosition(rint.getEndRLoc(), pos);
                if (hasArrow && up) drawArrow = true;
              }
            }
          }
        }
        if (drawArrow && curSectRenderedPath != null){
          double[] coords1 = new double[6];
          double[] coords2 = new double[6];
          PathIterator it = curSectRenderedPath.getPathIterator(null);
          it.currentSegment(coords1);
          it.next();
          it.currentSegment(coords2);
          if (up){
            it.next();
            while (!it.isDone()){
              coords1[0] = coords2[0];
              coords1[1] = coords2[1];
              it.currentSegment(coords2);
              it.next();
            }
          }
          p1 = new Point2D.Double(coords1[0], coords1[1]);
          p2 = new Point2D.Double(coords2[0], coords2[1]);
          drawArrow = false;
        }
        if (addResult && curSectRenderedPath != null){
          curveRendered.append(curSectRenderedPath,false);
          curve.append(curSectPath, false);
          if (!first)
            bounds.add(curSectRenderedPath.getBounds2D());
          else{
            first = false;
            bounds = new Rectangle2D.Double(0.0,0.0,0.0,0.0);
            bounds.setRect(curSectRenderedPath.getBounds2D());
          }
          curSectRenderedPath = null;
          curSectPath = null;
          addResult = false;
        }
      }
      boolean startSmaller = (startpoint.x < endpoint.x ||
          (startpoint.x == endpoint.x && startpoint.y <= endpoint.y));
      return new JSection(rint, p1, p2, curve, curveRendered, hasArrow, bounds,
                          startSmaller);
    }
    return null;
  }

  private void fillJunctions(ListExpr juncList){
    ListExpr rest = juncList;
    while (!rest.isEmpty()){
      junctions.add(new JJunction(rest.first()));
      rest = rest.rest();
    }
  }

  private void fillSections(ListExpr sectList){
    ListExpr rest = sectList;
    while(!rest.isEmpty()){
      sections.add(new JSection(rest.first()));
      rest = rest.rest();
    }
  }

  private void fillRoutes(ListExpr routList){
    ListExpr rest = routList;
    while(!rest.isEmpty()){
      routes.add(new JRoute(rest.first()));
      rest = rest.rest();
    }
  }

  private boolean isArrowType(int no){
    return (no >= (junctions.size() + sections.size()) && no < numOfShapes());
  }

  private Vector<Integer> getSids(int rid){
    return ((JRoute)routes.get(rid-1)).getSids();
  }

}



