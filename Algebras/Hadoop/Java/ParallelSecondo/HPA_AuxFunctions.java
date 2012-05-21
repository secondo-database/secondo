package ParallelSecondo;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

import sj.lang.ListExpr;

/**
 * 
 * @author Jiamin
 *
 * This class contains functions that prepared for processing auxliary functions
 * in parallel operators of the HadoopParallelAlgebra. 
 *
 *
 */

public class HPA_AuxFunctions extends ListExpr implements Constant{


/**
 * Prepare the file list for each mapper
 * 
 * This function deploy each row to a mapper running at the node 
 * where the data are kept. 
 * 
 * 
 * @param nameList
 * @param locList
 * @param mapTasksNum
 * @return
 */
	public static ListExpr flist2Mapper(ListExpr nameList, ListExpr locList, int mapTasksNum)
	{
		ListExpr[] mapLoc 			= new ListExpr[mapTasksNum];
		ListExpr[] mapLoc_last 	= new ListExpr[mapTasksNum];
//		ListExpr mapList;
		
		ListExpr locRest = locList;
		ListExpr nameRest = nameList;
		
		while (!locRest.isEmpty())
		{
			//One of the several flist parameters prepared for the map step.
			ListExpr aParaMatrix = locRest.first();
			ListExpr aParaName   = nameRest.first();
			int rowNumber = 1;
			String paraName = aParaName.stringValue();
			ListExpr[] AParaLoc 		= new ListExpr[mapTasksNum];
			ListExpr[] AParaLoc_last 	= new ListExpr[mapTasksNum];
			
/*
The row number is not explicitly indicated in the location list, 
hence if one row is only distributed to one mapper, 
then the other mappers should indicate that as an empty row.  
 
*/
			
			if (paraName.matches(INDLOFPattern))
			{
				while (!aParaMatrix.isEmpty())
				{
					ListExpr row = aParaMatrix.first();
					int slaveIdx = -1;
					if (!row.isEmpty()){
						slaveIdx = row.first().intValue() - 1;
					}
					
					for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
					{
						if (mcnt != slaveIdx){
							AParaLoc[mcnt] = ListExpr.concat(AParaLoc[mcnt], ListExpr.theEmptyList());
						}
						else{
							AParaLoc[mcnt] = ListExpr.concat(AParaLoc[mcnt], row);
						}
					}
					
					aParaMatrix = aParaMatrix.rest();
					rowNumber++;
				}
			}
			
			//Merge locations for each mapper
			//For input arguments, then divide its location to each mapper
			//Or else, copy its location to every mapper
			for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
			{
				ListExpr mapperLocList = aParaMatrix;
				if (AParaLoc[mcnt] != null)
					mapperLocList = AParaLoc[mcnt];
				
				if (mapLoc[mcnt] == null){
					mapLoc[mcnt] = ListExpr.oneElemList(mapperLocList);
					mapLoc_last[mcnt] = mapLoc[mcnt];
				}
				else{
					mapLoc_last[mcnt] = ListExpr.append(mapLoc_last[mcnt], mapperLocList);
				}
			}
			
			locRest = locRest.rest();
			nameRest = nameRest.rest();
		}
		
		ListExpr allMappers = new ListExpr(), allMappers_last = null;
		for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
		{
			ListExpr oneMapper = new ListExpr();
			if (mapLoc[mcnt] != null){
				oneMapper = mapLoc[mcnt];
			}
			
			if (allMappers.isEmpty()){
				allMappers = ListExpr.oneElemList(oneMapper);
				allMappers_last = allMappers;
			}
			else{
				allMappers_last = ListExpr.append(allMappers_last, oneMapper);
			}
		}

		return allMappers;
	}
	
	
	/**
	 * Prepare the file list for each mapper. 
	 * 
	 * This function deploy rows to mappers evenly, 
	 * and let each mapper reads data from its local disk as much as possible.
	 * 
	 * TODO : The problem is that how can I find a way that distributes rows  
	 * evenly on mappers, and guarantees each mapper reads the data from its 
	 * local disk as much as possible ????
	 * 
	 * 
	 * At present, first distribute files to more tasks. 
	 * 
	 * 
	 * @param nameList
	 * @param locList
	 * @param mapTasksNum
	 * @return
	 */
	public static ListExpr flist2Mapper2(
			ListExpr namesList, ListExpr locsList, int mapTasksNum, int slavesNum){
	
		ListExpr[] mapLoc = new ListExpr[mapTasksNum];
		ListExpr nameRest = namesList;
		ListExpr locRest = locsList;
		
		while (!locRest.isEmpty())
		{
			ListExpr nameList = nameRest.first();
			String name = nameList.stringValue();
			ListExpr matrixList = locRest.first();
			
			ListExpr[] oneLoc = new ListExpr[mapTasksNum]; 
			if (name.matches(INDLOFPattern))
			{
				int rowNum = matrixList.listLength();
				//The lower limit of files in each mapper
				int avgMRNum = rowNum / mapTasksNum;  

				while (!matrixList.isEmpty())
				{
					ListExpr row = matrixList.first();
					int mapperIdx = -1;
					if (!row.isEmpty()){
						int slaveIdx = row.first().intValue() - 1;
						mapperIdx = slaveIdx;
						//Decide insert this file to which mapper
						while (mapperIdx < mapTasksNum)
						{
							if (oneLoc[mapperIdx] == null){
								break;
							}
							else if (oneLoc[mapperIdx].listLength() <= avgMRNum){
								break;
							}
							else{
								int cand = mapperIdx + slavesNum;
								if (cand < mapTasksNum){
									mapperIdx = cand;
									continue;
								}
								else
									break;
							}
/*							if (oneLoc[mapperIdx] == null)
								break;
							else if (oneLoc[mapperIdx].listLength() <= avgMRNum){
								if (mapperIdx < slavesNum)
									break;
								else if (mapperIdx < mapTasksNum){
									mapperIdx++;
									break;
								}
							}
							else if (mapperIdx < slavesNum)
								mapperIdx = slavesNum;
							else
								mapperIdx++;
*/
						}
						
						//Set this file to the indicated mapper, 
						//and left an empty row for other mappers
						for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
						{
							if (mcnt != mapperIdx){
								oneLoc[mcnt] = ListExpr.concat(oneLoc[mcnt], ListExpr.theEmptyList());
							}
							else{
								oneLoc[mcnt] = ListExpr.concat(oneLoc[mcnt], row);
							}
						}
					}
					matrixList = matrixList.rest();
				}
			}

			//Merge locations for each mapper
			for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
			{
				ListExpr mapperLocList = matrixList;
				if (oneLoc[mcnt] != null)
					mapperLocList = oneLoc[mcnt];
				
				mapLoc[mcnt] = ListExpr.concat(mapLoc[mcnt], mapperLocList);
			}

			nameRest = nameRest.rest();
			locRest = locRest.rest();
		}

		ListExpr allMappers = null;
		for (ListExpr mapper : mapLoc )
		{
			if (mapper == null)
				allMappers = ListExpr.concat(allMappers, ListExpr.theEmptyList());
			else
				allMappers = ListExpr.concat(allMappers, mapper);
				
		}
		
		return allMappers;
	}
	
