package gui.idmanager;

/***************************
*
*  Autor   : Thomas Behr
*  Version : 1.0
*  Datum   : 22.6.1999
*
****************************/

public class IDManager{

 static private ID  CurrentID = new ID();

 /** returns a new ID */
 public static ID getNextID()

    { CurrentID = ID.getNextID(CurrentID);
      return CurrentID;
    }


}





