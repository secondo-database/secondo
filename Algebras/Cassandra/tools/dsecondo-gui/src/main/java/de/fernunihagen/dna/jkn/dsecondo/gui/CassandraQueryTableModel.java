package de.fernunihagen.dna.jkn.dsecondo.gui;

import java.util.List;

import javax.swing.table.AbstractTableModel;

final class CassandraQueryTableModel extends AbstractTableModel {

	List<CassandraQuery> queryCache;
	
	/**
	 * 
	 */
	public CassandraQueryTableModel(List<CassandraQuery> queryCache) {
		this.queryCache = queryCache;
	}

	private static final long serialVersionUID = 8593512480994197794L;

	/**
	 * Get table value for given position
	 * @param rowIndex
	 * @param columnIndex
	 * @return
	 */
	public Object getValueAt(int rowIndex, int columnIndex) {
		final CassandraQuery query = queryCache.get(rowIndex);
		
		if(queryCache.size() < rowIndex) {
			return "";
		}
		
		if(query == null) {
			return "";
		}
		
		if(columnIndex == 0) {
			return query.getId();
		}
		
		if(columnIndex == 1) {
			return query.getQuery();
		}
		
		if(columnIndex == 2) {
			return query.getVersion();
		}
		
		return "";
		
	}

	@Override
	public int getRowCount() {
		return queryCache.size();
	}

	@Override
	public int getColumnCount() {
		return 3;
	}

	@Override
	public boolean isCellEditable(int rowIndex, int columnIndex) {
		return false;
	}

	@Override
	public String getColumnName(int column) {
	   if(column == 0) {
		   return "Id";
	   } else if(column == 1) {
			return "Query";
	   } 
		
	   return "Version";
	}

	public List<CassandraQuery> getQueryCache() {
		return queryCache;
	}

	public void setQueryCache(List<CassandraQuery> queryCache) {
		this.queryCache = queryCache;
	}
}