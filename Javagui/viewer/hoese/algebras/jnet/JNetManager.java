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

import java.util.HashMap;
import java.util.Vector;

/**
 * JPoints, JLines and their moving counterparts can only be displayed in the
 * HoeseViewer if the jnetwork they belong two has allready been loaded.
 *
 * Each JNetwork will be registered at this JNetManager. When
 * it is needed it is available via it's id. If it has not been loaded an
 * exception will be thrown.
 *
 * WARNING: JNetworks are only released when a new jnetwork with the same id
 * is loaded!
 *
 * @author Simone Jandt
 *
 */
public class JNetManager
{
  /**
   * Instance for the Singleton-Pattern of GOF
   */
  private static final JNetManager INSTANCE = new JNetManager();

  /**
   * Get-Method returning the instance of the <code>JNetManager</code>
   *
   * @return The <code>JNetManager</code>
   */
  public static JNetManager getInstance()
  {
    return INSTANCE;
  }

  /**
   * All Networks loaded in the client
   */
  private HashMap mapJNets;

  /**
   * Private-Constructor for Singleton-Pattern
   *
   */
  private JNetManager()
  {
    mapJNets = new HashMap();
  }

  /**
   * Registers a network.
   * @param in_xNetwork A network
   */
  void addNetwork(JNetwork inNetwork)
  {
    // Old network with same id will be replaced
    mapJNets.put(inNetwork.getId(), inNetwork);
  }

  /**
   * Returns a network with a given id.
   *
   * @return A network
   * @throws Exception If the network had not yet been loaded.
   */
  public JNetwork getJNetwork(String nid)
    throws JNetworkNotAvailableException
  {
    if(!mapJNets.containsKey(nid))
    {
      throw new JNetworkNotAvailableException(nid);
    }
    return (JNetwork)mapJNets.get(nid);
  }
}

