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

package viewer.hoese.algebras.jnet;

import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;

import sj.lang.ListExpr;

/**
 * A position in a given network.
 *
 * @author Simone Jandt
 *
 */
public class JPoint
{
  /**
  * NetId of the JPoint
  */
  private String nid;

  /**
  * Networkposition JPoint
  */
  private JRouteLocation rloc;

  /**
  * Spatialposition of JPoint
  */
  private Point2D.Double pos;

  /**
   * Constructor.
   *
   * @param inList List in secondo-format
   * @throws JNetworkNotAvailableException
   */
  public JPoint(ListExpr inList)
    throws JNetworkNotAvailableException, Exception
  {
    // TODO: Check format before reading out the values

    nid = inList.first().stringValue();
    JNetwork net = JNetManager.getInstance().getJNetwork(nid);
    rloc = new JRouteLocation(inList.second());
    JRoute route = net.getRouteById(rloc.getRouteId());
    pos = route.getPointOnRoute(rloc.getPosition());
  }


  /**
  * Return network id of the JPoint
  *
  * @return netId
  */
  public String getNetworkId()
  {
    return nid;
  }

   /**
   * Returns this <code>JPoint</code> in a representation suitable for
   * displaying in the HoeseViewer.
   *
   * @return A point
   * @throws Exception
   * @throws Exception If the network had not yet been loaded.
   */
  public Point2D.Double getRenderObject()
    throws JNetworkNotAvailableException
  {
    return pos;
  }

}