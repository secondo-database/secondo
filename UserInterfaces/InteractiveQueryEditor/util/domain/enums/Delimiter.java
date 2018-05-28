package util.domain.enums;

/**
 * Used to identify special characters which separate
 * parameters or commands.
 * @author D.Merle
 */
public enum Delimiter {
	COMMA(","),
	COLON(":"),
	SEMICOLON(";");

	private String text;

	private Delimiter(final String text) {
		this.text = text;
	}

	public String getText() {
		return text;
	}
}