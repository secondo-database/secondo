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
public class LoadProfile {
	
	private String profileName;
	
	private List<RelationProfile> relations;

	public LoadProfile(String pName) {
		this.profileName = pName;
		this.relations = new ArrayList<RelationProfile>();
	}
	
	/**
	 * Adds or replaces the relation of the same name.
	 */
	public void addRelationProfile(RelationProfile pRelProfile){
		RelationProfile old = null;
		for (RelationProfile profile : this.relations){
			if (profile.getName().equals(pRelProfile.getName())){
				old = profile;
			}
		}
		if (old != null){
			this.relations.remove(old);
		}
		this.relations.add(pRelProfile);
	}
	
	/**
	 * Returns the name of the LoadProfile.
	 */
	public String getName(){
		return this.profileName;
	}
	
	/**
	 * Returns the FilterExpressions for the specified relation.
	 */
	public List<String> getFilterExpressions(String pRelName){
		List<String> result;
		RelationProfile relprof = this.getRelationProfile(pRelName);
		if (relprof != null){
			result = relprof.getFilterExpressions();
		}
		else{
			result = Collections.emptyList();
		}
		return result;
	}
		
	/**
	 * Returns the ProjectExpressions for the specified relation.
	 */
	public List<String> getProjectExpressions(String pRelName){
		List<String> result;
		RelationProfile relprof = this.getRelationProfile(pRelName);
		if (relprof != null){
			result = relprof.getProjectExpressions();
		}
		else{
			result = Collections.emptyList();
		}
		return result;
	}
	
	/**
	 * Returns the SortExpressions for the specified relation.
	 */
	public List<String> getSortExpressions(String pRelName){
		List<String> result;
		RelationProfile relprof = this.getRelationProfile(pRelName);
		if (relprof != null){
			result = relprof.getSortExpressions();
		}
		else{
			result = Collections.emptyList();
		}
		return result;
	}
	
	/**
	 * Returns the RelationProfile of the specified relation.
	 */
	public RelationProfile getRelationProfile(String pRelationName){
		RelationProfile result = null;
		for (RelationProfile relprof : this.relations){
			if (relprof.getName().equals(pRelationName)){
				result = relprof;
			};
		}
		return result;
	}
	
	/**
	 * Returns names of all the relations in the profile.
	 */
	public List<String> getRelations(){
		List<String> result = new ArrayList<String>();
		for (RelationProfile relprof : this.relations){
			result.add(relprof.getName());
		}
		return result;
	}		

	/**
	 * Removes the RelationProfile of the specified name.
	 */
	public void removeRelationProfile(String pRelName){
		RelationProfile old = null;
		for (RelationProfile profile : this.relations){
			if (profile.getName().equals(pRelName)){
				old = profile;
			}
		}
		if (old != null){
			this.relations.remove(old);
		}
	}
		
}

