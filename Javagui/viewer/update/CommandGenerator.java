/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

   04-2005, Matthias Zielke


*/
package viewer.update;

import viewer.*;
import java.util.*;

/*
This class generates commands for updating relations in 'Nested-List-Syntax'. The source
for the important information is the 'UpdateViewer' itself. 
 
*/
public class CommandGenerator {
	
	private UpdateViewer viewer;
	
	public CommandGenerator(UpdateViewer viewer){
		this.viewer = viewer;
	}
	
/*
Generates an insert-command for each tuple that shall be inserted. Actualizes the indices as
well.

*/
	public String[] generateInsert(String relName, Vector btreeNames, Vector btreeAttrNames,
									Vector rtreeNames, Vector rtreeAttrNames){
		String nextValue;
		String nextType;
		String[][] insertTuples = viewer.getInsertTuples();
		String[] insertCommands = new String[insertTuples.length];
		String[] attrTypes = viewer.getAttrTypes();
		for (int j = 0; j < insertCommands.length; j++){
			StringBuffer insertCommand = new StringBuffer("(query (count ");
			for (int k = 0; k < btreeNames.size(); k ++){
				insertCommand.append("(insertbtree ");
			}
			for (int k = 0; k < rtreeNames.size(); k ++){
				insertCommand.append("(insertrtree ");
			}
			insertCommand.append("(inserttuple " + relName + " (");
			for (int i = 0; i < attrTypes.length; i++){
				nextType = attrTypes[i].trim();
				if (nextType.equals("string") || nextType.equals("int") || nextType.equals("real") || nextType.equals("bool") ){
					if (nextType.equals("string")){
						nextValue = "\"" + insertTuples[j][i] + "\"";
					}
					else{
						nextValue = insertTuples[j][i];
					}
				}
				else{
					if (insertTuples[j][i].trim().startsWith("(") && insertTuples[j][i].trim().endsWith(")")){
						nextValue = "(" + nextType +  insertTuples[j][i] + ")";
					}
					else {
						nextValue = "(" + nextType + "(" +  insertTuples[j][i] + "))";
					}
				}
				insertCommand.append(nextValue + " ");		
			}
			insertCommand.append("))");
			for (int k = 0; k < rtreeNames.size(); k ++){
				insertCommand.append(rtreeNames.get(k)+ " " + rtreeAttrNames.get(k) + ")");
			}
			for (int k = 0; k < btreeNames.size(); k ++){
				insertCommand.append(btreeNames.get(k)+ " " + btreeAttrNames.get(k) + ")");
			}
			insertCommand.append("))");
			insertCommands[j] = insertCommand.toString();
		}
		return insertCommands;	
	}
	
/*
Generates a delete-command for each tuple that shall be deleted. Actualizes the indices as
well.

*/
	public String[] generateDelete(String relName, Vector btreeNames, Vector btreeAttrNames,
									Vector rtreeNames, Vector rtreeAttrNames){
		String nextValue;
		String nextType;
		String nextAttrName;
		String[][] deleteTuples = viewer.getDeleteTuples();
		String[] deleteCommands = new String[deleteTuples.length];
		int[] deleteRows = viewer.getDeleteRows();
		String nextTid;
		for (int j = 0; j < deleteRows.length; j++){
			StringBuffer deleteCommand = new StringBuffer("(query (count ");
			for (int k = 0; k < btreeNames.size(); k ++){
				deleteCommand.append("(deletebtree ");
			}
			for (int k = 0; k < rtreeNames.size(); k ++){
				deleteCommand.append("(deletertree ");
			}
			nextTid = viewer.getTupleId(deleteRows[j]);
			deleteCommand.append("(deletebyid " + relName + " (tid " + nextTid + " )) " );
			for (int k = 0; k < rtreeNames.size(); k ++){
				deleteCommand.append(rtreeNames.get(k)+ " " + rtreeAttrNames.get(k) + ")");
			}
			for (int k = 0; k < btreeNames.size(); k ++){
				deleteCommand.append(btreeNames.get(k)+ " " + btreeAttrNames.get(k) + ")");
			}
			deleteCommand.append("))");
			deleteCommands[j] = deleteCommand.toString();
		}
		return deleteCommands;	
	}

/*
Generates an update-command for each tuple that shall be updated. Actualizes the indices as
well.

*/
	public String[] generateUpdate(String relName, Vector btreeNames, Vector btreeAttrNames,
									Vector rtreeNames, Vector rtreeAttrNames){
		String nextValue;
		String newValue;
		String nextType;
		String nextAttrName;
		int[] updateTuples =viewer.updatedTuples();
		String[] nextUpdateTuple;
		String[] nextOriginalTuple;
		int[] changedAttributes;
		String[] updateCommands = new String[updateTuples.length];
		String[] attrTypes = viewer.getAttrTypes();
		String[] attrNames = viewer.getAttrNames();
		String nextTid;
		for (int j = 0; j < updateTuples.length; j++){
			StringBuffer updateCommand = new StringBuffer("(query (count  " );
			for (int k = 0; k < btreeNames.size(); k ++){
				updateCommand.append("(updatebtree ");
			}
			for (int k = 0; k < rtreeNames.size(); k ++){
				updateCommand.append("(updatertree ");
			}
			nextTid = viewer.getTupleId(updateTuples[j]);
			updateCommand.append("(updatebyid " );
			updateCommand.append("(feed " + relName + " ) " + relName + " (tid " + nextTid + " ) ");
			nextUpdateTuple = viewer.getUpdateTuple(updateTuples[j]);
			nextOriginalTuple = viewer.getOriginalTuple(updateTuples[j]);
			changedAttributes = viewer.getChangedAttributes(updateTuples[j]);
			updateCommand.append( "(");
			for (int i = 0; i < changedAttributes.length; i++){
				updateCommand.append("(" + attrNames[changedAttributes[i]].trim());
				updateCommand.append("( fun ( tuple" + (i+1) + " TUPLE )");
				nextType = attrTypes[changedAttributes[i]].trim();
				if (nextType.equals("string") || nextType.equals("int") || nextType.equals("real") || nextType.equals("bool") ){
					if (nextType.equals("string")){
						newValue = "\"" + nextUpdateTuple[changedAttributes[i]] + "\"";
					}
					else{
						newValue = nextUpdateTuple[changedAttributes[i]];
					}
				}
				else{
					if (nextUpdateTuple[changedAttributes[i]].trim().startsWith("(") && nextUpdateTuple[changedAttributes[i]].trim().endsWith(")")){
						newValue = "(" + nextType +  nextUpdateTuple[changedAttributes[i]] + ")";
					}
					else {
						newValue = "(" + nextType + "(" +  nextUpdateTuple[changedAttributes[i]] + "))";
					}
				}
				updateCommand.append(newValue + "))");
			}
			updateCommand.append("))");
			for (int k = 0; k < rtreeNames.size(); k ++){
				updateCommand.append(rtreeNames.get(k)+ " " + rtreeAttrNames.get(k) + ")");
			}
			for (int k = 0; k < btreeNames.size(); k ++){
				updateCommand.append(btreeNames.get(k)+ " " + btreeAttrNames.get(k) + ")");
			}
			updateCommand.append("))");
			updateCommands[j] = updateCommand.toString();
		}
		return updateCommands;	
	}

	
}
