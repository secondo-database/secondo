package com.secondo.webgui.client.mainview;

/**
 * A simple data type that represents a contact.
 */
public class TableItem {

	    private static int nextId = 0;
	    public final int id;
		private String type;
		private String data;

		public TableItem(String type, String data) {
			nextId++;
		    this.id = nextId;
		    this.type=type;
		    this.data = data;
		}

		public String getType() {
			return type;
		}

		public void setType(String type) {
			this.type = type;
		}

		public String getData() {
			return data;
		}

		public void setData(String data) {
			this.data = data;
		}

		public int getId() {
			return id;
		}
		

}
