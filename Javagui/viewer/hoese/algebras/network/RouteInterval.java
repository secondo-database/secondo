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

import java.awt.Shape;
import java.awt.geom.AffineTransform;

import sj.lang.ListExpr;
import tools.Reporter;

/**
 * A <code>RouteInterval</code> is a part of a <code>GLine</code>
 * 
 * @author Martin Scheppokat
 *
 */
public class RouteInterval 
{
  /**
   * The Id of the network this route belongs to.
   */
  private long m_lNetworkId;
  
  /**
   * The Id of the route 
   * The RouteInterval is a part of that route.
   */
  private int m_iRouteId;
  
  /**
   * Start 
   */
  private double m_dStart;
  
  /**
   * End
   */
  private double m_dEnd;

  /**
   * The part of the route for this RouteInterval
   */
  Route m_xRoutePart;
  
  /**
   * Constructor
   * 
   * @param in_lNetworkId
   * @param in_xValue
   * @throws NetworkNotAvailableException 
   */
  public RouteInterval(long in_lNetworkId,
                       ListExpr in_xValue) 
    throws NetworkNotAvailableException 
  {
    m_lNetworkId = in_lNetworkId;
    
    if(in_xValue.listLength() != 3)
    {
      Reporter.writeError("RouteInterval: List length must be 3.");
    }
    
    ListExpr xRouteIdList = in_xValue.first();
    ListExpr xStartList = in_xValue.second();
    ListExpr xEndList = in_xValue.third();
        
    if(!xRouteIdList.isAtom() ||
        xRouteIdList.atomType() != ListExpr.INT_ATOM)
    {
      Reporter.writeError("RouteInterval: First part of the list must be an int atom.");      
    }
    if(!xStartList.isAtom() ||
        xStartList.atomType() != ListExpr.REAL_ATOM)
    {
      Reporter.writeError("RouteInterval: Second part of the list must be an real atom.");      
    }
    if(!xEndList.isAtom() ||
        xEndList.atomType() != ListExpr.REAL_ATOM)
    {
      Reporter.writeError("RouteInterval: Third part of the list must be an real atom.");      
    }
    
    m_iRouteId = xRouteIdList.intValue();
    m_dStart = xStartList.realValue();
    m_dEnd = xEndList.realValue();
    
    
    // Load the part of the route.
    Network xNetwork = NetworkManager.getInstance().getNetwork(m_lNetworkId);
    Route xRoute = xNetwork.getRouteById(m_iRouteId);
    m_xRoutePart = xRoute.getPartOfRoute(m_dStart,
                                             m_dEnd);
  }

  /**
   * Returns a Shape to display this route in the HoeseViewer.
   * 
   * @return
   * @throws NetworkNotAvailableException
   */
  public Shape getRenderObject() 
    throws NetworkNotAvailableException 
  {
    return m_xRoutePart.getRenderObject();
  }

}
