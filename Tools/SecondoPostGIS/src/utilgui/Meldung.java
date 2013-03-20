/**
 * 
 */
package utilgui;

import javax.swing.JOptionPane;

/**
 * @author Bill
 *
 */
public class Meldung {
	
	public Meldung(String strMeldung)
	{
		JOptionPane.showMessageDialog(null, strMeldung, "Message", JOptionPane.INFORMATION_MESSAGE);
		//JOptionPane.showMessageDialog(null,strMeldung);
	
	}

}
