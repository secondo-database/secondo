package de.fernunihagen.dna.data;

import java.util.Date;

import android.webkit.DateSorter;

/**
 * DataTransferObject for the datatable Querys
 * @author Michael Küpper
 *
 */
public class QueryDto {

	private Integer id;
	private String database;
	private String query;
	private Date startTime;
	private int duration; // in seconds
	public String getDatabase() {
		return database;
	}
	public void setDatabase(String database) {
		this.database = database;
	}
	public String getQuery() {
		return query;
	}
	public void setQuery(String query) {
		this.query = query;
	}
	public Date getStartTime() {
		return startTime;
	}
	public void setStartTime(Date startTime) {
		this.startTime = startTime;
	}
	public int getDuration() {
		return duration;
	}
	public void setDuration(int duration) {
		this.duration = duration;
	}
	public Integer getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	
	@Override
	public String toString() {
		return query.substring(0, Math.min(50, query.length()));
	}
	
}
