package fuzzyobjects.composite;
import fuzzyobjects.GeoObject;
import java.io.Serializable;

public interface CompositeObject extends GeoObject,Serializable{

/** check the emptyness of this Object */
 boolean isEmpty();
/** returns the scale factor of this object */
 double getSF();
/** computes then membership value(s) on (x,y) */
 double[] ZRel(double x, double y);
/** compuets the maximal membership value */
 double maxZ();
/** is this a correct Object ? */
 boolean isValid();
/** returns the dimension of this object */
 int getDim();
/** returns the 9-intersectionMatrix from this and CO */
 M9Int basicTopolRelation(CompositeObject CO);
/** computes the topological relation between this and CO */
 FuzzyTopRel topolRelation(CompositeObject CO);
/** set the factor of scale */
 boolean setSF(double SF);
/** returns the bounding box from this */
 BoundingBox getBoundingBox();
}
