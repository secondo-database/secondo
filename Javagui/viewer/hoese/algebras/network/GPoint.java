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

package viewer.hoese.algebras.network;

import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;

import sj.lang.ListExpr;

/**
 * A point defined via a network.
 * 
 * @author Martin Scheppokat
 *
 */
public class GPoint
{

  /**
   * The id of the network this <code>GPoint</code> belongs to.
   */
  private int m_iNetworkId;
  
  /**
   * The route in the network this <code>GPoint</code> lies on.
   */
  private int m_iRouteId;
  
  /**
   * The distance between the start of the route and this <code>GPoint</code>.
   */
  private double m_dDistance;
  
  /**
   * Constructor. 
   * 
   * @param in_xList List in secondo-format
   */
  public GPoint(ListExpr in_xList) 
  {
    // TODO: Check format before reading out the values
    
    m_iNetworkId = in_xList.first().intValue();
    m_iRouteId = in_xList.second().intValue();
    m_dDistance = in_xList.third().realValue();
  }

  /**
   * Returns this <code>GPoint</code> in a representation suitable for
   * displaying in the HoeseViewer.
   *   
   * @return A point
   * @throws Exception 
   * @throws Exception If the network had not yet been loaded.
   */
  public Point2D.Double getRenderObject()
    throws NetworkNotAvailableException 
  {
    // Get Network. If the network is not loaded an exception is
    // thrown
    Network xNetwork = NetworkManager.getInstance().getNetwork(m_iNetworkId);
    
    // Calculate the point in absolute coordinates
    Route xRoute = xNetwork.getRouteById(m_iRouteId);
    return xRoute.getPointOnRoute(m_dDistance);
  }
}
