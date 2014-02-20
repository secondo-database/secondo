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


/**
 * copied from original package viewer.relsplit
 */

package viewer.update2;

import sj.lang.ListExpr;
import java.util.Vector;
import tools.Reporter;

public class Head
{

	private Vector<HeadEntry> V = new Vector<HeadEntry>();
	private int MaxTypeLength=0;
	private int MaxNameLength=0;
	
	
	/** returns the maximal length of a type name */
	public int getMaxTypeLength(){
		return MaxTypeLength;
	}
	
	/** returns the maximal length of a attribute name */
	public int getMaxNameLength(){
		return MaxNameLength;
	}
	
	
	/** check if LE = (<type> <value>) is a relation */
	public static boolean isRelation(ListExpr LE){
		if(LE.listLength()!=2)
			return false;
		ListExpr Type = LE.first();
		if(Type.listLength()!=2)
			return false;
		ListExpr R = Type.first();
		return (R.isAtom() 
					&& (R.atomType()==ListExpr.SYMBOL_ATOM) 
					&& (Head.isRelationType(R.symbolValue()))
				);
	}
	
	public static boolean isRelationType(String pType)
	{
		return (pType.equals("rel") 
				|| pType.equals("mrel")
				|| pType.equals("trel")
				|| pType.equals("orel")
				|| pType.equals("nrel")
				|| pType.equals("arel"));
	}
	
	
	public boolean readFromRelTypeLE(ListExpr LE){
		V.clear();
		MaxTypeLength = 0;
		MaxNameLength = 0;
		if(LE.listLength()!=2) // should be (xrel (tuple ... ))
			return false;
		ListExpr TName = LE.first();
		if(TName.atomType()!=ListExpr.SYMBOL_ATOM)
			return false;
		String Name = TName.symbolValue();
		if(!Head.isRelationType(Name))
			return false;
		
		ListExpr Tuple = LE.second();
		if(Tuple.listLength()!=2){
			Reporter.writeError("wrong tuple list length");
			return false;
		}
		
		
		ListExpr TupleType = Tuple.first();
		if ( !TupleType.isAtom() || !(TupleType.atomType()==ListExpr.SYMBOL_ATOM) ||
			!(TupleType.symbolValue().equals("tuple") | TupleType.symbolValue().equals("mtuple")) )
			return false;
		
		
		ListExpr TupleValue = Tuple.second(); // should be ((name type)(name type)...)
		boolean ok = true;
		
		
		while (TupleValue.listLength()>0 & ok) {
			ListExpr EntryList = TupleValue.first();
			if(EntryList.listLength()!=2){  // should be (name type)
				ok = false;
			}
			else{
				ListExpr NameList = EntryList.first();
				ListExpr TypeList = EntryList.second();
				if(! (NameList.isAtom() && NameList.atomType()==ListExpr.SYMBOL_ATOM)){
					ok =false;
				}
				// allow non-atomar attributes as well, e.g. attribute relations
				/*if(!( TypeList.isAtom() && TypeList.atomType()==ListExpr.SYMBOL_ATOM)){
					ok = false;
				}*/
				boolean isAtom = (TypeList.isAtom() && TypeList.atomType()==ListExpr.SYMBOL_ATOM);
				String typeValue;
				if(isAtom) {
					typeValue = TypeList.symbolValue();
				}
				else {
					typeValue = TypeList.toString().trim();
				}
				if(ok){
					V.add(new HeadEntry(NameList.symbolValue(),typeValue,isAtom) );
					MaxTypeLength = Math.max(MaxTypeLength,typeValue.length());
					MaxNameLength = Math.max(MaxNameLength,NameList.symbolValue().length());
				}
			}
			TupleValue = TupleValue.rest();
		}
		if(!ok){
			V.clear();
			MaxNameLength = 0;
			MaxTypeLength = 0;
		}
		return ok;
	}
	
	public int getSize(){
		return V.size();
	}
	
	public HeadEntry get(int index){
		if(index<0 | index>V.size())
			return null;
		else
			return (HeadEntry) V.get(index); 
	}
		
}

