package de.fernunihagen.dna.hoese;

import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import android.R.bool;
import android.util.Log;

import de.fernunihagen.dna.hoese.algebras.DisplayGraph;

import sj.lang.ListExpr;

public class QueryResult implements Serializable {
	private static final String TAG = "QueryResult";

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	/** The query command */
	private String command;
	/** The result given back by the query command as a Listexpr */
	private String resultString;
	/** Filename, in where to find the Queryresult */
	private String fileName;

	private transient ListExpr resultList;

	/** No. of tuples, no. of attributes of a tuple */
	private int TupelCount, AttrCount;
	/** The id of the stored Secondo-Object */
	private long id;
	
	/** Selected */
	private boolean selected;
	/** only one element is selected **/
	private boolean singleSelect = true;
	/** act selected element **/
	private int actSelectedElement = -1; // start value 

	
	// actual Item 
	private boolean actual;
	
	private Category category;

	private String error;
	private Vector<Viewer> viewer = new Vector<Viewer>(5);

	/**
	 * stores the interval where Time dependent objects are defined
	 **/
	private Interval interval;

	public QueryResult() {
		graphObjects = new Vector<DsplBase>(50);

	}

	private ArrayList<String> strings = new ArrayList<String>();
	/** A list of the ggraphic objects of this query */
	private Vector<DsplBase> graphObjects;

	public Vector<DsplBase> getEntries() {
		return graphObjects;
	}

	public List<String> getStrings() {
		return strings;
	}

	/**
	 * 
	 * @return graphical objects of this query-result
	 */
	public Vector<DsplBase> getGraphObjects() {
		return graphObjects;
	}

	public void addEntry(String entry) {
		strings.add(entry); // new String(entry.getBytes("Cp273")));
	}

	public void addEntry(DsplBase entry) {
		if (entry != null) {
			if (entry instanceof DsplBase) {
				if (entry instanceof DsplGraph) {
					graphObjects.add(entry);
				}
			} 
			if (entry instanceof DsplGeneric) {
				addEntry(entry.toString());
			}
		}
	}

	public boolean selectItem(int position, boolean selected) {
        Log.w(TAG, "selectItem Tag ["+ position + "] = " + selected);
		
		if (graphObjects.size() <= position) {
			return false; // if there is no graphic element, return
//			throw new IllegalArgumentException("position > GraphObject.size"); // position to big
		}
		
		// Only one selection
		if (singleSelect && selected) {
			for (DsplBase dsplBase : graphObjects) {
				dsplBase.setSelected(false);
			}
			actSelectedElement = position;
		}
		
		boolean oldSelectionState = false;
		DsplBase dsplBase = graphObjects.get(position);
		if (dsplBase != null) {
			oldSelectionState = dsplBase.getSelected(); 
			if (oldSelectionState != selected) {
				dsplBase.setSelected(selected);
				setDirtyFlag();
			}
		}
		
		return oldSelectionState;
	}
	

	public boolean getSelected(int position) {
	
		if (graphObjects.size() <= position) {
			return false; // if there is no graphic element, return
		//	throw new IllegalArgumentException("position > GraphObject.size"); // position to big
		}
		
		DsplBase dsplBase = graphObjects.get(position);
		if (dsplBase != null) {
	        Log.w(TAG, "getSelected Tag ["+ position + "] = " + dsplBase.getSelected());
			return dsplBase.getSelected(); 
		}
		
		return false;
	}

	
	public String getCommand() {
		return command;
	}

	public void setCommand(String command) {
		this.command = command;
	}

	public String getResultString() {
		if (this.fileName == null) {
			return "";
		}
		if (resultList == null) {
			resultList = new ListExpr();
			resultList.readFromFile(this.fileName);
		}
		
		return resultList.writeListExprToString();
	}

	public void setResultString(String resultString) {
		this.resultString = resultString;
		this.resultList = null;
	}

	public ListExpr getResultList() {
		if (this.fileName == null) {
			return null;
		}

		if (resultList == null) {
			resultList = new ListExpr();
			resultList.readFromFile(this.fileName);
		}
				
		return resultList;
	}

	public void setResultList(ListExpr resultList) {
		this.resultList = resultList;
	}

	public int getTupelCount() {
		return TupelCount;
	}

	public void setTupelCount(int tupelCount) {
		TupelCount = tupelCount;
	}

