package viewer.rtree.secondotypes;

/**
 * This enumeration contains all Secondo types that can be displayed.
 * 
 * If you want to add your own datatype, add an entry to this enumeration.
 * The name of the entry has to match the name of your datatype.
 * In addition, you have to add few lines of code in the DrawableFactory class.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public enum SecondoType {
	POINT, POINTS, IPOINT, UPOINT, MPOINT,
	LINE, SLINE,
	RECT, RECT3, RECT4, RECT8,
	REGION, INTIMEREGION, UREGION, MOVINGREGION;
}
