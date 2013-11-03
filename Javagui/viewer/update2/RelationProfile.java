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
public class RelationProfile
{
	
	private String relName;
	
	private List<String> filterExpressions;
	
	private List<String> projectExpressions;
	
	private List<String> sortExpressions;
	
	public RelationProfile(String pName)
	{
		relName = pName;
		filterExpressions = new ArrayList<String>();
		projectExpressions = new ArrayList<String>();
		sortExpressions = new ArrayList<String>();
	}
	
	public boolean addFilterExpression(String pExpression)
	{
		boolean result = false;
		if (!this.filterExpressions.contains(pExpression))
		{
			this.filterExpressions.add(pExpression);
			result = true;
		}
		return result;
	}
	
	public boolean addProjectExpression(String pExpression)
	{
		boolean result = false;
		if (!this.projectExpressions.contains(pExpression))
		{
			this.projectExpressions.add(pExpression);
			result = true;
		}
		return result;
	}
	
	public boolean addSortExpression(String pExpression)
	{
		boolean result = false;
		if (!this.sortExpressions.contains(pExpression))
		{
			this.sortExpressions.add(pExpression);
			result = true;
		}
		return result;
	}
	
	
	public String getName()
	{
		return this.relName;
	}	
	
	public List<String> getFilterExpressions()
	{
		return this.filterExpressions;
	}	

	public List<String> getProjectExpressions()
	{
		return this.projectExpressions;
	}	
	
	public List<String> getSortExpressions()
	{
		return this.sortExpressions;
	}	
	
	
	public void removeFilterExpression(String pExpression)
	{
		if (this.filterExpressions.contains(pExpression))
		{
			this.filterExpressions.remove(pExpression);
		}
	}
	
	public void removeProjectExpression(String pExpression)
	{
		if (this.projectExpressions.contains(pExpression))
		{
			this.projectExpressions.remove(pExpression);
		}
	}
	
	public void removeSortExpression(String pExpression)
	{
		if (this.sortExpressions.contains(pExpression))
		{
			this.sortExpressions.remove(pExpression);
		}
	}
	
	
	public void setFilterExpressions(List<String> pFilterExpressions)
	{
		this.filterExpressions = pFilterExpressions;
	}

	public void setName(String pName)
	{
		this.relName = pName;
	}	
	
	public void setProjectExpressions(List<String> pProjectExpressions)
	{
		this.projectExpressions = pProjectExpressions;
	}
		
	public void setSortExpressions(List<String> pSortExpressions)
	{
		this.sortExpressions = pSortExpressions;
	}
}

