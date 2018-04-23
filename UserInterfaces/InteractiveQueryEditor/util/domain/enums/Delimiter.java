package util.domain.enums;

public enum Delimiter {
	COMMA(","),
	SEMICOLON(";");

	private String text;

	private Delimiter(final String text) {
		this.text = text;
	}

	public String getText() {
		return text;
	}
}