	public int getAttrCount() {
		return AttrCount;
	}

	public void setAttrCount(int attrCount) {
		AttrCount = attrCount;
	}

	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public String getError() {
		return error;
	}

	public void setError(String error) {
		this.error = error;
	}
	
	public void addViewer(Viewer view) {
		this.viewer.add(view);
	}

	private void setDirtyFlag() {
		for (Viewer view : viewer) {
			view.setDirtyFlag();
		}
	}

	/**
	 * Returns the interval containing all definition times of object instances
	 * of Timed. If no such time exist, the result is null.
	 **/
	public Interval getBoundingInterval() {
		for (DsplBase entry : graphObjects) {
			if (entry instanceof Timed) {
				Interval oInterval = ((Timed) entry)
						.getBoundingInterval();
				if (oInterval != null) {
					if (this.interval == null) {
						this.interval = oInterval.copy();
					} else {
						this.interval.unionInternal(oInterval);
					}
				}
			}
		}

		return interval;
	}
	
	/**
	 * Returns the count of interval containing all definition times of object instances
	 * of Timed. If no such time exist, the result is 0.
	 **/
	public int getIntervalCount() {
		int nCount = 0;
		for (DsplBase entry : graphObjects) {
			if (entry instanceof Timed) {
				Vector intervals = ((Timed) entry).getIntervals();
				if (intervals != null) {
					nCount += intervals.size();
				}
			}
		}

		return nCount;
	}


	public String getFileName() {
		return fileName;
	}

	public void setFileName(String fileName) {
		this.fileName = fileName;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + AttrCount;
		result = prime * result + TupelCount;
		result = prime * result + ((graphObjects == null) ? 0  : graphObjects.size());
		result = prime * result + ((command == null) ? 0 : command.hashCode());
		result = prime * result + (int) (id ^ (id >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		QueryResult other = (QueryResult) obj;
		if (AttrCount != other.AttrCount)
			return false;
		if (TupelCount != other.TupelCount)
			return false;
		if (graphObjects == null) {
			if (other.graphObjects != null) {
				return false;
			}
		} else {
			// Only if there some Objects and the queryresuklt is initialised, otherwise true
			if (graphObjects.size() > 0 &&  other.graphObjects.size() > 0 && graphObjects.size() != other.graphObjects.size()) 
				return false;
		}
		
		if (command == null) {
			if (other.command != null)
				return false;
		} else if (!command.equals(other.command))
			return false;
		if (id != other.id)
			return false;
		return true;
	}
	
	@Override
	public String toString() {
		return this.command + " " + (this.graphObjects == null ? "null" : new Integer(this.graphObjects.size()).toString());
	}

	public boolean isSelected() {
		return selected;
	}

	public void setSelected(boolean selected) {
		if (selected != this.selected) {
			setDirtyFlag();
		}
		this.selected = selected;
	}

	public boolean isActual() {
		return actual;
	}

	public void setActual(boolean actual) {
		this.actual = actual;
	}

	public boolean isSingleSelect() {
		return singleSelect;
	}

	public void setSingleSelect(boolean singleSelect) {
		this.singleSelect = singleSelect;
	}

	public Category getCategory() {
		if (category == null) {
			category = new Category();
			category.setColor(Category.getDefaultCat().getColor());
			category.setStrokeWidth(Category.getDefaultCat().getStrokeWidth());
			category.setAlpha(Category.getDefaultCat().getAlpha());
		}
		return category;
	}

	public void setCategory(Category category) {
		this.category = category;
	}
	
	public int getActSelected() {
		actSelectedElement = -1; // no element selected
		if (!singleSelect)
			return actSelectedElement;
		int index = 0;
		for (DsplBase dsplBase: graphObjects) {
			if (dsplBase.getSelected()) {
				actSelectedElement = index;
				return actSelectedElement;
			}
			++index;
				
		}
		return actSelectedElement;
	}
	

	public int findNext(int startPosition, String searchString) {
		int position = 0;
		
		for (int index = 0; index < this.strings.size(); ++index) {
			if ("---------".equals(strings.get(index))) { // getting a new Item
				++position;
			}
			if (position <= startPosition) continue;
			
			String value = strings.get(index);
			if (value.toLowerCase().contains(searchString.toLowerCase())) {
				return position;
			}
		}
		
		return -1;
	}
}
