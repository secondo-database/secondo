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

/**
 * Exception that is thrown when a jnetwork is not available
 */

public class JNetworkNotAvailableException extends Exception
{
  /**
   * Id of the missing network
   */
  private String netId;

  /**
   * Constuctor
   */
  public JNetworkNotAvailableException(String id)
  {
    netId = id;
  }

  /**
   * Get-Message of class exception.
   */
  public String getMessage()
  {
    return "jnet with id " + netId + " has not been loaded.";
  }
}
