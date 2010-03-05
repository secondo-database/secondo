package viewer.rtree.datatypes;

import java.util.LinkedList;
import sj.lang.ListExpr;

/**
 * A moving face consists of multiple static cycles
 * 
 * The moving cycle class is used in the GenericRegion class
 * 
 * @author Benedikt Buer
 * @version 1.0
 * @since 23.01.2010
 *
 */
public class MovingFace extends Face{

	LinkedList<MovingCycle> cycles;
	
	public MovingFace(LinkedList<MovingCycle> cycles)
	{
		this.cycles = cycles;
	}

	
	public MovingFace clone()
	{
		LinkedList<MovingCycle> clonedCycles = new LinkedList<MovingCycle>();
		
		for (MovingCycle c : cycles)
		{
			clonedCycles.add(c.clone());
		}
		
		return new MovingFace(clonedCycles);
	}
	
	public static MovingFace readMovingFace(ListExpr leInterval, ListExpr leCycles)
	{
		ListExpr listOfCycles = leCycles; 
		LinkedList<MovingCycle> cycles = new LinkedList<MovingCycle>();
		
		while (! listOfCycles.isEmpty())
		{
			cycles.add( MovingCycle.fromListExpr(leInterval, listOfCycles.first()));
			listOfCycles = listOfCycles.rest();
		}
		
		return new MovingFace(cycles);
	}
	
	public LinkedList<Cycle> getCycles() {
		return new LinkedList<Cycle>( cycles );
	}
}
