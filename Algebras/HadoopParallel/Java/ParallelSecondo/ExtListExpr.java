package ParallelSecondo;


import java.util.List;

import sj.lang.ListExpr;

public class ExtListExpr extends ListExpr {
	public static 
//Change a nested-list, if it reads the relation from a db object, 
//then change it to ffeed operator
	
	ListExpr ReadFromFile( ListExpr oldList, String relName, 
			ListExpr newElement)
	{
		ListExpr newList = new ListExpr();
		if (!oldList.isEmpty()){
			if (oldList.listLength() == 2){
				if (oldList.first().isAtom()){
					if (oldList.first().symbolValue().compareTo("feed") == 0){
						if (oldList.second().isAtom()){
							if (oldList.second().atomType() == ListExpr.SYMBOL_ATOM 
								&& oldList.second().stringValue().startsWith(relName)){
								newList = newElement;
							}
						}
					}
				}
				return newList;
			}
			else if (oldList.isAtom()){
				return ListExpr.theEmptyList();
			}
		}
		else{
			return ListExpr.theEmptyList();
		}

		//oldList is not atom ...
		boolean changed = false;
		ListExpr first = oldList.first();
		newList = ReadFromFile(first, relName, newElement);
		ListExpr copyList = null;
		if (!newList.isEmpty()){
			changed = true;
			copyList = ListExpr.oneElemList(newList);
		}
		else
			copyList = ListExpr.oneElemList(first);
		
		ListExpr last = copyList;
		ListExpr restList = oldList.rest();
		while(!restList.isEmpty())
		{
			ListExpr firstElement = restList.first();
			if (!changed) {
				newList = ReadFromFile(firstElement, relName, newElement);
				if (!newList.isEmpty()){
					changed = true;
					last = ListExpr.append(last, newList);
				}
				else
					last = ListExpr.append(last, firstElement);
			}
			else
				last = ListExpr.append(last, firstElement);
			restList = restList.rest();
		}
		
		if (changed)
			return copyList;
		else
			return ListExpr.theEmptyList();
	}

	public static	
	ListExpr replaceFirst( ListExpr oldList, String TYPE, ListExpr newElement)
	{
		ListExpr newList = new ListExpr();
		if (!oldList.isEmpty()){
			if (oldList.isAtom()){
				if ( (oldList.atomType() == ListExpr.SYMBOL_ATOM)
						&& oldList.stringValue().startsWith(TYPE)){
					newList = newElement;
				}
				return newList;
			}
		}
		else{
			return ListExpr.theEmptyList();
		}

		//oldList is not atom ...
		boolean changed = false;
		ListExpr first = oldList.first();
		newList = replaceFirst(first, TYPE, newElement);
		ListExpr copyList = null;
		if (!newList.isEmpty()){
			changed = true;
			copyList = ListExpr.oneElemList(newList);
		}
		else
			copyList = ListExpr.oneElemList(first);
		
		ListExpr last = copyList;
		ListExpr restList = oldList.rest();
		while(!restList.isEmpty())
		{
			ListExpr firstElement = restList.first();
			if (!changed) {
				newList = replaceFirst(firstElement, TYPE, newElement);
				if (!newList.isEmpty()){
					changed = true;
					last = ListExpr.append(last, newList);
				}
				else
					last = ListExpr.append(last, firstElement);
			}
			else
				last = ListExpr.append(last, firstElement);
			restList = restList.rest();
		}
		
		if (changed)
			return copyList;
		else
			return ListExpr.theEmptyList();
	}

/*
This method is specified to find a relation's name. 
The relation will be feed into system, and be extended by a fixed 
attribute name: Partition. 

This method is a recursion method, and the argument ~findExt~ means
whether the caller has already found the extend operator, and the argument 
~findFeed~ means whether the caller has already found the feed operator. 
If both are true, then the closest atom element is assumed to be the 
relation's name. 

*/
	public static String findRelation(ListExpr list, boolean findFeed) {
		if (list.isAtom()) {
			if (/*findExt && */findFeed)
				return list.symbolValue();
			else
				return "";
		}

		String rName = "";
		boolean isFirstAtom = list.first().isAtom();

		if (isFirstAtom 
				&& list.first().symbolValue().compareTo("feed") == 0) {
			findFeed = true;
			rName = findRelation(list.second(), /*findExt,*/ findFeed);
			if (rName.length() > 0)
				return rName;
		}

		ListExpr restList;
		if (isFirstAtom)
			restList = list.rest();
		else
			restList = list;
		while (!restList.isEmpty()) {
			ListExpr firstList = restList.first();
			rName = findRelation(firstList, findFeed);
			if (rName.length() > 0)
				return rName;
			restList = restList.rest();
		}

		return "";
	}

/*
This method is used to find the outmost alias. 
If the input NL has several rename operators, then it will return 
the first alias it meets.  
 
*/

	public static String findAlias(ListExpr list)
	{
		if (list.isAtom())
			return "";
		
		boolean isFirstAtom = list.first().isAtom();
		if (isFirstAtom && list.first().symbolValue().compareTo("rename") == 0)
		{
			return list.third().symbolValue();
		}
		
		ListExpr restList;
		if (isFirstAtom)
			restList = list.rest();
		else
			restList = list;
		while (!restList.isEmpty())
		{
			ListExpr firstList = restList.first();
			String alias = findAlias(firstList);
			if (alias.length() > 0)
				return alias;
			restList = restList.rest();
		}
		
		return "";
	}

	
/*
This method replaces a pattern list with a new list
 
*/
	public static ListExpr replace(ListExpr input, ListExpr pattern, ListExpr newList)
	{
		if (input.isEmpty())
			return ListExpr.theEmptyList();
		
		if (input.listLength() == pattern.listLength())
		{
/*			System.err.println("+++++++++++++++++++++++");
			System.err.println("receive: " + input.toString());
			System.err.println("pattern: " + pattern.toString());
			System.err.println("equal? : " + input.equals(pattern));
*/			
			if (input.equals(pattern)){
				return newList;
			}
//			System.err.println("+++++++++++++++++++++++\n\n");
		}

		if (input.listLength() > 0)
			return ListExpr.cons(replace(input.first(), pattern, newList), 
					replace(input.rest(), pattern, newList));
		else{ 
			return input;
		}
	}
}
