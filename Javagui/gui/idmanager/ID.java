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

 } // class ID

