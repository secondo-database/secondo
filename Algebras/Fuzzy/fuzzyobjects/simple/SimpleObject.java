package fuzzyobjects.simple;

import fuzzyobjects.GeoObject;
import fuzzyobjects.basic.BasicObject;
import java.io.Serializable;

/**
 * a interface to unite fuzzy elementary objects
 * @author Thomas Behr
 */
public interface SimpleObject extends GeoObject,Serializable{

 /**
  * comutes the basic of this Object
  * @return the basic of this Object
  */
 BasicObject basic();

 /**
  * check whether this is a valid elementary Object
  * @return true if this is valid
  */
 boolean isValid();


 /** returns the min X of the bounding box */
 int getMinX();
 /** returns the max X of the bounding box */
 int getMaxX();
 /** returns the min Y of the bounding box */
 int getMinY();
 /** returns the max X of the bounding box */
 int getMaxY();

}
