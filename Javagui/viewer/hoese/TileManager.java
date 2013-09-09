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

import java.util.*;

/**
 * Class TileManager updates minimum value and maximum value
 * of a Tile Algebra object before this object in drawn in layer.
 *
 * author: Dirk Zacher
 */
 
public class TileManager
{
  /**
   * Member Object contains all graphical objects of the layer.
   */
  private Vector Objects;
  
  /**
   * Constructor of TileManager initializes member Objects
   * by given parameter objects
   * @param objects - reference to a vector containing 
   *                  all graphical objects of the layer
   * @author Dirk Zacher
   */
  public TileManager(Vector objects)
  {
    Objects = objects;
  }
    
  /**
   * Method updateObjects updates all Tile objects of the layer.
   * @author Dirk Zacher
   */
  public void updateObjects()
  {
    boolean hasTileObjects = false;
    Double minimumValue = Double.MAX_VALUE;
    Double maximumValue = Double.MIN_VALUE;
    
    for(int i = 0; i < Objects.size(); i++)
    {
      Object object = Objects.get(i);
      
      if(object instanceof Tile)
      {
        Tile tile = (Tile)object;
        
        if(tile != null)
        {
          hasTileObjects = true;
          Double tileMinimumValue = tile.GetMinimumValue();
          Double tileMaximumValue = tile.GetMaximumValue();
          
          if(tileMinimumValue < minimumValue)
          {
            minimumValue = tileMinimumValue;
          }
          
          if(tileMaximumValue > maximumValue)
          {
            maximumValue = tileMaximumValue;
          }
        }
      }
    }
    
    if(hasTileObjects == true)
    {
      for(int i = 0; i < Objects.size(); i++)
      {
        Object object = Objects.get(i);
        
        if(object instanceof Tile)
        {
          Tile tile = (Tile)object;
          
          if(tile != null)
          {
            tile.SetMinimumValue(minimumValue);
            tile.SetMaximumValue(maximumValue);
          }
        }
      }
    }
  }
}
