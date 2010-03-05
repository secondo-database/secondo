package viewer.rtree.datatypes;

import java.util.LinkedList;

import sj.lang.ListExpr;

/**
 * A StaticFace object consists of multiple static cycles.
 * 
 * The StaticFace class is used in the GenericRegion class.
 * 
 * @author Benedikt Buer
 * @author Christian Oevermann
 * @version 1.1
 * @since 23.01.2010
 */
public class StaticFace extends Face
{
	LinkedList<StaticCycle> cycles;

	// constructors
	
	/**
	 * Creates a new StaticFace object from a list of static cycles.
	 * @param cycles List of static cycles
	 */
	public StaticFace(LinkedList<StaticCycle> cycles)
	{
		this.cycles = cycles;
	}
	
	// public methods
	
	/**
	 * Clones the StaticFace object.
	 * @return Cloned StaticFace object
	 */
	public StaticFace clone()
	{
		LinkedList<StaticCycle> clonedCycles = new LinkedList<StaticCycle>();
		
		for (StaticCycle c : cycles)
		{
			clonedCycles.add(c.clone());
		}
		
		return new StaticFace(clonedCycles);
	}
	
	/**
	 * Checks whether a list expression represents a static face.
	 * @param le List expression
	 * @return True if the list contains a static face, otherwise false
	 */
	public static boolean isStaticFace(ListExpr le)
	{
		ListExpr cycleList = le;
		
		// face must at least contain one outer cycle
		if (cycleList.listLength() < 1 )
		{
			return false;
		}
		
		// check whether each element of the list is a cycle
		while (!cycleList.isEmpty())
		{
			if (!StaticCycle.isStaticCycle(cycleList.first()))
			{
				return false;
			}
			
			cycleList = cycleList.rest();
		}
		
		return true;
	}
	
	/**
	 * Reads a static face from a list expression.
	 * @param le List expression
	 * @return Static face
	 */
	public static StaticFace readStaticFace(ListExpr le)
	{
		ListExpr listOfCycles = le;
		LinkedList<StaticCycle> cycles = new LinkedList<StaticCycle>();
		
		while (!listOfCycles.isEmpty())
		{
			cycles.add(StaticCycle.readNonTemporalCycle(listOfCycles.first()));
			listOfCycles = listOfCycles.rest();
		}
		
		return new StaticFace(cycles);
	}

	/**
	 * Reads a temporal face from a list expression.
	 * @param le List expression
	 * @return Static face
	 */
	public static StaticFace readTemporalFace(ListExpr leInstant, ListExpr leCycles)
	{
		ListExpr listOfCycles = leCycles; 
		LinkedList<StaticCycle> cycles = new LinkedList<StaticCycle>();
		
		while (!listOfCycles.isEmpty())
		{
			cycles.add(StaticCycle.readTemporalCycle(leInstant, listOfCycles.first()));
			listOfCycles = listOfCycles.rest();
		}
		
		return new StaticFace(cycles);
	}
	
	/**
	 * Gets the list of cycles.
	 * @return List of cycles
	 */
	public LinkedList<Cycle> getCycles() 
	{
		return new LinkedList<Cycle>(this.cycles);
	}
}
