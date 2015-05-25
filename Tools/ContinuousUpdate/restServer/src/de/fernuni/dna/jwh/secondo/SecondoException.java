package de.fernuni.dna.jwh.secondo;

/**
 * Locally used exception to symbolize a Secondo-Command-Error
 * @author Jerome White
 *
 */
public class SecondoException extends Exception {

	private static final long serialVersionUID = -7519795122376827645L;

	public SecondoException() {
		super();
	}

	public SecondoException(String str) {
		super(str);
	}
}
