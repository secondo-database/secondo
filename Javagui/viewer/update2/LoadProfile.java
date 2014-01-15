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
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 * Load Profile for UpdateViewer2.	 
 */
public class LoadProfile 
{
	private String formatFields;
	private String profileName;
	private String formatScript;
	private String formatTemplate;
	private String outputDirectory;
	private List<RelationProfile> relations;

	
	public LoadProfile(String pName)
	{
		this.profileName = pName;
		this.formatFields = "";
		this.formatScript = "";
		this.formatTemplate = "";
		this.outputDirectory = "";
		this.relations = new ArrayList<RelationProfile>();
	}
	
	/**
	 * Adds or replaces the relation of the same name.
	 */
	public void addRelationProfile(RelationProfile pRelProfile)
	{
		RelationProfile old = null;
		
		for (RelationProfile profile : this.relations)
		{
			if (profile.getRelationName().equals(pRelProfile.getRelationName()))
			{
				old = profile;
			}
		}
		
		if (old != null)
		{
			this.relations.remove(old);
		}
		
		this.relations.add(pRelProfile);
	}
	
	/**
	 * Returns the name of the LoadProfile.
	 */
	public String getName()
	{
		return this.profileName;
	}
	
	public String getFormatFields()
	{
		return this.formatFields;
	}
	
	public String getFormatScript()
	{
		return this.formatScript;
	}
	
	public String getFormatTemplate()
	{
		return this.formatTemplate;
	}
	public String getOutputDirectory()
	{
		return this.outputDirectory;
	}
	
	
	/**
	 * Returns the RelationProfile of the specified relation.
	 */
	public RelationProfile getRelationProfile(String pRelationName)
	{
		RelationProfile result = null;
		
		for (RelationProfile relprof : this.relations)
		{
			if (relprof.getRelationName().equals(pRelationName))
			{
				result = relprof;
			}
		}
		return result;
	}
	
	/**
	 * Returns names of all the relations in the profile.
	 */
	public List<String> getRelationNames()
	{
		List<String> result = new ArrayList<String>();
		
		for (RelationProfile relprof : this.relations)
		{
			result.add(relprof.getRelationName());
		}
		return result;
	}
	
	/**
	 * Returns all the relation profiles.
	 */
	public List<RelationProfile> getRelationProfiles()
	{
		return this.relations;
	}

	/**
	 * Removes the RelationProfile of the specified name.
	 */
	public void removeRelationProfile(String pRelName)
	{
		RelationProfile old = null;
		
		for (RelationProfile profile : this.relations)
		{
			if (profile.getRelationName().equals(pRelName))
			{
				old = profile;
			}
		}
		
		if (old != null)
		{
			this.relations.remove(old);
		}
	}
	
	public void setFormatFields(String pFormatFields)
	{
		this.formatFields = pFormatFields;
	}
	
	public void setFormatScript(String pFormatScript)
	{
		this.formatScript = pFormatScript;
	}
	
	public void setFormatTemplate(String pFormatTemplate)
	{
		this.formatTemplate = pFormatTemplate;
	}
	
	public void setOutputDirectory(String pOutputDirectory)
	{
		this.outputDirectory = pOutputDirectory;
	}
}

