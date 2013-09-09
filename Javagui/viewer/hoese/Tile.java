/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

package viewer.hoese;

/**
 * Interface Tile must be implemented from Tile Algebra data types.
 *
 * author: Dirk Zacher
 */

public interface Tile
{
  /**
   * Method GetMinimumValue returns the minimum value of Tile Algebra object.
   * @return minimum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public Double GetMinimumValue();
  
  /**
   * Method GetMaximumValue returns the maximum value of Tile Algebra object.
   * @return maximum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public Double GetMaximumValue();
  
  /**
   * Method SetMinimumValue sets the minimum value of Tile Algebra object.
   * @param minimumValue - minimum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public void SetMinimumValue(Double minimumValue);
  
  /**
   * Method SetMaximumValue sets the maximum value of Tile Algebra object.
   * @param maximumValue - maximum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public void SetMaximumValue(Double maximumValue);
}
