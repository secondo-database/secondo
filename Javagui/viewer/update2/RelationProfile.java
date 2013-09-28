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

package  viewer.update2;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Profile for loading a relation.	 
 */
public class RelationProfile{
	
	private String relName;
	
	private List<String> filterExpressions;
	
	private List<String> projectExpressions;
	
	private List<String> sortExpressions;
	
	public RelationProfile(String pName){
		relName = pName;
		filterExpressions = new ArrayList<String>();
		projectExpressions = new ArrayList<String>();
		sortExpressions = new ArrayList<String>();
	}
	
	public String getName(){
		return this.relName;
	}	
	
	public void setName(String pName){
		this.relName = pName;
	}	
	
	public List<String> getFilterExpressions(){
		return this.filterExpressions;
	}	
	
	public void setFilterExpressions(List<String> pFilterExpressions){
		this.filterExpressions = pFilterExpressions;
	}
	
	public List<String> getProjectExpressions(){
		return this.projectExpressions;
	}	
	
	public void setProjectExpressions(List<String> pProjectExpressions){
		this.projectExpressions = pProjectExpressions;
	}
	
	public List<String> getSortExpressions(){
		return this.sortExpressions;
	}	
	
	public void getSortExpressions(List<String> pSortExpressions){
		this.sortExpressions = pSortExpressions;
	}
}

