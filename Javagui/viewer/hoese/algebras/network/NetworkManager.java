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

import java.util.HashMap;
import java.util.Vector;

/**
 * GPoints, GLines and their moving counterparts can only be displayed in the
 * HoeseViewer if the network they belong two has allready been loaded. 
 * 
 * Each Network will be registered at this NetworkManager. When
 * it is needed it is available via it's id. If it has not been loaded an 
 * exception will be thrown.
 * 
 * WARNING: Networks are only released when a new network with the same id 
 * is loaded! 
 * 
 * @author Martin Scheppokat
 *
 */
public class NetworkManager 
{
  /**
   * Instance for the Singleton-Pattern of GOF
   */
  private static final NetworkManager INSTANCE = new NetworkManager();
  
  /**
   * Get-Method returning the instance of the <code>NetworkManager</code>
   * 
   * @return The <code>NetworkManager</code>
   */
  public static NetworkManager getInstance()
  {
    return INSTANCE;
  }

  /**
   * All Networks loaded in the client
   */
  private HashMap m_xNetworks;

  /**
   * Private-Constructor for Singleton-Pattern
   *
   */
  private NetworkManager()
  {
    m_xNetworks = new HashMap();
  }
  
  /**
   * Registers a network.
   * @param in_xNetwork A network
   */
  void addNetwork(Network in_xNetwork)
  {
    // Old network with same id will be replaced
    m_xNetworks.put(Long.valueOf(in_xNetwork.getId()), 
                    in_xNetwork);
  }
  
  /**
   * Returns a network with a given id.
   * 
   * @return A network
   * @throws Exception If the network had not yet been loaded.
   */
  public Network getNetwork(long in_lId) 
  throws NetworkNotAvailableException
  {
    if(!m_xNetworks.containsKey(Long.valueOf(in_lId)))
    {
      throw new NetworkNotAvailableException(in_lId);
    }
    return (Network)m_xNetworks.get(Long.valueOf(in_lId));
  }
} 