	/**
	 * 
	 * Find the first slave index from the given location list for a mapper. 
	 * Usually all rows given to a mapper should all inside a same node 
	 * and we always take the first slave to be the executer of that mapper.
	 * 
	 * @param locList
	 * @return
	 */
	public static int findFirstSlave(ListExpr locList)
	{
		int slaveIdx = 0;
		
		ListExpr objs = locList;
		while (!objs.isEmpty())
		{
			ListExpr rows = objs.first();
			while (!rows.isEmpty()){
				if (!rows.first().isEmpty()){
					slaveIdx = rows.first().first().intValue();
					break;
				}
				rows = rows.rest();
			}
			objs = objs.rest();
		}
		
		return slaveIdx;
	}
	
	/**
	 * Used to divide the flists that are prepared for map and reduce tasks
	 * 
	 * The returned loc list contains two parts, one for mapper and the other one for reducer. 
	 * Each half contains two lists too, one for name and the other one for the locations. 
	 * 
	 * This function can only be used after the flist2Mapper is done

	 * 
	 * 
	 * @param nameList
	 * @param locList
	 * @param mapTasksNum
	 * @return
	 */
	public static ListExpr divMRDLO(ListExpr nameList, ListExpr locList, int mapTasksNum)
	{
		ListExpr locRest = locList;
		
		ListExpr mapperName = null, reducerName = null;
		ListExpr mapperLoc = null, reducerloc = null;
		mapperName = mapperLoc = reducerName = reducerloc = ListExpr.theEmptyList();

		for (int mcnt = 0; mcnt < mapTasksNum; mcnt++)
		{
			ListExpr mapLocs = locRest.first();
			
			ListExpr oneMapper = null, oneMapperName = null;
			ListExpr oneReducer = null, oneReducerName = null;
			oneMapperName = oneMapper = oneReducerName = oneReducer = ListExpr.theEmptyList();

			ListExpr nameRest = nameList;
			while (!nameRest.isEmpty())
			{
				String fileName = nameRest.first().stringValue();
				ListExpr fileLocation = mapLocs.first();
				
				if (fileName.matches(INDLOFPattern))
				{
					//Prepare this file for mappers
					oneMapperName = ListExpr.concat(oneMapperName, 
							ListExpr.oneElemList(ListExpr.stringAtom(fileName)));
					oneMapper = ListExpr.concat(oneMapper, fileLocation);
				}
				else
				{
					//Prepare this file for reducers
					oneReducerName = ListExpr.concat(oneReducerName, 
							ListExpr.oneElemList(ListExpr.stringAtom(fileName)));
					oneReducer = ListExpr.concat(oneReducer, fileLocation);
				}
				
				nameRest = nameRest.rest();
				mapLocs = mapLocs.rest();
			}
			
			mapperName = ListExpr.concat(mapperName, oneMapperName);
			mapperLoc = ListExpr.concat(mapperLoc, oneMapper);
			reducerName = ListExpr.concat(reducerName, oneReducerName);
			reducerloc = ListExpr.concat(reducerloc, oneReducer);
			
			locRest = locRest.rest();
		}
		
		return ListExpr.twoElemList(
				ListExpr.twoElemList(mapperName, mapperLoc), 
				ListExpr.twoElemList(reducerName, reducerloc));
	}
	
	
	public static String plainStr(ListExpr list)
	{
		return list.toString().replaceAll("\n", " ").
			replaceAll("\t", " ").replaceAll(" +", " ");
	}
	
	
	/**
	 * Check whether all objects are prepared for mappers in hadoopMap operator
	 * 
	 * @param locations
	 * @return
	 */
	public static boolean allMapperFOExist(ListExpr locations)
	{
		if (locations.isEmpty()) return true;
		
		ListExpr obj = locations;
		while (!obj.isEmpty()){
  		ListExpr rows = obj.first();
			while (!rows.isEmpty())
			{
				if (!rows.first().isEmpty()){
					return true;
				}
				rows = rows.rest();
			}
			return false;		//One file is empty on all rows
		}
		return false;
	}

