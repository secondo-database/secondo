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

package gui.idmanager;

/*****************************
*
*  Autor   : Thomas Behr
*  Version : 1.0
*  Datum   : 22.6.1999
*
******************************/

import java.io.*;

public class ID implements Serializable {

    /** the intern value */
    private int value=0;

    /** get a new ID */
    static ID getNextID(ID oldID)
     { ID newid = new ID();
          newid.value = oldID.value+1;
       return newid;
     }

    /** check for equality */
    public boolean equals(ID ID2) { return value == ID2.value; }
    
    /** set this id to the value of old  */
    public void equalize(ID old) { value = old.value; }


    /** returns a copy of this */
    public ID Duplicate() {
      ID  Copy = new ID();
      Copy.equalize(this);
      return Copy;
    }

    /** returns a readable representation of this */
    public String toString() { return "" + value; }

    //sw 2004-01-07
    /** The integer 'value' provides a good hashCode.
     *  @return the <code>value</code> of the ID.
     */
    public int hashCode() { return value; }
    
 } // class ID

