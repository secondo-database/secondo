//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package com.secondo.webgui.client.loginview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;

/**
 * This class contains the dialog with the imprint text.
 *  
 * @author Kristina Steiger
 */
public class ImprintDialog {
	
	/**The dialog box for the imprint text*/
	private DialogBox imprintDialogBox = new DialogBox();
	
	/**A panel for the content of the dialog box*/
    private FlowPanel dialogContents = new FlowPanel();
    
    /**A scrollable panel to contain scrollable text*/
    private ScrollPanel scrollContent = new ScrollPanel();
    
    /**The imprint text in german*/
	private HTML textgerman;
	
	/**A button to close the dialog*/
	private Button closeButton = new Button("Close");
	
	public ImprintDialog(){
		
		 // add title of the dialog
		imprintDialogBox.setText("Impressum");

	    // Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    imprintDialogBox.setWidget(dialogContents);
	    
	    textgerman= new HTML("<h1>Impressum</h1><p>Angaben gemaess § 5 TMG:<br/><br/></p><p>FernUniversitaet in Hagen<br />LG Datenbanksysteme fuer neue Anwendungen<br />Universitaetsstr. 1<br />58084 Hagen<br /></p>" +
	    		"<h2>Vertreten durch:</h2><p>Prof. Dr. Ralf Hartmut Gueting</p><h2>Kontakt:</h2><table><tr><td>E-Mail:</td><td>secondo@fernuni-hagen.de</td></tr></table>" +
	    		"<p> </p><h2>Haftungsausschluss (Disclaimer)</h2><p><strong>Haftung fuer Inhalte</strong></p> <p>Als Diensteanbieter sind wir gemaess § 7 Abs.1 TMG fuer eigene Inhalte auf diesen " +
	    		"Seiten nach den allgemeinen Gesetzen verantwortlich. Nach § 8 bis 10 TMG sind wir als Diensteanbieter jedoch nicht verpflichtet, uebermittelte oder gespeicherte fremde " +
	    		"Informationen zu ueberwachen oder nach Umstaenden zu forschen, die auf eine rechtswidrige Taetigkeit hinweisen. Verpflichtungen zur Entfernung oder Sperrung der Nutzung von " +
	    		"Informationen nach den allgemeinen Gesetzen bleiben hiervon unberuehrt. Eine diesbezuegliche Haftung ist jedoch erst ab dem Zeitpunkt der Kenntnis einer konkreten " +
	    		"Rechtsverletzung moeglich. Bei Bekanntwerden von entsprechenden Rechtsverletzungen werden wir diese Inhalte umgehend entfernen.</p>" +
	    		"<p><strong>Haftung fuer Links</strong></p> <p>Unser Angebot enthaelt Links zu externen Webseiten Dritter, auf deren Inhalte wir keinen Einfluss haben. " +
	    		"Deshalb koennen wir fuer diese fremden Inhalte auch keine Gewaehr uebernehmen. Fuer die Inhalte der verlinkten Seiten ist stets der jeweilige Anbieter oder Betreiber der " +
	    		"Seiten verantwortlich. Die verlinkten Seiten wurden zum Zeitpunkt der Verlinkung auf moegliche Rechtsverstoesse ueberprueft. Rechtswidrige Inhalte waren zum Zeitpunkt der " +
	    		"Verlinkung nicht erkennbar. Eine permanente inhaltliche Kontrolle der verlinkten Seiten ist jedoch ohne konkrete Anhaltspunkte einer Rechtsverletzung nicht zumutbar. " +
	    		"Bei Bekanntwerden von Rechtsverletzungen werden wir derartige Links umgehend entfernen.</p> " +
	    		"<p><strong>Urheberrecht</strong></p> <p>Die durch die Seitenbetreiber erstellten Inhalte und Werke auf diesen Seiten unterliegen dem deutschen Urheberrecht. Die " +
	    		"Vervielfaeltigung, Bearbeitung, Verbreitung und jede Art der Verwertung ausserhalb der Grenzen des Urheberrechtes beduerfen der schriftlichen Zustimmung des jeweiligen Autors " +
	    		"bzw. Erstellers. Downloads und Kopien dieser Seite sind nur fuer den privaten, nicht kommerziellen Gebrauch gestattet. Soweit die Inhalte auf dieser Seite nicht vom Betreiber " +
	    		"erstellt wurden, werden die Urheberrechte Dritter beachtet. Insbesondere werden Inhalte Dritter als solche gekennzeichnet. Sollten Sie trotzdem auf eine Urheberrechtsverletzung" +
	    		" aufmerksam werden, bitten wir um einen entsprechenden Hinweis. Bei Bekanntwerden von Rechtsverletzungen werden wir derartige Inhalte umgehend entfernen.</p><p> </p>" +
	    		"<h2>Datenschutzerklaerung:</h2><p><strong>Datenschutz</strong></p> <p>Die Nutzung unserer Webseite ist in der Regel ohne Angabe personenbezogener Daten moeglich. " +
	    		"Soweit auf unseren Seiten personenbezogene Daten (beispielsweise Name, Anschrift oder eMail-Adressen) erhoben werden, erfolgt dies, soweit moeglich, stets auf freiwilliger " +
	    		"Basis. Diese Daten werden ohne Ihre ausdrueckliche Zustimmung nicht an Dritte weitergegeben. </p> <p>Wir weisen darauf hin, dass die Datenuebertragung im Internet " +
	    		"(z.B. bei der Kommunikation per E-Mail) Sicherheitsluecken aufweisen kann. Ein lueckenloser Schutz der Daten vor dem Zugriff durch Dritte ist nicht moeglich. </p> " +
	    		"<p>Der Nutzung von im Rahmen der Impressumspflicht veroeffentlichten Kontaktdaten durch Dritte zur Uebersendung von nicht ausdruecklich angeforderter Werbung und " +
	    		"Informationsmaterialien wird hiermit ausdruecklich widersprochen. Die Betreiber der Seiten behalten sich ausdruecklich rechtliche Schritte im Falle der unverlangten " +
	    		"Zusendung von Werbeinformationen, etwa durch Spam-Mails, vor.</p><p> </p>" +
	    		"<p><i>Quelle: eRecht24, Portal zum Internetrecht von Rechtsanwalt Soeren Siebert</i></p>" +
	    		"");
		
		// Add the text to the dialog
	    textgerman.setSize("700px", "500px");
	    scrollContent.add(textgerman);
	    dialogContents.add(scrollContent);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            imprintDialogBox.hide();}
	        });
	    dialogContents.add(closeButton);		
	}

	/**Returns the dialog box containing the imprint text
	 * 
	 * @return The dialog box with the imprint text
	 * */
	public DialogBox getImprintDialogBox() {
		return imprintDialogBox;
	}
}