	/**
	 * Check whether all objects prepared for hadoopReduce & hadoopReduce2
	 * 
	 * @param names
	 * @param locs
	 * @return
	 */
	public static boolean allObjectExist(ListExpr names, ListExpr locs)
	{
		if (names.isEmpty())
			return true;

		ListExpr nmRest = names;
		ListExpr lcRest = locs;
		
		while (!nmRest.isEmpty()){

			ListExpr rows = lcRest.first();
			while (!rows.isEmpty()){
				if (!rows.first().isEmpty()){
					return true;
				}
				rows = rows.rest();
			}

			lcRest = lcRest.rest();
			nmRest = nmRest.rest();
			
		}
		
		
		return false;
	}
	
	
	/**
	 * Use the flist location to replace the flist subsititution mark.
	 * 
	 * 
	 * 
	 * @param queryList
	 * @param nameList
	 * @param loclList
	 * @param dupTimes
	 * @return queryList
	 */
	public static ListExpr loc2Ffeed (ListExpr queryList, ListExpr nameList,
			ListExpr loclList, int dupTimes)
	{
		ListExpr nameRest = nameList;
		ListExpr loclRest = loclList;
		int allRowNum = 0;
		boolean replaced = true;
		while (!nameRest.isEmpty())
		{
			ListExpr fileNameList = nameRest.first();
			String fileName = fileNameList.stringValue();

			if (fileName.matches(DLOFPattern))
				fileName = fileName.substring(fileName.lastIndexOf(':') + 1, 
						fileName.lastIndexOf("/>"));
			

			ListExpr fileLoc = loclRest.first();
			if (fileLoc.isEmpty()){
				System.out.println("Mission failed, as fileLoc is: " + fileLoc.toString());
				replaced = false;
				break;
			}
			
			//For one flist mark
			ListExpr inputStreamList = null;
			if (!fileLoc.isEmpty())
			{
				int rowNum = 0; 
                                int appendRowNum = 0;
				ListExpr fl_rest = fileLoc;
				while (!fl_rest.isEmpty())
				{
					ListExpr rowInfo = fl_rest.first();
					if (!rowInfo.isEmpty())
					{
						appendRowNum++;
						int locSlave = rowInfo.first().intValue();
						ListExpr columns = rowInfo.second();
						String filePath = rowInfo.third().stringValue();
						
						ListExpr columnTRel = null;
						if (!columns.isEmpty()){
							ListExpr trelLast = null;
							while (!columns.isEmpty()){
								int column = columns.first().intValue();
								
								if (columnTRel == null){
									columnTRel = ListExpr.oneElemList(
											ListExpr.oneElemList(ListExpr.intAtom(column)));
									trelLast = columnTRel;
								}
								else{
									trelLast = ListExpr.append( trelLast, 
											ListExpr.oneElemList(ListExpr.intAtom(column)));
								}
								columns = columns.rest();
							}
						}
						else{
							columnTRel = ListExpr.theEmptyList();
						}
						
						// Avoid generating normal relation files in database
						columnTRel = ListExpr.twoElemList(
								ListExpr.twoElemList(
										ListExpr.symbolAtom("trel"), 
										ListExpr.twoElemList(
												ListExpr.symbolAtom("tuple"), 
												ListExpr.oneElemList(
														ListExpr.twoElemList(
																ListExpr.symbolAtom(fsName), 
																ListExpr.symbolAtom("int"))))), 
																columnTRel);
						columnTRel = ListExpr.twoElemList(
								ListExpr.symbolAtom("feed"),
								columnTRel);
						
						String tupleParaName = "XxxTP" + allRowNum;
						ListExpr ffeedOneRow = 
							ListExpr.oneElemList(ListExpr.symbolAtom("ffeed"));
						ListExpr ffeed_last = ffeedOneRow;
						ffeed_last = 
							ListExpr.append(ffeed_last, ListExpr.stringAtom(fileName));
						ffeed_last = ListExpr.append(ffeed_last, 
								ListExpr.threeElemList(ListExpr.textAtom(filePath), 
										ListExpr.intAtom(rowNum + 1),
										ListExpr.threeElemList(
												ListExpr.symbolAtom("attr"), 
												ListExpr.symbolAtom(tupleParaName), 
												ListExpr.symbolAtom(fsName))));
						ffeed_last = ListExpr.append(ffeed_last, ListExpr.theEmptyList()); 
						//Search type file at local disk
						
						ffeed_last = ListExpr.append(ffeed_last, 
								ListExpr.threeElemList(
										ListExpr.intAtom(locSlave), 
										ListExpr.intAtom(locSlave), 
										ListExpr.intAtom(dupTimes)));
						
						ffeedOneRow = ListExpr.threeElemList(
								ListExpr.symbolAtom("loopsel"), 
								columnTRel, 
								ListExpr.threeElemList(
										ListExpr.symbolAtom("fun"), 
										ListExpr.twoElemList(
												ListExpr.symbolAtom(tupleParaName), 
												ListExpr.symbolAtom("TUPLE")),
												ffeedOneRow));
						
						if (inputStreamList == null){
							inputStreamList = ffeedOneRow;
						}
						else{
							//Concat the former one with the current one.
							inputStreamList = ListExpr.threeElemList(
									ListExpr.symbolAtom("concat"), 
									inputStreamList, ffeedOneRow);
						}
					}
					
					fl_rest = fl_rest.rest();
					rowNum++;
					allRowNum++;
				}
				if (appendRowNum == 0){
					replaced = false; //all rows are empty
				}
			}
			
			if (replaced){
//				System.out.println("In loc2Ffeed, queryList: " + queryList.toString());
//				System.out.println("In loc2Ffeed, fileNameList: " + fileNameList.toString());
//				System.out.println("In loc2Ffeed, inputStreamList: " + inputStreamList.toString());
				queryList = ExtListExpr.replace(queryList, fileNameList, inputStreamList);
			}
			else{
				return ListExpr.theEmptyList();
			}
			
			nameRest = nameRest.rest();
			loclRest = loclRest.rest();
		}
		
		return queryList;
	}
	
