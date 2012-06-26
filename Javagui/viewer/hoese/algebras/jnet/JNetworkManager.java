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

import java.util.HashMap;
import java.util.Vector;

/**
 * JPoint, JLine and their moving counterparts can only be displayed in the
 * HoeseViewer if the network they belong two has allready been loaded.
 *
 * Each Network will be registered at this JNetworkManager. When
 * it is needed it is available via it's id. If it has not been loaded an
 * exception will be thrown.
 *
 * WARNING: Networks are only released when a new jnet with the same id
 * is loaded!
 */
public class JNetworkManager
{
  /**
   * Instance for the Singleton-Pattern of GOF
   */
  private static final JNetworkManager INSTANCE = new JNetworkManager();

  /**
   * All Networks loaded in the client
   */
  private HashMap netlist;

  /**
   * Get-Method returning the instance of the <code>JNetworkManager</code>
   *
   * @return The <code>JNetworkManager</code>
   */
  public static JNetworkManager getInstance()
  {
    return INSTANCE;
  }

  /**
   * Private-Constructor for Singleton-Pattern
   *
   */
  private JNetworkManager()
  {
    netlist = new HashMap();
  }

  /**
   * Registers a network.
   * @param jnet
   */
  void addNetwork(JNetwork jnet)
  {
    // Old network with same id will be replaced
    netlist.put(jnet.toString(), jnet);
  }

  /**
   * Returns a network with a given id.
   * @return jnet
   * @throws Exception If the network had not yet been loaded.
   */
  public JNetwork getNetwork(String id) throws JNetworkNotAvailableException
  {
    if(!netlist.containsKey(id))
    {
      throw new JNetworkNotAvailableException(id);
    }
    return (JNetwork) netlist.get(id);
  }
}

