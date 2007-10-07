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

import viewer.hoese.Interval;

/**
 * A unit of a GPoint
 * @author Martin Scheppokat
 *
 */
public class UGPoint 
{

  /**
   * Time period this unit is valid
   */
  private Interval m_xInterval;
  
  /**
   * Id of the network
   */
  private long m_lNetworkId;
  
  /**
   * Id of the Route
   */
  private int m_iRouteId;
  
  /** 
   * Side of the route
   */
  private int m_iSideId;
  
  /** 
   * Position at start of time interval
   */
  private double m_dPos1;
  
  /**
   * Position at the end of the time interval.
   * 
   */
  private double m_dPos2;

  /**
   * Constructor
   * 
   * @param in_xInterval
   * @param in_lNetworkId
   * @param in_iRouteId
   * @param in_iSideId
   * @param in_dPos1
   * @param in_dPos2
   */
  public UGPoint(Interval in_xInterval, 
                 long in_lNetworkId, 
                 int in_iRouteId, 
                 int in_iSideId, 
                 double in_dPos1, 
                 double in_dPos2) 
  {
    m_xInterval = in_xInterval;
    m_lNetworkId = in_lNetworkId;
    m_iRouteId = in_iRouteId;
    m_iSideId = in_iSideId;
    m_dPos1 = in_dPos1;
    m_dPos2 = in_dPos2;
  }

  /**
   * Get-Method for the id of the route.
   * 
   * @return Id
   */
  public int getRouteId() 
  {
    return m_iRouteId;
  }

  /**
   * Get-Method for the first position.
   * 
   * @return Position on route
   */
  public double getPos1() 
  {
    return m_dPos1;
  }

  /**
   * Get-Method for the second position.
   * 
   * @return Position on route
   */
  public double getPos2() 
  {
    return m_dPos2;
  }

  /**
   * Get-Method for the id of the network.
   * 
   * @return Id
   */
  public long getNetworkId()
  {
    return m_lNetworkId;
  }
}
