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

package  viewer.hoese;

  /**
   * This interface need to be implemented for a new entry in ViewConfig's rendering type
   * list (e.g. OutLineWidth).
   * @see generic.OutLineWidth
   * @see generic.ViewConfig
   */
public interface CalcCategory {

  /**
   * This method creates the new characteristic category, based on a template one.
   * @param actValue A value between 0..valueNr
   * @param valueNr  The maximal Nr of different values
   * @param templCat The template category
   * @return A new category
   * @see generic.Category
   */
  public Category calcCategory (int actValue, int valueNr, Category templCat);



  /**
   * 
   * @return The entry for the rendering type list. 
   */
  public String toString ();
}



