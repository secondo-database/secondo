package ui.console;

import javax.swing.text.AttributeSet;
import javax.swing.text.DocumentFilter.FilterBypass;

public interface ConsoleDocumentFilterInterceptor {
	public void replace(final FilterBypass fb, final int offset, int length, final String string, final AttributeSet attr);
	public void remove(final FilterBypass fb, final int offset, final int length);
}