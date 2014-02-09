package de.fernunihagen.dna.hoese;

public enum Encoding {
	none("none"),
	UTF8("UTF-8"),
	ISO_8859_1("ISO-8859-1");
	
	private String name;

	Encoding(String encodingname) {
		this.name = encodingname;
	}


	public String getName() {
		return this.name;
	}
}
