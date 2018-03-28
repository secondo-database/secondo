package ui.console;

import javax.swing.text.AttributeSet;
import javax.swing.text.DocumentFilter.FilterBypass;

/**
 * An interface which allows other classes to dynamically change the
 * behaviour of the ConsoleDocumentFilter.
 * @author D.Merle
 */
public interface ConsoleDocumentFilterInterceptor {
	public void replace(final FilterBypass fb, final int offset, int length, final String string, final AttributeSet attr);
	public void remove(final FilterBypass fb, final int offset, final int length);
}