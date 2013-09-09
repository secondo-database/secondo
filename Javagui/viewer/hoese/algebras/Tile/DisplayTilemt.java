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

package viewer.hoese.algebras.Tile;

import viewer.hoese.algebras.raster2.*;
import viewer.hoese.*;

/**
 * Class DisplayTilemt represents the display class
 * for Tile Algebra mt data types.
 *
 * author: Dirk Zacher
 */
 
public class DisplayTilemt extends DisplayRaster2ms
                           implements Tile
{
  /**
   * Method GetMinimumValue returns the minimum value of Tile Algebra object.
   * @return minimum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public Double GetMinimumValue()
  {
    Double minimumValue = new Double(0.0);
    
    if(minValue != null)
    {
      if(minValue instanceof Integer)
      {
        minimumValue = new Double((Integer)minValue);
      }

      else if(minValue instanceof Double)
      {
        minimumValue = new Double((Double)minValue);
      }

      else if(minValue instanceof Boolean)
      {
        if((Boolean)minValue == Boolean.FALSE)
        {
          minimumValue = new Double(0.0);
        }

        else
        {
          minimumValue = new Double(1.0);
        }
      }

      else if(minValue instanceof String)
      {
        minimumValue = new Double(((String)minValue).hashCode());
      }     
    }
    
    return minimumValue;
  }
    
  /**
   * Method GetMaximumValue returns the maximum value of Tile Algebra object.
   * @return maximum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public Double GetMaximumValue()
  {
    Double maximumValue = new Double(0.0);
      
    if(maxValue != null)
    {
      if(maxValue instanceof Integer)
      {
        maximumValue = new Double((Integer)maxValue);
      }

      else if(maxValue instanceof Double)
      {
        maximumValue = new Double((Double)maxValue);
      }

      else if(maxValue instanceof Boolean)
      {
        if((Boolean)minValue == Boolean.FALSE)
        {
          maximumValue = new Double(0.0);
        }

        else
        {
          maximumValue = new Double(1.0);
        }
      }

      else if(maxValue instanceof String)
      {
        maximumValue = new Double(((String)maxValue).hashCode());
      }     
    }
    
    return maximumValue;
  }
    
  /**
   * Method SetMinimumValue sets the minimum value of Tile Algebra object.
   * @param minimumValue - minimum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public void SetMinimumValue(Double minimumValue)
  {
    this.minValue = minimumValue;
  }
    
  /**
   * Method SetMaximumValue sets the maximum value of Tile Algebra object.
   * @param maximumValue - maximum value of Tile Algebra object
   * @author Dirk Zacher
   */
  public void SetMaximumValue(Double maximumValue)
  {
    this.maxValue = maximumValue;
  }
}
