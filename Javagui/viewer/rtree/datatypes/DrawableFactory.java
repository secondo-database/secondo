package viewer.rtree.datatypes;

import viewer.rtree.*;
import viewer.rtree.secondotypes.SecondoType;

import java.util.*;
import sj.lang.*;
import viewer.rtree.secondotypes.*;
/**
 * DrawableFactory converts list expressions into drawable items.
 * 
 * @author Oliver Feuer
 * @author Christian Oevermann
 * @author Benedikt Buer
 * @since 08.03.2010
 * @version 1.3
 */
public class DrawableFactory 
{
	// public methods
	
	/**
	 * Creates a list of Drawable objects from a given relation and list of tuple ids.
	 * @param referenceParams Reference parameters
	 * @param tupleIds List of tuple ids
	 * @return List of Drawable objects
	 */
	public static LinkedList<Drawable>GetReferencedItems(ReferenceParameters referenceParams, 
			LinkedList tupleIds)
	{
		ListExpr itemList = retrieveItemList(referenceParams, tupleIds);
		LinkedList<Drawable> items = new LinkedList<Drawable>();
		SecondoType type = SecondoType.valueOf( referenceParams.getType().toUpperCase());
		
		
		// retrieve all items from the query result
		while ( ! itemList.isEmpty() )
		{
			ListExpr object = itemList.first().first();
			int tupleId = itemList.first().second().intValue();
			
			switch (type)
			{
				case POINT: 
				{		
					viewer.rtree.secondotypes.Point p = 
							viewer.rtree.secondotypes.Point.fromListExpr(object);
					p.setTupleId( tupleId );
					items.add( p );
					break;
				}
					
				case POINTS:
				 { 
					Points ps = Points.fromListExpr(object);
					ps.setTupleId( tupleId );
					items.add( ps ); 
					break;
				 }
				 
				case RECT:
				 { 
					Rect r = Rect.fromListExpr(object);
					r.setTupleId( tupleId );
					items.add( r ); 
					break;
				 }

				case RECT3:
				 { 
					Rect3 r3 = Rect3.fromListExpr(object);
					r3.setTupleId( tupleId );
					items.add( r3 ); 
					break;
				 }
				 
				case RECT4:
				 { 
					Rect4 r4 = Rect4.fromListExpr(object);
					r4.setTupleId( tupleId );
					items.add( r4 ); 
					break;
				 }
				 
				case RECT8:
				 { 
					Rect8 r8 = Rect8.fromListExpr(object);
					r8.setTupleId( tupleId );
					items.add( r8 ); 
					break;
				 }

				case LINE:
				 { 
					 Line l = Line.fromListExpr(object);
					 l.setTupleId( tupleId );
					 items.add( l ); 
					 break;
				 }
				 
				case SLINE:
				 { 
					SLine ls = SLine.fromListExpr(object);
					ls.setTupleId( tupleId );
					items.add( ls ); 
					break;
				 }
				 
				case IPOINT:
				 { 
					IPoint pi = IPoint.fromListExpr(object);
					pi.setTupleId( tupleId );
					items.add( pi ); 
					break;
				 }
				 
				case UPOINT:
				 { 
					UPoint pu = UPoint.fromListExpr(object);
					pu.setTupleId( tupleId );
					items.add( pu ); 
					break;
				 }
				 
				case MPOINT:
				 { 
					MPoint pm = MPoint.fromListExpr(object);
					pm.setTupleId( tupleId );
					items.add( pm ); 
					break;
				 }
				 
				case REGION:
				 { 
					Region rg = Region.fromListExpr(object);
					rg.setTupleId( tupleId );
					items.add( rg ); 
					break;
				 }
				 
				case INTIMEREGION:
				 { 
					InTimeRegion ri = InTimeRegion.fromListExpr(object);
					ri.setTupleId( tupleId );
					items.add( ri ); 
					break;
				 }
				 
				case UREGION:
				 { 
					URegion ru = URegion.fromListExpr(object);
					ru.setTupleId( tupleId );
					items.add( ru ); 
					break;
				 }
				 
				case MOVINGREGION:
				 { 
					MovingRegion rm = MovingRegion.fromListExpr(object);
					rm.setTupleId( tupleId );
					items.add( rm ); 
					break;
				 }
				 
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				// Add your types here. See SecondoType for additional information
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				 
				default: return null; // unknown type
			}
			
			itemList = itemList.rest();
		}
		
		return items;
	}
	
	// private methods
	
	/**
	 * Creates a list expression from a given relation and list of tuple ids.
	 * @param referenceParams Reference parameters
	 * @param tupleIds List of tuple ids
	 * @return List expression
	 */
	private static ListExpr retrieveItemList(ReferenceParameters referenceParams,
			LinkedList tupleIds)
	{
		ListExpr itemList = sendGetItemList(referenceParams, tupleIds);

		if (itemList.isAtom())
		{
			// empty list, relation does not match tree
			return null;
		}
		else
		{
			return itemList;
		}
	}
	
	/**
	 * Sends the getItemList command to Secondo.
	 * @param referenceParams Reference parameters
	 * @param tupleIds List of tuple ids
	 * @return Query result 
	 */
	private static ListExpr sendGetItemList(ReferenceParameters referenceParams,
			LinkedList tupleIds)
	{
		StringBuilder cmd = new StringBuilder();
		cmd.append("query ");
		cmd.append(referenceParams.getRelation());
		cmd.append(" feed {x} addtupleid filter[");
		for (int i = 0; i < tupleIds.size(); i++) 
		{
			if (i > 0)
			{
				cmd.append(" or ");
			}
			cmd.append("(.id = [const tid value ");
			cmd.append(String.valueOf(tupleIds.get(i)));
			cmd.append("])");
		}
		cmd.append("] project [");
		cmd.append(referenceParams.getAttribute());
		cmd.append("_x, id] consume");
		
		SecondoManager secondo = new SecondoManager();
		ListExpr itemList = secondo.sendCommand(cmd.toString(), "getItemList");
		
		return itemList.second();
	}
}