	public static ListExpr feedRows(Iterator<Integer> it)
	{
		ListExpr result = ListExpr.theEmptyList();
		while (it.hasNext())
		{
			int row = it.next();
			result = ListExpr.concat(result, 
				ListExpr.twoElemList(
						ListExpr.intAtom(row), ListExpr.intAtom(row)) );
		}
		
		result = ListExpr.twoElemList(
				ListExpr.twoElemList(
						ListExpr.symbolAtom("trel"), 
						ListExpr.twoElemList(
								ListExpr.symbolAtom("tuple"),
										ListExpr.twoElemList(
												ListExpr.twoElemList(
														ListExpr.symbolAtom(frsName), 
														ListExpr.symbolAtom("int")), 
												ListExpr.twoElemList(
														ListExpr.symbolAtom(fssName), 
														ListExpr.symbolAtom("int"))))), 	//TRel Definition 
						result																						//TRel Value
			);
		result = 
			ListExpr.twoElemList( ListExpr.symbolAtom("feed"), result);
		return result;
	}

	public static ListExpr feedInterResult(int columnNo, String interResultName, 
			String interFileTupleName, int typeNodeIdx, ListExpr feedList)
	{
		ListExpr result = ListExpr.oneElemList(ListExpr.symbolAtom("ffeed"));
		ListExpr last = result;
		last = ListExpr.append(last, ListExpr.stringAtom(
				interResultName.substring(
						interResultName.lastIndexOf(":") + 1, 
						interResultName.lastIndexOf("/>"))));
		last = ListExpr.append(last, ListExpr.threeElemList(
				ListExpr.textAtom(""), 
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(interFileTupleName), 
					ListExpr.symbolAtom(frsName)),  //get file row_columnNo   
				ListExpr.intAtom(columnNo)));
		last = ListExpr.append(last, ListExpr.oneElemList(
				ListExpr.intAtom(typeNodeIdx))); //Type Remote node

		last = ListExpr.append(last, ListExpr.threeElemList(
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(interFileTupleName), 
					ListExpr.symbolAtom(fssName)),  //create by a slave  
				ListExpr.threeElemList(
					ListExpr.symbolAtom("attr"), 
					ListExpr.symbolAtom(interFileTupleName), 
					ListExpr.symbolAtom(fssName)), 	//also locate at it
				ListExpr.intAtom(1)));				//without duplication
		result = 					
			ListExpr.threeElemList(
				ListExpr.symbolAtom("loopsel"), 
				feedList, 
				ListExpr.threeElemList(
					ListExpr.symbolAtom("fun"), 
						ListExpr.twoElemList(
							ListExpr.symbolAtom(interFileTupleName), 
							ListExpr.symbolAtom("TUPLE")),
						result));

		return result;
	}
	
	
}
